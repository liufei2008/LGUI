// Copyright 2019-Present LexLiu. All Rights Reserved.

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
#include "Core/ActorComponent/LGUICanvas.h"


#define LOCTEXT_NAMESPACE "LGUIMeshComponent"
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
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
	TArray<FLGUIMeshVertex> Vertices;
	virtual void InitRHI()override
	{
		const uint32 SizeInBytes = Vertices.Num() * sizeof(FLGUIMeshVertex);

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
	UMaterialInterface* Material = nullptr;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	FLGUIHudVertexBuffer HudVertexBuffers;
	/** Index buffer for this section */
	FLGUIMeshIndexBuffer IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;
	/** Whether this section is currently visible */
	bool bSectionVisible = true;

	int RenderPriority = 0;

	FLGUIMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: VertexFactory(InFeatureLevel, "FLGUIMeshProxySection")
	{}

	static inline void InitOrUpdateResource(FRenderResource* Resource)
	{
		if (!Resource->IsInitialized())
		{
			Resource->InitResource();
		}
		else
		{
			Resource->UpdateRHI();
		}
	}

	void InitFromLGUIHudVertexData(TArray<FLGUIMeshVertex>& Vertices)
	{
		auto LightMapIndex = 0;
		VertexBuffers.StaticMeshVertexBuffer.SetUseFullPrecisionUVs(true);
		if (Vertices.Num())
		{
			VertexBuffers.PositionVertexBuffer.Init(Vertices.Num());
			VertexBuffers.StaticMeshVertexBuffer.Init(Vertices.Num(), LGUI_VERTEX_TEXCOORDINATE_COUNT);
			VertexBuffers.ColorVertexBuffer.Init(Vertices.Num());

			for (int32 i = 0; i < Vertices.Num(); i++)
			{
				const auto& Vertex = Vertices[i];

				VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector());
				for (uint32 j = 0; j < LGUI_VERTEX_TEXCOORDINATE_COUNT; j++)
				{
					VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, j, Vertex.TextureCoordinate[j]);
				}
				VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
			}
		}
		else
		{
			VertexBuffers.PositionVertexBuffer.Init(1);
			VertexBuffers.StaticMeshVertexBuffer.Init(1, 1);
			VertexBuffers.ColorVertexBuffer.Init(1);

			VertexBuffers.PositionVertexBuffer.VertexPosition(0) = FVector(0, 0, 0);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(0, FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1));
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(0, 0, FVector2D(0, 0));
			VertexBuffers.ColorVertexBuffer.VertexColor(0) = FColor(1, 1, 1, 1);
			LightMapIndex = 0;
		}

		FStaticMeshVertexBuffers* Self = &VertexBuffers;
		FLocalVertexFactory* VertexFactoryPtr = &VertexFactory;
		ENQUEUE_RENDER_COMMAND(StaticMeshVertexBuffersLegacyInit)(
			[VertexFactoryPtr, Self, LightMapIndex](FRHICommandListImmediate& RHICmdList)
			{
				InitOrUpdateResource(&Self->PositionVertexBuffer);
				InitOrUpdateResource(&Self->StaticMeshVertexBuffer);
				InitOrUpdateResource(&Self->ColorVertexBuffer);

				FLocalVertexFactory::FDataType Data;
				Self->PositionVertexBuffer.BindPositionVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindTangentVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindLightMapVertexBuffer(VertexFactoryPtr, Data, LightMapIndex);
				Self->ColorVertexBuffer.BindColorVertexBuffer(VertexFactoryPtr, Data);
				VertexFactoryPtr->SetData(Data);

				InitOrUpdateResource(VertexFactoryPtr);
			});
	}
};

