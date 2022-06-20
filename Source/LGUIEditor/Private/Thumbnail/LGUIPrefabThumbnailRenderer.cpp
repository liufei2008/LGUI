// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Thumbnail/LGUIPrefabThumbnailRenderer.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "SceneView.h"
#include "Engine/EngineTypes.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "LGUIEditorUtils.h"

ULGUIPrefabThumbnailRenderer::ULGUIPrefabThumbnailRenderer()
{

}

bool ULGUIPrefabThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (auto prefab = Cast<ULGUIPrefab>(Object))
		return true;
	return false;
}
void ULGUIPrefabThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	auto prefab = Cast<ULGUIPrefab>(Object);
	if (IsValid(prefab))
	{
		TSharedRef<FLGUIPrefabThumbnailScene> ThumbnailScene = ThumbnailScenes.EnsureThumbnailScene(prefab->GetPathName());
		ThumbnailScene->SetPrefab(prefab);
		if (!ThumbnailScene->IsValidForVisualization())
			return;

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;

		ThumbnailScene->GetView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily);
	}
	//draw prefab icon
	LGUIEditorUtils::DrawThumbnailIcon(TEXT("LGUI/Resources/Icons/Prefab_40x.png"), X, Y, Width, Height, Canvas);
}
void ULGUIPrefabThumbnailRenderer::BeginDestroy()
{
	ThumbnailScenes.Clear();
	Super::BeginDestroy();
}