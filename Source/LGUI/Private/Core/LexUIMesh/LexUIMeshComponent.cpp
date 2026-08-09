// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIMesh/LexUIMeshComponent.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BodySetup.h"
#include "StaticMeshResources.h"
#include "Materials/Material.h"
#include "Core/LexUIRender/ILexUIRendererPrimitive.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "Engine/Engine.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Materials/MaterialRenderProxy.h"
#include "MaterialDomain.h"
#include "PrimitiveSceneProxy.h"
#include "Core/LexUIDrawCall.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "Core/Components/LexVisualDirectMesh.h"
#include "Core/Components/LexVisualPostProcess.h"
#include "Core/Components/LexWidget.h"
#include "RHIResourceUtils.h"


#define LOCTEXT_NAMESPACE "LexUIMeshComponent"


enum class ELexUIRenderSectionProxyType :uint8
{
	Mesh, PostProcess, ChildCanvas,
};
struct FLexUIRenderSectionProxy
{
	virtual ~FLexUIRenderSectionProxy() 
	{

	}

	ELexUIRenderSectionProxyType Type;

	/** Sort order */
	int SectionRenderPriority = 0;
	bool bCanRender = true;

	virtual void Disable() = 0;
};
/** Class representing a single section of the LexUI mesh */
struct FLexUISectionProxy_Mesh : public FLexUIRenderSectionProxy
{
	/** Material applied to this section */
	UMaterialInterface* Material = nullptr;
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	FLexUIMeshVertexBuffer LexUIVertexBuffers;
	/** Index buffer for this section */
	FLexUIMeshIndexBuffer IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;

	bool bShouldKeepDataWhenDisable = false;
	uint32 ValidVerticesCount = 0;
	uint32 NumPrimitives = 0;

