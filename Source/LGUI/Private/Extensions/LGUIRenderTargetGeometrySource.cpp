﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/LGUIRenderTargetGeometrySource.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/BodySetup.h"
#include "Utils/LGUIUtils.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "LTweenBPLibrary.h"
#include "Materials/MaterialRenderProxy.h"
#include "SceneInterface.h"
#include "RayTracingInstance.h"
#include "RayTracingGeometry.h"
#if WITH_EDITOR
#include "PrefabSystem/LGUIPrefabManager.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIRenderTargetGeometrySource"

#define MIN_SEG 4
#define MAX_SEG 32

class FLGUIRenderTargetGeometrySourceMeshProxySection
{
public:
	/** Vertex buffer for this section */
	FStaticMeshVertexBuffers VertexBuffers;
	/** Index buffer for this section */
	FDynamicMeshIndexBuffer16 IndexBuffer;
	/** Vertex factory for this section */
	FLocalVertexFactory VertexFactory;

#if RHI_RAYTRACING
	FRayTracingGeometry RayTracingGeometry;
#endif

	FLGUIRenderTargetGeometrySourceMeshProxySection(ERHIFeatureLevel::Type InFeatureLevel)
		: VertexFactory(InFeatureLevel, "FLGUIRenderTargetGeometrySourceMeshProxySection")
	{}
};

class FLGUIRenderTargetGeometrySource_SceneProxy : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	FLGUIRenderTargetGeometrySource_SceneProxy(ULGUIRenderTargetGeometrySource* Component)
		: FPrimitiveSceneProxy(Component)
		, RenderTarget(Component->GetRenderTarget())
		, MaterialInstance(Component->GetMaterialInstance())
		, GeometryMode(Component->GetGeometryMode())
	{
		MaterialRelevance = MaterialInstance->GetRelevance_Concurrent(GetScene().GetFeatureLevel());

		Section = new FLGUIRenderTargetGeometrySourceMeshProxySection(GetScene().GetFeatureLevel());

		// Copy index buffer
		Section->IndexBuffer.Indices = Component->Triangles;

		Section->VertexBuffers.InitFromDynamicVertex(&Section->VertexFactory, Component->Vertices, 4);

		// Enqueue initialization of render resource
		BeginInitResource(&Section->VertexBuffers.PositionVertexBuffer);
		BeginInitResource(&Section->VertexBuffers.StaticMeshVertexBuffer);
		BeginInitResource(&Section->VertexBuffers.ColorVertexBuffer);
		BeginInitResource(&Section->IndexBuffer);
		BeginInitResource(&Section->VertexFactory);

#if RHI_RAYTRACING
		if (IsRayTracingEnabled())
		{
			ENQUEUE_RENDER_COMMAND(InitProceduralMeshRayTracingGeometry)(
				[this, DebugName = Component->GetFName()](FRHICommandListImmediate& RHICmdList)
				{
					FRayTracingGeometryInitializer Initializer;
					Initializer.DebugName = DebugName;
					Initializer.IndexBuffer = nullptr;
					Initializer.TotalPrimitiveCount = 0;
					Initializer.GeometryType = RTGT_Triangles;
					Initializer.bFastBuild = true;
					Initializer.bAllowUpdate = false;

					Section->RayTracingGeometry.SetInitializer(Initializer);
					Section->RayTracingGeometry.InitResource(RHICmdList);

					Section->RayTracingGeometry.Initializer.IndexBuffer = Section->IndexBuffer.IndexBufferRHI;
					Section->RayTracingGeometry.Initializer.TotalPrimitiveCount = Section->IndexBuffer.Indices.Num() / 3;

					FRayTracingGeometrySegment Segment;
					Segment.VertexBuffer = Section->VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
					Segment.NumPrimitives = Section->RayTracingGeometry.Initializer.TotalPrimitiveCount;
					Segment.MaxVertices = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices();
					Section->RayTracingGeometry.Initializer.Segments.Add(Segment);

					//#dxr_todo: add support for segments?

					Section->RayTracingGeometry.UpdateRHI(RHICmdList);
				});
		}
#endif
	}

	virtual ~FLGUIRenderTargetGeometrySource_SceneProxy()
	{
		if (Section != nullptr)
		{
			Section->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Section->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Section->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Section->IndexBuffer.ReleaseResource();
			Section->VertexFactory.ReleaseResource();

#if RHI_RAYTRACING
			if (IsRayTracingEnabled())
			{
				Section->RayTracingGeometry.ReleaseResource();
			}
#endif

			delete Section;
		}
	}

	/** Called on render thread to assign new dynamic data */
	void UpdateSection_RenderThread(FRHICommandListBase& RHICmdList, FDynamicMeshVertex* MeshVertexData, int32 NumVerts, uint16* MeshIndexData, int32 NumIndex)
	{
		check(IsInRenderingThread());

		// Iterate through vertex data, copying in new info
		for (int32 i = 0; i < NumVerts; i++)
		{
			auto& Vertex = MeshVertexData[i];

			Section->VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertex.Position;
			Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertex.TangentX.ToFVector3f(), Vertex.GetTangentY(), Vertex.TangentZ.ToFVector3f());
			Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertex.TextureCoordinate[0]);
			Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 1, Vertex.TextureCoordinate[1]);
			Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 2, Vertex.TextureCoordinate[2]);
			Section->VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 3, Vertex.TextureCoordinate[3]);
			Section->VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertex.Color;
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

		// Lock index buffer
		auto IndexBufferData = RHICmdList.LockBuffer(Section->IndexBuffer.IndexBufferRHI, 0, NumIndex, RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, (void*)MeshIndexData, NumIndex);
		RHICmdList.UnlockBuffer(Section->IndexBuffer.IndexBufferRHI);

