// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Thumbnail/LGUIPrefabThumbnailScene.h"
#include "Components/PrimitiveComponent.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUIEditorModule.h"



FLGUIPrefabInstanceThumbnailScene::FLGUIPrefabInstanceThumbnailScene()
{
	InstancedThumbnailScenes.Reserve(MAX_NUM_SCENES);
}
TSharedPtr<FLGUIPrefabThumbnailScene> FLGUIPrefabInstanceThumbnailScene::FindThumbnailScene(const FString& InPrefabPath)const
{
	return InstancedThumbnailScenes.FindRef(InPrefabPath);
}
TSharedRef<FLGUIPrefabThumbnailScene> FLGUIPrefabInstanceThumbnailScene::EnsureThumbnailScene(const FString& InPrefabPath)
{
	TSharedPtr<FLGUIPrefabThumbnailScene> ExistingThumbnailScene = InstancedThumbnailScenes.FindRef(InPrefabPath);
	if (!ExistingThumbnailScene.IsValid())
	{
		if (InstancedThumbnailScenes.Num() >= MAX_NUM_SCENES)
		{
			InstancedThumbnailScenes.Reset();
		}
		ExistingThumbnailScene = MakeShareable(new FLGUIPrefabThumbnailScene());
		InstancedThumbnailScenes.Add(InPrefabPath, ExistingThumbnailScene);
	}
	return ExistingThumbnailScene.ToSharedRef();
}
void FLGUIPrefabInstanceThumbnailScene::Clear()
{
	InstancedThumbnailScenes.Reset();
}



