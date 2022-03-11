// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Thumbnail/LGUIPrefabThumbnailRenderer.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "SceneView.h"
#include "Engine/EngineTypes.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

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
			.SetTime(UThumbnailRenderer::GetTime()));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;

		auto View = ThumbnailScene->CreateView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily, View);
	}
	//draw prefab icon
	{
		auto PrefabIconTexture = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/PrefabThumbnailOverlay"));
		if (PrefabIconTexture != nullptr && PrefabIconTexture->GetResource() != nullptr)
		{
			const float Scale = 0.3f;
			float triangleWidth = Width * Scale, triangleHeight = Height * Scale;
			float textureWidth = PrefabIconTexture->GetSurfaceWidth();
			float textureHeight = PrefabIconTexture->GetSurfaceHeight();
			float xOffset = 0, yOffset = 0;
			if (textureWidth > textureHeight)
			{
				triangleHeight = Height * textureHeight / textureWidth;
				yOffset = (Height - triangleHeight) * 0.5f;
			}
			else if (textureWidth < textureHeight)
			{
				triangleWidth = Width * textureWidth / textureHeight;
				xOffset = (Width - triangleWidth) * 0.5f;
			}
			TArray<FCanvasUVTri> Triangles;
			const FLinearColor SpriteColor(FLinearColor::White);

			FCanvasUVTri* Triangle1 = new (Triangles) FCanvasUVTri();
			Triangle1->V0_Pos = FVector2D(X + triangleWidth + xOffset, Y + yOffset); Triangle1->V0_UV = FVector2D(1.0f, 0); Triangle1->V0_Color = SpriteColor;
			Triangle1->V1_Pos = FVector2D(X + xOffset, Y + yOffset); Triangle1->V1_UV = FVector2D(0, 0); Triangle1->V1_Color = SpriteColor;
			Triangle1->V2_Pos = FVector2D(X + xOffset, Y + triangleHeight + yOffset); Triangle1->V2_UV = FVector2D(0, 1.0f); Triangle1->V2_Color = SpriteColor;

			FCanvasUVTri* Triangle2 = new (Triangles) FCanvasUVTri();
			Triangle2->V0_Pos = FVector2D(X + triangleWidth + xOffset, Y + yOffset); Triangle2->V0_UV = FVector2D(1.0f, 0); Triangle2->V0_Color = SpriteColor;
			Triangle2->V1_Pos = FVector2D(X + xOffset, Y + triangleHeight + yOffset); Triangle2->V1_UV = FVector2D(0, 1.0f); Triangle2->V1_Color = SpriteColor;
			Triangle2->V2_Pos = FVector2D(X + triangleWidth + xOffset, Y + triangleHeight + yOffset); Triangle2->V2_UV = FVector2D(1.0f, 1.0f); Triangle2->V2_Color = SpriteColor;

			FCanvasTriangleItem CanvasTriangle(Triangles, PrefabIconTexture->GetResource());
			CanvasTriangle.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
			Canvas->DrawItem(CanvasTriangle);
		}
	}
}
void ULGUIPrefabThumbnailRenderer::BeginDestroy()
{
	ThumbnailScenes.Clear();
	Super::BeginDestroy();
}