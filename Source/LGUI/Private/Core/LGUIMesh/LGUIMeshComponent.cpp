// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "Containers/ResourceArray.h"
#include "StaticMeshResources.h"
#include "Materials/Material.h"
#include "Core/LGUIRender/ILGUIRendererPrimitive.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "Engine/Engine.h"
#include "LGUI.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialRenderProxy.h"
#include "MaterialDomain.h"
#include "PrimitiveSceneProxy.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "PrimitiveSceneInfo.h"


#define LOCTEXT_NAMESPACE "LGUIMeshComponent"
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif
class FLGUIMeshVertexResourceArray : public FResourceArrayInterface
{
public:
	FLGUIMeshVertexResourceArray(void* InData, uint32 InSize)
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
class FLGUIVertexBuffer : public FVertexBuffer
{
public:
	TArray<FLGUIMeshVertex> Vertices;
	virtual void InitRHI(FRHICommandListBase& RHICmdList)override
	{
		const uint32 SizeInBytes = Vertices.Num() * sizeof(FLGUIMeshVertex);

		FLGUIMeshVertexResourceArray ResourceArray(Vertices.GetData(), SizeInBytes);
		FRHIResourceCreateInfo CreateInfo(TEXT("LGUIVertexBuffer"), &ResourceArray);
		VertexBufferRHI = RHICmdList.CreateVertexBuffer(SizeInBytes, BUF_Dynamic, CreateInfo);
	}
};


struct FLGUIRenderSectionProxy
{
	virtual ~FLGUIRenderSectionProxy()
	{

	}

	ELGUIRenderSectionType Type;

	/** Sort order */
	int SectionRenderPriority = 0;
};
/** Class representing a single section of the LGUI mesh */
struct FLGUIMeshSectionProxy : public FLGUIRenderSectionProxy
{
	/** Material applied to this section */
	UMaterialInterface* Material = nullptr;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	FLGUIVertexBuffer LGUIVertexBuffers;
	/** Index buffer for this section */
	FLGUIMeshIndexBuffer IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;

	FLGUIMeshSectionProxy(ERHIFeatureLevel::Type InFeatureLevel)
		: VertexFactory(InFeatureLevel, "FLGUIMeshProxySection")
	{
		Type = ELGUIRenderSectionType::Mesh;
	}
	~FLGUIMeshSectionProxy()
	{
		IndexBuffer.ReleaseResource();
		LGUIVertexBuffers.ReleaseResource();
		VertexBuffers.PositionVertexBuffer.ReleaseResource();
		VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
		VertexBuffers.ColorVertexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

	static inline void InitOrUpdateResource(FRHICommandListImmediate& RHICmdList, FRenderResource* Resource)
	{
		if (!Resource->IsInitialized())
		{
			Resource->InitResource(RHICmdList);
		}
		else
		{
			Resource->UpdateRHI(RHICmdList);
		}
	}

	void InitFromLGUIVertexData(TArray<FLGUIMeshVertex>& Vertices)
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
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
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

			VertexBuffers.PositionVertexBuffer.VertexPosition(0) = FVector3f(0, 0, 0);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(0, FVector3f(1, 0, 0), FVector3f(0, 1, 0), FVector3f(0, 0, 1));
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(0, 0, FVector2f(0, 0));
			VertexBuffers.ColorVertexBuffer.VertexColor(0) = FColor(1, 1, 1, 1);
			LightMapIndex = 0;
		}

		FStaticMeshVertexBuffers* Self = &VertexBuffers;
		FLocalVertexFactory* VertexFactoryPtr = &VertexFactory;
		ENQUEUE_RENDER_COMMAND(StaticMeshVertexBuffersLegacyInit)(
			[VertexFactoryPtr, Self, LightMapIndex](FRHICommandListImmediate& RHICmdList)
			{
				InitOrUpdateResource(RHICmdList, &Self->PositionVertexBuffer);
				InitOrUpdateResource(RHICmdList, &Self->StaticMeshVertexBuffer);
				InitOrUpdateResource(RHICmdList, &Self->ColorVertexBuffer);

				FLocalVertexFactory::FDataType Data;
				Self->PositionVertexBuffer.BindPositionVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindTangentVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(VertexFactoryPtr, Data);
				Self->StaticMeshVertexBuffer.BindLightMapVertexBuffer(VertexFactoryPtr, Data, LightMapIndex);
				Self->ColorVertexBuffer.BindColorVertexBuffer(VertexFactoryPtr, Data);
				VertexFactoryPtr->SetData(RHICmdList, Data);

				InitOrUpdateResource(RHICmdList, VertexFactoryPtr);
			});
	}
};
struct FLGUIPostProcessSectionProxy : public FLGUIRenderSectionProxy
{
	FLGUIPostProcessSectionProxy()
	{
		Type = ELGUIRenderSectionType::PostProcess;
	}

	TWeakPtr<FUIPostProcessRenderProxy> PostProcessRenderProxy = nullptr;
};
struct FLGUIChildCanvasSectionProxy : public FLGUIRenderSectionProxy
{
	FLGUIChildCanvasSectionProxy()
	{
		Type = ELGUIRenderSectionType::ChildCanvas;
	}

	FPrimitiveComponentId PrimitiveComponentID;
	FLGUIRenderSceneProxy* ChildCanvasSceneProxy = nullptr;
};

DECLARE_CYCLE_STAT(TEXT("LGUIMesh CreateRenderSection"), STAT_CreateRenderSection, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_RT"), STAT_UpdateMeshSectionRT, STATGROUP_LGUI);
/** LGUI render scene proxy */
class FLGUIRenderSceneProxy : public FPrimitiveSceneProxy, public ILGUIRendererPrimitive
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	FLGUIRenderSceneProxy(ULGUIMeshComponent* InComponent, ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder, FLGUIRenderSceneProxy* InParentSceneProxy)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
		, RenderPriority(InComponent->TranslucencySortPriority)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreateRenderSection);
