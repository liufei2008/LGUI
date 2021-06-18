// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "Containers/ResourceArray.h"
#include "StaticMeshResources.h"
#include "Materials/Material.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Engine/Engine.h"
#include "LGUI.h"
#include "Core/HudRender/LGUIHudVertex.h"


class FLGUIHudMeshVertexResourceArray : public FResourceArrayInterface
{
public:
	FLGUIHudMeshVertexResourceArray(void* InData, uint32 InSize)
		:Data(InData)
		,Size(InSize)
	{

	}
	virtual const void* GetResourceData() const override { return Data; }
	virtual uint32 GetResourceDataSize() const override { return Size; }
	virtual void Discard() override { }
	virtual bool IsStatic() const override { return false; }
	virtual bool GetAllowCPUAccess() const override { return false; }
	virtual void SetAllowCPUAccess(bool bInNeedsCPUAccess) override { }
private: 
	void* Data;
	uint32 Size;
};
class FLGUIHudVertexBuffer : public FVertexBuffer
{
public:
	TArray<FLGUIHudVertex> Vertices;
	virtual void InitRHI()override
	{
		const uint32 SizeInBytes = Vertices.Num() * sizeof(FLGUIHudVertex);

		FLGUIHudMeshVertexResourceArray ResourceArray(Vertices.GetData(), SizeInBytes);
		FRHIResourceCreateInfo CreateInfo(&ResourceArray);
		VertexBufferRHI = RHICreateVertexBuffer(SizeInBytes, BUF_Static, CreateInfo);
	}
};


/** Class representing a single section of the LGUI mesh */
class FLGUIMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	FLGUIHudVertexBuffer HudVertexBuffers;
	/** Index buffer for this section */
	FLGUIMeshIndexBuffer IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible;

	FLGUIMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: Material(NULL)
		, VertexFactory(InFeatureLevel, "FLGUIMeshProxySection")
		, bSectionVisible(true)
	{}
};

DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_RT"), STAT_UpdateMeshSectionRT, STATGROUP_LGUI);
/** LGUI mesh scene proxy */
class FLGUIMeshSceneProxy : public FPrimitiveSceneProxy, public ILGUIHudPrimitive
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	FLGUIMeshSceneProxy(ULGUIMeshComponent* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, BodySetup(InComponent->GetBodySetup())
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, RenderPriority(InComponent->TranslucencySortPriority)
	{
		LGUIRenderer = InComponent->LGUIRenderer;
		if (LGUIRenderer.IsValid())
		{
			auto TempRenderer = LGUIRenderer;
			auto HudPrimitive = this;
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSceneProxy_AddHudPrimitive)(
				[TempRenderer, HudPrimitive](FRHICommandListImmediate& RHICmdList)
				{
					if (TempRenderer.IsValid())
					{
						TempRenderer.Pin()->AddHudPrimitive_RenderThread(HudPrimitive);
					}
				}
			);
			IsSupportScreenSpace = true;
		}
		IsSupportWorldSpace = InComponent->IsSupportWorldSpace;

		FLGUIMeshSection& SrcSection = InComponent->MeshSection;
		if (SrcSection.vertices.Num() > 0)
		{
			FLGUIMeshProxySection* NewSection = new FLGUIMeshProxySection(GetScene().GetFeatureLevel());
			// vertex and index buffer
			const auto& SrcVertices = SrcSection.vertices;
			int NumVerts = SrcVertices.Num();
			if (IsSupportScreenSpace)
			{
				auto& HudVertices = NewSection->HudVertexBuffers.Vertices;
				HudVertices.SetNumUninitialized(NumVerts);
				for (int i = 0; i < NumVerts; i++)
				{
					FLGUIHudVertex& HudVert = HudVertices[i];
					auto& Vert = SrcVertices[i];
					HudVert.Position = Vert.Position;
					HudVert.Color = Vert.Color;
					HudVert.TextureCoordinate0 = Vert.TextureCoordinate[0];
					HudVert.TextureCoordinate1 = Vert.TextureCoordinate[1];
					HudVert.TextureCoordinate2 = Vert.TextureCoordinate[2];
					HudVert.TextureCoordinate3 = Vert.TextureCoordinate[3];
				}
				NewSection->IndexBuffer.Indices = SrcSection.triangles;

				// Enqueue initialization of render resource
				BeginInitResource(&NewSection->IndexBuffer);
				BeginInitResource(&NewSection->HudVertexBuffers);
			}
			if (IsSupportWorldSpace)
			{
				NewSection->IndexBuffer.Indices = SrcSection.triangles;
				NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, SrcSection.vertices, 4);

				// Enqueue initialization of render resource
				BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
				BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
				BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
				BeginInitResource(&NewSection->IndexBuffer);
				BeginInitResource(&NewSection->VertexFactory);
			}

			// Grab material
			NewSection->Material = InComponent->GetMaterial(0);
			if (NewSection->Material == NULL)
			{
				NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			// Copy visibility info
			NewSection->bSectionVisible = SrcSection.bSectionVisible;

			// Save ref to new section
			Section = NewSection;
		}
	}

	virtual ~FLGUIMeshSceneProxy()
	{
		if (Section != nullptr)
		{
			if (IsSupportScreenSpace)
			{
				Section->IndexBuffer.ReleaseResource();
				Section->HudVertexBuffers.ReleaseResource();
			}
			if (IsSupportWorldSpace)
			{
				Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
				Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
				Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
				Section->IndexBuffer.ReleaseResource();
				Section->VertexFactory.ReleaseResource();
			}
			delete Section;
		}
		if (LGUIRenderer.IsValid())
		{
			LGUIRenderer.Pin()->RemoveHudPrimitive_RenderThread(this);
			LGUIRenderer.Reset();
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FDynamicMeshVertex* MeshVertexData, int32 NumVerts, FLGUIIndexType* MeshIndexData
		, uint32 IndexDataLength, int8 AdditionalChannelFlags)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionRT);

		check(IsInRenderingThread());

		// Check it references a valid section
		if (Section != nullptr)
		{
			//vertex buffer
			if (IsSupportScreenSpace)
			{
				FLGUIHudVertex* HudVertexBufferData = new FLGUIHudVertex[NumVerts];
				if (AdditionalChannelFlags == 0)
				{
					for (int i = 0; i < NumVerts; i++)
					{
						FLGUIHudVertex& HudVert = HudVertexBufferData[i];
						auto& Vert = MeshVertexData[i];
						HudVert.Position = Vert.Position;
						HudVert.Color = Vert.Color;
						HudVert.TextureCoordinate0 = Vert.TextureCoordinate[0];
					}
				}
				else
				{
					for (int i = 0; i < NumVerts; i++)
					{
						FLGUIHudVertex& HudVert = HudVertexBufferData[i];
						auto& Vert = MeshVertexData[i];
						HudVert.Position = Vert.Position;
						HudVert.Color = Vert.Color;
						HudVert.TextureCoordinate0 = Vert.TextureCoordinate[0];
						HudVert.TextureCoordinate1 = Vert.TextureCoordinate[1];
						HudVert.TextureCoordinate2 = Vert.TextureCoordinate[2];
						HudVert.TextureCoordinate3 = Vert.TextureCoordinate[3];
					}
				}

				uint32 vertexDataLength = NumVerts * sizeof(FLGUIHudVertex);
				void* VertexBufferData = RHILockVertexBuffer(Section->HudVertexBuffers.VertexBufferRHI, 0, vertexDataLength, RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, HudVertexBufferData, vertexDataLength);
				RHIUnlockVertexBuffer(Section->HudVertexBuffers.VertexBufferRHI);
				delete[] HudVertexBufferData;
			}
			if(IsSupportWorldSpace)
			{
				int meshNumVert = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices();
				if (meshNumVert != NumVerts)//for some unknown reason, in mobile platform, VertexBuffers.PositionVertexBuffer/StaticMeshVertexBuffer/ColorVertexBuffer 's vertex count changed
				{
					UE_LOG(LGUI, Warning, TEXT("Vert count not equal, NewData NumVerts:%d, VertBuffer NumVert:%d"), NumVerts, meshNumVert);
					NumVerts = FMath::Min(NumVerts, meshNumVert);
				}
				if (AdditionalChannelFlags == 0)
				{
					for (int i = 0; i < NumVerts; i++)
					{
						const FDynamicMeshVertex& LGUIVert = MeshVertexData[i];
						Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = LGUIVert.Position;
						Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, LGUIVert.TextureCoordinate[0]);
						Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = LGUIVert.Color;
					}
				}
				else
				{
					bool requireNormal = (AdditionalChannelFlags & (1 << 0)) != 0;
					bool requireTangent = (AdditionalChannelFlags & (1 << 1)) != 0;
					bool requireNormalOrTangent = requireNormal || requireTangent;
					bool requireUV1 = (AdditionalChannelFlags & (1 << 2)) != 0;
					bool requireUV2 = (AdditionalChannelFlags & (1 << 3)) != 0;
					bool requireUV3 = (AdditionalChannelFlags & (1 << 4)) != 0;
					for (int i = 0; i < NumVerts; i++)
					{
						const FDynamicMeshVertex& LGUIVert = MeshVertexData[i];
						Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = LGUIVert.Position;
						Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = LGUIVert.Color;
						if (requireNormalOrTangent)
							Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, LGUIVert.TangentX.ToFVector(), LGUIVert.GetTangentY(), LGUIVert.TangentZ.ToFVector());
						Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, LGUIVert.TextureCoordinate[0]);
						if (requireUV1)
							Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, LGUIVert.TextureCoordinate[1]);
						if (requireUV2)
							Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, LGUIVert.TextureCoordinate[2]);
						if (requireUV3)
							Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, LGUIVert.TextureCoordinate[3]);
					}
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.PositionVertexBuffer;
					void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
					RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.ColorVertexBuffer;
					void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
					RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
					void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
					RHIUnlockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
					void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
					RHIUnlockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
				}
			}


			// Lock index buffer
			auto IndexBufferData = RHILockIndexBuffer(Section->IndexBuffer.IndexBufferRHI, 0, IndexDataLength, RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, (void*)MeshIndexData, IndexDataLength);
			RHIUnlockIndexBuffer(Section->IndexBuffer.IndexBufferRHI);
		}
		delete[]MeshVertexData;
		delete[]MeshIndexData;
	}

	void SetSectionVisibility_RenderThread(bool bNewVisibility)
	{
		check(IsInRenderingThread());

		if (Section != nullptr)
		{
			Section->bSectionVisible = bNewVisibility;
		}
	}
	void SetRenderPriority_RenderThread(int32 NewPriority)
	{
		RenderPriority = NewPriority;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_GetMeshElements);
		if (!IsSupportWorldSpace) return;
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = NULL;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		if (Section != nullptr && Section->bSectionVisible)
		{
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			// For each view..
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->IndexBuffer;
					Mesh.bWireframe = bWireframe;
					Mesh.VertexFactory = &Section->VertexFactory;
					Mesh.MaterialRenderProxy = MaterialProxy;

					bool bHasPrecomputedVolumetricLightmap;
					FMatrix PreviousLocalToWorld;
					int32 SingleCaptureIndex;
					bool bOutputVelocity;
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), bOutputVelocity);
					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	//begin ILGUIHudPrimitive interface
	virtual FMeshBatch GetMeshElement(FMeshElementCollector* Collector) override
	{
		//if (Section != nullptr && Section->bSectionVisible)//check CanRender before call GetMeshElement, so this line is not necessary
		if (IsSupportScreenSpace)
		{
			FMaterialRenderProxy* MaterialProxy = Section->Material->GetRenderProxy();

			// Draw the mesh.
			FMeshBatch Mesh;
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &Section->IndexBuffer;
			BatchElement.PrimitiveIdMode = PrimID_ForceZero;
			Mesh.bWireframe = false;
			Mesh.VertexFactory = &Section->VertexFactory;
			Mesh.MaterialRenderProxy = MaterialProxy;

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector->AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, false, false);
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
			//BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, UseEditorDepthTest());

			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.bCanApplyViewModeOverrides = false;
			return Mesh;
		}
		return FMeshBatch();
	}
	virtual int GetRenderPriority()const override
	{
		return RenderPriority;
	}
	virtual bool CanRender()const override
	{
		return Section != nullptr && Section->bSectionVisible;
	}
	virtual FRHIVertexBuffer* GetVertexBufferRHI()override
	{
		return Section->HudVertexBuffers.VertexBufferRHI;
	}
	virtual uint32 GetNumVerts()override
	{
		return Section->HudVertexBuffers.Vertices.Num();
	}
	virtual bool GetIsPostProcess()override { return false; }
	//end ILGUIHudPrimitive interface

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	FLGUIMeshProxySection* Section = nullptr;
	UBodySetup* BodySetup = nullptr;

	FMaterialRelevance MaterialRelevance;
	int32 RenderPriority = 0;
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool IsSupportScreenSpace = false;
	bool IsSupportWorldSpace = true;
};