FLGUIPrefabThumbnailScene::FLGUIPrefabThumbnailScene()
	:FThumbnailPreviewScene()
	, NumStartingActors(0)
	, CurrentPrefab(nullptr)
{
	NumStartingActors = GetWorld()->GetCurrentLevel()->Actors.Num();
}
void FLGUIPrefabThumbnailScene::SpawnPreviewActor()
{
	if (CurrentPrefab.IsValid())
	{
		if (auto rootActor = CurrentPrefab->LoadPrefabInEditor(GetWorld(), nullptr))
		{
			if (auto rootCanvas = rootActor->FindComponentByClass<ULGUICanvas>())
			{
				rootCanvas->MarkCanvasUpdate(true, true, true, true);

				bIsUI = true;
			}
			else if (auto rootUIItem = Cast<UUIItem>(rootActor->GetRootComponent()))
			{
				auto AgentRootActor = GetWorld()->SpawnActor<AUIContainerActor>();
				AgentRootActor->GetUIItem()->SetSizeDelta(FVector2D(1920, 1080));
				rootCanvas = NewObject<ULGUICanvas>(AgentRootActor);
				rootCanvas->RegisterComponent();
				rootActor->AttachToComponent(AgentRootActor->GetUIItem(), FAttachmentTransformRules::KeepRelativeTransform);
				rootActor->AddInstanceComponent(rootCanvas);
				rootCanvas->MarkCanvasUpdate(true, true, true, true);

				bIsUI = true;
			}
			else
			{
				bIsUI = false;
			}
			bool isFirstBounds = true;
			GetBoundsRecursive(rootActor->GetRootComponent(), PreviewActorsBound, isFirstBounds);
			if (isFirstBounds)
			{
				PreviewActorsBound = FBoxSphereBounds(EForceInit::ForceInitToZero);
			}
			if (PreviewActorsBound.SphereRadius < KINDA_SMALL_NUMBER)//if bounds is too small, set to 1x1 box
			{
				PreviewActorsBound = FBoxSphereBounds(FBox(FVector(-0.5f, -0.5f, -0.5f), FVector(0.5f, 0.5f, 0.5f)));
			}
		}
	}
}
void FLGUIPrefabThumbnailScene::GetBoundsRecursive(USceneComponent* RootComp, FBoxSphereBounds& OutBounds, bool& IsFirstPrimitive)const
{
	if (!IsValid(RootComp))return;
	bool canCalculate = true;
	if (auto UIItem = Cast<UUIItem>(RootComp))
	{
		canCalculate = UIItem->GetIsUIActiveInHierarchy();
	}
	if (canCalculate)
	{
		if (IsFirstPrimitive)
		{
			OutBounds = RootComp->CalcBounds(RootComp->GetComponentTransform());
			IsFirstPrimitive = false;
		}
		else
		{
			OutBounds = OutBounds + RootComp->CalcBounds(RootComp->GetComponentTransform());
		}
	}

	auto childrenArray = RootComp->GetAttachChildren();
	for (auto childComp : childrenArray)
	{
		GetBoundsRecursive(childComp, OutBounds, IsFirstPrimitive);
	}
}
void FLGUIPrefabThumbnailScene::ClearStaleActors()
{
	ULevel* Level = GetWorld()->GetCurrentLevel();

	for (int32 i = NumStartingActors; i < Level->Actors.Num(); ++i)
	{
		if (Level->Actors[i])
		{
			Level->Actors[i]->Destroy();
		}
	}
}
bool FLGUIPrefabThumbnailScene::IsValidForVisualization()
{
	if (CurrentPrefab.Get())
	{
		if (CurrentPrefab->BinaryData.Num() == 0)
			return false;
	}
	if (PreviewActorsBound.ContainsNaN())
	{
		UE_LOG(LGUIEditor, Warning, TEXT("[FLGUIPrefabThumbnailScene::IsValidForVisualization]prefab:%s bounds is invalid!"), *(CurrentPrefab->GetPathName()));
		return false;
	}
	return true;
}
void FLGUIPrefabThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom)const
{
	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;

	const float PreviewSize = PreviewActorsBound.SphereRadius * 0.8;
	const float BoundsZOffset = GetBoundsZOffset(PreviewActorsBound);
	const float TargetDistance = PreviewSize / FMath::Tan(HalfFOVRadians);

	USceneThumbnailInfo* ThumbnailInfo = GetSceneThumbnailInfo(TargetDistance);
	check(ThumbnailInfo);

	OutOrigin = -1 * PreviewActorsBound.Origin;
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + ThumbnailInfo->OrbitZoom;
}
void FLGUIPrefabThumbnailScene::SetPrefab(class ULGUIPrefab* Prefab)
{
	if (CurrentPrefab.IsStale())
	{
		CurrentPrefab = nullptr;
		ClearStaleActors();
	}
	if (CurrentPrefab.IsValid() && IsValid(Prefab))
	{
		if (CurrentPrefab == Prefab && !CurrentPrefab->ThumbnailDirty)
		{
			return;
		}
		ClearStaleActors();
	}
	CurrentPrefab = Prefab;
	CurrentPrefab->ThumbnailDirty = false;
	if (IsValid(Prefab))
	{
		SpawnPreviewActor();
	}
}
USceneThumbnailInfo* FLGUIPrefabThumbnailScene::GetSceneThumbnailInfo(const float TargetDistance)const
{
	ULGUIPrefab* Prefab = CurrentPrefab.Get();
	check(Prefab);
	USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(Prefab->ThumbnailInfo);
	if (!IsValid(ThumbnailInfo))
	{
		ThumbnailInfo = NewObject<USceneThumbnailInfo>(Prefab);
		Prefab->ThumbnailInfo = ThumbnailInfo;
	}
	if (bIsUI)
	{
		ThumbnailInfo->OrbitPitch = 0;
		ThumbnailInfo->OrbitYaw = 90.0f;
		ThumbnailInfo->OrbitZoom = 0;
	}
	else
	{
		ThumbnailInfo->OrbitPitch = -11.25f;
		ThumbnailInfo->OrbitYaw = -157.5f;
		ThumbnailInfo->OrbitZoom = 0;
	}
	return ThumbnailInfo;
}