#if !UE_BUILD_SHIPPING
		DebugName = FName(FString::Printf(TEXT("%s_SceneProxy_%d"), *InComponent->GetName(), DebugNameIndex++));
#endif
		LGUIRenderer = InComponent->LGUIRenderer;
		RenderCanvasPtr = InCanvasPtr;
		CanvasLastRenderTime = &RenderCanvasPtr->LastRenderTime;
		bIsLGUIRenderToWorld = InComponent->bIsLGUIRenderToWorld;
		ParentSceneProxy = InParentSceneProxy;
		if (LGUIRenderer.IsValid())
		{
			auto TempRenderer = LGUIRenderer;
			auto SceneProxy = this;
			auto IsRenderToWorld = bIsLGUIRenderToWorld;
			ENQUEUE_RENDER_COMMAND(FLGUIRenderSceneProxy_AddPrimitive)(
				[TempRenderer, SceneProxy, InCanvasPtr, InCanvasSortOrder, IsRenderToWorld](FRHICommandListImmediate& RHICmdList)
				{
					if (TempRenderer.IsValid())
					{
						if (IsRenderToWorld)
						{
							TempRenderer.Pin()->AddWorldSpacePrimitive_RenderThread((ULGUICanvas*)InCanvasPtr, SceneProxy);
						}
						else
						{
							TempRenderer.Pin()->AddScreenSpacePrimitive_RenderThread(SceneProxy);
						}
					}
				}
			);
			bIsSupportLGUIRenderer = true;
		}
		bIsSupportUERenderer = InComponent->bIsSupportUERenderer;

		auto& SrcSections = InComponent->RenderSections;
		Sections.SetNumZeroed(SrcSections.Num());
		for (int SectionIndex = 0; SectionIndex < SrcSections.Num(); SectionIndex++)
		{
			Sections[SectionIndex] = CreateSectionData(SrcSections[SectionIndex].Get());
		}
		bNeedToSortRenderSections = true;
	}

	void AddSectionData(FLGUIRenderSection* SrcSection)
	{
		auto Section = CreateSectionData(SrcSection);
		if (Section != nullptr)
		{
			auto RenderProxy = this;
			ENQUEUE_RENDER_COMMAND(FLGUIRenderSceneProxy_AddSectionData)(
				[RenderProxy, Section](FRHICommandListImmediate& RHICmdList)
				{
					RenderProxy->AddSectionData_RenderThread(Section);
				}
			);
			bNeedToSortRenderSections = true;
		}
	}
	void AddSectionData_RenderThread(FLGUIRenderSectionProxy* Section)
	{
		Sections.Add(Section);
	}

	FLGUIRenderSectionProxy* CreateSectionData(FLGUIRenderSection* InSrcSection)
	{
		switch (InSrcSection->Type)
		{
		case ELGUIRenderSectionType::Mesh:
		{
			auto SrcSection = (FLGUIMeshSection*)InSrcSection;
			if (SrcSection->vertices.Num() == 0 || SrcSection->triangles.Num() == 0)
			{
				SrcSection->RenderProxy = nullptr;
				return nullptr;
			}
			FLGUIMeshSectionProxy* NewSectionProxy = new FLGUIMeshSectionProxy(GetScene().GetFeatureLevel());
			// vertex and index buffer
			const auto& SrcVertices = SrcSection->vertices;
			int NumVerts = SrcVertices.Num();
			if (bIsSupportLGUIRenderer)
			{
				auto& LGUIVertices = NewSectionProxy->LGUIVertexBuffers.Vertices;
				LGUIVertices.SetNumUninitialized(NumVerts);
				FMemory::Memcpy(LGUIVertices.GetData(), SrcVertices.GetData(), NumVerts * sizeof(FLGUIMeshVertex));
				NewSectionProxy->IndexBuffer.Indices = SrcSection->triangles;

				// Enqueue initialization of render resource
				BeginInitResource(&NewSectionProxy->IndexBuffer);
				BeginInitResource(&NewSectionProxy->LGUIVertexBuffers);
			}
			if (bIsSupportUERenderer)
			{
				NewSectionProxy->IndexBuffer.Indices = SrcSection->triangles;
				NewSectionProxy->InitFromLGUIVertexData(SrcSection->vertices);

				// Enqueue initialization of render resource
				BeginInitResource(&NewSectionProxy->VertexBuffers.PositionVertexBuffer);
				BeginInitResource(&NewSectionProxy->VertexBuffers.StaticMeshVertexBuffer);
				BeginInitResource(&NewSectionProxy->VertexBuffers.ColorVertexBuffer);
				BeginInitResource(&NewSectionProxy->IndexBuffer);
				BeginInitResource(&NewSectionProxy->VertexFactory);
			}

			// Grab material
			NewSectionProxy->Material = SrcSection->material;
			if (NewSectionProxy->Material == NULL)
			{
				NewSectionProxy->Material = UMaterial::GetDefaultMaterial(MD_Surface);
			}

			// Copy info
			NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
			SrcSection->RenderProxy = NewSectionProxy;

			return NewSectionProxy;
		}
		break;
		case ELGUIRenderSectionType::PostProcess:
		{
			auto SrcSection = (FLGUIPostProcessSection*)InSrcSection;
			FLGUIPostProcessSectionProxy* NewSectionProxy = new FLGUIPostProcessSectionProxy();

			NewSectionProxy->PostProcessRenderProxy = SrcSection->PostProcessRenderableObject->GetRenderProxy();

			// Copy info
			NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
			SrcSection->RenderProxy = NewSectionProxy;

			return NewSectionProxy;
		}
		break;
		case ELGUIRenderSectionType::ChildCanvas:
		{
			auto SrcSection = (FLGUIChildCanvasSection*)InSrcSection;
			auto NewSectionProxy = new FLGUIChildCanvasSectionProxy();

			auto& ChildCanvasMeshItem = SrcSection->ChildCanvasMeshComponent;
			NewSectionProxy->PrimitiveComponentID = ChildCanvasMeshItem->GetPrimitiveSceneId();
			if (ChildCanvasMeshItem->SceneProxy != nullptr)
			{
				NewSectionProxy->ChildCanvasSceneProxy = (FLGUIRenderSceneProxy*)ChildCanvasMeshItem->SceneProxy;
				NewSectionProxy->ChildCanvasSceneProxy->ParentSceneProxy = this;
			}

			// Copy info
			NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
			SrcSection->RenderProxy = NewSectionProxy;

			return NewSectionProxy;
		}
		break;
		}
		check(0);
		return nullptr;
	}
	void SetChildCanvasSectionData_RenderThread(FPrimitiveComponentId CompID, FLGUIRenderSceneProxy* SceneProxy)
	{
		for (int i = 0; i < Sections.Num(); i++)
		{
			auto Section = Sections[i];
			if (Section == nullptr)continue;
			if (Section->Type == ELGUIRenderSectionType::ChildCanvas)
			{
				auto ChildCanvasSection = (FLGUIChildCanvasSectionProxy*)Section;
				if (ChildCanvasSection->PrimitiveComponentID == CompID
					&& (ChildCanvasSection->ChildCanvasSceneProxy == nullptr || ChildCanvasSection->ChildCanvasSceneProxy->ParentSceneProxy == this)//check this because ParentSceneProxy could be a new one
					)
				{
					ChildCanvasSection->ChildCanvasSceneProxy = SceneProxy;
				}
			}
		}
	}
	void ClearChildCanvasSectionData_RenderThread(FLGUIRenderSceneProxy* SceneProxy)
	{
		for (int i = 0; i < Sections.Num(); i++)
		{
			auto Section = Sections[i];
			if (Section == nullptr)continue;
			if (Section->Type == ELGUIRenderSectionType::ChildCanvas)
			{
				auto ChildCanvasSection = (FLGUIChildCanvasSectionProxy*)Section;
				if (ChildCanvasSection->ChildCanvasSceneProxy == SceneProxy)//child could already get new proxy, so need to check it
				{
					ChildCanvasSection->ChildCanvasSceneProxy = nullptr;
					return;
				}
			}
		}
	}

	void DeleteSectionData_RenderThread(FLGUIRenderSectionProxy* Section, bool RemoveFromArray)
	{
		if (RemoveFromArray)
		{
			Sections.Remove(Section);
		}
		delete Section;
	}

	void RecreateSectionData(FLGUIRenderSection* SrcSection)
	{
		auto OldSection = SrcSection->RenderProxy;
		auto NewSection = CreateSectionData(SrcSection);
		auto RenderProxy = this;
		ENQUEUE_RENDER_COMMAND(FLGUIRenderSceneProxy_ReplaceSectionData)(
			[RenderProxy, OldSection, NewSection](FRHICommandListImmediate& RHICmdList) {
				RenderProxy->ReassignSectionData_RenderThread(OldSection, NewSection);
			});
	}
	void ReassignSectionData_RenderThread(FLGUIRenderSectionProxy* OldSection, FLGUIRenderSectionProxy* NewSection)
	{
		auto SectionIndex = Sections.IndexOfByKey(OldSection);
		DeleteSectionData_RenderThread(OldSection, false);
		check(SectionIndex >= 0);
		Sections[SectionIndex] = NewSection;
	}

	void SetRenderPriority_RenderThread(int32 NewPriority)
	{
		RenderPriority = NewPriority;
	}

	void SetRenderSectionRenderPriority_RenderThread(FLGUIRenderSectionProxy* Section, int32 NewPriority)
	{
		Section->SectionRenderPriority = NewPriority;
		bNeedToSortRenderSections = true;
	}

	void SortMeshSectionRenderPriority_RenderThread()
	{
		Algo::Sort(Sections, [](const FLGUIRenderSectionProxy* A, const FLGUIRenderSectionProxy* B) {
			if (A != nullptr && B != nullptr)
			{
				return A->SectionRenderPriority < B->SectionRenderPriority;
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

	virtual ~FLGUIRenderSceneProxy()
	{
		for(auto Section : Sections)
		{
			if (Section != nullptr)
			{
				switch (Section->Type)
				{
				case ELGUIRenderSectionType::ChildCanvas:
					auto ChildCanvasSection = (FLGUIChildCanvasSectionProxy*)Section;
					if (ChildCanvasSection->ChildCanvasSceneProxy)
					{
						if (ChildCanvasSection->ChildCanvasSceneProxy->ParentSceneProxy == this)//child canvas's ParentSceneProxy could already be new one, so check it
						{
							ChildCanvasSection->ChildCanvasSceneProxy->ParentSceneProxy = nullptr;
						}
					}
					break;
				}
				delete Section;
			}
		}
		Sections.Empty();
		if (ParentSceneProxy)
		{
			ParentSceneProxy->ClearChildCanvasSectionData_RenderThread(this);
		}
		if (LGUIRenderer.IsValid())
		{
			if (bIsLGUIRenderToWorld)
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
	void UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList
		, FLGUIMeshVertex* MeshVertexData, const int32& NumVerts
		, FLGUIMeshIndexBufferType* MeshIndexData, const uint32& IndexDataLength
		, const int8& AdditionalChannelFlags
		, FLGUIMeshSectionProxy* Section)
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionRT);

		check(IsInRenderingThread());

		// Check it references a valid section
		if (Section != nullptr)
		{
			//vertex buffer
			if (bIsSupportLGUIRenderer)
			{
				uint32 VertexDataLength = NumVerts * sizeof(FLGUIMeshVertex);
				void* VertexBufferData = RHICmdList.LockBuffer(Section->LGUIVertexBuffers.VertexBufferRHI, 0, VertexDataLength, RLM_WriteOnly);
				FMemory::Memcpy(VertexBufferData, MeshVertexData, VertexDataLength);
				RHICmdList.UnlockBuffer(Section->LGUIVertexBuffers.VertexBufferRHI);
			}
			if(bIsSupportUERenderer)
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
							Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, LGUIVert.TangentX.ToFVector3f(), LGUIVert.GetTangentY(), LGUIVert.TangentZ.ToFVector3f());
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
					void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
					RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.ColorVertexBuffer;
					void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
					RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
					void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
					RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
				}

				{
					auto& VertexBuffer = Section->VertexBuffers.StaticMeshVertexBuffer;
					void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
					FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
					RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
				}
			}


			// Lock index buffer
			auto IndexBufferData = RHICmdList.LockBuffer(Section->IndexBuffer.IndexBufferRHI, 0, IndexDataLength, RLM_WriteOnly);
			FMemory::Memcpy(IndexBufferData, (void*)MeshIndexData, IndexDataLength);
			RHICmdList.UnlockBuffer(Section->IndexBuffer.IndexBufferRHI);
		}
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (!bIsSupportUERenderer) return;
		if (ParentSceneProxy != nullptr && !bIsRenderFromParent)return;
		if (bNeedToSortRenderSections)
		{
			auto LGUIMeshSceneProxy = const_cast<FLGUIRenderSceneProxy*>(this);
			LGUIMeshSceneProxy->bNeedToSortRenderSections = false;
			LGUIMeshSceneProxy->SortMeshSectionRenderPriority_RenderThread();
		}
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
			auto RenderSection = Sections[i];
			if (RenderSection == nullptr)continue;
			
			switch (RenderSection->Type)
			{
			case ELGUIRenderSectionType::Mesh:
			{
				auto Section = (FLGUIMeshSectionProxy*)RenderSection;
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
						DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity);
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
			break;
			case ELGUIRenderSectionType::PostProcess:
				break;
			case ELGUIRenderSectionType::ChildCanvas:
			{
				auto Section = (FLGUIChildCanvasSectionProxy*)RenderSection;
				auto ChildSceneProxy = Section->ChildCanvasSceneProxy;
				if (ChildSceneProxy != nullptr)
				{
					ChildSceneProxy->bIsRenderFromParent = true;
					ChildSceneProxy->GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);
					ChildSceneProxy->bIsRenderFromParent = false;
				}
			}
			break;
			}
		}
	}

	//begin ILGUIRendererPrimitive interface
	virtual FVector3f GetWorldPositionForSortTranslucent()const override 
	{
		return (FVector3f)(GetLocalToWorld().GetOrigin()); 
	}
	virtual void CollectRenderData(TArray<FLGUIPrimitiveDataContainer>& OutRenderData, float CurrentWorldTime) override
	{
		if (ParentSceneProxy != nullptr)return;
		CollectRenderData_Implement(OutRenderData, CurrentWorldTime);
	}
	virtual void GetMeshElements(const FSceneViewFamily& ViewFamily, FMeshElementCollector* Collector, const FLGUIPrimitiveDataContainer& PrimitiveData, TArray<FLGUIMeshBatchContainer>& ResultArray) override
	{
		if (!bIsSupportLGUIRenderer)return;
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FMaterialRenderProxy* WireframeMaterialInstance = NULL;
		if (bWireframe)
		{
			WireframeMaterialInstance = GEngine->WireframeMaterial->GetRenderProxy();
		}

		for (int i = 0; i < PrimitiveData.Sections.Num(); i++)
		{
			auto SectionData = PrimitiveData.Sections[i];
			auto RenderSection = (FLGUIRenderSectionProxy*)SectionData.SectionPointer;

			auto Section = (FLGUIMeshSectionProxy*)RenderSection;
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			// Draw the mesh.
			FMeshBatch Mesh;
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &Section->IndexBuffer;
			BatchElement.PrimitiveIdMode = PrimID_ForceZero;
			Mesh.bWireframe = bWireframe;
			Mesh.MaterialRenderProxy = MaterialProxy;

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector->AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(Collector->GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, false);
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
			//BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, UseEditorDepthTest());

			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = Section->LGUIVertexBuffers.Vertices.Num() - 1;
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.bCanApplyViewModeOverrides = false;

			FLGUIMeshBatchContainer MeshBatchContainer;
			MeshBatchContainer.Mesh = Mesh;
			MeshBatchContainer.VertexBufferRHI = Section->LGUIVertexBuffers.VertexBufferRHI;
			MeshBatchContainer.NumVerts = Section->LGUIVertexBuffers.Vertices.Num();
			ResultArray.Add(MeshBatchContainer);
		}
	}

	virtual FUIPostProcessRenderProxy* GetPostProcessElement(const void* SectionPtr)const override
	{
		auto RenderSection = (FLGUIRenderSectionProxy*)SectionPtr;
		check(RenderSection->Type == ELGUIRenderSectionType::PostProcess);
		return ((FLGUIPostProcessSectionProxy*)RenderSection)->PostProcessRenderProxy.Pin().Get();
	}
	virtual int GetRenderPriority()const override
	{
		return RenderPriority;
	}
	virtual bool CanRender()const override
	{
		return ParentSceneProxy == nullptr && Sections.Num() > 0;
	}
	virtual FPrimitiveComponentId GetPrimitiveComponentId() const override 
	{
		return FPrimitiveSceneProxy::GetPrimitiveComponentId();
	}
	virtual FBoxSphereBounds GetWorldBounds()const override { return FPrimitiveSceneProxy::GetBounds(); }
	//end ILGUIRendererPrimitive interface
	void CollectRenderData_Implement(TArray<FLGUIPrimitiveDataContainer>& OutRenderDataArray, float CurrentWorldTime)
	{
		if (Sections.Num() <= 0)return;
		if (bNeedToSortRenderSections)
		{
			bNeedToSortRenderSections = false;
			this->SortMeshSectionRenderPriority_RenderThread();
		}
		*CanvasLastRenderTime = CurrentWorldTime;

		if (Sections[0] == nullptr)return;
		auto PrevRenderSectionType = Sections[0]->Type;
		auto PrevPrimitiveType = PrevRenderSectionType == ELGUIRenderSectionType::PostProcess ? ELGUIRendererPrimitiveType::PostProcess : ELGUIRendererPrimitiveType::Mesh;
		FLGUIPrimitiveDataContainer CurrentRenderData;
		CurrentRenderData.Primitive = this;
		CurrentRenderData.Type = PrevPrimitiveType;
		for (int i = 0; i < Sections.Num(); i++)
		{
			auto RenderSection = Sections[i];
			if (RenderSection != nullptr)
			{
				if (RenderSection->Type != PrevRenderSectionType)//render section type change, collect prev data
				{
					if (CurrentRenderData.Sections.Num() > 0)
					{
						OutRenderDataArray.Add(CurrentRenderData);
					}
					PrevRenderSectionType = RenderSection->Type;
					CurrentRenderData = FLGUIPrimitiveDataContainer();
					CurrentRenderData.Primitive = this;
					auto ItemPrimitiveType = RenderSection->Type == ELGUIRenderSectionType::PostProcess ? ELGUIRendererPrimitiveType::PostProcess : ELGUIRendererPrimitiveType::Mesh;
					CurrentRenderData.Type = ItemPrimitiveType;
				}

				switch (RenderSection->Type)
				{
				case ELGUIRenderSectionType::Mesh:
				{
					FLGUIPrimitiveSectionDataContainer SectionData;
					SectionData.SectionPointer = RenderSection;
					CurrentRenderData.Sections.Add(SectionData);
				}
				break;
				case ELGUIRenderSectionType::PostProcess:
				{
					auto Section = (FLGUIPostProcessSectionProxy*)RenderSection;
					auto PostProcessProxy = Section->PostProcessRenderProxy.Pin();
					if (PostProcessProxy.IsValid() && PostProcessProxy->CanRender())
					{
						FLGUIPrimitiveSectionDataContainer SectionData;
						SectionData.SectionPointer = RenderSection;
						CurrentRenderData.Sections.Add(SectionData);
					}
				}
				break;
				case ELGUIRenderSectionType::ChildCanvas:
				{
					auto Section = (FLGUIChildCanvasSectionProxy*)RenderSection;
					auto ChildSceneProxy = Section->ChildCanvasSceneProxy;
					if (ChildSceneProxy != nullptr)
					{
						ChildSceneProxy->CollectRenderData_Implement(OutRenderDataArray, CurrentWorldTime);
					}
				}
				break;
				}
			}
		}
		if (CurrentRenderData.Sections.Num() > 0)
		{
			OutRenderDataArray.Add(CurrentRenderData);
		}
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		if (bIsSupportUERenderer)
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
		return bIsSupportUERenderer && !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

	void SetParentSceneProxy_RenderThread(FLGUIRenderSceneProxy* InParentSceneProxy)
	{
		ParentSceneProxy = InParentSceneProxy;
	}
private:
	TArray<FLGUIRenderSectionProxy*> Sections;

	FMaterialRelevance MaterialRelevance;
	int32 RenderPriority = 0;
	TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool bIsSupportLGUIRenderer = false;
	bool bIsSupportUERenderer = true;
	bool bIsLGUIRenderToWorld = false;
	bool bNeedToSortRenderSections = true;
	ULGUICanvas* RenderCanvasPtr = nullptr;
#if !UE_BUILD_SHIPPING
	FName DebugName;
	static uint32 DebugNameIndex;
#endif
	bool bIsRenderFromParent = false;
	/** If have parent then render in parent */
	FLGUIRenderSceneProxy* ParentSceneProxy = nullptr;
	/**
	 * This is a pointer to LGUICanvas's LastRenderTime.
	 * Why it is safe to use? Check PrimitiveSceneInfo.h OwnerLastRenderTime
	 */
	float* CanvasLastRenderTime = nullptr;
};
#if !UE_BUILD_SHIPPING
uint32 FLGUIRenderSceneProxy::DebugNameIndex = 0;
#endif