#if RHI_RAYTRACING
		if (IsRayTracingEnabled())
		{
			Section->RayTracingGeometry.ReleaseResource();

			FRayTracingGeometryInitializer Initializer;
			Initializer.IndexBuffer = Section->IndexBuffer.IndexBufferRHI;
			Initializer.TotalPrimitiveCount = Section->IndexBuffer.Indices.Num() / 3;
			Initializer.GeometryType = RTGT_Triangles;
			Initializer.bFastBuild = true;
			Initializer.bAllowUpdate = false;

			Section->RayTracingGeometry.SetInitializer(Initializer);
			Section->RayTracingGeometry.InitResource(RHICmdList);

			FRayTracingGeometrySegment Segment;
			Segment.VertexBuffer = Section->VertexBuffers.PositionVertexBuffer.VertexBufferRHI;
			Segment.NumPrimitives = Section->RayTracingGeometry.Initializer.TotalPrimitiveCount;
			Segment.MaxVertices = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices();
			Section->RayTracingGeometry.Initializer.Segments.Add(Segment);

			Section->RayTracingGeometry.UpdateRHI(RHICmdList);
		}
#endif
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		if (GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh)
		{
			return;
		}
#if WITH_EDITOR
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			FLinearColor(0, 0.5f, 1.f)
		);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* ParentMaterialProxy = nullptr;
		if (bWireframe)
		{
			ParentMaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			ParentMaterialProxy = MaterialInstance->GetRenderProxy();
		}
#else
		const bool bWireframe = false;
		FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();
#endif

		if (RenderTarget)
		{
			auto TextureResource = RenderTarget->GetResource();
			if (TextureResource)
			{
				switch (GeometryMode)
				{
				case ELGUIRenderTargetGeometryMode::Plane:
				case ELGUIRenderTargetGeometryMode::Cylinder:
				{
					for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
					{
						FDynamicMeshBuilder MeshBuilder(Views[ViewIndex]->GetFeatureLevel());

						if (VisibilityMap & (1 << ViewIndex))
						{
							const FSceneView* View = Views[ViewIndex];
							// Draw the mesh.
							FMeshBatch& Mesh = Collector.AllocateMesh();
							FMeshBatchElement& BatchElement = Mesh.Elements[0];
							BatchElement.IndexBuffer = &Section->IndexBuffer;
							Mesh.bWireframe = bWireframe;
							Mesh.VertexFactory = &Section->VertexFactory;
							Mesh.MaterialRenderProxy = ParentMaterialProxy;

							bool bHasPrecomputedVolumetricLightmap;
							FMatrix PreviousLocalToWorld;
							int32 SingleCaptureIndex;
							bool bOutputVelocity;
							GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
							bOutputVelocity |= AlwaysHasVelocity();

							FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
							DynamicPrimitiveUniformBuffer.Set(Collector.GetRHICommandList(), GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
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
				}
			}
		}
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		bool bVisible = true;

		FPrimitiveViewRelevance Result;

		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		Result.bDrawRelevance = IsShown(View) && bVisible && View->Family->EngineShowFlags.WidgetComponents;
		Result.bDynamicRelevance = true;
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bEditorPrimitiveRelevance = false;
		Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;

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

	virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override
	{
		bDynamic = false;
		bRelevant = false;
		bLightMapped = false;
		bShadowMapped = false;
	}


#if RHI_RAYTRACING
	virtual bool IsRayTracingRelevant() const override { return true; }

	virtual bool HasRayTracingRepresentation() const override { return true; }

	virtual void GetDynamicRayTracingInstances(FRayTracingMaterialGatheringContext& Context, TArray<FRayTracingInstance>& OutRayTracingInstances) override final
	{
		if (Section != nullptr)
		{
			FMaterialRenderProxy* MaterialProxy = MaterialInstance->GetRenderProxy();

			if (Section->RayTracingGeometry.RayTracingGeometryRHI.IsValid())
			{
				check(Section->RayTracingGeometry.Initializer.IndexBuffer.IsValid());

				FRayTracingInstance RayTracingInstance;
				RayTracingInstance.Geometry = &Section->RayTracingGeometry;
				RayTracingInstance.InstanceTransforms.Add(GetLocalToWorld());

				uint32 SectionIdx = 0;
				FMeshBatch MeshBatch;

				MeshBatch.VertexFactory = &Section->VertexFactory;
				MeshBatch.SegmentIndex = 0;
				MeshBatch.MaterialRenderProxy = MaterialInstance->GetRenderProxy();
				MeshBatch.ReverseCulling = IsLocalToWorldDeterminantNegative();
				MeshBatch.Type = PT_TriangleList;
				MeshBatch.DepthPriorityGroup = SDPG_World;
				MeshBatch.bCanApplyViewModeOverrides = false;
				MeshBatch.CastRayTracedShadow = IsShadowCast(Context.ReferenceView);

				FMeshBatchElement& BatchElement = MeshBatch.Elements[0];
				BatchElement.IndexBuffer = &Section->IndexBuffer;

				bool bHasPrecomputedVolumetricLightmap;
				FMatrix PreviousLocalToWorld;
				int32 SingleCaptureIndex;
				bool bOutputVelocity;
				GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, PreviousLocalToWorld, SingleCaptureIndex, bOutputVelocity);
				bOutputVelocity |= AlwaysHasVelocity();

				FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = Context.RayTracingMeshResourceCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
				DynamicPrimitiveUniformBuffer.Set(Context.RHICmdList, GetLocalToWorld(), PreviousLocalToWorld, GetBounds(), GetLocalBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, bOutputVelocity, GetCustomPrimitiveData());
				BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = Section->IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = Section->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;

				RayTracingInstance.Materials.Add(MeshBatch);
				OutRayTracingInstances.Add(RayTracingInstance);
			}
		}
	}

#endif

private:
	UTextureRenderTarget2D* RenderTarget = nullptr;
	UMaterialInstanceDynamic* MaterialInstance = nullptr;
	ELGUIRenderTargetGeometryMode GeometryMode = ELGUIRenderTargetGeometryMode::Plane;
	FLGUIRenderTargetGeometrySourceMeshProxySection* Section = nullptr;

	FMaterialRelevance MaterialRelevance;
};


#define PARAMETER_NAME_MAINTEXTURE "MainTexture"
#define PARAMETER_NAME_FLIPY "FlipY"


ULGUIRenderTargetGeometrySource::ULGUIRenderTargetGeometrySource()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TargetCanvas = FLGUIComponentReference(ULGUICanvas::StaticClass());
}

void ULGUIRenderTargetGeometrySource::BeginPlay()
{
	Super::BeginPlay();
	BeginCheckRenderTarget();
}
void ULGUIRenderTargetGeometrySource::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	EndCheckRenderTarget();
}