//////////////////////////////////////////////////////////////////////////

void ULGUIMeshComponent::CreateMeshSection()
{
	UpdateLocalBounds(); // Update overall bounds
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_GT"), STAT_UpdateMeshSectionGT, STATGROUP_LGUI);
void ULGUIMeshComponent::UpdateMeshSection(bool InVertexPositionChanged, int8 AdditionalShaderChannelFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionGT);
	if (InVertexPositionChanged)
	{
		UpdateLocalBounds();
	}
	if (SceneProxy)
	{
		//vertex data
		const int32 NumVerts = MeshSection.vertices.Num();
		FDynamicMeshVertex* VertexBufferData = new FDynamicMeshVertex[NumVerts];
		FMemory::Memcpy(VertexBufferData, MeshSection.vertices.GetData(), NumVerts * sizeof(FDynamicMeshVertex));
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		const int32 NumIndices = MeshSection.triangles.Num();
		const uint32 IndexDataLength = NumIndices * sizeof(FLGUIIndexType);
		FLGUIIndexType* IndexBufferData = new FLGUIIndexType[NumIndices];
		FMemory::Memcpy(IndexBufferData, MeshSection.triangles.GetData(), IndexDataLength);
		//update data
		ENQUEUE_RENDER_COMMAND(FLGUIMeshUpdate)(
			[LGUIMeshSceneProxy, VertexBufferData, NumVerts, IndexBufferData, IndexDataLength, AdditionalShaderChannelFlags](FRHICommandListImmediate& RHICmdList)
			{
				LGUIMeshSceneProxy->UpdateSection_RenderThread(VertexBufferData, NumVerts, IndexBufferData, IndexDataLength, AdditionalShaderChannelFlags);
			});
	}
}