void FLGUIMeshSection::UpdateSectionBox(const FTransform& LocalToWorld)
{
	BoundingBox = FBox(EForceInit::ForceInit);

	int vertCount = vertices.Num();
	if (vertCount > 2)
	{
		// Get maximum and minimum X, Y and Z positions of vectors
		for (int32 i = 0; i < vertCount; i++)
		{
			auto& VertPos = vertices[i].Position;
			BoundingBox += (FVector)VertPos;
		}
	}
	BoundingBox = BoundingBox.TransformBy(LocalToWorld);
}
void FLGUIPostProcessSection::UpdateSectionBox(const FTransform& LocalToWorld)
{
	BoundingBox = FBox(EForceInit::ForceInit);

	FVector2D Min, Max;
	PostProcessRenderableObject->GetGeometryBoundsInLocalSpace(Min, Max);
	auto WorldMin = PostProcessRenderableObject->GetComponentToWorld().TransformPosition(FVector(0, Min.X, Min.Y));
	auto WorldMax = PostProcessRenderableObject->GetComponentToWorld().TransformPosition(FVector(0, Max.X, Max.Y));
	BoundingBox += WorldMin;
	BoundingBox += WorldMax;
}
void FLGUIChildCanvasSection::UpdateSectionBox(const FTransform& LocalToWorld)
{
	BoundingBox = FBox(EForceInit::ForceInit);

	BoundingBox = ChildCanvasMeshComponent->Bounds.GetBox();
}


