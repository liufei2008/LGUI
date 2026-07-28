// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Thumbnail/LexUIPrefabThumbnailScene.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "Core/Components/LexCanvas.h"
#include "LGUIEditorModule.h"
#include "Core/Components/LexWidget.h"
#include "PrefabSystem/LexUIPrefab.h"

FLexUIPrefabThumbnailScene::FLexUIPrefabThumbnailScene()
	:FThumbnailPreviewScene()
	, NumStartingActors(0)
	, CurrentPrefab(nullptr)
{
	NumStartingActors = GetWorld()->GetCurrentLevel()->Actors.Num();
}
void FLexUIPrefabThumbnailScene::SpawnPreviewWidget()
{
	if (!CurrentPrefab.IsValid())return;
	if (RootAgentWidget.IsValid())return;
	auto CanvasSize = CurrentPrefab->PrefabDataForPrefabEditor.CanvasSize;

	//create Canvas for UI
	auto RootWidget = NewObject<ULexWidget>(this->GetWorld(), FName("[RootAgent]"));
	RootWidget->SetSizeDelta(CanvasSize);
	RootWidget->SetDisplayName(TEXT("[RootAgent]"));
	RootWidget->OnRegister();
	RootAgentWidget = RootWidget;

	CurrentPrefab->LoadPrefab(this->GetWorld(), RootWidget);
	auto Canvas = RootWidget->AddComponent<ULexCanvas>();
	
	auto RenderMode = (ELexRenderMode)CurrentPrefab->PrefabDataForPrefabEditor.CanvasRenderMode;
	Canvas->SetRenderMode(RenderMode);
	Canvas->bFixedSizeInEditMode = true;

	Canvas->UpdateRootCanvas();//for update draw-call immediately
	GetBoundsRecursive(RootWidget, PreviewBounds);
	if (PreviewBounds.SphereRadius < KINDA_SMALL_NUMBER)//if bounds is too small, set to 1x1 box
	{
		PreviewBounds = FBoxSphereBounds(FBox(FVector(-0.5f, -0.5f, -0.5f), FVector(0.5f, 0.5f, 0.5f)));
	}
}
void FLexUIPrefabThumbnailScene::GetBoundsRecursive(ULexWidget* RootWidget, FBoxSphereBounds& OutBounds)const
{
	if (!IsValid(RootWidget))
	{
		OutBounds = FBoxSphereBounds(EForceInit::ForceInitToZero);
		return;
	}
	struct LOCAL
	{
		static void GetBounds(ULexWidget* InWidget, bool& bIsFirstBounds, FBox& OutBox)
		{
			if (auto Visual = InWidget->GetVisual())
			{
				FVector Min, Max;
				Visual->GetGeometryBounds3DInLocalSpace(Min, Max);
				FBox Box = FBox(Min, Max);
				Box = Box.TransformBy(InWidget->GetWorldTransform());
				if (bIsFirstBounds)
				{
					bIsFirstBounds = false;
					OutBox = Box;
				}
				else
				{
					OutBox += Box;
				}
			}
			for (auto Child : InWidget->GetChildren())
			{
				GetBounds(Child, bIsFirstBounds, OutBox);
			}
		}
	};
	
	bool bIsFirstBounds = true;
	FBox BoxBounds;
	LOCAL::GetBounds(RootWidget, bIsFirstBounds, BoxBounds);
	OutBounds = BoxBounds;
}
void FLexUIPrefabThumbnailScene::ClearOldWidgets()
{
	if (RootAgentWidget.IsValid())
	{
		RootAgentWidget->DestroyWidget();
		RootAgentWidget.Reset();
	}
}
bool FLexUIPrefabThumbnailScene::IsValidForVisualization()const
{
	if (CurrentPrefab.Get())
	{
		if (CurrentPrefab->BinaryData.Num() == 0)
			return false;
	}
	if (PreviewBounds.ContainsNaN())
	{
		UE_LOG(LGUIEditor, Warning, TEXT("[%s].%d Prefab:'%s' bounds is invalid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(CurrentPrefab->GetPathName()));
		return false;
	}
	return true;
}
void FLexUIPrefabThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom)const
{
	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;

	const float PreviewSize = PreviewBounds.SphereRadius * 1.2f;
	const float BoundsZOffset = GetBoundsZOffset(PreviewBounds);
	const float TargetDistance = PreviewSize / FMath::Tan(HalfFOVRadians);

	USceneThumbnailInfo* ThumbnailInfo = GetSceneThumbnailInfo(TargetDistance);
	check(ThumbnailInfo);

	OutOrigin = -1 * PreviewBounds.Origin;
	OutOrbitPitch = ThumbnailInfo->OrbitPitch;
	OutOrbitYaw = ThumbnailInfo->OrbitYaw;
	OutOrbitZoom = TargetDistance + ThumbnailInfo->OrbitZoom;
}
void FLexUIPrefabThumbnailScene::SetPrefab(class ULexUIPrefab* Prefab)
{
	if (!CurrentPrefab.IsValid())
	{
		CurrentPrefab = nullptr;
		ClearOldWidgets();
	}
	if (CurrentPrefab.IsValid() && IsValid(Prefab))
	{
		if (CurrentPrefab == Prefab && !CurrentPrefab->bThumbnailDirty)
		{
			return;
		}
		ClearOldWidgets();
	}
	CurrentPrefab = Prefab;
	CurrentPrefab->bThumbnailDirty = false;
	if (IsValid(Prefab))
	{
		SpawnPreviewWidget();
	}
}
USceneThumbnailInfo* FLexUIPrefabThumbnailScene::GetSceneThumbnailInfo(const float TargetDistance)const
{
	ULexUIPrefab* Prefab = CurrentPrefab.Get();
	check(Prefab);
	USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(Prefab->ThumbnailInfo);
	if (!IsValid(ThumbnailInfo))
	{
		ThumbnailInfo = NewObject<USceneThumbnailInfo>(Prefab);
		Prefab->ThumbnailInfo = ThumbnailInfo;
	}
	ThumbnailInfo->OrbitPitch = 0;
	ThumbnailInfo->OrbitYaw = 90.0f;
	ThumbnailInfo->OrbitZoom = 0;
	return ThumbnailInfo;
}