void ULGUIMeshComponent::ClearMesh()
{
	MeshSection.Reset();
	UpdateLocalBounds();
	MarkRenderStateDirty();
}

void ULGUIMeshComponent::SetUIMeshVisibility(bool bNewVisibility)
{
	this->SetVisibility(bNewVisibility);
	// Set game thread state
	if (MeshSection.bSectionVisible == bNewVisibility)return;
	MeshSection.bSectionVisible = bNewVisibility;

	if (SceneProxy)
	{
		// Enqueue command to modify render thread info
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshVisibilityUpdate)(
			[LGUIMeshSceneProxy, bNewVisibility](FRHICommandListImmediate& RHICmdList)
			{
				LGUIMeshSceneProxy->SetSectionVisibility_RenderThread(bNewVisibility);
			});
	}
}

bool ULGUIMeshComponent::IsMeshVisible() const
{
	return MeshSection.bSectionVisible;
}

void ULGUIMeshComponent::SetUITranslucentSortPriority(int32 NewTranslucentSortPriority)
{
	UPrimitiveComponent::SetTranslucentSortPriority(NewTranslucentSortPriority);
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMesh_SetUITranslucentSortPriority)(
			[LGUIMeshSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
		{
			LGUIMeshSceneProxy->SetRenderPriority_RenderThread(NewTranslucentSortPriority);
		}
		);
	}
}