DECLARE_CYCLE_STAT(TEXT("LGUIMesh CreateMeshSection"), STAT_CreateMeshSection, STATGROUP_LGUI);
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
	FLGUIMeshSceneProxy(ULGUIMeshComponent* InComponent, ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, RenderPriority(InComponent->TranslucencySortPriority)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreateMeshSection);
		LGUIRenderer = InComponent->LGUIRenderer;
		RenderCanvasPtr = InCanvasPtr;
		IsLGUIRenderToWorld = InComponent->IsLGUIRenderToWorld;
		if (LGUIRenderer.IsValid())
		{
			auto TempRenderer = LGUIRenderer;
			auto HudPrimitive = this;
			auto IsRenderToWorld = IsLGUIRenderToWorld;
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSceneProxy_AddHudPrimitive)(
				[TempRenderer, HudPrimitive, InCanvasPtr, InCanvasSortOrder, IsRenderToWorld](FRHICommandListImmediate& RHICmdList)
				{
					if (TempRenderer.IsValid())
					{
						if (IsRenderToWorld)
						{
							TempRenderer.Pin()->AddWorldSpacePrimitive_RenderThread((ULGUICanvas*)InCanvasPtr, HudPrimitive);
						}
						else
						{
							TempRenderer.Pin()->AddScreenSpacePrimitive_RenderThread(HudPrimitive);
						}
					}
				}
			);
			IsSupportLGUIRenderer = true;
		}
		IsSupportUERenderer = InComponent->IsSupportUERenderer;

		auto& SrcSections = InComponent->MeshSections;
		Sections.SetNumZeroed(SrcSections.Num());
		for (int SectionIndex = 0; SectionIndex < SrcSections.Num(); SectionIndex++)
		{
			Sections[SectionIndex] = CreateSectionData(SrcSections[SectionIndex].Get());
		}

		auto LGUIMeshSceneProxy = this;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshSectionProxy_SortMeshSectionRenderPriority_Create)(
			[LGUIMeshSceneProxy](FRHICommandListImmediate& RHICmdList) {
				LGUIMeshSceneProxy->SortMeshSectionRenderPriority_RenderThread();
			});
	}

	void AddSectionData(FLGUIMeshSection* SrcSection)
	{
		auto Section = CreateSectionData(SrcSection);
		if (Section != nullptr)
		{
			auto RenderProxy = this;
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSceneProxy_AddSectionData)(
				[RenderProxy, Section](FRHICommandListImmediate& RHICmdList)
				{
					RenderProxy->AddSectionData_RenderThread(Section);
					RenderProxy->SortMeshSectionRenderPriority_RenderThread();
				}
			);
		}
	}
	void AddSectionData_RenderThread(FLGUIMeshProxySection* Section)
	{
		Sections.Add(Section);
	}

	FLGUIMeshProxySection* CreateSectionData(FLGUIMeshSection* SrcSection)
	{
		if (SrcSection->vertices.Num() == 0 || SrcSection->triangles.Num() == 0)
		{
			SrcSection->renderProxy = nullptr;
			return nullptr;
		}
		FLGUIMeshProxySection* NewSection = new FLGUIMeshProxySection(GetScene().GetFeatureLevel());
		// vertex and index buffer
		const auto& SrcVertices = SrcSection->vertices;
		int NumVerts = SrcVertices.Num();
		if (IsSupportLGUIRenderer)
		{
			auto& HudVertices = NewSection->HudVertexBuffers.Vertices;
			HudVertices.SetNumUninitialized(NumVerts);
			FMemory::Memcpy(HudVertices.GetData(), SrcVertices.GetData(), NumVerts * sizeof(FLGUIMeshVertex));
			NewSection->IndexBuffer.Indices = SrcSection->triangles;

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->HudVertexBuffers);
		}
		if (IsSupportUERenderer)
		{
			NewSection->IndexBuffer.Indices = SrcSection->triangles;
			NewSection->InitFromLGUIHudVertexData(SrcSection->vertices);

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->VertexBuffers.PositionVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.StaticMeshVertexBuffer);
			BeginInitResource(&NewSection->VertexBuffers.ColorVertexBuffer);
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->VertexFactory);
		}

		// Grab material
		NewSection->Material = SrcSection->material;
		if (NewSection->Material == NULL)
		{
			NewSection->Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}

		// Copy visibility info
		NewSection->bSectionVisible = SrcSection->bSectionVisible;
		NewSection->RenderPriority = SrcSection->renderPriority;
		SrcSection->renderProxy = NewSection;

		return NewSection;
	}

	void DeleteSectionData_RenderThread(FLGUIMeshProxySection* Section, bool RemoveFromArray)
	{
		if (RemoveFromArray)
		{
			Sections.Remove(Section);
		}
		if (this->IsSupportLGUIRenderer)
		{
			Section->IndexBuffer.ReleaseResource();
			Section->HudVertexBuffers.ReleaseResource();
		}
		if (this->IsSupportUERenderer)
		{
			Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();
		}
		delete Section;
	}

	void RecreateSectionData(FLGUIMeshSection* SrcSection)
	{
		auto OldSection = SrcSection->renderProxy;
		auto NewSection = CreateSectionData(SrcSection);
		auto RenderProxy = this;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshSceneProxy_ReplaceSectionData)(
			[RenderProxy, OldSection, NewSection](FRHICommandListImmediate& RHICmdList) {
				RenderProxy->RecreateSectionData_RenderThread(OldSection, NewSection);
			});
	}
	void RecreateSectionData_RenderThread(FLGUIMeshProxySection* OldSection, FLGUIMeshProxySection* NewSection)
	{
		auto SectionIndex = Sections.IndexOfByKey(OldSection);
		DeleteSectionData_RenderThread(OldSection, false);
		check(SectionIndex >= 0);
		Sections[SectionIndex] = NewSection;
	}

	void SetSectionVisibility_RenderThread(bool bNewVisibility, int SectionIndex)
	{
		auto Section = Sections[SectionIndex];
		if (Section != nullptr)
		{
			Section->bSectionVisible = bNewVisibility;
		}
	}
	void SetRenderPriority_RenderThread(int32 NewPriority)
	{
		RenderPriority = NewPriority;
	}

	void SetMeshSectionRenderPriority_RenderThread(FLGUIMeshProxySection* Section, int32 NewPriority)
	{
		Section->RenderPriority = NewPriority;
	}

	void SortMeshSectionRenderPriority_RenderThread()
	{
		Algo::Sort(Sections, [](const FLGUIMeshProxySection* A, const FLGUIMeshProxySection* B) {
			if (A != nullptr && B != nullptr)
			{
				return A->RenderPriority < B->RenderPriority;
			}
			else if (A == nullptr)
			{
				return false;
			}
			else if (B == nullptr)
			{
				return true;
			}
			return false;
			});
	}

	virtual ~FLGUIMeshSceneProxy()
	{
		for(auto Section : Sections)
		{
			if (Section != nullptr)
			{
				if (IsSupportLGUIRenderer)
				{
					Section->IndexBuffer.ReleaseResource();
					Section->HudVertexBuffers.ReleaseResource();
				}
				if (IsSupportUERenderer)
				{
					Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
					Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
					Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
					Section->IndexBuffer.ReleaseResource();
					Section->VertexFactory.ReleaseResource();
				}
				delete Section;
			}
		}
		Sections.Empty();
		if (LGUIRenderer.IsValid())
		{
			if (IsLGUIRenderToWorld)
			{
				LGUIRenderer.Pin()->RemoveWorldSpacePrimitive_RenderThread(RenderCanvasPtr, this);
			}
			else
			{
				LGUIRenderer.Pin()->RemoveScreenSpacePrimitive_RenderThread(this);
			}
			LGUIRenderer.Reset();
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FLGUIMeshVertex* MeshVertexData, const int32& NumVerts
		, FLGUIIndexType* MeshIndexData, const uint32& IndexDataLength
		, const int8& AdditionalChannelFlags
		, FLGUIMeshProxySection* Section)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionRT);

		check(IsInRenderingThread());

		// Check it references a valid section
		if (Section != nullptr)
		{
			//vertex buffer
			if (IsSupportLGUIRenderer)
			{
				uint32 VertexDataLength = NumVerts * sizeof(FLGUIMeshVertex);
				void* VertexBufferData = RHILockVertexBuffer(Section->HudVertexBuffers.VertexBufferRHI, 0, VertexDataLength, RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, MeshVertexData, VertexDataLength);
				RHIUnlockVertexBuffer(Section->HudVertexBuffers.VertexBufferRHI);
			}
			if(IsSupportUERenderer)
			{
				if (AdditionalChannelFlags == 0)
				{
					for (int i = 0; i < NumVerts; i++)
					{
						const FLGUIMeshVertex& LGUIVert = MeshVertexData[i];
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
						const FLGUIMeshVertex& LGUIVert = MeshVertexData[i];
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
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_GetMeshElements);
		if (!IsSupportUERenderer) return;
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

		for (int i = 0; i < Sections.Num(); i++)
		{
			auto Section = Sections[i];
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
	}

	//begin ILGUIHudPrimitive interface
	virtual void GetMeshElements(const FSceneViewFamily& ViewFamily, FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& ResultArray) override
	{
		if (!IsSupportLGUIRenderer)return;
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FMaterialRenderProxy* WireframeMaterialInstance = NULL;
		if (bWireframe)
		{
			WireframeMaterialInstance = GEngine->WireframeMaterial->GetRenderProxy();
		}

		ResultArray.Reserve(Sections.Num());
		for (int i = 0; i < Sections.Num(); i++)
		{
			auto Section = Sections[i];
			if (Section != nullptr && Section->bSectionVisible)
			{
				FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

				// Draw the mesh.
				FMeshBatch Mesh;
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &Section->IndexBuffer;
				BatchElement.PrimitiveIdMode = PrimID_ForceZero;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &Section->VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector->AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, false, false);
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
				//BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, UseEditorDepthTest());

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = Section->HudVertexBuffers.Vertices.Num() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;

				FLGUIMeshBatchContainer MeshBatchContainer;
				MeshBatchContainer.Mesh = Mesh;
				MeshBatchContainer.VertexBufferRHI = Section->HudVertexBuffers.VertexBufferRHI;
				MeshBatchContainer.NumVerts = Section->HudVertexBuffers.Vertices.Num();
				ResultArray.Add(MeshBatchContainer);
			}
		}
	}
	virtual int GetRenderPriority()const override
	{
		return RenderPriority;
	}
	virtual bool CanRender()const override
	{
		return Sections.Num() > 0;
	}
	virtual ELGUIHudPrimitiveType GetPrimitiveType()const override { return ELGUIHudPrimitiveType::Mesh; }
	virtual bool PostProcessRequireOriginScreenColorTexture()const override { return false; }
	virtual FPrimitiveComponentId GetMeshPrimitiveComponentId() const override { return GetPrimitiveComponentId(); }
	virtual ULGUICanvas* GetCanvas()const override { return RenderCanvasPtr; }
	//end ILGUIHudPrimitive interface

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		if (IsSupportUERenderer)
		{
			Result.bDrawRelevance = IsShown(View);
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bDynamicRelevance = true;
			Result.bStaticRelevance = false;
			Result.bRenderInMainPass = ShouldRenderInMainPass();
			Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
			Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		}
		else
		{
			Result.bDrawRelevance = false;
			Result.bShadowRelevance = false;
			Result.bDynamicRelevance = false;
			Result.bStaticRelevance = false;
			Result.bRenderInMainPass = false;
			Result.bUsesLightingChannels = false;
			Result.bRenderCustomDepth = false;
		}
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return IsSupportUERenderer && !MaterialRelevance.bDisableDepthTest;
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
	TArray<FLGUIMeshProxySection*> Sections;

	FMaterialRelevance MaterialRelevance;
	int32 RenderPriority = 0;
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool IsSupportLGUIRenderer = false;
	bool IsLGUIRenderToWorld = false;
	bool IsSupportUERenderer = true;
	ULGUICanvas* RenderCanvasPtr = nullptr;
};

//////////////////////////////////////////////////////////////////////////

ULGUIMeshComponent::ULGUIMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}
#include "Utils/LGUIUtils.h"
void ULGUIMeshComponent::CreateMeshSectionData(TSharedPtr<FLGUIMeshSection> InMeshSection)
{
#if WITH_EDITOR
	for (auto& MeshSection : MeshSections)
	{
		if (MeshSection->vertices.Num() > LGUI_MAX_VERTEX_COUNT)
		{
			auto errorMsg = FText::Format(LOCTEXT("TooManyTrianglesInSingleDdrawcall", "{0} Too many vertices ({1}) in single drawcall! This will cause issue!")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
				, MeshSection->vertices.Num());
			LGUIUtils::EditorNotification(errorMsg, 10);
			UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
		}
	}
#endif
	UpdateLocalBounds(); // Update overall bounds
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		if (InMeshSection->renderProxy != nullptr)
		{
			LGUIMeshSceneProxy->RecreateSectionData(InMeshSection.Get());
		}
		else
		{
			LGUIMeshSceneProxy->AddSectionData(InMeshSection.Get());
		}
	}
	else
	{
		MarkRenderStateDirty(); // New section requires recreating scene proxy
	}
}

DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_GT"), STAT_UpdateMeshSectionGT, STATGROUP_LGUI);
void ULGUIMeshComponent::UpdateMeshSectionData(TSharedPtr<FLGUIMeshSection> InMeshSection, bool InVertexPositionChanged, int8 AdditionalShaderChannelFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionGT);
	if (InVertexPositionChanged)
	{
		UpdateLocalBounds();
	}
	if (SceneProxy)
	{
		struct UpdateMeshSectionDataStruct
		{
			TArray<FLGUIMeshVertex> VertexBufferData;
			int32 NumVerts;
			TArray<FLGUIIndexType> IndexBufferData;
			uint32 IndexBufferDataLength;
			int8 AdditionalShaderChannelFlags;
			FLGUIMeshProxySection* Section;
			FLGUIMeshSceneProxy* SceneProxy;
		};
		UpdateMeshSectionDataStruct* UpdateData = new UpdateMeshSectionDataStruct();
		UpdateData->Section = InMeshSection->renderProxy;
		//vertex data
		const int32 NumVerts = InMeshSection->vertices.Num();
		UpdateData->VertexBufferData.AddUninitialized(NumVerts);
		FMemory::Memcpy(UpdateData->VertexBufferData.GetData(), InMeshSection->vertices.GetData(), NumVerts * sizeof(FLGUIMeshVertex));
		UpdateData->NumVerts = NumVerts;
		UpdateData->SceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		const int32 NumIndices = InMeshSection->triangles.Num();
		const uint32 IndexBufferDataLength = NumIndices * sizeof(FLGUIIndexType);
		UpdateData->IndexBufferData.AddUninitialized(NumIndices);
		FMemory::Memcpy(UpdateData->IndexBufferData.GetData(), InMeshSection->triangles.GetData(), IndexBufferDataLength);
		UpdateData->IndexBufferDataLength = IndexBufferDataLength;
		UpdateData->AdditionalShaderChannelFlags = AdditionalShaderChannelFlags;
		//update data
		ENQUEUE_RENDER_COMMAND(FLGUIMeshUpdate)(
			[UpdateData](FRHICommandListImmediate& RHICmdList)
			{
				UpdateData->SceneProxy->UpdateSection_RenderThread(
					UpdateData->VertexBufferData.GetData()
					, UpdateData->NumVerts
					, UpdateData->IndexBufferData.GetData()
					, UpdateData->IndexBufferDataLength
					, UpdateData->AdditionalShaderChannelFlags
					, UpdateData->Section
				);
				delete UpdateData;
			});
	}
}

