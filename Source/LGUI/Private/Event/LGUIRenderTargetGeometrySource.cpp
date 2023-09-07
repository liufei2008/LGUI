// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LGUIRenderTargetGeometrySource.h"
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
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "LTweenBPLibrary.h"

#define LOCTEXT_NAMESPACE "LGUIRenderTargetGeometrySource"

#define MIN_SEG 4
#define MAX_SEG 32

class FLGUIRenderTargetGeometrySource_Sceneproxy : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}
	FLGUIRenderTargetGeometrySource_Sceneproxy(ULGUIRenderTargetGeometrySource* InComponent)
		: FPrimitiveSceneProxy(InComponent)
		, RenderTarget(InComponent->GetRenderTarget())
		, MaterialInstance(InComponent->GetMaterialInstance())
		, GeometryMode(InComponent->GetGeometryMode())
		, Vertices(InComponent->GetMeshVertices())
		, Triangles(InComponent->GetMeshIndices())
	{
		MaterialRelevance = MaterialInstance->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
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
		FMaterialRenderProxy* ParentMaterialProxy = MaterialInstance->GetRenderProxy();
#endif

		const FMatrix& ViewportLocalToWorld = GetLocalToWorld();

		FMatrix PreviousLocalToWorld;

		if (!GetScene().GetPreviousLocalToWorld(GetPrimitiveSceneInfo(), PreviousLocalToWorld))
		{
			PreviousLocalToWorld = GetLocalToWorld();
		}

		if (RenderTarget)
		{
			auto TextureResource = RenderTarget->Resource;
			if (TextureResource)
			{
				const EShaderPlatform ShaderPlatform = GShaderPlatformForFeatureLevel[ViewFamily.GetFeatureLevel()];
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
							MeshBuilder.AddVertices(Vertices);
							MeshBuilder.AddTriangles(Triangles);

							FDynamicMeshBuilderSettings Settings;
							Settings.bDisableBackfaceCulling = false;
							Settings.bReceivesDecals = true;
							Settings.bUseSelectionOutline = true;
							MeshBuilder.GetMesh(ViewportLocalToWorld, PreviousLocalToWorld, ParentMaterialProxy, SDPG_World, Settings, nullptr, ViewIndex, Collector, FHitProxyId());
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

	virtual void GetLightRelevance(const FLightSceneProxy* LightSceneProxy, bool& bDynamic, bool& bRelevant, bool& bLightMapped, bool& bShadowMapped) const override
	{
		bDynamic = false;
		bRelevant = false;
		bLightMapped = false;
		bShadowMapped = false;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

private:
	UTextureRenderTarget2D* RenderTarget = nullptr;
	UMaterialInstanceDynamic* MaterialInstance = nullptr;
	ELGUIRenderTargetGeometryMode GeometryMode = ELGUIRenderTargetGeometryMode::Plane;
	TArray<FDynamicMeshVertex> Vertices;
	TArray<uint32> Triangles;

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
	if (CheckRenderTargetTickDelegate.IsValid())return;
	CheckRenderTargetTickDelegate = ULTweenBPLibrary::RegisterUpdateEvent(this, [=, WeakThis = TWeakObjectPtr<ULGUIRenderTargetGeometrySource>(this)](float deltaTime) {
		if (WeakThis.IsValid())
		{
			WeakThis->CheckRenderTargetTick();
		}
		});
}
void ULGUIRenderTargetGeometrySource::EndCheckRenderTarget()
{
	if (!CheckRenderTargetTickDelegate.IsValid())return;
	ULTweenBPLibrary::UnregisterUpdateEvent(this, CheckRenderTargetTickDelegate);
	CheckRenderTargetTickDelegate.Reset();
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
						//delay call, or the bPostTickComponentUpdate check will break
						ULTweenBPLibrary::DelayFrameCall(this->GetWorld(), 1, [=] {
							StaticMeshComp->SetMaterial(0, MaterialInstance);
							});
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
			return new FLGUIRenderTargetGeometrySource_Sceneproxy(this);
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
				CollisionData->UVs[0].Add(Vert.TextureCoordinate[0]);
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
	auto RenderTarget = GetRenderTarget();
	if (!RenderTarget)return;
	Vertices.Reset();
	Triangles.Reset();
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

		Vert.Position = FVector(0, U, V);
		Vert.TextureCoordinate[0] = FVector2D(0, 1);
		Vertices.Add(Vert);
		Vert.Position = FVector(0, UL, V);
		Vert.TextureCoordinate[0] = FVector2D(1, 1);
		Vertices.Add(Vert);
		Vert.Position = FVector(0, U, VL);
		Vert.TextureCoordinate[0] = FVector2D(0, 0);
		Vertices.Add(Vert);
		Vert.Position = FVector(0, UL, VL);
		Vert.TextureCoordinate[0] = FVector2D(1, 0);
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
		const FVector CenterPoint = FVector(0, HalfChordLength + PivotOffsetX, V);
		auto Vert = FDynamicMeshVertex();
		Vert.Color = FColor::White;
		Vert.Position = FVector(0, Radius * FMath::Sin(Angle) + PivotOffsetX, V);
		auto TangentX2D = FVector2D(CenterPoint) - FVector2D(Vert.Position);
		TangentX2D.Normalize();
		auto TangentZ = FVector(TangentX2D, 0);
		auto TangentY = FVector(0, 0, 1);
		auto TangentX = FVector::CrossProduct(TangentY, TangentZ);
		Vert.SetTangents(TangentX, TangentY, TangentZ);
		Vert.TextureCoordinate[0] = FVector2D(0, 1);
		Vertices.Add(Vert);
		Vert.Position.Z = VL;
		Vert.TextureCoordinate[0] = FVector2D(0, 0);
		Vertices.Add(Vert);

		float UVInterval = 1.0f / NumSegments;
		float UVX = 0;
		int32 TriangleIndex = 0;
		for (int32 Segment = 0; Segment < NumSegments; Segment++)
		{
			Angle += RadiansPerStep;
			UVX += UVInterval;

			Vert.Position = FVector(ArcAngleSign * (Radius * FMath::Cos(Angle) - Apothem), Radius * FMath::Sin(Angle) + PivotOffsetX, V);
			TangentX2D = FVector2D(Vert.Position) - FVector2D(CenterPoint);
			TangentX2D.Normalize();
			TangentZ = FVector(TangentX2D, 0);
			TangentX = FVector::CrossProduct(TangentY, TangentZ);
			Vert.SetTangents(TangentX, TangentY, TangentZ);
			Vert.TextureCoordinate[0] = FVector2D(UVX, 1);
			Vertices.Add(Vert);
			Vert.Position.Z = VL;
			Vert.TextureCoordinate[0] = FVector2D(UVX, 0);
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
		UE_LOG(LGUI, Warning, TEXT("[ULGUIRenderTargetGeometrySource::GetCanvas]TargetCanvas's render mode must be RenderTarget, and must have a valid RenderTarget2D"));
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
		MarkRenderStateDirty(); // New section requires recreating scene proxy
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
		MarkRenderStateDirty(); // New section requires recreating scene proxy
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
		MarkRenderStateDirty(); // New section requires recreating scene proxy
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
		MarkRenderStateDirty(); // New section requires recreating scene proxy
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
#if PLATFORM_ANDROID
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
		const FVector Origin = FVector(
			Thickness * 0.5f,
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
					//delay call, or the bPostTickComponentUpdate check will break
					ULTweenBPLibrary::DelayFrameCall(this, 1, [=] {
						StaticMeshComp->SetMaterial(0, MaterialInstance);
						});
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
#if PLATFORM_ANDROID
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

				FVector Pos0 = Vertices[Index0].Position;
				FVector Pos1 = Vertices[Index1].Position;
				FVector Pos2 = Vertices[Index2].Position;

				FVector2D UV0 = Vertices[Index0].TextureCoordinate[0];
				FVector2D UV1 = Vertices[Index1].TextureCoordinate[0];
				FVector2D UV2 = Vertices[Index2].TextureCoordinate[0];

				// Transform hit location from world to local space.
				// Find barycentric coords
				FVector BaryCoords = FMath::ComputeBaryCentric2D(LocalHitPoint, Pos0, Pos1, Pos2);
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
			auto Position0 = Vert0.Position;
			auto Position1 = Vert1.Position;

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
				auto Position2 = Vert2.Position;
				auto Position3 = Vert3.Position;

				auto Y = Position2 - Position0;
				Y.Normalize();
				auto Z = FVector(0, 0, 1);
				auto X = FVector::CrossProduct(Y, Z);
				X.Normalize();

				auto LocalRectMatrix = FMatrix(X, Y, Z, Position0);
				auto ToLocalRectMatrix = LocalRectMatrix.Inverse();

				auto RectSpaceRayOrigin = ToLocalRectMatrix.TransformPosition(LocalSpaceRayOrigin);
				auto RectSpaceRayEnd = ToLocalRectMatrix.TransformPosition(LocalSpaceRayEnd);
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
					float Right = (FVector2D(Position2) - FVector2D(Position0)).Size();
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