void ULGUIMeshComponent::UpdateLocalBounds()
{
	UpdateBounds();// Update global bounds
	// Need to send to render thread
	MarkRenderTransformDirty();
}

FPrimitiveSceneProxy* ULGUIMeshComponent::CreateSceneProxy()
{
	//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_CreateSceneProxy);

	FLGUIMeshSceneProxy* Proxy = NULL;
	if (MeshSection.vertices.Num() > 0)
	{
		Proxy = new FLGUIMeshSceneProxy(this);
	}
	return Proxy;
}

void ULGUIMeshComponent::SetSupportScreenSpace(bool supportOrNot, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> HudRenderer)
{
	if (supportOrNot)
	{
		LGUIRenderer = HudRenderer;
	}
	else
	{
		LGUIRenderer.Reset();
	}
}

void ULGUIMeshComponent::SetSupportWorldSpace(bool supportOrNot)
{
	IsSupportWorldSpace = supportOrNot;
}

int32 ULGUIMeshComponent::GetNumMaterials() const
{
	return 1;
}

FBoxSphereBounds ULGUIMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	const auto& vertices = MeshSection.vertices;
	int vertCount = vertices.Num();
	if (vertCount < 2)return Super::CalcBounds(LocalToWorld);

	FVector vecMin = vertices[0].Position;
	FVector vecMax = vecMin;

	// Get maximum and minimum X, Y and Z positions of vectors
	for (int32 i = 1; i < vertCount; i++)
	{
		auto vertPos = vertices[i].Position;
		vecMin.X = (vecMin.X > vertPos.X) ? vertPos.X : vecMin.X;
		vecMin.Y = (vecMin.Y > vertPos.Y) ? vertPos.Y : vecMin.Y;
		vecMin.Z = (vecMin.Z > vertPos.Z) ? vertPos.Z : vecMin.Z;

		vecMax.X = (vecMax.X < vertPos.X) ? vertPos.X : vecMax.X;
		vecMax.Y = (vecMax.Y < vertPos.Y) ? vertPos.Y : vecMax.Y;
		vecMax.Z = (vecMax.Z < vertPos.Z) ? vertPos.Z : vecMax.Z;
	}

	FVector vecOrigin = ((vecMax - vecMin) / 2) + vecMin;	/* Origin = ((Max Vertex's Vector - Min Vertex's Vector) / 2 ) + Min Vertex's Vector */
	FVector BoxPoint = vecMax - vecMin;			/* The difference between the "Maximum Vertex" and the "Minimum Vertex" is our actual Bounds Box */

	return FBoxSphereBounds(vecOrigin, BoxPoint, BoxPoint.Size()).TransformBy(LocalToWorld);
}

void ULGUIMeshComponent::SetColor(FColor InColor)
{
	auto& vertices = MeshSection.vertices;
	int count = vertices.Num();
	for (int i = 0; i < count; i++)
	{
		vertices[i].Color = InColor;
	}
	UpdateMeshSection(false);
}
FColor ULGUIMeshComponent::GetColor()const
{
	if (MeshSection.vertices.Num() > 0)
	{
		return MeshSection.vertices[0].Color;
	}
	else
	{
		return FColor::White;
	}
}