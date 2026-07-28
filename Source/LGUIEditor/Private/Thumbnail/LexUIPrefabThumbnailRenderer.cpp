// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Thumbnail/LexUIPrefabThumbnailRenderer.h"
#include "SceneView.h"
#include "LexUIEditorUtils.h"
#include "Interfaces/IPluginManager.h"
#include "PrefabSystem/LexUIPrefab.h"

ULexUIPrefabThumbnailRenderer::ULexUIPrefabThumbnailRenderer()
{
	ThumbnailScene = nullptr;
}

bool ULexUIPrefabThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{
	if (Object->IsA(ULexUIPrefab::StaticClass()))
		return true;
	return false;
}
void ULexUIPrefabThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	if (auto Prefab = Cast<ULexUIPrefab>(Object))
	{
		if (ThumbnailScene == nullptr)
		{
			ThumbnailScene = MakeUnique<FLexUIPrefabThumbnailScene>();
		}
		ThumbnailScene->SetPrefab(Prefab);
		if (!ThumbnailScene->IsValidForVisualization())
			return;

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetTime(UThumbnailRenderer::GetTime()));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;

		auto View = ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily, View);

		//draw prefab icon
		static FString LGUIBasePath = IPluginManager::Get().FindPlugin(TEXT("LGUI"))->GetBaseDir();
		FLexUIEditorUtils::DrawThumbnailIcon(LGUIBasePath + (Prefab->GetIsPrefabVariant() ? TEXT("/Resources/Icons/PrefabVariant_40x.png") : TEXT("/Resources/Icons/Prefab_40x.png"))
			, X, Y, Width, Height, Canvas);

		Prefab->bThumbnailDirty = false;
	}
}
void ULexUIPrefabThumbnailRenderer::BeginDestroy()
{
	ThumbnailScene.Reset();
	Super::BeginDestroy();
}