ULGUIMeshComponent::ULGUIMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	this->bCanEverAffectNavigation = false;
}
#include "Utils/LGUIUtils.h"
void ULGUIMeshComponent::CreateRenderSectionRenderData(TSharedPtr<FLGUIRenderSection> InRenderSection)
{
#if WITH_EDITOR
	for (auto& RenderSection : RenderSections)
	{
		if (RenderSection->Type == ELGUIRenderSectionType::Mesh)
		{
			auto MeshSection = (FLGUIMeshSection*)RenderSection.Get();
			if (MeshSection->vertices.Num() > LGUI_MAX_VERTEX_COUNT)
			{
				auto errorMsg = FText::Format(LOCTEXT("TooManyVerticesInSingleDdrawcall", "{0} Too many vertices ({1}) in single drawcall! This will cause issue!")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
					, MeshSection->vertices.Num());
				LGUIUtils::EditorNotification(errorMsg, 10);
				UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
			}
		}
	}
#endif
	InRenderSection->UpdateSectionBox(GetComponentTransform());

	if (InRenderSection->Type == ELGUIRenderSectionType::ChildCanvas)
	{
		auto ChildCanvasSection = (FLGUIChildCanvasSection*)InRenderSection.Get();
		auto ChildCanvasMeshCom = ChildCanvasSection->ChildCanvasMeshComponent;
		ChildCanvasMeshCom->OnSceneProxyCreated.AddWeakLambda(this, [this](ULGUIMeshComponent* InMesh, FLGUIRenderSceneProxy* InSceneProxy) {
			if (this->SceneProxy != nullptr)
			{
				auto ThisSceneProxy = (FLGUIRenderSceneProxy*)this->SceneProxy;//SceneProxy could change before the RENDER_COMMAND execute, so do necessary check in SetChildCanvasSectionData_RenderThread
				ENQUEUE_RENDER_COMMAND(FLGUIRenderSceneProxy_ReassignChildCanvasSectionData)(
					[ThisSceneProxy, CompID = InMesh->GetPrimitiveSceneId(), InSceneProxy](FRHICommandListImmediate& RHICmdList) {
						ThisSceneProxy->SetChildCanvasSectionData_RenderThread(CompID, InSceneProxy);
					});
			}
			});
	}

	if (SceneProxy)
	{
		auto ThisSceneProxy = (FLGUIRenderSceneProxy*)SceneProxy;
		if (InRenderSection->RenderProxy != nullptr)
		{
			ThisSceneProxy->RecreateSectionData(InRenderSection.Get());
		}
		else
		{
			ThisSceneProxy->AddSectionData(InRenderSection.Get());
		}
	}
	else
	{
		MarkRenderStateDirty(); // New section requires recreating scene proxy
	}
}