void ULGUIRenderTargetGeometrySource::BeginCheckRenderTarget()
{
	if (CheckRenderTargetTickTweener.IsValid())return;
	CheckRenderTargetTickTweener = ULTweenBPLibrary::UpdateCall(this, [=, WeakThis = TWeakObjectPtr<ULGUIRenderTargetGeometrySource>(this)](float deltaTime) {
		if (WeakThis.IsValid())
		{
			WeakThis->CheckRenderTargetTick();
		}
		});
	if (CheckRenderTargetTickTweener.IsValid())
	{
		CheckRenderTargetTickTweener->SetAffectByGamePause(false)->SetAffectByTimeDilation(false);
	}
}
void ULGUIRenderTargetGeometrySource::EndCheckRenderTarget()
{
	if (!CheckRenderTargetTickTweener.IsValid())return;
	ULTweenBPLibrary::KillIfIsTweening(this, CheckRenderTargetTickTweener.Get());
	CheckRenderTargetTickTweener.Reset();
}
void ULGUIRenderTargetGeometrySource::CheckRenderTargetTick()
{
	if (IsValid(GetRenderTarget()))
	{
		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
		MarkRenderStateDirty(); // New section requires recreating scene proxy

		EndCheckRenderTarget();
	}
}

bool ULGUIRenderTargetGeometrySource::CheckStaticMesh()const
{
	if (StaticMeshComp.IsValid())return true;
	if (GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh)
	{
		if (auto ParentComp = this->GetAttachParent())
		{
			StaticMeshComp = Cast<UStaticMeshComponent>(ParentComp);
			if (StaticMeshComp.IsValid())
			{
				if (MaterialInstance != nullptr)//already called UpdateMaterialInstance, so we need to manually set material to static mesh
				{
					if (bOverrideStaticMeshMaterial)
					{
#if WITH_EDITOR
						if (!this->GetWorld()->IsGameWorld())
						{
							ULGUIPrefabManagerObject::AddOneShotTickFunction([this] {
								StaticMeshComp->SetMaterial(0, MaterialInstance);
								}, 1);
						}
						else
#endif
						//delay call, or the bPostTickComponentUpdate check will break
						ULTweenBPLibrary::DelayFrameCall(this->GetWorld(), 1, [this] {
							StaticMeshComp->SetMaterial(0, MaterialInstance);
							})->SetAffectByGamePause(false);
					}
				}
				return true;
			}
			else
			{
				auto ErrorMsg = LOCTEXT("StaticMeshComponentNotValid", "[ULGUIRenderTargetGeometrySource::BeginPlay]StaticMesh component not valid!");
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg.ToString());
#if WITH_EDITOR
				LGUIUtils::EditorNotification(ErrorMsg);
#endif
			}
		}
	}
	return false;
}

FPrimitiveSceneProxy* ULGUIRenderTargetGeometrySource::CreateSceneProxy()
{
	if (GetCanvas())
	{
		UpdateMaterialInstance();
		if (MaterialInstance != nullptr)
		{
			if (Vertices.Num() > 0 && Triangles.Num() > 0)
			{
#if WITH_EDITOR
				bIsValidSceneProxy = true;
#endif
				return new FLGUIRenderTargetGeometrySource_SceneProxy(this);
			}
		}
	}

#if WITH_EDITOR
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		// make something so we can see this component in the editor
		class FWidgetBoxProxy final : public FPrimitiveSceneProxy
		{
		public:
			SIZE_T GetTypeHash() const override
			{
				static size_t UniquePointer;
				return reinterpret_cast<size_t>(&UniquePointer);
			}

			FWidgetBoxProxy(const ULGUIRenderTargetGeometrySource* InComponent)
				: FPrimitiveSceneProxy(InComponent)
				, BoxExtents(1.f, InComponent->GetRenderTargetSize().X / 2.0f, InComponent->GetRenderTargetSize().Y / 2.0f)
			{
				bWillEverBeLit = false;
			}

			virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
			{
				QUICK_SCOPE_CYCLE_COUNTER(STAT_BoxSceneProxy_GetDynamicMeshElements);

				const FMatrix& LocalToWorld = GetLocalToWorld();

				for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
				{
					if (VisibilityMap & (1 << ViewIndex))
					{
						const FSceneView* View = Views[ViewIndex];

						const FLinearColor DrawColor = GetViewSelectionColor(FColor::White, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

						FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
						DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, DrawColor, SDPG_World);
					}
				}
			}

			virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
			{
				FPrimitiveViewRelevance Result;
				if (!View->bIsGameView)
				{
					// Should we draw this because collision drawing is enabled, and we have collision
					const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();
					Result.bDrawRelevance = IsShown(View) || bShowForCollision;
					Result.bDynamicRelevance = true;
					Result.bShadowRelevance = IsShadowCast(View);
					Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
				}
				return Result;
			}
			virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
			uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

		private:
			const FVector	BoxExtents;
		};

		bIsValidSceneProxy = false;
		return new FWidgetBoxProxy(this);
	}
