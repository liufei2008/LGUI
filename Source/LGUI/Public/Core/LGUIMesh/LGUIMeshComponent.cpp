// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIMeshComponent.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "Containers/ResourceArray.h"
#include "StaticMeshResources.h"
#include "Materials/Material.h"
#include "Core/Render/ILGUIHudPrimitive.h"
#include "Core/Render/LGUIRenderer.h"
#include "Engine.h"
#include "LGUI.h"


/** Class representing a single section of the LGUI mesh */
class FLGUIMeshProxySection
{
public:
	/** Material applied to this section */
	UMaterialInterface* Material;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FDynamicMeshIndexBuffer16 IndexBuffer;
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

DECLARE_CYCLE_STAT(TEXT("LGUIUpdateMeshSection_RenderThread"), STAT_LGUIMesh_UpdateSectionRT, STATGROUP_LGUI);
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
		FLGUIMeshSection& SrcSection = InComponent->MeshSection;
		if (SrcSection.vertices.Num() > 0)
		{
			FLGUIMeshProxySection* NewSection = new FLGUIMeshProxySection(GetScene().GetFeatureLevel());

			// Copy data from vertex buffer
			const int32 NumVerts = SrcSection.vertices.Num();

			// Allocate verts
			TArray<FDynamicMeshVertex> Vertices;
			Vertices.SetNumUninitialized(NumVerts);
			// Copy verts
			const auto& vertices = SrcSection.vertices;
			const auto& uvs = SrcSection.uvs;
			const auto& uvs1 = SrcSection.uvs1;
			const auto& uvs2 = SrcSection.uvs2;
			const auto& uvs3 = SrcSection.uvs3;
			const auto& colors = SrcSection.colors;
			const auto& normals = SrcSection.normals;
			const auto& tangents = SrcSection.tangents;
			for (int VertIdx = 0; VertIdx < NumVerts; VertIdx++)
			{
				FDynamicMeshVertex& Vert = Vertices[VertIdx];
				Vert.Position = vertices[VertIdx];
				Vert.Color = colors[VertIdx];
				Vert.TextureCoordinate[0] = uvs[VertIdx];
				Vert.TextureCoordinate[1] = uvs1[VertIdx];
				Vert.TextureCoordinate[2] = uvs2[VertIdx];
				Vert.TextureCoordinate[3] = uvs3[VertIdx];
				Vert.TangentZ = normals[VertIdx];
				Vert.TangentZ.Vector.W = -127;
				Vert.TangentX = tangents[VertIdx];
			}

			// Copy index buffer
			NewSection->IndexBuffer.Indices = SrcSection.triangles;

			NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, Vertices, 4);

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->VertexFactory);

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
		LGUIHudRenderer = InComponent->LGUIHudRenderer;
		if (LGUIHudRenderer.IsValid())
		{
			LGUIHudRenderer.Pin()->AddHudPrimitive(this);
			IsHudOrWorldSpace = true;
		}
		else
		{
			IsHudOrWorldSpace = false;
		}
	}

	virtual ~FLGUIMeshSceneProxy()
	{
		if (Section != nullptr)
		{
			Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();
			delete Section;
		}
		if (LGUIHudRenderer.IsValid())
		{
			LGUIHudRenderer.Pin()->RemoveHudPrimitive(this);
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FDynamicMeshVertex* MeshVertexData, int32 NumVerts, uint16* MeshIndexData, uint32 IndexDataLength)
	{
		SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_UpdateSectionRT);

		check(IsInRenderingThread());

		// Check it references a valid section
		if (Section != nullptr)
		{
			for (int i = 0; i < NumVerts; i++)
			{
				const FDynamicMeshVertex& LGUIVert = MeshVertexData[i];
				FDynamicMeshVertex Vertex = LGUIVert;

				Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i
					, Vertex.TangentX.ToFVector()
					, Vertex.GetTangentY()
					, Vertex.TangentZ.ToFVector());
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, Vertex.TextureCoordinate[1]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, Vertex.TextureCoordinate[2]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, Vertex.TextureCoordinate[3]);
				Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
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
	void SetToHud_RenderThread()
	{
		IsHudOrWorldSpace = true;
	}
	void SetToWorld_RenderThread()
	{
		IsHudOrWorldSpace = false;
	}
	void SetRenderPriority_RenderThread(int32 NewPriority)
	{
		RenderPriority = NewPriority;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_GetMeshElements);
		if (IsHudOrWorldSpace)return;
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
					GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex);

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, UseEditorDepthTest());
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
	virtual FMeshBatch GetMeshElement() override
	{
		//if (Section != nullptr && Section->bSectionVisible)//check CanRender before call GetMeshElement, so this line is not necessary
		if (IsHudOrWorldSpace)
		{
			FMaterialRenderProxy* MaterialProxy = Section->Material->GetRenderProxy(false);

			// Draw the mesh.
			FMeshBatch Mesh;
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &Section->IndexBuffer;
			Mesh.bWireframe = false;
			Mesh.VertexFactory = &Section->VertexFactory;
			Mesh.MaterialRenderProxy = MaterialProxy;
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
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
	FLGUIMeshProxySection* Section;
	UBodySetup* BodySetup;

	FMaterialRelevance MaterialRelevance;
	int32 RenderPriority = 0;
	TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> LGUIHudRenderer;
	bool IsHudOrWorldSpace = false;
};

//////////////////////////////////////////////////////////////////////////

void ULGUIMeshComponent::CreateMeshSection()
{
	//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_CreateMeshSection);
	MeshSection.SectionLocalBox.Init();
	const auto& vertices = MeshSection.vertices;
	int vertexCount = vertices.Num();
	for (int i = 0; i < vertexCount; i++)
	{
		MeshSection.SectionLocalBox += vertices[i];
	}

	UpdateLocalBounds(); // Update overall bounds
	MarkRenderStateDirty(); // New section requires recreating scene proxy
}

DECLARE_CYCLE_STAT(TEXT("LGUIUpdateMeshSection"), STAT_LGUIUpdateMeshSection, STATGROUP_LGUI);
void ULGUIMeshComponent::UpdateMeshSection(bool InVertexPositionChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_LGUIUpdateMeshSection);
	if (InVertexPositionChanged)
	{
		MeshSection.SectionLocalBox.Init();
		const auto& vertices = MeshSection.vertices;
		int vertexCount = vertices.Num();
		for (int i = 0; i < vertexCount; i++)
		{
			MeshSection.SectionLocalBox += vertices[i];
		}
		UpdateLocalBounds();
	}
	if (SceneProxy)
	{
		const auto& vertices = MeshSection.vertices;
		const auto& uvs = MeshSection.uvs;
		const auto& uvs1 = MeshSection.uvs1;
		const auto& uvs2 = MeshSection.uvs2;
		const auto& uvs3 = MeshSection.uvs3;
		const auto& colors = MeshSection.colors;
		const auto& normals = MeshSection.normals;
		const auto& tangents = MeshSection.tangents;
		const int32 NumVerts = vertices.Num();
		FDynamicMeshVertex* VertexBufferData = new FDynamicMeshVertex[NumVerts];
		// Iterate through vertex data, copying in new info
		for (int32 VertIdx = 0; VertIdx < NumVerts; VertIdx++)
		{
			FDynamicMeshVertex& Vert = VertexBufferData[VertIdx];
			Vert.Position = vertices[VertIdx];
			Vert.Color = colors[VertIdx];
			Vert.TextureCoordinate[0] = uvs[VertIdx];
			Vert.TextureCoordinate[1] = uvs1[VertIdx];
			Vert.TextureCoordinate[2] = uvs2[VertIdx];
			Vert.TextureCoordinate[3] = uvs3[VertIdx];
			Vert.TangentZ = normals[VertIdx];
			Vert.TangentZ.Vector.W = -127;
			Vert.TangentX = tangents[VertIdx];
		}
		//index data
		auto& triangles = MeshSection.triangles;
		const int32 NumTriangles = triangles.Num();
		uint32* IndexBufferData = new uint32[NumTriangles];
		int32 IndexDataLength = NumTriangles * sizeof(int32);
		FMemory::Memcpy(IndexBufferData, triangles.GetData(), IndexDataLength);
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshUpdate)(
			[LGUIMeshSceneProxy, VertexBufferData, NumVerts, IndexBufferData, IndexDataLength](FRHICommandListImmediate& RHICmdList)
			{
				LGUIMeshSceneProxy->UpdateSection_RenderThread(VertexBufferData, NumVerts, IndexBufferData, IndexDataLength);
			});
	}
}