DECLARE_CYCLE_STAT(TEXT("LGUIMesh UpdateMeshSection_GT"), STAT_UpdateMeshSectionGT, STATGROUP_LGUI);
void ULGUIMeshComponent::UpdateMeshSectionRenderData(TSharedPtr<FLGUIRenderSection> InRenderSection, bool InVertexPositionChanged, int8 AdditionalShaderChannelFlags)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionGT);
	if (InVertexPositionChanged)
	{
		InRenderSection->UpdateSectionBox(GetComponentTransform());
	}
	if (SceneProxy)
	{
		check(InRenderSection->Type == ELGUIRenderSectionType::Mesh);
		auto MeshSection = (FLGUIMeshSection*)InRenderSection.Get();

		struct UpdateMeshSectionDataStruct
		{
			TArray<FLGUIMeshVertex> VertexBufferData;
			int32 NumVerts;
			TArray<FLGUIMeshIndexBufferType> IndexBufferData;
			uint32 IndexBufferDataLength;
			int8 AdditionalShaderChannelFlags;
			FLGUIMeshSectionProxy* Section;
			FLGUIRenderSceneProxy* SceneProxy;
		};
		UpdateMeshSectionDataStruct* UpdateData = new UpdateMeshSectionDataStruct();
		UpdateData->Section = (FLGUIMeshSectionProxy*)MeshSection->RenderProxy;
		//vertex data
		const int32 NumVerts = MeshSection->vertices.Num();
		UpdateData->VertexBufferData.AddUninitialized(NumVerts);
		FMemory::Memcpy(UpdateData->VertexBufferData.GetData(), MeshSection->vertices.GetData(), NumVerts * sizeof(FLGUIMeshVertex));
		UpdateData->NumVerts = NumVerts;
		UpdateData->SceneProxy = (FLGUIRenderSceneProxy*)SceneProxy;
		const int32 NumIndices = MeshSection->triangles.Num();
		const uint32 IndexBufferDataLength = NumIndices * sizeof(FLGUIMeshIndexBufferType);
		UpdateData->IndexBufferData.AddUninitialized(NumIndices);
		FMemory::Memcpy(UpdateData->IndexBufferData.GetData(), MeshSection->triangles.GetData(), IndexBufferDataLength);
		UpdateData->IndexBufferDataLength = IndexBufferDataLength;
		UpdateData->AdditionalShaderChannelFlags = AdditionalShaderChannelFlags;
		//update data
		ENQUEUE_RENDER_COMMAND(FLGUIMeshUpdate)(
			[UpdateData](FRHICommandListImmediate& RHICmdList)
			{
				UpdateData->SceneProxy->UpdateSection_RenderThread(
					RHICmdList
					, UpdateData->VertexBufferData.GetData()
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

void ULGUIMeshComponent::DeleteRenderSection(TSharedPtr<FLGUIRenderSection> InRenderSection)
{
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIRenderSceneProxy*)SceneProxy;
		auto SectionProxy = InRenderSection->RenderProxy;
		if (SectionProxy != nullptr)
		{
			ENQUEUE_RENDER_COMMAND(FLGUIRenderSceneProxy_DeleteSectionData)(
				[LGUIMeshSceneProxy, SectionProxy](FRHICommandListImmediate& RHICmdList)
				{
					LGUIMeshSceneProxy->DeleteSectionData_RenderThread(SectionProxy, true);
				}
			);
		}
	}

	if (InRenderSection->Type == ELGUIRenderSectionType::ChildCanvas)
	{
		auto ChildCanvasSection = (FLGUIChildCanvasSection*)InRenderSection.Get();
		ChildCanvasSection->ChildCanvasMeshComponent->ClearParentCavansMeshComp(this);
	}

	auto index = RenderSections.IndexOfByKey(InRenderSection);
	if (index != INDEX_NONE)
	{
		RenderSections.RemoveAt(index);
	}
}

void ULGUIMeshComponent::SetRenderSectionRenderPriority(TSharedPtr<FLGUIRenderSection> InMeshSection, int32 InSortPriority)
{
	InMeshSection->RenderPriority = InSortPriority;
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIRenderSceneProxy*)SceneProxy;
		auto Section = InMeshSection->RenderProxy;
		if (Section != nullptr)
		{
			auto RenderProxy = this;
			ENQUEUE_RENDER_COMMAND(FLGUIMeshSectionProxy_SetMeshSectionRenderPriority)(
				[LGUIMeshSceneProxy, Section, InSortPriority](FRHICommandListImmediate& RHICmdList) {
					LGUIMeshSceneProxy->SetRenderSectionRenderPriority_RenderThread(Section, InSortPriority);
				});
		}
	}
}

