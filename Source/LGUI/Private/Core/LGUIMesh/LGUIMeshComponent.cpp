﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

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


//PRAGMA_DISABLE_OPTIMIZATION
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
	FLGUIMeshSceneProxy(ULGUIMeshComponent* InComponent, void* InCanvasPtr, int32 InCanvasSortOrder)
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
							TempRenderer.Pin()->AddWorldSpacePrimitive_RenderThread((ULGUICanvas*)InCanvasPtr, InCanvasSortOrder, HudPrimitive);
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
			auto SrcSection = SrcSections[SectionIndex];
			Sections[SectionIndex] = CreateSectionData(SrcSection);
		}

		auto LGUIMeshSceneProxy = this;
		ENQUEUE_RENDER_COMMAND(FLGUIMeshSectionProxy_SortMeshSectionRenderPriority_Create)(
			[LGUIMeshSceneProxy](FRHICommandListImmediate& RHICmdList) {
				LGUIMeshSceneProxy->SortMeshSectionRenderPriority_RenderThread();
			});
	}

	void AddSectionData(TSharedPtr<FLGUIMeshSection> SrcSection)
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

	FLGUIMeshProxySection* CreateSectionData(TSharedPtr<FLGUIMeshSection> SrcSection)
	{
		if (SrcSection->vertices.Num() == 0 || SrcSection->triangles.Num() == 0)return nullptr;
		FLGUIMeshProxySection* NewSection = new FLGUIMeshProxySection(GetScene().GetFeatureLevel());
		// vertex and index buffer
		const auto& SrcVertices = SrcSection->vertices;
		int NumVerts = SrcVertices.Num();
		if (IsSupportLGUIRenderer)
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
			NewSection->IndexBuffer.Indices = SrcSection->triangles;

			// Enqueue initialization of render resource
			BeginInitResource(&NewSection->IndexBuffer);
			BeginInitResource(&NewSection->HudVertexBuffers);
		}
		if (IsSupportUERenderer)
		{
			NewSection->IndexBuffer.Indices = SrcSection->triangles;
			NewSection->VertexBuffers.InitFromDynamicVertex(&NewSection->VertexFactory, SrcSection->vertices, 4);

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

	void RecreateSectionData(TSharedPtr<FLGUIMeshSection> SrcSection)
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
				LGUIRenderer.Pin()->RemoveWorldSpacePrimitive_RenderThread((ULGUICanvas*)RenderCanvasPtr, this);
			}
			else
			{
				LGUIRenderer.Pin()->RemoveScreenSpacePrimitive_RenderThread(this);
			}
			LGUIRenderer.Reset();
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FDynamicMeshVertex* MeshVertexData, int32 NumVerts
		, FLGUIIndexType* MeshIndexData, uint32 IndexDataLength
		, int8 AdditionalChannelFlags
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
				HudVertexUpdateData.SetNumUninitialized(NumVerts, false);
				if (AdditionalChannelFlags == 0)
				{
					for (int i = 0; i < NumVerts; i++)
					{
						FLGUIHudVertex& HudVert = HudVertexUpdateData[i];
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
						FLGUIHudVertex& HudVert = HudVertexUpdateData[i];
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
				FMemory::Memcpy(VertexBufferData, HudVertexUpdateData.GetData(), vertexDataLength);
				RHIUnlockVertexBuffer(Section->HudVertexBuffers.VertexBufferRHI);
			}
			if(IsSupportUERenderer)
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
	virtual void GetMeshElement(FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& ResultArray) override
	{
		if (IsSupportLGUIRenderer)
		{
			ResultArray.Reserve(Sections.Num());
			for (int i = 0; i < Sections.Num(); i++)
			{
				auto Section = Sections[i];
				if (Section != nullptr && Section->bSectionVisible)
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
	}
	virtual int GetRenderPriority()const override
	{
		return RenderPriority;
	}
	virtual bool CanRender()const override
	{
		return Sections.Num() > 0;
	}
	virtual bool GetIsPostProcess()override { return false; }
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
	TArray<FLGUIHudVertex> HudVertexUpdateData;
	void* RenderCanvasPtr = nullptr;
};

//////////////////////////////////////////////////////////////////////////

#include "Utils/LGUIUtils.h"
void ULGUIMeshComponent::CreateMeshSection(TSharedPtr<FLGUIMeshSection> InMeshSection)
{
#if WITH_EDITOR
	for (auto& MeshSection : MeshSections)
	{
		if (MeshSection->triangles.Num() >= MAX_TRIANGLE_COUNT)
		{
			auto errorMsg = FString::Printf(TEXT("[ULGUIMeshComponent] Too many triangles in single drawcall! This will cause issue!"));
			LGUIUtils::EditorNotification(FText::FromString(errorMsg), 10);
			UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg);
		}
	}
#endif
	UpdateLocalBounds(); // Update overall bounds
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		if (InMeshSection->renderProxy != nullptr)
		{
			LGUIMeshSceneProxy->RecreateSectionData(InMeshSection);
		}
		else
		{
			LGUIMeshSceneProxy->AddSectionData(InMeshSection);
		}
	}
	else
	{
		MarkRenderStateDirty(); // New section requires recreating scene proxy
	}
}

DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_GT"), STAT_UpdateMeshSectionGT, STATGROUP_LGUI);
void ULGUIMeshComponent::UpdateMeshSection(TSharedPtr<FLGUIMeshSection> InMeshSection, bool InVertexPositionChanged, int8 AdditionalShaderChannelFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionGT);
	if (InVertexPositionChanged)
	{
		UpdateLocalBounds();
	}
	if (SceneProxy)
	{
		auto Section = InMeshSection->renderProxy;
		//vertex data
		const int32 NumVerts = InMeshSection->vertices.Num();
		FDynamicMeshVertex* VertexBufferData = new FDynamicMeshVertex[NumVerts];
		FMemory::Memcpy(VertexBufferData, InMeshSection->vertices.GetData(), NumVerts * sizeof(FDynamicMeshVertex));
		auto LGUIMeshSceneProxy = (FLGUIMeshSceneProxy*)SceneProxy;
		const int32 NumIndices = InMeshSection->triangles.Num();
		const uint32 IndexDataLength = NumIndices * sizeof(FLGUIIndexType);
		FLGUIIndexType* IndexBufferData = new FLGUIIndexType[NumIndices];
		FMemory::Memcpy(IndexBufferData, InMeshSection->triangles.GetData(), IndexDataLength);
		//update data
		ENQUEUE_RENDER_COMMAND(FLGUIMeshUpdate)(
			[LGUIMeshSceneProxy, VertexBufferData, NumVerts, IndexBufferData, IndexDataLength, AdditionalShaderChannelFlags, Section](FRHICommandListImmediate& RHICmdList)
			{
				LGUIMeshSceneProxy->UpdateSection_RenderThread(VertexBufferData, NumVerts, IndexBufferData, IndexDataLength, AdditionalShaderChannelFlags, Section);
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

FPrimitiveSceneProxy* ULGUIMeshComponent::CreateSceneProxy()
{
	//SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_CreateSceneProxy);

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
//PRAGMA_ENABLE_OPTIMIZATION