void ULGUIMeshComponent::ClearMesh()
{
	MeshSection.Reset();
	UpdateLocalBounds();
	MarkRenderStateDirty();
}

void ULGUIMeshComponent::SetMeshVisible(bool bNewVisibility)
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
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FLGUIMesh_SetUITranslucentSortPriority,
			FLGUIMeshSceneProxy*, LGUIMeshSceneProxy, (FLGUIMeshSceneProxy*)SceneProxy,
			int32, NewRenderPriority, NewTranslucentSortPriority,
			{
				LGUIMeshSceneProxy->SetRenderPriority_RenderThread(NewRenderPriority);
			}
		)
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
	HudPrimitive = (ILGUIHudPrimitive*)Proxy;
	return Proxy;
}

void ULGUIMeshComponent::SetToLGUIHud(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> HudRenderer)
{
	LGUIHudRenderer = HudRenderer;
	if (SceneProxy)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			LGUIMeshComponent_SetToLGUIHud,
			FLGUIMeshSceneProxy*, LGUIMeshSceneProxy, (FLGUIMeshSceneProxy*)SceneProxy,
			{
				LGUIMeshSceneProxy->SetToHud_RenderThread();
			}
		);
	}
}

void ULGUIMeshComponent::SetToLGUIWorld()
{
	LGUIHudRenderer.Reset();
	if (SceneProxy)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			LGUIMeshComponent_SetToLGUIWorld,
			FLGUIMeshSceneProxy*, LGUIMeshSceneProxy, (FLGUIMeshSceneProxy*)SceneProxy,
			{
				LGUIMeshSceneProxy->SetToWorld_RenderThread();
			}
		);
	}
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

	FVector vecMin = vertices[0];
	FVector vecMax = vertices[0];

	// Get maximum and minimum X, Y and Z positions of vectors
	for (int32 i = 1; i < vertCount; i++)
	{
		auto vertPos = vertices[i];
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
	auto& colors = MeshSection.colors;
	int count = colors.Num();
	for (int i = 0; i < count; i++)
	{
		colors[i] = InColor;
	}
	UpdateMeshSection(false);
}
FColor ULGUIMeshComponent::GetColor()const
{
	if (MeshSection.colors.Num() > 0)
	{
		return MeshSection.colors[0];
	}
	else
	{
		return FColor::White;
	}
}