void ULGUIMeshComponent::SetMeshSectionMaterial(TSharedPtr<FLGUIRenderSection> InRenderSection, UMaterialInterface* InMaterial)
{
	check(InRenderSection->Type == ELGUIRenderSectionType::Mesh);
	((FLGUIMeshSection*)InRenderSection.Get())->material = InMaterial;
}

void ULGUIMeshComponent::VerifyMaterials()
{
	this->EmptyOverrideMaterials();

	int MatIndex = 0;
	for (auto& RenderSectionItem : RenderSections)
	{
		switch (RenderSectionItem->Type)
		{
		case ELGUIRenderSectionType::Mesh:
		{
			auto MeshSection = (FLGUIMeshSection*)RenderSectionItem.Get();
			this->SetMaterial(MatIndex++, MeshSection->material);
		}
		break;
		case ELGUIRenderSectionType::ChildCanvas:
		{
			auto ChildCanvasSection = (FLGUIChildCanvasSection*)RenderSectionItem.Get();
			for (auto ChildMat : ChildCanvasSection->ChildCanvasMeshComponent->OverrideMaterials)
			{
				this->SetMaterial(MatIndex++, ChildMat);
			}
		}
		break;
		}
	}
}

void ULGUIMeshComponent::SetParentCavansMeshComp(ULGUIMeshComponent* InMesh)
{
	if (ParentCanvasMeshComp != InMesh)
	{
		ParentCanvasMeshComp = InMesh;
	}
}
void ULGUIMeshComponent::ClearParentCavansMeshComp(ULGUIMeshComponent* InMesh)
{
	if (ParentCanvasMeshComp == InMesh)//check, incase parent already change
	{
		ParentCanvasMeshComp = nullptr;
	}
}