#endif
	return nullptr;
}
FBoxSphereBounds ULGUIRenderTargetGeometrySource::CalcBounds(const FTransform& LocalToWorld) const
{
	auto RenderTargetSize = GetRenderTargetSize();
	const float Width = ComputeComponentWidth();
		const float Height = ComputeComponentHeight();
		const float Thickness = ComputeComponentThickness();
		const FVector Origin = FVector(
			Thickness * 0.5f,
			Width * (0.5f - Pivot.X),
			Height * (0.5f - Pivot.Y));
	auto BoxExtent = FVector(Thickness * 0.5f, Width * 0.5f, Height * 0.5f);

	FBoxSphereBounds NewBounds(Origin, BoxExtent, RenderTargetSize.Size() / 2.0f);
	NewBounds = NewBounds.TransformBy(LocalToWorld);

	NewBounds.BoxExtent *= BoundsScale;
	NewBounds.SphereRadius *= BoundsScale;

	return NewBounds;
}
UBodySetup* ULGUIRenderTargetGeometrySource::GetBodySetup()
{
	UpdateBodySetup(false);
	return BodySetup;
}
FCollisionShape ULGUIRenderTargetGeometrySource::GetCollisionShape(float Inflation) const
{
	auto RenderTargetSize = GetRenderTargetSize();

	FVector BoxHalfExtent = (FVector(0.01f, RenderTargetSize.X * 0.5f, RenderTargetSize.Y * 0.5f) * GetComponentTransform().GetScale3D()) + Inflation;

	if (Inflation < 0.0f)
	{
		// Don't shrink below zero size.
		BoxHalfExtent = BoxHalfExtent.ComponentMax(FVector::ZeroVector);
	}

	return FCollisionShape::MakeBox(BoxHalfExtent);
}
void ULGUIRenderTargetGeometrySource::OnRegister()
{
	Super::OnRegister();
	if (this->GetWorld() != nullptr)
	{
		if (GetMaterial(0) == nullptr)
		{
			if (auto PresetMaterial = GetPresetMaterial())
			{
				SetMaterial(0, PresetMaterial);
				this->MarkPackageDirty();
			}
		}
#if WITH_EDITOR
		if (!this->GetWorld()->IsGameWorld())//only do it in Editor world, because Game world can do it by BeginCheckRenderTarget
		{
			UpdateMeshData();
		}
#endif
	}
}
void ULGUIRenderTargetGeometrySource::OnUnregister()
{
	Super::OnUnregister();
}
void ULGUIRenderTargetGeometrySource::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
}
UMaterialInterface* ULGUIRenderTargetGeometrySource::GetMaterial(int32 MaterialIndex) const
{
	return Super::GetMaterial(MaterialIndex);
}
void ULGUIRenderTargetGeometrySource::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	Super::SetMaterial(ElementIndex, Material);
	MaterialInstance = nullptr;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
	}
	else
#endif
		UpdateMaterialInstance();
}

void ULGUIRenderTargetGeometrySource::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	if (MaterialInstance)
	{
		OutMaterials.AddUnique(MaterialInstance);
	}
}

int32 ULGUIRenderTargetGeometrySource::GetNumMaterials() const
{
	return FMath::Max<int32>(OverrideMaterials.Num(), 1);
}

bool ULGUIRenderTargetGeometrySource::GetTriMeshSizeEstimates(struct FTriMeshCollisionDataEstimates& OutTriMeshEstimates, bool bInUseAllTriData) const
{
	if (!GetRenderTarget())return false;
	if (GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh)return true;
	if (Vertices.Num() == 0 || Triangles.Num() == 0)return true;
	OutTriMeshEstimates.VerticeCount = Vertices.Num();
	return true;
}
bool ULGUIRenderTargetGeometrySource::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	auto RenderTarget = GetRenderTarget();
	if (!RenderTarget)return false;

	switch (GeometryMode)
	{
	case ELGUIRenderTargetGeometryMode::Plane:
	case ELGUIRenderTargetGeometryMode::Cylinder:
	{
		// See if we should copy UVs
		bool bCopyUVs = UPhysicsSettings::Get()->bSupportUVFromHitResults;
		if (bCopyUVs)
		{
			CollisionData->UVs.AddZeroed(1); // only one UV channel
		}

		CollisionData->Vertices.Reserve(Vertices.Num());
		if (bCopyUVs)
		{
			CollisionData->UVs[0].Reserve(Vertices.Num());
		}
		for (int i = 0; i < Vertices.Num(); i++)
		{
			auto& Vert = Vertices[i];
			CollisionData->Vertices.Add(Vert.Position);
			if (bCopyUVs)
			{
				CollisionData->UVs[0].Add(FVector2D(Vert.TextureCoordinate[0]));
			}
		}
		CollisionData->Indices.Reserve(Triangles.Num());
		for (int i = 0; i < Triangles.Num(); i+=3)
		{
			FTriIndices Triangle;

			Triangle.v0 = Triangles[i];
			Triangle.v1 = Triangles[i + 1];
			Triangle.v2 = Triangles[i + 2];
			CollisionData->Indices.Add(Triangle);
		}

		CollisionData->bFlipNormals = true;
		CollisionData->bDeformableMesh = true;
		CollisionData->bFastCook = true;
		return true;
	}
	break;
	case ELGUIRenderTargetGeometryMode::StaticMesh:
	{
		return false;
	}
	break;
	}
	return false;
}
bool ULGUIRenderTargetGeometrySource::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	if (!GetRenderTarget())return false;
	if (GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh)return false;
	if (Vertices.Num() == 0 || Triangles.Num() == 0)return false;
	return true;
}
bool ULGUIRenderTargetGeometrySource::WantsNegXTriMesh() 
{
	return false; 
}