void ULGUIMeshComponent::DeleteMeshSection(TSharedPtr<FLGUIMeshSection> InMeshSection)
{
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		auto Section = InMeshSection->renderProxy;
		if (Section != nullptr)
		{
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSceneProxy_DeleteSectionData)(
				[LGUIMeshSceneProxy, Section](FRHICommandListImmediate& RHICmdList)
				{
					LGUIMeshSceneProxy->DeleteSectionData_RenderThread(Section, true);
				}
			);
		}
	}
	InMeshSection->renderProxy = nullptr;
	InMeshSection->material = nullptr;

	auto index = MeshSections.IndexOfByKey(InMeshSection);
	check(index >= 0);
	MeshSections.RemoveAt(index);
	PooledMeshSections.Add(InMeshSection);
	//verify material
	UMeshComponent::EmptyOverrideMaterials();
	for (int i = 0; i < MeshSections.Num(); i++)
	{
		UMeshComponent::SetMaterial(i, MeshSections[i]->material);
	}
}

void ULGUIMeshComponent::ClearAllMeshSection()
{
	MeshSections.Empty();
	PooledMeshSections.Empty();
	UMeshComponent::EmptyOverrideMaterials();
}

void ULGUIMeshComponent::SetMeshSectionRenderPriority(TSharedPtr<FLGUIMeshSection> InMeshSection, int32 InSortPriority)
{
	InMeshSection->renderPriority = InSortPriority;
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		auto Section = InMeshSection->renderProxy;
		if (Section != nullptr)
		{
			auto RenderProxy = this;
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSectionProxy_SetMeshSectionRenderPriority)(
				[LGUIMeshSceneProxy, Section, InSortPriority](FRHICommandListImmediate& RHICmdList) {
					LGUIMeshSceneProxy->SetMeshSectionRenderPriority_RenderThread(Section, InSortPriority);
				});
		}
	}
}
void ULGUIMeshComponent::SortMeshSectionRenderPriority()
{
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshSectionProxy_SortMeshSectionRenderPriority)(
			[LGUIMeshSceneProxy](FRHICommandListImmediate& RHICmdList) {
				LGUIMeshSceneProxy->SortMeshSectionRenderPriority_RenderThread();
			});
	}
}