void ULGUIMeshComponent::SetUITranslucentSortPriority(int32 NewTranslucentSortPriority)
{
	UPrimitiveComponent::SetTranslucentSortPriority(NewTranslucentSortPriority);
	if (SceneProxy)
	{
		auto LGUIMeshSceneProxy = (FLGUIRenderSceneProxy*)SceneProxy;
		ENQUEUE_RENDER_COMMAND(FLGUIMesh_SetUITranslucentSortPriority)(
			[LGUIMeshSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
		{
			LGUIMeshSceneProxy->SetRenderPriority_RenderThread(NewTranslucentSortPriority);
		}
		);
	}
}

void ULGUIMeshComponent::UpdateChildCanvasSectionBox()
{
	struct LOCAL
	{
		static void UpdateChildCanvasSectionBox_Recursive(const TArray<TSharedPtr<FLGUIRenderSection>>& InRenderSections)
		{
			for (auto& RenderSectionItem : InRenderSections)
			{
				if (RenderSectionItem->Type == ELGUIRenderSectionType::ChildCanvas)
				{
					auto ChildCanvasSection = (FLGUIChildCanvasSection*)RenderSectionItem.Get();
					if (ChildCanvasSection->ChildCanvasMeshComponent != nullptr)
					{
						UpdateChildCanvasSectionBox_Recursive(ChildCanvasSection->ChildCanvasMeshComponent->RenderSections);
						ChildCanvasSection->UpdateSectionBox(ChildCanvasSection->ChildCanvasMeshComponent->GetComponentToWorld());
					}
				}
			}
		}
	};
	LOCAL::UpdateChildCanvasSectionBox_Recursive(RenderSections);
}

void ULGUIMeshComponent::UpdateLocalBounds()
{
	UpdateBounds();// Update global bounds		
	MarkRenderTransformDirty();// Need to send to render thread
}

struct FLGUIPrimitiveComponentIdTemporaryModifier
{
	ULGUIMeshComponent* Comp = nullptr;
	FPrimitiveComponentId OriginId;
	FLGUIPrimitiveComponentIdTemporaryModifier(ULGUIMeshComponent* InComp, FPrimitiveComponentId InNewId)
	{
		Comp = InComp;
		OriginId = Comp->GetPrimitiveSceneId();
		Comp->GetPrimitiveSceneId() = InNewId;
	}
	~FLGUIPrimitiveComponentIdTemporaryModifier()
	{
		Comp->GetPrimitiveSceneId() = OriginId;
	}
};

DECLARE_CYCLE_STAT(TEXT("LGUIMesh CreateSceneProxy"), STAT_LGUIMesh_CreateSceneProxy, STATGROUP_LGUI);
FPrimitiveSceneProxy* ULGUIMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_LGUIMesh_CreateSceneProxy);

	FLGUIRenderSceneProxy* Proxy = NULL;
	if (RenderSections.Num() > 0)
	{
		//change component id to RootCanvasUIMesh's component id, so when check visibility it will use RootCanvas's id
		{
			//turns out not work as I want, so comment the codes
			//auto RootCanvasUIMesh = RenderCanvas->GetRootCanvas()->GetUIMesh();
			//FLGUIPrimitiveComponentIdTemporaryModifier TempModifier(this, RootCanvasUIMesh->ComponentId);
			Proxy = new FLGUIRenderSceneProxy(this, RenderCanvas.Get(), RenderCanvas->GetActualSortOrder(), ParentCanvasMeshComp.IsValid() ? (FLGUIRenderSceneProxy*)ParentCanvasMeshComp->SceneProxy : nullptr);
		}
		OnSceneProxyCreated.Broadcast(this, Proxy);
	}
	return Proxy;
}

void ULGUIMeshComponent::SetRenderCanvas(ULGUICanvas* InCanvas)
{
	RenderCanvas = InCanvas;
}
void ULGUIMeshComponent::SetSupportLGUIRenderer(bool InSupportOrNot, TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> InLGUIRenderer, bool InIsRenderToWorld)
{
	if (InSupportOrNot)
	{
		LGUIRenderer = InLGUIRenderer;
		bIsLGUIRenderToWorld = InIsRenderToWorld;
	}
	else
	{
		LGUIRenderer.Reset();
	}
}

void ULGUIMeshComponent::SetSupportUERenderer(bool InSupportOrNot)
{
	bIsSupportUERenderer = InSupportOrNot;
}
void ULGUIMeshComponent::ClearRenderData()
{
	MarkRenderStateDirty();//mark dirty to recreate SceneProxy
	RenderSections.Reset();
	OnSceneProxyCreated.Clear();
	LGUIRenderer = nullptr;
}

int32 ULGUIMeshComponent::GetNumMaterials() const
{
	int Result = 0;
	for (auto& RenderSectionItem : RenderSections)
	{
		switch (RenderSectionItem->Type)
		{
		case ELGUIRenderSectionType::Mesh:
			Result++;
			break;
		case ELGUIRenderSectionType::ChildCanvas:
			auto ChildCanvasSection = (FLGUIChildCanvasSection*)RenderSectionItem.Get();
			Result += ChildCanvasSection->ChildCanvasMeshComponent->GetNumMaterials();
			break;
		}
	}
	return Result;
}

TSharedPtr<FLGUIRenderSection> ULGUIMeshComponent::CreateRenderSection(ELGUIRenderSectionType type)
{
	TSharedPtr<FLGUIRenderSection> Result = nullptr;
	switch (type)
	{
	case ELGUIRenderSectionType::Mesh:
		Result = MakeShared<FLGUIMeshSection>();
		break;
	case ELGUIRenderSectionType::PostProcess:
		Result = MakeShared<FLGUIPostProcessSection>();
		break;
	case ELGUIRenderSectionType::ChildCanvas:
		Result = MakeShared<FLGUIChildCanvasSection>();
		break;
	}
	RenderSections.Add(Result);
	return Result;
}

FBoxSphereBounds ULGUIMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (RenderSections.Num() <= 0)
	{
		return FBoxSphereBounds(EForceInit::ForceInitToZero);
	}

	FBox ResultBox = FBox(EForceInit::ForceInit);
	for (auto& RenderSection : RenderSections)
	{
		switch (RenderSection->Type)
		{
		case ELGUIRenderSectionType::Mesh:
		{
			ResultBox += RenderSection->BoundingBox;
		}
		break;
		case ELGUIRenderSectionType::PostProcess:
		{
			if (LGUIRenderer.IsValid())
			{
				ResultBox += RenderSection->BoundingBox;
			}
		}
		break;
		case ELGUIRenderSectionType::ChildCanvas:
		{
			ResultBox += RenderSection->BoundingBox;
		}
		break;
		}
	}

	return FBoxSphereBounds(ResultBox);
}
#undef LOCTEXT_NAMESPACE
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif