// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Thumbnail/LGUISpriteDataBaseObjectThumbnailRenderer.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "SceneView.h"
#include "Engine/EngineTypes.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "CanvasItem.h"
#include "EditorStyleSet.h"
#include "CanvasTypes.h"
#include "LGUIEditorUtils.h"

ULGUISpriteDataBaseObjectThumbnailRenderer::ULGUISpriteDataBaseObjectThumbnailRenderer()
{

}
void ULGUISpriteDataBaseObjectThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas, bool bAdditionalViewFamily)
{
	if (auto sprite = Cast<ULGUISpriteData_BaseObject>(Object))
	{
		DrawFrame(sprite, X, Y, Width, Height, RenderTarget, Canvas, nullptr);
	}
	
}
void ULGUISpriteDataBaseObjectThumbnailRenderer::DrawFrame(class ULGUISpriteData_BaseObject* Sprite, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, FBoxSphereBounds* OverrideRenderBounds)
{
	const UTexture2D* SourceTexture = SourceTexture = Sprite->GetAtlasTexture();

	if (SourceTexture != nullptr)
	{
		const bool bUseTranslucentBlend = SourceTexture->HasAlphaChannel();
		DrawGrid(X, Y, Width, Height, Canvas);

		auto SpriteInfo = Sprite->GetSpriteInfo();
		// Draw triangles
		if (SourceTexture->Resource != nullptr)
		{
			float triangleWidth = Width, triangleHeight = Height;
			float spriteWidth = SpriteInfo.width;
			float spriteHeight = SpriteInfo.height;
			float xOffset = 0, yOffset = 0;
			if (spriteWidth > spriteHeight)
			{
				triangleHeight = Height * spriteHeight / spriteWidth;
				yOffset = (Height - triangleHeight) * 0.5f;
			}
			else if (spriteWidth < spriteHeight)
			{
				triangleWidth = Width * spriteWidth / spriteHeight;
				xOffset = (Width - triangleWidth) * 0.5f;
			}
			TArray<FCanvasUVTri> Triangles;
			const FLinearColor SpriteColor(FLinearColor::White);

			FCanvasUVTri* Triangle1 = new (Triangles) FCanvasUVTri();
			Triangle1->V0_Pos = FVector2D(X + triangleWidth + xOffset, Y + yOffset); Triangle1->V0_UV = SpriteInfo.GetUV3(); Triangle1->V0_Color = SpriteColor;
			Triangle1->V1_Pos = FVector2D(X + xOffset, Y + yOffset); Triangle1->V1_UV = SpriteInfo.GetUV2(); Triangle1->V1_Color = SpriteColor;
			Triangle1->V2_Pos = FVector2D(X + xOffset, Y + triangleHeight + yOffset); Triangle1->V2_UV = SpriteInfo.GetUV0(); Triangle1->V2_Color = SpriteColor;

			FCanvasUVTri* Triangle2 = new (Triangles) FCanvasUVTri();
			Triangle2->V0_Pos = FVector2D(X + triangleWidth + xOffset, Y + yOffset); Triangle2->V0_UV = SpriteInfo.GetUV3(); Triangle2->V0_Color = SpriteColor;
			Triangle2->V1_Pos = FVector2D(X + xOffset, Y + triangleHeight + yOffset); Triangle2->V1_UV = SpriteInfo.GetUV0(); Triangle2->V1_Color = SpriteColor;
			Triangle2->V2_Pos = FVector2D(X + triangleWidth + xOffset, Y + triangleHeight + yOffset); Triangle2->V2_UV = SpriteInfo.GetUV1(); Triangle2->V2_Color = SpriteColor;

			FCanvasTriangleItem CanvasTriangle(Triangles, SourceTexture->Resource);
			CanvasTriangle.BlendMode = bUseTranslucentBlend ? ESimpleElementBlendMode::SE_BLEND_Translucent : ESimpleElementBlendMode::SE_BLEND_Opaque;
			Canvas->DrawItem(CanvasTriangle);
		}
	}
	else
	{
		// Fallback for a bogus sprite
		DrawGrid(X, Y, Width, Height, Canvas);
	}
	//draw sprite icon
	LGUIEditorUtils::DrawThumbnailIcon(TEXT("LGUI/Resources/Icons/UISprite_40x.png"), X, Y, Width, Height, Canvas);
}
void ULGUISpriteDataBaseObjectThumbnailRenderer::DrawGrid(int32 X, int32 Y, uint32 Width, uint32 Height, FCanvas* Canvas)
{
	static UTexture2D* GridTexture = Cast<UTexture2D>(FEditorStyle::GetBrush("Checkerboard")->GetResourceObject());
	if (GridTexture == nullptr)
	{
		GridTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid"), nullptr, LOAD_None, nullptr);
	}

	const bool bAlphaBlend = false;

	Canvas->DrawTile(
		(float)X,
		(float)Y,
		(float)Width,
		(float)Height,
		0.0f,
		0.0f,
		4.0f,
		4.0f,
		FLinearColor(0.15f, 0.15f, 0.15f),
		GridTexture->Resource,
		bAlphaBlend);
}
void ULGUISpriteDataBaseObjectThumbnailRenderer::BeginDestroy()
{
	Super::BeginDestroy();
}