void ULGUIMeshComponent::SetMeshSectionMaterial(TSharedPtr<FLGUIMeshSection> InMeshSection, UMaterialInterface* InMaterial)
{
	check(IsValid(InMaterial));
	InMeshSection->material = InMaterial;
	int index = MeshSections.IndexOfByKey(InMeshSection);
	Super::SetMaterial(index, InMaterial);
}

void ULGUIMeshComponent::SetMeshSectionVisibility(bool bNewVisibility, int InSectionIndex)
{
	// Set game thread state
	if (MeshSections[InSectionIndex]->bSectionVisible == bNewVisibility)return;
	MeshSections[InSectionIndex]->bSectionVisible = bNewVisibility;

	if (SceneProxy)
	{
		// Enqueue command to modify render thread info
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshVisibilityUpdate)(
			[LGUIMeshSceneProxy, bNewVisibility, InSectionIndex](FRHICommandListImmediate& RHICmdList)
			{
				LGUIMeshSceneProxy->SetSectionVisibility_RenderThread(bNewVisibility, InSectionIndex);
			});
	}
}

bool ULGUIMeshComponent::IsMeshSectionVisible(int InSectionIndex) const
{
	return MeshSections[InSectionIndex]->bSectionVisible;
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
	if (IsSupportUERenderer)//screen space UI no need to update bounds
	{
		// Need to send to render thread
		MarkRenderTransformDirty();
	}
}