void ULGUIRenderTargetGeometrySource::UpdateCollision()
{
	UpdateBodySetup(true);
	RecreatePhysicsState();
}
void ULGUIRenderTargetGeometrySource::UpdateMeshData()
{
	auto PrevNumVerts = Vertices.Num();
	auto PrevNumIndex = Triangles.Num();
	Vertices.Reset();
	Triangles.Reset();

	auto RenderTarget = GetRenderTarget();
	if (!RenderTarget)
	{
		if (PrevNumVerts != 0 || PrevNumIndex != 0)
		{
			MarkRenderStateDirty();
		}
		return;
	}
	
	switch (GeometryMode)
	{
	case ELGUIRenderTargetGeometryMode::Plane:
	{
		float U = -RenderTarget->SizeX * Pivot.X;
		float V = -RenderTarget->SizeY * Pivot.Y;
		float UL = RenderTarget->SizeX * (1.0f - Pivot.X);
		float VL = RenderTarget->SizeY * (1.0f - Pivot.Y);

		Vertices.Reserve(4);
		Triangles.Reserve(6);

		auto Vert = FDynamicMeshVertex();
		Vert.TangentX = FVector(0, -1, 0);
		Vert.TangentZ = FVector(1, 0, 0);
		Vert.Color = FColor::White;

		Vert.Position = FVector3f(0, U, V);
		Vert.TextureCoordinate[0] = FVector2f(0, 1);
		Vertices.Add(Vert);
		Vert.Position = FVector3f(0, UL, V);
		Vert.TextureCoordinate[0] = FVector2f(1, 1);
		Vertices.Add(Vert);
		Vert.Position = FVector3f(0, U, VL);
		Vert.TextureCoordinate[0] = FVector2f(0, 0);
		Vertices.Add(Vert);
		Vert.Position = FVector3f(0, UL, VL);
		Vert.TextureCoordinate[0] = FVector2f(1, 0);
		Vertices.Add(Vert);

		Triangles.Add(0);
		Triangles.Add(3);
		Triangles.Add(2);
		Triangles.Add(0);
		Triangles.Add(1);
		Triangles.Add(3);
	}
	break;
	case ELGUIRenderTargetGeometryMode::Cylinder:
	{
		auto ArcAngle = FMath::Max(FMath::DegreesToRadians(FMath::Abs(GetCylinderArcAngle())), 0.01f);
		auto ArcAngleSign = FMath::Sign(GetCylinderArcAngle());

		const int32 NumSegments = FMath::Lerp(MIN_SEG, MAX_SEG, ArcAngle / PI);

		const float Radius = RenderTarget->SizeX / ArcAngle;
		const float Apothem = Radius * FMath::Cos(0.5f * ArcAngle);
		const float ChordLength = 2.0f * Radius * FMath::Sin(0.5f * ArcAngle);
		const float HalfChordLength = ChordLength * 0.5f;

		const float PivotOffsetX = ChordLength * (0.5 - Pivot.X);
		const float V = -RenderTarget->SizeY * Pivot.Y;
		const float VL = RenderTarget->SizeY * (1.0f - Pivot.Y);

		//@todo: normal and tangent calculate wrong
		Vertices.Reserve(2 + NumSegments * 2);
		Triangles.Reserve(6 * NumSegments);
		const float RadiansPerStep = ArcAngle / NumSegments;
		float Angle = -ArcAngle * 0.5f;
		const FVector3f CenterPoint = FVector3f(0, HalfChordLength + PivotOffsetX, V);
		auto Vert = FDynamicMeshVertex();
		Vert.Color = FColor::White;
		Vert.Position = FVector3f(0, Radius * FMath::Sin(Angle) + PivotOffsetX, V);
		auto TangentX2D = FVector2f(CenterPoint) - FVector2f(Vert.Position);
		TangentX2D.Normalize();
		auto TangentZ = FVector3f(TangentX2D, 0);
		auto TangentY = FVector3f(0, 0, 1);
		auto TangentX = FVector3f::CrossProduct(TangentY, TangentZ);
		Vert.SetTangents(TangentX, TangentY, TangentZ);
		Vert.TextureCoordinate[0] = FVector2f(0, 1);
		Vertices.Add(Vert);
		Vert.Position.Z = VL;
		Vert.TextureCoordinate[0] = FVector2f(0, 0);
		Vertices.Add(Vert);

		float UVInterval = 1.0f / NumSegments;
		float UVX = 0;
		int32 TriangleIndex = 0;
		for (int32 Segment = 0; Segment < NumSegments; Segment++)
		{
			Angle += RadiansPerStep;
			UVX += UVInterval;

			Vert.Position = FVector3f(ArcAngleSign * (Radius * FMath::Cos(Angle) - Apothem), Radius * FMath::Sin(Angle) + PivotOffsetX, V);
			TangentX2D = FVector2f(Vert.Position) - FVector2f(CenterPoint);
			TangentX2D.Normalize();
			TangentZ = FVector3f(TangentX2D, 0);
			TangentX = FVector3f::CrossProduct(TangentY, TangentZ);
			Vert.SetTangents(TangentX, TangentY, TangentZ);
			Vert.TextureCoordinate[0] = FVector2f(UVX, 1);
			Vertices.Add(Vert);
			Vert.Position.Z = VL;
			Vert.TextureCoordinate[0] = FVector2f(UVX, 0);
			Vertices.Add(Vert);

			Triangles.Add(TriangleIndex);
			Triangles.Add(TriangleIndex + 3);
			Triangles.Add(TriangleIndex + 1);
			Triangles.Add(TriangleIndex);
			Triangles.Add(TriangleIndex + 2);
			Triangles.Add(TriangleIndex + 3);
			TriangleIndex += 2;
		}
	}
	break;
	}

	if (PrevNumVerts == Vertices.Num() && PrevNumIndex == Triangles.Num())//if buffer size not change then we can do update
	{
		if (SceneProxy != nullptr
#if WITH_EDITOR
			&& bIsValidSceneProxy
#endif
			)
		{
			struct FUpdateMeshDataStruct
			{
				TArray<FDynamicMeshVertex> VertexData;
				TArray<uint16> IndexData;
				FLGUIRenderTargetGeometrySource_SceneProxy* Proxy = nullptr;
			};
			auto UpdateData = new FUpdateMeshDataStruct();
			UpdateData->VertexData = Vertices;
			UpdateData->IndexData = Triangles;
			UpdateData->Proxy = (FLGUIRenderTargetGeometrySource_SceneProxy*)SceneProxy;
			ENQUEUE_RENDER_COMMAND(FLGUIRenderTargetGeometrySourceMeshSectionUpdate)
				([UpdateData](FRHICommandListImmediate& RHICmdList) {
				UpdateData->Proxy->UpdateSection_RenderThread(
					RHICmdList
					, UpdateData->VertexData.GetData()
					, UpdateData->VertexData.Num()
					, UpdateData->IndexData.GetData()
					, UpdateData->IndexData.Num()
				);
				delete UpdateData;
					});
		}
	}
	else
	{
		MarkRenderStateDirty();
	}
}

ULGUICanvas* ULGUIRenderTargetGeometrySource::GetTargetCanvas_Implementation()const
{
	return GetCanvas();
}
bool ULGUIRenderTargetGeometrySource::PerformLineTrace_Implementation(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)
{
	return LineTraceHitUV(InHitFaceIndex, InHitPoint, InLineStart, InLineEnd, OutHitUV);
}

#if WITH_EDITOR
bool ULGUIRenderTargetGeometrySource::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUIRenderTargetGeometrySource, CylinderArcAngle))
		{
			return GeometryMode == ELGUIRenderTargetGeometryMode::Cylinder;
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUIRenderTargetGeometrySource, bOverrideStaticMeshMaterial))
		{
			return GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh;
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUIRenderTargetGeometrySource, bEnableInteractOnBackside))
		{
			return GeometryMode != ELGUIRenderTargetGeometryMode::StaticMesh;
		}
	}

	return Super::CanEditChange(InProperty);
}
void ULGUIRenderTargetGeometrySource::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetName();
		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUIRenderTargetGeometrySource, CylinderArcAngle))
		{
			CylinderArcAngle = FMath::Sign(CylinderArcAngle) * FMath::Clamp(FMath::Abs(CylinderArcAngle), 1.0f, 180.0f);
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUIRenderTargetGeometrySource, TargetCanvas))
		{
			if (!TargetCanvas.IsValidComponentReference())
			{
				TargetCanvasObject = nullptr;
			}
		}

		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
		MarkRenderStateDirty(); // New section requires recreating scene proxy
	}
}
#endif

ULGUICanvas* ULGUIRenderTargetGeometrySource::GetCanvas()const
{
	if (TargetCanvasObject.IsValid())
	{
		return TargetCanvasObject.Get();
	}
	if (!TargetCanvas.IsValidComponentReference())
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUIRenderTargetGeometrySource::GetCanvas]TargetCanvas not valid!"));
		return nullptr;
	}
	auto Canvas = TargetCanvas.GetComponent<ULGUICanvas>();
	if (Canvas == nullptr)
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUIRenderTargetGeometrySource::GetCanvas]TargetCanvas not valid!"));
		return nullptr;
	}
	if (!Canvas->IsRootCanvas())
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUIRenderTargetGeometrySource::GetCanvas]TargetCanvas must be a root canvas!"));
		return nullptr;
	}
	if (Canvas->GetRenderMode() != ELGUIRenderMode::RenderTarget || !IsValid(Canvas->GetRenderTarget()))
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUIRenderTargetGeometrySource::GetCanvas]TargetCanvas's render mode must be RenderTarget!"));
		return nullptr;
	}
	TargetCanvasObject = Canvas;
	return Canvas;
}

void ULGUIRenderTargetGeometrySource::SetCanvas(ULGUICanvas* Value)
{
	if (TargetCanvasObject.Get() != Value)
	{
		TargetCanvasObject = Value;
		BeginCheckRenderTarget();

		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
	}
}

void ULGUIRenderTargetGeometrySource::SetGeometryMode(ELGUIRenderTargetGeometryMode Value)
{
	if (GeometryMode != Value)
	{
		GeometryMode = Value;
		
		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
	}
}
void ULGUIRenderTargetGeometrySource::SetPivot(const FVector2D Value)
{
	if (Pivot != Value)
	{
		Pivot = Value;
		
		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
	}
}
void ULGUIRenderTargetGeometrySource::SetCylinderArcAngle(float Value)
{
	if (CylinderArcAngle != Value)
	{
		CylinderArcAngle = Value;
		CylinderArcAngle = FMath::Sign(CylinderArcAngle) * FMath::Clamp(FMath::Abs(CylinderArcAngle), 1.0f, 180.0f);
		
		UpdateMeshData();
		UpdateLocalBounds(); // Update overall bounds
		UpdateCollision(); // Mark collision as dirty
	}
}

void ULGUIRenderTargetGeometrySource::SetEnableInteractOnBackside(bool Value)
{
	if (bEnableInteractOnBackside != Value)
	{
		bEnableInteractOnBackside = Value;
	}
}
void ULGUIRenderTargetGeometrySource::SetFlipVerticalOnGLES(bool Value)
{
	if (bFlipVerticalOnGLES != Value)
	{
		bFlipVerticalOnGLES = Value;
#if PLATFORM_ANDROID && 0//UE5.1 don't need this
		if (MaterialInstance != nullptr)
		{
			auto ShaderPlatform = GShaderPlatformForFeatureLevel[GetWorld()->FeatureLevel];
			if (ShaderPlatform == EShaderPlatform::SP_OPENGL_ES3_1_ANDROID)
			{
				MaterialInstance->SetScalarParameterValue(PARAMETER_NAME_FLIPY, bFlipVerticalOnGLES ? 1.0f : 0.0f);
			}
		}
#endif
	}
}

FIntPoint ULGUIRenderTargetGeometrySource::GetRenderTargetSize()const
{
	if (auto RenderTarget = GetRenderTarget())
	{
		return FIntPoint(RenderTarget->GetSurfaceWidth(), RenderTarget->GetSurfaceHeight());
	}
	return FIntPoint(2, 2);
}

void ULGUIRenderTargetGeometrySource::UpdateLocalBounds()
{
	// Update global bounds
	UpdateBounds();
	// Need to send to render thread
	MarkRenderTransformDirty();
}
void ULGUIRenderTargetGeometrySource::UpdateBodySetup(bool bIsDirty)
{
	if (!GetRenderTarget())return;
	if (!BodySetup || bIsDirty)
	{
		BodySetup = NewObject<UBodySetup>(this);
		BodySetup->CollisionTraceFlag = CTF_UseDefault;
		BodySetup->AggGeom.BoxElems.Add(FKBoxElem());
		BodySetup->bHasCookedCollisionData = false;

		FKBoxElem* BoxElem = BodySetup->AggGeom.BoxElems.GetData();

		const float Width = ComputeComponentWidth();
		const float Height = ComputeComponentHeight();
		const float Thickness = ComputeComponentThickness();
		const FVector Origin = FVector(.5f,
			Width * (0.5f - Pivot.X),
			Height * (0.5f - Pivot.Y));

		BoxElem->X = Thickness;
		BoxElem->Y = Width;
		BoxElem->Z = Height;

		BoxElem->SetTransform(FTransform::Identity);
		BoxElem->Center = Origin;
	}
}

void ULGUIRenderTargetGeometrySource::UpdateMaterialInstance()
{
	if (MaterialInstance == nullptr)
	{
		auto SourceMat = GetMaterial(0);
		if (SourceMat == nullptr)
		{
			SourceMat = GetPresetMaterial();
		}
		if (SourceMat)
		{
			MaterialInstance = UMaterialInstanceDynamic::Create(SourceMat, this);
			if (GeometryMode == ELGUIRenderTargetGeometryMode::StaticMesh && bOverrideStaticMeshMaterial)
			{
				if (CheckStaticMesh())
				{
#if WITH_EDITOR
					if (!this->GetWorld()->IsGameWorld())
					{
						ULGUIPrefabManagerObject::AddOneShotTickFunction([this] {
							StaticMeshComp->SetMaterial(0, MaterialInstance);
							}, 1);
					}
					else
#endif
					//delay call, or the bPostTickComponentUpdate check will break
					ULTweenBPLibrary::DelayFrameCall(this, 1, [this] {
						StaticMeshComp->SetMaterial(0, MaterialInstance);
						})->SetAffectByGamePause(false);
				}
			}
		}
	}
	UpdateMaterialInstanceParameters();
}

void ULGUIRenderTargetGeometrySource::UpdateMaterialInstanceParameters()
{
	if (MaterialInstance)
	{
		MaterialInstance->SetTextureParameterValue(PARAMETER_NAME_MAINTEXTURE, GetRenderTarget());
#if PLATFORM_ANDROID && 0//UE5.1 don't need this
		auto ShaderPlatform = GShaderPlatformForFeatureLevel[GetWorld()->FeatureLevel];
		if (ShaderPlatform == EShaderPlatform::SP_OPENGL_ES3_1_ANDROID)
		{
			MaterialInstance->SetScalarParameterValue(PARAMETER_NAME_FLIPY, bFlipVerticalOnGLES ? 1.0f : 0.0f);
		}
#endif
	}
}
UMaterialInterface* ULGUIRenderTargetGeometrySource::GetPresetMaterial()const
{
	auto MatPath = TEXT("/LGUI/Materials/LGUI_RenderTargetMaterial");
	return LoadObject<UMaterialInterface>(NULL, MatPath);
}

UMaterialInstanceDynamic* ULGUIRenderTargetGeometrySource::GetMaterialInstance()const
{
	return MaterialInstance;
}

UTextureRenderTarget2D* ULGUIRenderTargetGeometrySource::GetRenderTarget()const
{
	if (auto Canvas = GetCanvas())
	{
		return Canvas->GetRenderTarget();
	}
	return nullptr;
}

float ULGUIRenderTargetGeometrySource::ComputeComponentWidth() const
{
	auto RenderTargetSize = GetRenderTargetSize();
	switch (GeometryMode)
	{
	default:
		return 0.0f;
		break;
	case ELGUIRenderTargetGeometryMode::Plane:
		return RenderTargetSize.X;
		break;

	case ELGUIRenderTargetGeometryMode::Cylinder:
		const float ArcAngleRadians = FMath::DegreesToRadians(CylinderArcAngle);
		const float Radius = RenderTargetSize.X / ArcAngleRadians;
		return 2.0f * Radius * FMath::Sin(0.5f * ArcAngleRadians);
		break;
	}
}

float ULGUIRenderTargetGeometrySource::ComputeComponentHeight() const
{
	auto RenderTargetSize = GetRenderTargetSize();
	switch (GeometryMode)
	{
	default:
		return 0.0f;
		break;
	case ELGUIRenderTargetGeometryMode::Plane:
	case ELGUIRenderTargetGeometryMode::Cylinder:
		return RenderTargetSize.Y;
		break;
	}
}

float ULGUIRenderTargetGeometrySource::ComputeComponentThickness() const
{
	auto RenderTargetSize = GetRenderTargetSize();
	switch (GeometryMode)
	{
	default:
		return 0.0f;
		break;
	case ELGUIRenderTargetGeometryMode::Plane:
		return 0.00f;
		break;

	case ELGUIRenderTargetGeometryMode::Cylinder:
		const float ArcAngleRadians = FMath::DegreesToRadians(CylinderArcAngle);
		const float Radius = RenderTargetSize.X / ArcAngleRadians;
		return Radius * (1.0f - FMath::Cos(0.5f * ArcAngleRadians));
		break;
	}
}

#include "Kismet/GameplayStatics.h"
bool ULGUIRenderTargetGeometrySource::LineTraceHitUV(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)const
{
	switch (GeometryMode)
	{
	default:
	case ELGUIRenderTargetGeometryMode::Plane:
	{
		auto InverseTf = GetComponentTransform().Inverse();
		auto LocalHitPoint = InverseTf.TransformPosition(InHitPoint);

		auto RenderTargetSize = this->GetRenderTargetSize();
		OutHitUV.X = LocalHitPoint.Y / RenderTargetSize.X;
		OutHitUV.Y = LocalHitPoint.Z / RenderTargetSize.Y;
		OutHitUV += Pivot;

		return true;
	}
	break;
	case ELGUIRenderTargetGeometryMode::Cylinder:
	{
		if (InHitFaceIndex >= 0)
		{
			auto InverseTf = GetComponentTransform().Inverse();
			auto LocalHitPoint = InverseTf.TransformPosition(InHitPoint);
			if (Triangles.IsValidIndex(InHitFaceIndex * 3 + 2))
			{
				int32 Index0 = Triangles[InHitFaceIndex * 3 + 0];
				int32 Index1 = Triangles[InHitFaceIndex * 3 + 1];
				int32 Index2 = Triangles[InHitFaceIndex * 3 + 2];

				auto Pos0 = (FVector)Vertices[Index0].Position;
				auto Pos1 = (FVector)Vertices[Index1].Position;
				auto Pos2 = (FVector)Vertices[Index2].Position;

				auto UV0 = (FVector2D)Vertices[Index0].TextureCoordinate[0];
				auto UV1 = (FVector2D)Vertices[Index1].TextureCoordinate[0];
				auto UV2 = (FVector2D)Vertices[Index2].TextureCoordinate[0];

				// Transform hit location from world to local space.
				// Find barycentric coords
				auto BaryCoords = FMath::ComputeBaryCentric2D(LocalHitPoint, Pos0, Pos1, Pos2);
				// Use to blend UVs
				OutHitUV = (BaryCoords.X * UV0) + (BaryCoords.Y * UV1) + (BaryCoords.Z * UV2);
				OutHitUV.Y = 1.0f - OutHitUV.Y;
				return true;
			}
		}
		else//don't have valid faceIndex, then do traditional hit test
		{
			auto InverseTf = GetComponentTransform().Inverse();
			auto LocalSpaceRayOrigin = InverseTf.TransformPosition(InLineStart);
			auto LocalSpaceRayEnd = InverseTf.TransformPosition(InLineEnd);

			auto RenderTargetSize = this->GetRenderTargetSize();
			auto ArcAngle = FMath::DegreesToRadians(FMath::Abs(GetCylinderArcAngle()));

			const int32 NumSegments = FMath::Lerp(MIN_SEG, MAX_SEG, ArcAngle / PI);

			auto Vert0 = Vertices[0];
			auto Vert1 = Vertices[1];
			auto Position0 = (FVector)Vert0.Position;
			auto Position1 = (FVector)Vert1.Position;

			float UVInterval = 1.0f / NumSegments;
			int32 TriangleIndex = 0;

			struct FHitResultContainer
			{
				FVector2D UV;
				FVector HitPoint;
				float DistSquare;
				FMatrix RectMatrix;
			};
			TArray<FHitResultContainer> MultiHitResult;
			MultiHitResult.Reset();
			for (int32 Segment = 0; Segment < NumSegments; Segment++)
			{
				auto Vert2 = Vertices[(Segment + 1) * 2];
				auto Vert3 = Vertices[(Segment + 1) * 2 + 1];
				auto Position2 = (FVector)Vert2.Position;
				auto Position3 = (FVector)Vert3.Position;

				auto Y = Position2 - Position0;
				Y.Normalize();
				auto Z = FVector(0, 0, 1);
				auto X = FVector::CrossProduct(Y, Z);
				X.Normalize();

				auto LocalRectMatrix = FMatrix(X, Y, Z, Position0);
				auto ToLocalRectMatrix = LocalRectMatrix.Inverse();

				auto RectSpaceRayOrigin = (FVector)ToLocalRectMatrix.TransformPosition(LocalSpaceRayOrigin);
				auto RectSpaceRayEnd = (FVector)ToLocalRectMatrix.TransformPosition(LocalSpaceRayEnd);
				//start and end point must be different side of X plane
				if (FMath::Sign(RectSpaceRayOrigin.X) != FMath::Sign(RectSpaceRayEnd.X))
				{
					if (RectSpaceRayOrigin.X > 0 && !bEnableInteractOnBackside)//ray origin is on backside but backside can't interact
					{
						continue;
					}
					auto HitPoint = FMath::LinePlaneIntersection(RectSpaceRayOrigin, RectSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
					//hit point inside rect area
					float Left = 0;
					float Right = (Position2 - Position0).Size();
					float Bottom = 0;
					float Top = RenderTargetSize.Y;
					if (HitPoint.Y > Left && HitPoint.Y < Right && HitPoint.Z > Bottom && HitPoint.Z < Top)
					{
						FHitResultContainer HitResult;
						HitResult.UV.X = Segment * UVInterval + UVInterval * HitPoint.Y / Right;
						HitResult.UV.Y = HitPoint.Z / Top;
						HitResult.DistSquare = FVector::DistSquared(LocalSpaceRayOrigin, HitPoint);
						HitResult.HitPoint = HitPoint;
						HitResult.RectMatrix = LocalRectMatrix;

						MultiHitResult.Add(HitResult);
					}
				}

				Position0 = Position2;
				Position1 = Position3;
			}
			if (MultiHitResult.Num() > 0)
			{
				MultiHitResult.Sort([](const FHitResultContainer& A, const FHitResultContainer& B) {
					return A.DistSquare < B.DistSquare;
					});
				auto& HitResult = MultiHitResult[0];

				OutHitUV = HitResult.UV;

				return true;
			}
		}
	}
	break;
	case ELGUIRenderTargetGeometryMode::StaticMesh:
	{
		if (CheckStaticMesh())
		{
			FHitResult HitResult;
			if (InHitFaceIndex >= 0)
			{
				HitResult.Component = StaticMeshComp;
				HitResult.Location = InHitPoint;
				HitResult.FaceIndex = InHitFaceIndex;
			}
			else
			{
				FCollisionQueryParams QueryParams;
				QueryParams.bTraceComplex = true;
				QueryParams.bReturnFaceIndex = true;
				StaticMeshComp->LineTraceComponent(HitResult, InLineStart, InLineEnd, QueryParams);
			}
			if (UGameplayStatics::FindCollisionUV(HitResult, 0, OutHitUV))
			{
				OutHitUV.Y = 1.0f - OutHitUV.Y;
				return true;
			}
		}
	}
	break;
	}
	return false;
}

#undef LOCTEXT_NAMESPACE