	FLexUISectionProxy_Mesh(ERHIFeatureLevel::Type InFeatureLevel, bool InShouldKeepDataWhenDisable)
		: VertexFactory(InFeatureLevel, "FLexUISectionProxy_Mesh")
	{
		Type = ELexUIRenderSectionProxyType::Mesh;
		bShouldKeepDataWhenDisable = InShouldKeepDataWhenDisable;
	}
	virtual ~FLexUISectionProxy_Mesh()override
	{
		IndexBuffer.ReleaseResource();
		LexUIVertexBuffers.ReleaseResource();
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

	void InitFromLexUIVertexData(TArray<FLexUIMeshVertex>& Vertices)
	{
		auto LightMapIndex = 0;
		VertexBuffers.StaticMeshVertexBuffer.SetUseFullPrecisionUVs(true);
		if (Vertices.Num())
		{
			VertexBuffers.PositionVertexBuffer.Init(Vertices.Num());
			VertexBuffers.StaticMeshVertexBuffer.Init(Vertices.Num(), LEXUI_VERTEX_TEXCOORDINATE_COUNT);
			VertexBuffers.ColorVertexBuffer.Init(Vertices.Num());

			for (int32 i = 0; i < Vertices.Num(); i++)
			{
				const auto& Vertex = Vertices[i];

				VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
				for (uint32 j = 0; j < LEXUI_VERTEX_TEXCOORDINATE_COUNT; j++)
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
		ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_InitFromLexUIVertexData)(
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

	virtual void Disable() override
	{
		if (!bShouldKeepDataWhenDisable)
		{
			Material = nullptr;
			bCanRender = false;
		}
	}
};
struct FLexUIRenderSectionProxy_PostProcess : public FLexUIRenderSectionProxy
{
	FLexUIRenderSectionProxy_PostProcess()
	{
		Type = ELexUIRenderSectionProxyType::PostProcess;
	}

	FLexVisualPostProcessRenderProxy* PostProcessRenderProxy = nullptr;

	virtual void Disable() override
	{
		PostProcessRenderProxy = nullptr;
		bCanRender = false;
	}
};
struct FLexUIRenderSectionProxy_ChildCanvas : public FLexUIRenderSectionProxy
{
	FLexUIRenderSectionProxy_ChildCanvas()
	{
		Type = ELexUIRenderSectionProxyType::ChildCanvas;
	}

	FPrimitiveComponentId PrimitiveComponentID;
	FLexUIRenderSceneProxy* ChildCanvasSceneProxy = nullptr;

	virtual void Disable() override
	{
		PrimitiveComponentID = FPrimitiveComponentId();
		ChildCanvasSceneProxy = nullptr;
		bCanRender = false;
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FLexUIRenderSceneProxyReleaseDelegate, class FLexUIRenderSceneProxy*);

DECLARE_CYCLE_STAT(TEXT("LexUIMesh CreateRenderSection"), STAT_CreateRenderSection, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LexUIMesh UpdateMeshSection_RT"), STAT_UpdateMeshSectionRT, STATGROUP_LGUI);

/** LexUI render scene proxy */
class FLexUIRenderSceneProxy : public FPrimitiveSceneProxy, public ILexUIRendererPrimitive
{
public:
	virtual SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	FLexUIRenderSceneProxy(ULexUIMeshComponent* InComponent, ULexCanvas* InCanvasPtr, bool InIsRenderCanvas)
		: FPrimitiveSceneProxy(InComponent)
		, MaterialRelevance(InComponent->GetMaterialRelevance(GetScene().GetShaderPlatform()))
		, RenderPriority(InComponent->TranslucencySortPriority)
	{
		SCOPE_CYCLE_COUNTER(STAT_CreateRenderSection);
#if !UE_BUILD_SHIPPING
		static int DebugNameSuffix = 0;
		DebugName = FString::Printf(TEXT("%s_SceneProxy_%d"), *InCanvasPtr->GetWidget()->GetDisplayName(), DebugNameSuffix++);
#endif
		LexUIRenderer = InComponent->LexUIRenderer;
		RenderCanvasPtr = InCanvasPtr;
		bIsLexUIRenderToWorld = InComponent->bIsLexUIRenderToWorld;
		bIsRenderCanvas = InIsRenderCanvas;
		if (LexUIRenderer.IsValid())
		{
			auto TempRenderer = LexUIRenderer;
			auto SceneProxy = this;
			auto IsRenderToWorld = bIsLexUIRenderToWorld;
			auto BlendDepth = InCanvasPtr->GetActualBlendDepth();
			auto DepthFade = InCanvasPtr->GetActualDepthFade();
			ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_AddPrimitive)(
				[TempRenderer, SceneProxy, InCanvasPtr, BlendDepth, DepthFade, IsRenderToWorld](FRHICommandListImmediate& RHICmdList)
				{
					if (TempRenderer.IsValid())
					{
						if (IsRenderToWorld)
						{
							TempRenderer.Pin()->AddWorldSpacePrimitive_RenderThread(InCanvasPtr, BlendDepth, DepthFade, SceneProxy);
						}
						else
						{
							TempRenderer.Pin()->AddScreenSpacePrimitive_RenderThread(SceneProxy);
						}
					}
				}
			);
			bIsSupportLexUIRenderer = true;
		}
		bIsSupportUERenderer = InComponent->bIsSupportUERenderer;

		auto& SrcSections = InComponent->RenderSectionArray;
		SectionArray.SetNumZeroed(SrcSections.Num());
		for (int SectionIndex = 0; SectionIndex < SrcSections.Num(); SectionIndex++)
		{
			auto Section = CreateSectionData(SrcSections[SectionIndex].Get());
			SectionArray[SectionIndex] = Section;
		}
		bNeedToSortRenderSections = true;
	}

	void AddSectionData(FLexUIRenderSection* SrcSection)
	{
		auto Section = CreateSectionData(SrcSection);
		check (Section);
		ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_AddSectionData)(
			[this, Section](FRHICommandListImmediate& RHICmdList)
			{
				SectionArray.Add(Section);
			}
		);
		bNeedToSortRenderSections = true;
	}

	void RecreateSectionData(FLexUIRenderSection* InSrcSection)
	{
		auto OldSection = InSrcSection->RenderProxy;
		auto NewSection = CreateSectionData(InSrcSection);
		check(NewSection);
		ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_ReplaceSectionData)(
			[this, OldSection, NewSection](FRHICommandListImmediate& RHICmdList) {
				auto SectionIndex = SectionArray.IndexOfByKey(OldSection);
				SectionArray[SectionIndex] = NewSection;
				delete OldSection;
			});
	}
	void UpdatePostProcessSection(FLexUIRenderSection_PostProcess* InSrcSection, FLexVisualPostProcessRenderProxy* InRenderProxy)
	{
		ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_ReplaceSectionData)(
			[this, InSrcSection, InRenderProxy](FRHICommandListImmediate& RHICmdList) {
				auto PostProcessRenderProxy = static_cast<FLexUIRenderSectionProxy_PostProcess*>(InSrcSection->RenderProxy);
				PostProcessRenderProxy->PostProcessRenderProxy = InRenderProxy;
				PostProcessRenderProxy->bCanRender = true;
			});
	}
	void UpdateChildCanvasSection(FLexUIRenderSection_ChildCanvas* InSrcSection, ULexUIMeshComponent* InComp)
	{
		ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_ReplaceSectionData)(
			[this, InSrcSection, CompID = InComp->GetPrimitiveSceneId(), SceneProxy = InComp->SceneProxy](FRHICommandListImmediate& RHICmdList) {
				auto ChildCanvasRenderProxy = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(InSrcSection->RenderProxy);
				ChildCanvasRenderProxy->PrimitiveComponentID = CompID;
				if (SceneProxy != nullptr)
				{
					ChildCanvasRenderProxy->ChildCanvasSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
				}
				ChildCanvasRenderProxy->bCanRender = true;
			});
	}

	FLexUIRenderSectionProxy* CreateSectionData(FLexUIRenderSection* InSrcSection)
	{
		switch (InSrcSection->Type)
		{
		case ELexUIRenderSectionType::Mesh:
		case ELexUIRenderSectionType::DirectMesh:
			{
				auto SrcSection = static_cast<FLexUIRenderSection_Mesh*>(InSrcSection);
				if (SrcSection->Vertices.Num() == 0 || SrcSection->TriangleIndices.Num() == 0)
				{
					SrcSection->RenderProxy = nullptr;
					check(0);
					return nullptr;
				}
				auto NewSectionProxy = new FLexUISectionProxy_Mesh(GetScene().GetFeatureLevel()
					, InSrcSection->Type == ELexUIRenderSectionType::DirectMesh//direct mesh should not clear data when pooling
					);
				// vertex and index buffer
				auto& Indices = NewSectionProxy->IndexBuffer.Indices;
				Indices.SetNumUninitialized(SrcSection->TriangleIndices.Num());
				FMemory::Memcpy(Indices.GetData(), SrcSection->TriangleIndices.GetData(), SrcSection->TriangleIndices.Num() * sizeof(FLexUIMeshIndex));

				auto& SrcVertices = SrcSection->Vertices;
				
				NewSectionProxy->ValidVerticesCount = SrcSection->ValidVerticesNum;
				NewSectionProxy->NumPrimitives = SrcSection->ValidTriangleIndicesNum / 3;
				if (bIsSupportLexUIRenderer)
				{
					auto& Vertices = NewSectionProxy->LexUIVertexBuffers.Vertices;
					Vertices.SetNumUninitialized(SrcVertices.Num());
					FMemory::Memcpy(Vertices.GetData(), SrcVertices.GetData(), SrcVertices.Num() * sizeof(FLexUIMeshVertex));
					
					// Enqueue initialization of render resource
					BeginInitResource(&NewSectionProxy->IndexBuffer);
					BeginInitResource(&NewSectionProxy->LexUIVertexBuffers);
				}
				if (bIsSupportUERenderer)
				{
					NewSectionProxy->InitFromLexUIVertexData(SrcVertices);

					// Enqueue initialization of render resource
					BeginInitResource(&NewSectionProxy->VertexBuffers.PositionVertexBuffer);
					BeginInitResource(&NewSectionProxy->VertexBuffers.StaticMeshVertexBuffer);
					BeginInitResource(&NewSectionProxy->VertexBuffers.ColorVertexBuffer);
					BeginInitResource(&NewSectionProxy->IndexBuffer);
					BeginInitResource(&NewSectionProxy->VertexFactory);
				}

				// Grab material
				NewSectionProxy->Material = SrcSection->Material;
				if (NewSectionProxy->Material == nullptr)
				{
					NewSectionProxy->Material = UMaterial::GetDefaultMaterial(MD_Surface);
				}

				// Copy info
				NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
				SrcSection->RenderProxy = NewSectionProxy;

				return NewSectionProxy;
			}
		case ELexUIRenderSectionType::PostProcess:
			{
				auto SrcSection = static_cast<FLexUIRenderSection_PostProcess*>(InSrcSection);
				auto NewSectionProxy = new FLexUIRenderSectionProxy_PostProcess();
				NewSectionProxy->PostProcessRenderProxy = SrcSection->PostProcessVisualObject->GetRenderProxy();

				// Copy info
				NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
				SrcSection->RenderProxy = NewSectionProxy;

				return NewSectionProxy;
			}
		case ELexUIRenderSectionType::ChildCanvas:
			{
				auto SrcSection = static_cast<FLexUIRenderSection_ChildCanvas*>(InSrcSection);
				auto NewSectionProxy = new FLexUIRenderSectionProxy_ChildCanvas();
				auto& ChildCanvasMeshItem = SrcSection->ChildCanvasMeshComponent;
				NewSectionProxy->PrimitiveComponentID = ChildCanvasMeshItem->GetPrimitiveSceneId();
				if (ChildCanvasMeshItem->SceneProxy != nullptr)
				{
					auto ChildSceneProxy = static_cast<FLexUIRenderSceneProxy*>(ChildCanvasMeshItem->SceneProxy);
					NewSectionProxy->ChildCanvasSceneProxy = ChildSceneProxy;
					ChildSceneProxy->OnRelease.AddRaw(this, &FLexUIRenderSceneProxy::ClearChildCanvasSectionData_RenderThread);
				}

				// Copy info
				NewSectionProxy->SectionRenderPriority = SrcSection->RenderPriority;
				SrcSection->RenderProxy = NewSectionProxy;

				return NewSectionProxy;
			}
		}
		check(0);
		return nullptr;
	}
	void SetChildCanvasSectionData_RenderThread(FPrimitiveComponentId CompID, FLexUIRenderSceneProxy* SceneProxy)
	{
		for (int i = 0; i < SectionArray.Num(); i++)
		{
			auto Section = SectionArray[i];
			if (Section == nullptr)continue;
			if (Section->Type == ELexUIRenderSectionProxyType::ChildCanvas)
			{
				auto ChildCanvasSection = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(Section);
				if (ChildCanvasSection->PrimitiveComponentID == CompID
					)
				{
					ChildCanvasSection->ChildCanvasSceneProxy = SceneProxy;
					ChildCanvasSection->ChildCanvasSceneProxy->OnRelease.AddRaw(this, &FLexUIRenderSceneProxy::ClearChildCanvasSectionData_RenderThread);
				}
			}
		}
	}
	void ClearChildCanvasSectionData_RenderThread(FLexUIRenderSceneProxy* SceneProxy)
	{
		for (int i = 0; i < SectionArray.Num(); i++)
		{
			auto Section = SectionArray[i];
			if (Section == nullptr)continue;
			if (Section->Type == ELexUIRenderSectionProxyType::ChildCanvas)
			{
				auto ChildCanvasSection = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(Section);
				if (ChildCanvasSection->ChildCanvasSceneProxy == SceneProxy)//child could already get new proxy, so need to check it
				{
					ChildCanvasSection->ChildCanvasSceneProxy->OnRelease.RemoveAll(this);
					ChildCanvasSection->ChildCanvasSceneProxy = nullptr;
					return;
				}
			}
		}
	}
	
	void PoolAllSectionData_RenderThread()
	{
		for (int i = 0; i < SectionArray.Num(); i++)
		{
			auto Section = SectionArray[i];
			DetachChildCanvasSection_RenderThread(Section);
			Section->Disable();
		}
	}

	void SetRenderPriority_RenderThread(int32 NewPriority)
	{
		RenderPriority = NewPriority;
	}
	void SetMeshSectionMaterial_RenderThread(FLexUIRenderSectionProxy* Section, UMaterialInterface* Material)
	{
		(static_cast<FLexUISectionProxy_Mesh*>(Section))->Material = Material;
	}

	void SetRenderSectionRenderPriority_RenderThread(FLexUIRenderSectionProxy* Section, int32 NewPriority)
	{
		Section->SectionRenderPriority = NewPriority;
		bNeedToSortRenderSections = true;
	}

	void SortMeshSectionRenderPriority_RenderThread()
	{
		Algo::Sort(SectionArray, [](const FLexUIRenderSectionProxy* A, const FLexUIRenderSectionProxy* B) {
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

	void DetachChildCanvasSection_RenderThread(FLexUIRenderSectionProxy* Section)
	{
		if (Section == nullptr || Section->Type != ELexUIRenderSectionProxyType::ChildCanvas)
		{
			return;
		}
		auto ChildCanvasSection = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(Section);
		if (ChildCanvasSection->ChildCanvasSceneProxy != nullptr)
		{
			ChildCanvasSection->ChildCanvasSceneProxy->OnRelease.RemoveAll(this);
		}
		ChildCanvasSection->ChildCanvasSceneProxy = nullptr;
	}

	virtual ~FLexUIRenderSceneProxy()override
	{
		OnRelease.Broadcast(this);
#if !UE_BUILD_SHIPPING
		DebugName = FString::Printf(TEXT("%s_Deleted"), *DebugName);
#endif
		for(auto Section : SectionArray)
		{
			if (Section != nullptr)
			{
				DetachChildCanvasSection_RenderThread(Section);
				delete Section;
			}
		}
		SectionArray.Empty();
		if (LexUIRenderer.IsValid())
		{
			if (bIsLexUIRenderToWorld)
			{
				LexUIRenderer.Pin()->RemoveWorldSpacePrimitive_RenderThread(this);
			}
			else
			{
				LexUIRenderer.Pin()->RemoveScreenSpacePrimitive_RenderThread(this);
			}
			LexUIRenderer.Reset();
		}
	}
#if DEBUG_PRINT_MESH_MEMORY
	uint32 CalculateMeshMemorySize_RT()
	{
		MeshMemorySize = sizeof(FLexUISectionProxy_Mesh);
		MaxVertexBufferSize = 0;
		for (auto Section : SectionArray)
		{
			if (Section->Type == ELexUIRenderSectionProxyType::Mesh)
			{
				auto MeshSection = static_cast<FLexUISectionProxy_Mesh*>(Section);
				auto VertexBufferSize = MeshSection->LexUIVertexBuffers.Vertices.Num() * sizeof(FLexUIMeshVertexBuffer);
				if (MaxVertexBufferSize < VertexBufferSize) MaxVertexBufferSize = VertexBufferSize;
				MeshMemorySize += VertexBufferSize;
				MeshMemorySize += MeshSection->IndexBuffer.Indices.Num() * sizeof(FLexUIMeshIndexBufferType);
			}
		}
		return MeshMemorySize;
	}
#endif

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FRHICommandListImmediate& RHICmdList
		, const TArray<FLexUIMeshVertex>& MeshVertexData, const int32& NumVerts
		, const TArray<FLexUIMeshIndex>& MeshIndexData, const int32& NumTriangles
		, bool RequireNormalAndTangent
		, FLexUISectionProxy_Mesh* Section)const
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionRT);

		check(IsInRenderingThread());

		// Check it references a valid section
		check(Section != nullptr);
		Section->ValidVerticesCount = NumVerts;
		Section->bCanRender = true;
		//vertex buffer
		if (bIsSupportLexUIRenderer)
		{
			uint32 VertexDataLength = NumVerts * sizeof(FLexUIMeshVertex);
			void* VertexBufferData = RHICmdList.LockBuffer(Section->LexUIVertexBuffers.VertexBufferRHI, 0, VertexDataLength, RLM_WriteOnly);
			FMemory::Memcpy(VertexBufferData, MeshVertexData.GetData(), VertexDataLength);
			RHICmdList.UnlockBuffer(Section->LexUIVertexBuffers.VertexBufferRHI);
		}
		if(bIsSupportUERenderer)
		{
			for (int i = 0; i < NumVerts; i++)
			{
				auto& LexUIVert = MeshVertexData[i];
				Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = LexUIVert.Position;
				Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = LexUIVert.Color;
				if (RequireNormalAndTangent)
					Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, LexUIVert.TangentX.ToFVector3f(), LexUIVert.GetTangentY(), LexUIVert.TangentZ.ToFVector3f());
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, LexUIVert.TextureCoordinate[0]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, LexUIVert.TextureCoordinate[1]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, LexUIVert.TextureCoordinate[2]);
				Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, LexUIVert.TextureCoordinate[3]);
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

			if (RequireNormalAndTangent)
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

		Section->NumPrimitives = NumTriangles;
		uint32 IndicesDataLength = NumTriangles * 3 * sizeof(FLexUIMeshIndex);
		// Lock index buffer
		auto IndexBufferData = RHICmdList.LockBuffer(Section->IndexBuffer.IndexBufferRHI, 0, IndicesDataLength, RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, MeshIndexData.GetData(), IndicesDataLength);
		RHICmdList.UnlockBuffer(Section->IndexBuffer.IndexBufferRHI);
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (!bIsSupportUERenderer) return;
		if (!LexUI_CanRender())return;
		GetMeshElements_UERenderer(Views, ViewFamily, VisibilityMap, Collector);
	}
	void GetMeshElements_UERenderer(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
	{
		if (bNeedToSortRenderSections)
		{
			auto LexUIMeshSceneProxy = const_cast<FLexUIRenderSceneProxy*>(this);
			LexUIMeshSceneProxy->bNeedToSortRenderSections = false;
			LexUIMeshSceneProxy->SortMeshSectionRenderPriority_RenderThread();
		}
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(
				GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
				FLinearColor(0, 0.5f, 1.f)
			);

			Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		for (int i = 0; i < SectionArray.Num(); i++)
		{
			auto RenderSection = SectionArray[i];
			if (RenderSection == nullptr)continue;
			if (!RenderSection->bCanRender)continue;
			
			switch (RenderSection->Type)
			{
			case ELexUIRenderSectionProxyType::Mesh:
			{
				auto Section = static_cast<FLexUISectionProxy_Mesh*>(RenderSection);
				FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

				// For each view..
				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (VisibilityMap & (1 << ViewIndex))
					{
						// Draw the mesh.
						check(Section->VertexFactory.IsInitialized());
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
						BatchElement.NumPrimitives = Section->NumPrimitives;
						BatchElement.MinVertexIndex = 0;
						BatchElement.MaxVertexIndex = Section->ValidVerticesCount - 1;
						Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
						Mesh.Type = PT_TriangleList;
						Mesh.DepthPriorityGroup = SDPG_World;
						Mesh.bCanApplyViewModeOverrides = false;
						Collector.AddMesh(ViewIndex, Mesh);
					}
				}
			}
			break;
			case ELexUIRenderSectionProxyType::PostProcess:
				break;
			case ELexUIRenderSectionProxyType::ChildCanvas:
			{
				auto Section = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(RenderSection);
				auto ChildSceneProxy = Section->ChildCanvasSceneProxy;
				if (ChildSceneProxy != nullptr)
				{
					ChildSceneProxy->GetMeshElements_UERenderer(Views, ViewFamily, VisibilityMap, Collector);
				}
			}
			break;
			}
		}
	}

	//begin ILexUIRendererPrimitive interface
	virtual FVector3f LexUI_GetWorldPositionForSortTranslucent()const override 
	{
		return FVector3f(GetLocalToWorld().GetOrigin()); 
	}
	virtual void LexUI_CollectRenderData(TArray<FLexUIPrimitiveDataContainer>& OutRenderData) override
	{
#if DEBUG_PRINT_MESH_MEMORY
		CalculateMeshMemorySize_RT();
#endif
		CollectRenderData_Implement(OutRenderData);
	}
	virtual void LexUI_GetMeshElements(const FSceneViewFamily& ViewFamily, FMeshElementCollector& Collector, const FLexUIPrimitiveDataContainer& PrimitiveData, TArray<FLexUIMeshBatchContainer>& ResultArray) override
	{
		if (!bIsSupportLexUIRenderer)return;
		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (bWireframe)
		{
			WireframeMaterialInstance = GEngine->WireframeMaterial->GetRenderProxy();
		}

		for (int i = 0; i < PrimitiveData.Sections.Num(); i++)
		{
			auto SectionData = PrimitiveData.Sections[i];
			auto RenderSection = SectionData.SectionPointer;

			auto Section = static_cast<FLexUISectionProxy_Mesh*>(RenderSection);
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			// Draw the mesh.
			FMeshBatch Mesh;
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &Section->IndexBuffer;
			BatchElement.PrimitiveIdMode = PrimID_ForceZero;
			Mesh.bWireframe = bWireframe;
			Mesh.MaterialRenderProxy = MaterialProxy;

			FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
			DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, false, false);
			BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;
			//BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), false, UseEditorDepthTest());

			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = Section->NumPrimitives;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = Section->ValidVerticesCount - 1;
			Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
			Mesh.Type = PT_TriangleList;
			Mesh.DepthPriorityGroup = SDPG_World;
			Mesh.bCanApplyViewModeOverrides = false;

			FLexUIMeshBatchContainer MeshBatchContainer;
			MeshBatchContainer.Mesh = Mesh;
			MeshBatchContainer.VertexBufferRHI = Section->LexUIVertexBuffers.VertexBufferRHI;
			MeshBatchContainer.NumVerts = Section->ValidVerticesCount;
			ResultArray.Add(MeshBatchContainer);
		}
	}

	virtual FLexVisualPostProcessRenderProxy* LexUI_GetPostProcessElement(FLexUIRenderSectionProxy* SectionPtr)const override
	{
		check(SectionPtr->Type == ELexUIRenderSectionProxyType::PostProcess);
		return (static_cast<FLexUIRenderSectionProxy_PostProcess*>(SectionPtr))->PostProcessRenderProxy;
	}
	virtual int LexUI_GetRenderPriority()const override
	{
		return RenderPriority;
	}
	virtual bool LexUI_CanRender()const override
	{
		return bIsRenderCanvas && SectionArray.Num() > 0;
	}
	virtual FPrimitiveComponentId LexUI_GetPrimitiveComponentId() const override 
	{
		return FPrimitiveSceneProxy::GetPrimitiveComponentId();
	}
	virtual FBoxSphereBounds LexUI_GetWorldBounds()const override { return FPrimitiveSceneProxy::GetBounds(); }
	//end ILexUIRendererPrimitive interface
	void CollectRenderData_Implement(TArray<FLexUIPrimitiveDataContainer>& OutRenderDataArray)
	{
		if (SectionArray.Num() <= 0)return;
		if (bNeedToSortRenderSections)
		{
			bNeedToSortRenderSections = false;
			this->SortMeshSectionRenderPriority_RenderThread();
		}

		if (SectionArray[0] == nullptr)return;
		auto PrevRenderSectionType = SectionArray[0]->Type;
		auto PrevPrimitiveType = PrevRenderSectionType == ELexUIRenderSectionProxyType::PostProcess ? ELexUIRendererPrimitiveType::PostProcess : ELexUIRendererPrimitiveType::Mesh;
		FLexUIPrimitiveDataContainer CurrentRenderData;
		CurrentRenderData.Primitive = this;
		CurrentRenderData.Type = PrevPrimitiveType;
		for (int i = 0; i < SectionArray.Num(); i++)
		{
			auto RenderSection = SectionArray[i];
			if (RenderSection == nullptr)continue;
			if (!RenderSection->bCanRender)continue;
			if (RenderSection->Type != PrevRenderSectionType)//render section type change, collect prev data
			{
				if (CurrentRenderData.Sections.Num() > 0)
				{
					OutRenderDataArray.Add(CurrentRenderData);
				}
				PrevRenderSectionType = RenderSection->Type;
				CurrentRenderData = FLexUIPrimitiveDataContainer();
				CurrentRenderData.Primitive = this;
				auto ItemPrimitiveType = RenderSection->Type == ELexUIRenderSectionProxyType::PostProcess ? ELexUIRendererPrimitiveType::PostProcess : ELexUIRendererPrimitiveType::Mesh;
				CurrentRenderData.Type = ItemPrimitiveType;
			}

			switch (RenderSection->Type)
			{
			case ELexUIRenderSectionProxyType::Mesh:
				{
					FLexUIPrimitiveSectionDataContainer SectionData;
					SectionData.SectionPointer = RenderSection;
					CurrentRenderData.Sections.Add(SectionData);
				}
				break;
			case ELexUIRenderSectionProxyType::PostProcess:
				{
					auto Section = static_cast<FLexUIRenderSectionProxy_PostProcess*>(RenderSection);
					if (Section->PostProcessRenderProxy->CanRender())
					{
						FLexUIPrimitiveSectionDataContainer SectionData;
						SectionData.SectionPointer = RenderSection;
						CurrentRenderData.Sections.Add(SectionData);
					}
				}
				break;
			case ELexUIRenderSectionProxyType::ChildCanvas:
				{
					auto Section = static_cast<FLexUIRenderSectionProxy_ChildCanvas*>(RenderSection);
					auto ChildSceneProxy = Section->ChildCanvasSceneProxy;
					if (ChildSceneProxy != nullptr)
					{
						ChildSceneProxy->CollectRenderData_Implement(OutRenderDataArray);
					}
				}
				break;
			}
		}
		if (CurrentRenderData.Sections.Num() > 0)
		{
			OutRenderDataArray.Add(CurrentRenderData);
		}
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
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

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return(sizeof(*this) + GetAllocatedSize());
	}
#if DEBUG_PRINT_MESH_MEMORY
	uint32 GetMeshMemorySize()const { return MeshMemorySize; }
	uint32 GetMaxVertexBufferSize()const{return MaxVertexBufferSize;}
#endif
private:
	TArray<FLexUIRenderSectionProxy*> SectionArray;
#if DEBUG_PRINT_MESH_MEMORY
	uint32 MeshMemorySize = 0;
	uint32 MaxVertexBufferSize = 0;
#endif
	FMaterialRelevance MaterialRelevance;
	int32 RenderPriority = 0;
	TWeakPtr<FLexUIRenderer, ESPMode::ThreadSafe> LexUIRenderer;
	bool bIsSupportLexUIRenderer = false;
	bool bIsSupportUERenderer = true;
	bool bIsLexUIRenderToWorld = false;
	bool bNeedToSortRenderSections = true;
	bool bIsRenderCanvas = false;
	TWeakObjectPtr<ULexCanvas> RenderCanvasPtr = nullptr;
	FLexUIRenderSceneProxyReleaseDelegate OnRelease;
};



void FLexUIRenderSection_Mesh::ClearBeforePool()
{
	Material = nullptr;
}

void FLexUIRenderSection_PostProcess::ClearBeforePool()
{
	PostProcessVisualObject = nullptr;
}

void FLexUIRenderSection_ChildCanvas::ClearBeforePool()
{
	ChildCanvasMeshComponent = nullptr;
}


ULexUIMeshComponent::ULexUIMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	this->bCanEverAffectNavigation = false;
}

void ULexUIMeshComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

TSharedPtr<FLexUIRenderSection> ULexUIMeshComponent::SetupRenderSection(ELexUIRenderSectionType InType, FLexUIDrawCall* InDrawCallData)
{
	auto GetMeshRenderSectionFromPool = [&](int32 NumVertices)
	{
		auto& RenderSections = GetRenderSectionMeshPool(NumVertices);
		if (RenderSections.Num() == 0)
		{
			return TSharedPtr<FLexUIRenderSection_Mesh>();
		}
		auto HeadNode = RenderSections.GetHead();
		auto RenderSection = HeadNode->GetValue();
		RenderSections.RemoveNode(HeadNode);
		return RenderSection;
	};
	auto GetRenderSectionFromPool = [&]()
	{
		for (auto Node = RenderSectionPool.GetHead(); Node != nullptr; Node = Node->GetNextNode() )
		{
			auto RenderSection = Node->GetValue();
			if (RenderSection->Type == InType)
			{
				RenderSectionPool.RemoveNode(Node);
				return RenderSection;
			}
		}
		return TSharedPtr<FLexUIRenderSection>();
	};
	auto GetDirectMeshRenderSectionFromPool = [&](const ULexVisualDirectMesh* DirectMesh)
	{
		for (auto Node = RenderSectionPool.GetHead(); Node != nullptr; Node = Node->GetNextNode() )
		{
			auto RenderSection = Node->GetValue();
			if (RenderSection->Type == ELexUIRenderSectionType::DirectMesh)
			{
				auto DirectMeshRenderSection = static_cast<FLexUIRenderSection_DirectMesh*>(RenderSection.Get());
				if (DirectMeshRenderSection->DirectMeshVisualObject == DirectMesh//keep reference of DirectMeshVisualObject
					|| DirectMeshRenderSection->DirectMeshVisualObject == nullptr//if old one is deleted then we can use it
					)
				{
					RenderSectionPool.RemoveNode(Node);
					return RenderSection;
				}
			}
		}
		return TSharedPtr<FLexUIRenderSection>();
	};

	TSharedPtr<FLexUIRenderSection> RenderSection;
	switch (InType)
	{
	case ELexUIRenderSectionType::Mesh:
		RenderSection = GetMeshRenderSectionFromPool(InDrawCallData->CombinedBatchMeshGeometryVertices.Num());
		break;
	case ELexUIRenderSectionType::DirectMesh:
		RenderSection = GetDirectMeshRenderSectionFromPool(InDrawCallData->DirectMeshVisualObject.Get());
		break;
	case ELexUIRenderSectionType::PostProcess:
	case ELexUIRenderSectionType::ChildCanvas:
		RenderSection = GetRenderSectionFromPool();
		break;
	}
	if (!RenderSection)
	{
		switch (InType)
		{
		case ELexUIRenderSectionType::Mesh:
			RenderSection = MakeShared<FLexUIRenderSection_Mesh>();
			break;
		case ELexUIRenderSectionType::PostProcess:
			RenderSection = MakeShared<FLexUIRenderSection_PostProcess>();
			break;
		case ELexUIRenderSectionType::ChildCanvas:
			RenderSection = MakeShared<FLexUIRenderSection_ChildCanvas>();
			break;
		case ELexUIRenderSectionType::DirectMesh:
			RenderSection = MakeShared<FLexUIRenderSection_DirectMesh>();
			break;
		}
	}
	
	switch (InType)
	{
	case ELexUIRenderSectionType::Mesh:
		{
			auto MeshSectionPtr = static_cast<FLexUIRenderSection_Mesh*>(RenderSection.Get());
			bool bNeedExpandMeshSection = false;
			if (MeshSectionPtr->Vertices.Num() < InDrawCallData->CombinedBatchMeshGeometryVertices.Num())
			{
				MeshSectionPtr->Vertices.SetNumUninitialized(InDrawCallData->CombinedBatchMeshGeometryVertices.Num());
				bNeedExpandMeshSection = true;
			}
			MeshSectionPtr->ValidVerticesNum = InDrawCallData->CombinedBatchMeshGeometryVertices.Num();
			FMemory::Memcpy(MeshSectionPtr->Vertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.Num() * sizeof(FLexUIMeshVertex));
			if (MeshSectionPtr->TriangleIndices.Num() < InDrawCallData->CombinedBatchMeshGeometryTriangles.Num())
			{
				MeshSectionPtr->TriangleIndices.SetNumUninitialized(InDrawCallData->CombinedBatchMeshGeometryTriangles.Num());
				bNeedExpandMeshSection = true;
			}
			MeshSectionPtr->ValidTriangleIndicesNum = InDrawCallData->CombinedBatchMeshGeometryTriangles.Num();
			FMemory::Memcpy(MeshSectionPtr->TriangleIndices.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.Num() * sizeof(FLexUIMeshIndex));
			MeshSectionPtr->BoundingBox = InDrawCallData->CombinedBounds.TransformBy(GetComponentTransform());
			if (MeshSectionPtr->RenderProxy)//if we have valid render-proxy then recreate data or update data
			{
				if (bNeedExpandMeshSection)
				{
#if DEBUG_PRINT_MESH_MEMORY
					ExpandMeshSectionCount++;
#endif
					ExpandMeshSectionRenderData(MeshSectionPtr);
				}
				else
				{
					UpdateMeshSectionRenderData(MeshSectionPtr, RenderCanvas->GetActualRequireNormalAndTangent());
				}
			}
			else//no valid render-proxy, because it is newly created
			{
				if (this->SceneProxy != nullptr)
				{
					auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);
					ThisSceneProxy->AddSectionData(MeshSectionPtr);
				}
			}
		}
		break;
	case ELexUIRenderSectionType::DirectMesh:
		{
			auto DirectMeshSectionPtr = StaticCastSharedPtr<FLexUIRenderSection_DirectMesh>(RenderSection);
			auto DirectMeshVisualObject = InDrawCallData->DirectMeshVisualObject;
			DirectMeshSectionPtr->DirectMeshVisualObject = DirectMeshVisualObject;
			auto BoundingBox = FBox(EForceInit::ForceInit);
			FVector Min, Max;
			DirectMeshVisualObject->GetGeometryBounds3DInLocalSpace(Min, Max);
			BoundingBox += Min;
			BoundingBox += Max;
			DirectMeshSectionPtr->BoundingBox = BoundingBox;
			DirectMeshVisualObject->OnSupplyMeshSection(this, DirectMeshSectionPtr);
		}
		break;
	case ELexUIRenderSectionType::PostProcess:
		{
			auto PostProcessSectionPtr = static_cast<FLexUIRenderSection_PostProcess*>(RenderSection.Get());
			auto PostProcessVisualObject = InDrawCallData->PostProcessVisualObject;
			PostProcessSectionPtr->PostProcessVisualObject = PostProcessVisualObject;
			auto BoundingBox = FBox(EForceInit::ForceInit);
			FVector Min, Max;
			PostProcessVisualObject->GetGeometryBounds3DInLocalSpace(Min, Max);
			BoundingBox += Min;
			BoundingBox += Max;
			PostProcessSectionPtr->BoundingBox = BoundingBox;
			if (PostProcessSectionPtr->RenderProxy)//if we have valid render-proxy then update data
			{
				if (this->SceneProxy != nullptr)
				{
					auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);//SceneProxy could change before the RENDER_COMMAND execute, so do necessary check in SetChildCanvasSectionData_RenderThread
					auto RenderProxy = PostProcessSectionPtr->PostProcessVisualObject->GetRenderProxy();
					ThisSceneProxy->UpdatePostProcessSection(PostProcessSectionPtr, RenderProxy);
				}
			}
			else
			{
				if (this->SceneProxy != nullptr)
				{
					auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);
					ThisSceneProxy->AddSectionData(PostProcessSectionPtr);
				}
			}
		}
		break;
	case ELexUIRenderSectionType::ChildCanvas:
		{
			auto ChildCanvasSectionPtr = static_cast<FLexUIRenderSection_ChildCanvas*>(RenderSection.Get());
			ChildCanvasSectionPtr->ChildCanvasMeshComponent = InDrawCallData->ChildCanvas->GetUIMesh();
			ChildCanvasSectionPtr->ChildCanvasMeshComponent->SetParentCanvasMeshComp(this);
			if (ChildCanvasSectionPtr->RenderProxy)
			{
				if (this->SceneProxy != nullptr)
				{
					auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);//SceneProxy could change before the RENDER_COMMAND execute, so do necessary check in SetChildCanvasSectionData_RenderThread
					ThisSceneProxy->UpdateChildCanvasSection(ChildCanvasSectionPtr, InDrawCallData->ChildCanvas->GetUIMesh());
				}
			}
			else
			{
				if (this->SceneProxy != nullptr)
				{
					auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);
					ThisSceneProxy->AddSectionData(ChildCanvasSectionPtr);
				}
			}
		}
		break;
	}

	RenderSectionArray.Add(RenderSection);
	return RenderSection;
}

void ULexUIMeshComponent::UpdateMeshSection(int Index, FLexUIDrawCall* InDrawCallData)
{
	auto& RenderSection = RenderSectionArray[Index];
	
	auto MeshSectionPtr = static_cast<FLexUIRenderSection_Mesh*>(RenderSection.Get());
	if (MeshSectionPtr->RenderProxy)//if we have valid render-proxy then recreate or update data
	{
		MeshSectionPtr->BoundingBox = InDrawCallData->CombinedBounds.TransformBy(GetComponentTransform());
		FMemory::Memcpy(MeshSectionPtr->Vertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.Num() * sizeof(FLexUIMeshVertex));
		FMemory::Memcpy(MeshSectionPtr->TriangleIndices.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.Num() * sizeof(FLexUIMeshIndex));
		UpdateMeshSectionRenderData(MeshSectionPtr, RenderCanvas->GetActualRequireNormalAndTangent());
	}
	else//no valid render-proxy, because it is newly created
	{
		MeshSectionPtr->BoundingBox = InDrawCallData->CombinedBounds.TransformBy(GetComponentTransform());
		FMemory::Memcpy(MeshSectionPtr->Vertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.GetData(), InDrawCallData->CombinedBatchMeshGeometryVertices.Num() * sizeof(FLexUIMeshVertex));
		FMemory::Memcpy(MeshSectionPtr->TriangleIndices.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.GetData(), InDrawCallData->CombinedBatchMeshGeometryTriangles.Num() * sizeof(FLexUIMeshIndex));
		if (this->SceneProxy != nullptr)
		{
			auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);
			ThisSceneProxy->AddSectionData(MeshSectionPtr);
		}
	}
}

void ULexUIMeshComponent::SetupDirectMeshRenderSection(FLexUIRenderSection_DirectMesh* InDirectMeshSection, bool bNeedExpandMeshSection, UMaterialInterface* InMaterial)
{
	InDirectMeshSection->BoundingBox = InDirectMeshSection->BoundingBox.TransformBy(GetComponentTransform());
	
	if (InDirectMeshSection->RenderProxy)//if we have valid render-proxy then recreate data or update data
	{
		if (bNeedExpandMeshSection)
		{
			ExpandMeshSectionRenderData(InDirectMeshSection);
		}
		else
		{
			UpdateMeshSectionRenderData(InDirectMeshSection, RenderCanvas->GetActualRequireNormalAndTangent());
		}
	}
	else//no valid render-proxy, because it is newly created
	{
		if (this->SceneProxy != nullptr)
		{
			auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(this->SceneProxy);
			ThisSceneProxy->AddSectionData(InDirectMeshSection);
		}
	}

	SetDirectMeshRenderSectionMaterial(InDirectMeshSection, InMaterial);
}

void ULexUIMeshComponent::SetDirectMeshRenderSectionMaterial(FLexUIRenderSection_DirectMesh* InDirectMeshSection, UMaterialInterface* InMaterial)
{
	InDirectMeshSection->Material = InMaterial;
	if (SceneProxy)
	{
		if (InDirectMeshSection->RenderProxy)
		{
			UpdateMeshSectionMaterialDataStruct UpdateData;
			UpdateData.SectionProxy = InDirectMeshSection->RenderProxy;
			UpdateData.Material = InMaterial;

			auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
			ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_SetMeshSectionMaterial)(
				[LexUIMeshSceneProxy, UpdateData = MoveTemp(UpdateData)](FRHICommandListImmediate& RHICmdList) {
					LexUIMeshSceneProxy->SetMeshSectionMaterial_RenderThread(UpdateData.SectionProxy, UpdateData.Material);
				});
		}
	}
}

#define LATE_FLUSH_RENDER_CMD 1
DECLARE_CYCLE_STAT(TEXT("LexUIMesh UpdateMeshSection_GT"), STAT_UpdateMeshSectionGT, STATGROUP_LGUI);
void ULexUIMeshComponent::UpdateMeshSectionRenderData(FLexUIRenderSection_Mesh* InMeshSection, bool InRequireNormalAndTangent)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateMeshSectionGT);
	if (SceneProxy)
	{
		UpdateMeshSectionDataStruct UpdateData;
		UpdateData.Section = static_cast<FLexUISectionProxy_Mesh*>(InMeshSection->RenderProxy);
		//vertex data
		const int32 NumVerts = InMeshSection->ValidVerticesNum;
		UpdateData.VertexBufferData.AddUninitialized(NumVerts);
		FMemory::Memcpy(UpdateData.VertexBufferData.GetData(), InMeshSection->Vertices.GetData(), NumVerts * sizeof(FLexUIMeshVertex));
		UpdateData.NumVerts = NumVerts;
		const int32 NumIndices = InMeshSection->ValidTriangleIndicesNum;
		UpdateData.IndexBufferData.AddUninitialized(NumIndices);
		UpdateData.NumTriangles = NumIndices / 3;
		FMemory::Memcpy(UpdateData.IndexBufferData.GetData(), InMeshSection->TriangleIndices.GetData(), NumIndices * sizeof(FLexUIMeshIndex));
		UpdateData.RequireNormalAndTangent = InRequireNormalAndTangent;
		//update data
#if LATE_FLUSH_RENDER_CMD
		PendingUpdateMeshSectionDataArray.Add(MoveTemp(UpdateData));
#else
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMeshUpdate)(
			[LexUIMeshSceneProxy, UpdateData = MoveTemp(UpdateData)](FRHICommandListImmediate& RHICmdList)
			{
				LexUIMeshSceneProxy->UpdateSection_RenderThread(
					RHICmdList
					, UpdateData.VertexBufferData.GetData()
					, UpdateData.NumVerts
					, UpdateData.IndexBufferData.GetData()
					, UpdateData.NumTriangles
					, UpdateData.RequireNormalAndTangent
					, UpdateData.Section
				);
			});
#endif
	}
}

void ULexUIMeshComponent::ExpandMeshSectionRenderData(FLexUIRenderSection_Mesh* InMeshSection)
{
	if (SceneProxy)
	{
		auto ThisSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ThisSceneProxy->RecreateSectionData(InMeshSection);
	}
}

void ULexUIMeshComponent::PoolAllRenderSection()
{
	if (SceneProxy)
	{
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_PoolAllSectionData)(
			[LexUIMeshSceneProxy](FRHICommandListImmediate& RHICmdList) {
				LexUIMeshSceneProxy->PoolAllSectionData_RenderThread();
			});
	}
	for (auto& RenderSection : RenderSectionArray)
	{
		if (RenderSection->Type == ELexUIRenderSectionType::ChildCanvas)
		{
			auto ChildCanvasSection = static_cast<FLexUIRenderSection_ChildCanvas*>(RenderSection.Get());
			auto ChildCanvasMeshCom = ChildCanvasSection->ChildCanvasMeshComponent;
			if (ChildCanvasMeshCom.IsValid())
			{
				ChildCanvasMeshCom->ClearParentCanvasMeshComp(this);
				ChildCanvasMeshCom->OnSceneProxyCreated.RemoveAll(this);
			}
		}

		RenderSection->ClearBeforePool();
		switch (RenderSection->Type)
		{
		case ELexUIRenderSectionType::Mesh:
			{
				auto MeshSection = StaticCastSharedPtr<FLexUIRenderSection_Mesh>(RenderSection);
				int32 NumVertices = MeshSection->Vertices.Num();
				auto& RenderSections = GetRenderSectionMeshPool(NumVertices);
				RenderSections.AddTail(MeshSection);
			}
			break;
		default:
			RenderSectionPool.AddTail(RenderSection);
			break;
		}
	}
	RenderSectionArray.Reset();
}

void ULexUIMeshComponent::SetRenderSectionRenderPriority(int32 InSectionIndex, int32 InSortPriority)
{
	auto RenderSection = RenderSectionArray[InSectionIndex];
	RenderSection->RenderPriority = InSortPriority;
	if (SceneProxy)
	{
		if (RenderSection->RenderProxy)
		{
			UpdateRenderSectionPriority UpdateData;
			UpdateData.SectionProxy = RenderSection->RenderProxy;
			UpdateData.RenderPriority = InSortPriority;
#if LATE_FLUSH_RENDER_CMD
			PendingUpdateRenderSectionPriorityArray.Add(MoveTemp(UpdateData));
#else
			auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
			ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_SetMeshSectionRenderPriority)(
				[LexUIMeshSceneProxy, UpdateData = MoveTemp(UpdateData)](FRHICommandListImmediate& RHICmdList) {
					LexUIMeshSceneProxy->SetRenderSectionRenderPriority_RenderThread(UpdateData.SectionProxy, UpdateData.RenderPriority);
				});
#endif
		}
	}
}

void ULexUIMeshComponent::SetMeshSectionMaterial(int32 InSectionIndex, UMaterialInterface* InMaterial)
{
	auto RenderSection = RenderSectionArray[InSectionIndex];
	check(RenderSection->Type == ELexUIRenderSectionType::Mesh);
	(static_cast<FLexUIRenderSection_Mesh*>(RenderSection.Get()))->Material = InMaterial;
	if (SceneProxy)
	{
		if (RenderSection->RenderProxy)
		{
			UpdateMeshSectionMaterialDataStruct UpdateData;
			UpdateData.SectionProxy = RenderSection->RenderProxy;
			UpdateData.Material = InMaterial;
#if LATE_FLUSH_RENDER_CMD
			PendingUpdateMeshSectionMaterialDataArray.Add(MoveTemp(UpdateData));
#else
			auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
			ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_SetMeshSectionMaterial)(
				[LexUIMeshSceneProxy, UpdateData = MoveTemp(UpdateData)](FRHICommandListImmediate& RHICmdList) {
					LexUIMeshSceneProxy->SetMeshSectionMaterial_RenderThread(UpdateData.SectionProxy, UpdateData.Material);
				});
#endif
		}
	}
}

void ULexUIMeshComponent::VerifyMaterials()
{
#if 1
	if (OverrideMaterials.Num())
	{
		for (int32 MatIndex = 0; MatIndex < OverrideMaterials.Num(); MatIndex++)
		{
			if (UMaterialInterface* MatInterface = OverrideMaterials[MatIndex].Get())
			{
				MatInterface->OnRemovedAsOverride(this);
			}
		}
		// Precache PSOs again
		// PrecachePSOs();
		OverrideMaterials.Reset();
	}
	auto SetMaterialForUI = [=, this](int ElementIndex, UMaterialInterface* Material)
	{
		// Grow the array if the new index is too large
		if (OverrideMaterials.Num() <= ElementIndex)
		{
			OverrideMaterials.AddZeroed(ElementIndex + 1 - OverrideMaterials.Num());
		}

		// Set the material and invalidate things
		OverrideMaterials[ElementIndex] = Material;

		if (Material)
		{
			Material->OnAssignedAsOverride(this);
		}

		// Precache PSOs again
		// PrecachePSOs();

		if (Material)
		{
			Material->AddToCluster(this, true);
		}
	};
	int MatIndex = 0;
	for (auto& RenderSectionItem : RenderSectionArray)
	{
		switch (RenderSectionItem->Type)
		{
		case ELexUIRenderSectionType::Mesh:
			{
				auto MeshSection = static_cast<FLexUIRenderSection_Mesh*>(RenderSectionItem.Get());
				SetMaterialForUI(MatIndex++, MeshSection->Material);
			}
			break;
		case ELexUIRenderSectionType::ChildCanvas:
			{
				auto ChildCanvasSection = static_cast<FLexUIRenderSection_ChildCanvas*>(RenderSectionItem.Get());
				for (auto ChildMat : ChildCanvasSection->ChildCanvasMeshComponent->OverrideMaterials)
				{
					SetMaterialForUI(MatIndex++, ChildMat);
				}
			}
			break;
		}
	}
#else
	this->EmptyOverrideMaterials();

	int MatIndex = 0;
	for (auto& RenderSectionItem : RenderSectionArray)
	{
		switch (RenderSectionItem->Type)
		{
		case ELexUIRenderSectionType::Mesh:
		{
			auto MeshSection = (FLexUIRenderSection_Mesh*)RenderSectionItem.Get();
			this->SetMaterial(MatIndex++, MeshSection->material);
		}
		break;
		case ELexUIRenderSectionType::ChildCanvas:
		{
			auto ChildCanvasSection = (FLexUIRenderSection_ChildCanvas*)RenderSectionItem.Get();
			for (auto ChildMat : ChildCanvasSection->ChildCanvasMeshComponent->OverrideMaterials)
			{
				this->SetMaterial(MatIndex++, ChildMat);
			}
		}
		break;
		}
	}
#endif
}

void ULexUIMeshComponent::SetParentCanvasMeshComp(ULexUIMeshComponent* InParentCanvasMeshComp)
{
	if (ParentCanvasMeshComp != InParentCanvasMeshComp)
	{
		auto ChildCanvasMeshCom = this;
		if (ParentCanvasMeshComp != nullptr)
		{
			ChildCanvasMeshCom->OnSceneProxyCreated.RemoveAll(ParentCanvasMeshComp.Get());
		}
		
		ParentCanvasMeshComp = InParentCanvasMeshComp;

		ChildCanvasMeshCom->OnSceneProxyCreated.AddWeakLambda(InParentCanvasMeshComp, [InParentCanvasMeshComp](ULexUIMeshComponent* InChildMeshComp, FLexUIRenderSceneProxy* InSceneProxy) {
			if (InParentCanvasMeshComp->SceneProxy != nullptr)
			{
				auto ParentSceneProxy = static_cast<FLexUIRenderSceneProxy*>(InParentCanvasMeshComp->SceneProxy);//SceneProxy could change before the RENDER_COMMAND execute, so do necessary check in SetChildCanvasSectionData_RenderThread
				ENQUEUE_RENDER_COMMAND(FLexUIRenderSceneProxy_ReassignChildCanvasSectionData)(
					[ParentSceneProxy, CompID = InChildMeshComp->GetPrimitiveSceneId(), InSceneProxy](FRHICommandListImmediate& RHICmdList) {
						ParentSceneProxy->SetChildCanvasSectionData_RenderThread(CompID, InSceneProxy);
					});
			}
			});
	}
}
void ULexUIMeshComponent::ClearParentCanvasMeshComp(ULexUIMeshComponent* InParentCanvasMeshComp)
{
	if (ParentCanvasMeshComp == InParentCanvasMeshComp)//check, incase parent already change
	{
		auto ChildCanvasMeshCom = this;
		if (ParentCanvasMeshComp != nullptr)
		{
			ChildCanvasMeshCom->OnSceneProxyCreated.RemoveAll(ParentCanvasMeshComp.Get());
		}
		ParentCanvasMeshComp = nullptr;
	}
}

void ULexUIMeshComponent::SetUITranslucentSortPriority(int32 NewTranslucentSortPriority)
{
	UPrimitiveComponent::SetTranslucentSortPriority(NewTranslucentSortPriority);
	if (SceneProxy)
	{
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMesh_SetUITranslucentSortPriority)(
			[LexUIMeshSceneProxy, NewTranslucentSortPriority](FRHICommandListImmediate& RHICmdList)
		{
			LexUIMeshSceneProxy->SetRenderPriority_RenderThread(NewTranslucentSortPriority);
		}
		);
	}
}

void ULexUIMeshComponent::UpdateChildCanvasSectionBox()
{
	struct LOCAL
	{
		static void UpdateChildCanvasSectionBox_Recursive(const TArray<TSharedPtr<FLexUIRenderSection>>& InRenderSections)
		{
			for (auto& RenderSectionItem : InRenderSections)
			{
				if (RenderSectionItem->Type == ELexUIRenderSectionType::ChildCanvas)
				{
					auto ChildCanvasSection = StaticCastSharedPtr<FLexUIRenderSection_ChildCanvas>(RenderSectionItem);
					if (ChildCanvasSection->ChildCanvasMeshComponent != nullptr)
					{
						UpdateChildCanvasSectionBox_Recursive(ChildCanvasSection->ChildCanvasMeshComponent->RenderSectionArray);
						ChildCanvasSection->BoundingBox = ChildCanvasSection->ChildCanvasMeshComponent->Bounds.GetBox();//how we can be sure that children canvas bounds is ready? because we update child canvas drawcall before parent
					}
				}
			}
		}
	};
	LOCAL::UpdateChildCanvasSectionBox_Recursive(RenderSectionArray);

#if DEBUG_PRINT_MESH_MEMORY
	if (SceneProxy)
	{
		auto Proxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		auto MemSize = (double)Proxy->GetMeshMemorySize();
		FString MemSizeStr;
		if (MemSize > 1024 * 1024 * 1024)
		{
			MemSizeStr = FString::Printf(TEXT("%fGB"), MemSize / (1024 * 1024 * 1024));
		}
		else if (MemSize > 1024 * 1024)
		{
			MemSizeStr = FString::Printf(TEXT("%fMB"), MemSize / (1024 * 1024));
		}
		else
		{
			MemSizeStr = FString::Printf(TEXT("%fKB"), MemSize / 1024);
		}
		auto MaxVertexBufferSize = (double)Proxy->GetMaxVertexBufferSize();
		FString MaxVertexBufferSizeStr;
		if (MaxVertexBufferSize > 1024 * 1024)
		{
			MaxVertexBufferSizeStr = FString::Printf(TEXT("%fMB"), MaxVertexBufferSize / (1024 * 1024));
		}
		else if (MaxVertexBufferSize > 1024)
		{
			MaxVertexBufferSizeStr = FString::Printf(TEXT("%fKB"), MaxVertexBufferSize / 1024);
		}
		else
		{
			MaxVertexBufferSizeStr = FString::Printf(TEXT("%fB"), MaxVertexBufferSize);
		}
		auto DebugMsg = FString::Printf(TEXT("RenderProxy:%s UsingSectionCount:%d ExpandMeshSectionCount:%d MeshMemorySize:%s MaxVertexBufferSize:%s"), *Proxy->DebugName, RenderSectionArray.Num(), ExpandMeshSectionCount, *MemSizeStr, *MaxVertexBufferSizeStr);
		UE_LOG(LGUI, Error, TEXT("%s"), *DebugMsg);
		if (ExpandMeshSectionCount > 0)
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, DebugMsg);
		}
		ExpandMeshSectionCount = 0;
	}
#endif
}

void ULexUIMeshComponent::UpdateLocalBounds() 
{
	UpdateBounds();// Update global bounds		
	MarkRenderTransformDirty();// Need to send to render thread
}

struct FLexUIPrimitiveComponentIdTemporaryModifier
{
	ULexUIMeshComponent* Comp = nullptr;
	FPrimitiveComponentId OriginId;
	FLexUIPrimitiveComponentIdTemporaryModifier(ULexUIMeshComponent* InComp, FPrimitiveComponentId InNewId)
	{
		Comp = InComp;
		OriginId = Comp->GetPrimitiveSceneId();
		Comp->GetPrimitiveSceneId() = InNewId;
	}
	~FLexUIPrimitiveComponentIdTemporaryModifier()
	{
		Comp->GetPrimitiveSceneId() = OriginId;
	}
};

DECLARE_CYCLE_STAT(TEXT("LexUIMesh CreateSceneProxy"), STAT_LexUIMesh_CreateSceneProxy, STATGROUP_LGUI);
FPrimitiveSceneProxy* ULexUIMeshComponent::CreateSceneProxy()
{
	SCOPE_CYCLE_COUNTER(STAT_LexUIMesh_CreateSceneProxy);
	//clear section data
	RenderSectionPool.Empty();
	for (auto& RenderSectionPoolItem : RenderSectionMesh_CascadePool)
	{
		RenderSectionPoolItem.RenderSections.Empty();
	}

	FLexUIRenderSceneProxy* Proxy = nullptr;
	if (RenderSectionArray.Num() > 0)
	{
		Proxy = new FLexUIRenderSceneProxy(this, RenderCanvas.Get()
			, !ParentCanvasMeshComp.IsValid()//child canvas is render by it's parent
			);
		OnSceneProxyCreated.Broadcast(this, Proxy);
	}
	return Proxy;
}

void ULexUIMeshComponent::Init(ULexCanvas* InCanvas)
{
	RenderCanvas = InCanvas;
	TArray<int32> VertexBufferRangeSlices = {0, 128, 1024, 8192, 32768, 65535, TNumericLimits<int32>::Max()};
	for (int i = 1; i < VertexBufferRangeSlices.Num(); i++)
	{
		FMeshRenderSectionPool Pool;
		RenderSectionMesh_CascadePool.Add(MoveTemp(Pool));
	}
}
TDoubleLinkedList<TSharedPtr<FLexUIRenderSection_Mesh>>& ULexUIMeshComponent::GetRenderSectionMeshPool(int32 InNumVertices)
{
	auto GetRange = [](int32 x)
	{
		if (x <= 127) return 0;
		if (x <= 1023) return 1;
		if (x <= 8191) return 2;
		if (x <= 32767) return 3;
		if (x <= 65535) return 4;
		return 5;
	};
	int32 Index = GetRange(InNumVertices);
	return RenderSectionMesh_CascadePool[Index].RenderSections;
}
void ULexUIMeshComponent::SetSupportLexUIRenderer(bool InSupportOrNot, TWeakPtr<FLexUIRenderer, ESPMode::ThreadSafe> InLexUIRenderer, bool InIsRenderToWorld)
{
	if (InSupportOrNot)
	{
		LexUIRenderer = InLexUIRenderer;
		bIsLexUIRenderToWorld = InIsRenderToWorld;
	}
	else
	{
		LexUIRenderer.Reset();
	}
}

void ULexUIMeshComponent::SetSupportUERenderer(bool InSupportOrNot)
{
	bIsSupportUERenderer = InSupportOrNot;
}
void ULexUIMeshComponent::ClearRenderData()
{
	MarkRenderStateDirty();//mark dirty to recreate SceneProxy
	RenderSectionArray.Empty();
	RenderSectionPool.Empty();
	for (auto& RenderSectionPoolItem : RenderSectionMesh_CascadePool)
	{
		RenderSectionPoolItem.RenderSections.Empty();
	}
	OnSceneProxyCreated.Clear();
	ParentCanvasMeshComp = nullptr;
	LexUIRenderer = nullptr;
}

DECLARE_CYCLE_STAT(TEXT("LexUIMesh FlushRenderCommand"), STAT_LexUIMesh_FlushRenderCommand, STATGROUP_LGUI);
void ULexUIMeshComponent::FlushRenderCommand()
{
	SCOPE_CYCLE_COUNTER(STAT_LexUIMesh_FlushRenderCommand)
	if (PendingUpdateMeshSectionDataArray.Num() > 0)
	{
		//update data
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMeshUpdate)(
			[LexUIMeshSceneProxy, PendingUpdateMeshSectionDataArray = MoveTemp(PendingUpdateMeshSectionDataArray)](FRHICommandListImmediate& RHICmdList)
			{
				for (auto& UpdateData : PendingUpdateMeshSectionDataArray)
				{
					LexUIMeshSceneProxy->UpdateSection_RenderThread(
						RHICmdList
						, UpdateData.VertexBufferData
						, UpdateData.NumVerts
						, UpdateData.IndexBufferData
						, UpdateData.NumTriangles
						, UpdateData.RequireNormalAndTangent
						, UpdateData.Section
					);
				}
			});
	}
	if (PendingUpdateRenderSectionPriorityArray.Num() > 0)
	{
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_SetMeshSectionRenderPriority)(
			[LexUIMeshSceneProxy, PendingUpdateRenderSectionPriorityArray = MoveTemp(PendingUpdateRenderSectionPriorityArray)](FRHICommandListImmediate& RHICmdList) {
				for (auto& UpdateData : PendingUpdateRenderSectionPriorityArray)
				{
					LexUIMeshSceneProxy->SetRenderSectionRenderPriority_RenderThread(UpdateData.SectionProxy, UpdateData.RenderPriority);
				}
			});
	}
	if (PendingUpdateMeshSectionMaterialDataArray.Num() > 0)
	{
		auto LexUIMeshSceneProxy = static_cast<FLexUIRenderSceneProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FLexUIMeshSectionProxy_SetMeshSectionMaterial)(
			[LexUIMeshSceneProxy, PendingUpdateMeshSectionMaterialDataArray = MoveTemp(PendingUpdateMeshSectionMaterialDataArray)](FRHICommandListImmediate& RHICmdList) {
				for (auto& UpdateData : PendingUpdateMeshSectionMaterialDataArray)
				{
					LexUIMeshSceneProxy->SetMeshSectionMaterial_RenderThread(UpdateData.SectionProxy, UpdateData.Material);
				}
			});
	}
}

int32 ULexUIMeshComponent::GetNumMaterials() const
{
	int Result = 0;
	for (auto& RenderSectionItem : RenderSectionArray)
	{
		switch (RenderSectionItem->Type)
		{
		case ELexUIRenderSectionType::Mesh:
		case ELexUIRenderSectionType::DirectMesh:
			Result++;
			break;
		case ELexUIRenderSectionType::ChildCanvas:
			auto ChildCanvasSection = static_cast<FLexUIRenderSection_ChildCanvas*>(RenderSectionItem.Get());
			Result += ChildCanvasSection->ChildCanvasMeshComponent->GetNumMaterials();
			break;
		}
	}
	return Result;
}

FBoxSphereBounds ULexUIMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	if (RenderSectionArray.Num() <= 0)
	{
		return FBoxSphereBounds(EForceInit::ForceInitToZero);
	}

	FBox ResultBox = FBox(EForceInit::ForceInit);
	for (auto& RenderSection : RenderSectionArray)
	{
		switch (RenderSection->Type)
		{
		case ELexUIRenderSectionType::DirectMesh:
			{
				ResultBox += RenderSection->BoundingBox;
			}
			break;
		case ELexUIRenderSectionType::Mesh:
			{
				ResultBox += RenderSection->BoundingBox;
			}
			break;
		case ELexUIRenderSectionType::PostProcess:
			{
				if (LexUIRenderer.IsValid())
				{
					ResultBox += RenderSection->BoundingBox;
				}
			}
			break;
		case ELexUIRenderSectionType::ChildCanvas:
			{
				ResultBox += RenderSection->BoundingBox;
			}
			break;
		}
	}

	return FBoxSphereBounds(ResultBox);
}
#undef LOCTEXT_NAMESPACE