void ULGUIMeshComponent::DestroyRenderState_Concurrent()
{
	Super::DestroyRenderState_Concurrent();
}

DECLARE_CYCLE_STAT(TEXT("LGUIMesh CreateSceneProxy"), STAT_LGUIMesh_CreateSceneProxy, STATGROUP_LGUI);
FPrimitiveSceneProxy* ULGUIMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_CreateSceneProxy);

	FLGUIMeshSceneProxy* Proxy = NULL;
	if (MeshSections.Num() > 0)
	{
		Proxy = new FLGUIMeshSceneProxy(this, RenderCanvas.Get(), RenderCanvas.IsValid() ? RenderCanvas->GetSortOrder() : 0);
	}
	return Proxy;
}

void ULGUIMeshComponent::SetSupportLGUIRenderer(bool supportOrNot, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> HudRenderer, ULGUICanvas* InCanvas, bool InIsRenderToWorld)
{
	if (supportOrNot)
	{
		LGUIRenderer = HudRenderer;
		RenderCanvas = InCanvas;
		IsLGUIRenderToWorld = InIsRenderToWorld;
	}
	else
	{
		LGUIRenderer.Reset();
	}
}

void ULGUIMeshComponent::SetSupportUERenderer(bool supportOrNot)
{
	IsSupportUERenderer = supportOrNot;
}

int32 ULGUIMeshComponent::GetNumMaterials() const
{
	return MeshSections.Num();
}

TSharedPtr<FLGUIMeshSection> ULGUIMeshComponent::GetMeshSection()
{
	TSharedPtr<FLGUIMeshSection> Result;
	if (PooledMeshSections.Num() == 0)
	{
		Result = TSharedPtr<FLGUIMeshSection>(new FLGUIMeshSection());
	}
	else
	{
		Result = PooledMeshSections[PooledMeshSections.Num() - 1];
		PooledMeshSections.RemoveAt(PooledMeshSections.Num() - 1);
	}
	check(Result.IsValid());
	MeshSections.Add(Result);
	return Result;
}

FBoxSphereBounds ULGUIMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	//screen space UI no need to update bounds
	if (IsSupportUERenderer)
	{
		if (MeshSections.Num() <= 0)
		{
			return FBoxSphereBounds(EForceInit::ForceInitToZero);
		}
		if (MeshSections[0]->vertices.Num() <= 0)
		{
			return FBoxSphereBounds(EForceInit::ForceInitToZero);
		}

		FVector vecMin = MeshSections[0]->vertices[0].Position;
		FVector vecMax = vecMin;
		for (auto& MeshSection : MeshSections)
		{
			const auto& vertices = MeshSection->vertices;
			int vertCount = vertices.Num();
			if (vertCount < 2)return Super::CalcBounds(LocalToWorld);

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
		}
		FVector vecOrigin = ((vecMax - vecMin) / 2) + vecMin;	/* Origin = ((Max Vertex's Vector - Min Vertex's Vector) / 2 ) + Min Vertex's Vector */
		FVector BoxPoint = vecMax - vecMin;			/* The difference between the "Maximum Vertex" and the "Minimum Vertex" is our actual Bounds Box */

		return FBoxSphereBounds(vecOrigin, BoxPoint, BoxPoint.Size()).TransformBy(LocalToWorld);
	}
	else
	{
		return FBoxSphereBounds(FSphere(FVector::ZeroVector, 1.0f)).TransformBy(LocalToWorld);
	}
}
#undef LOCTEXT_NAMESPACE
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif