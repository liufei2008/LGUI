// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Thumbnail/LGUIPrefabThumbnailRenderer.h"
#include "EngineModule.h"
#include "RendererInterface.h"
#include "SceneView.h"
#include "Engine/EngineTypes.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"

#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"

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
	if (prefab != nullptr && !prefab->IsPendingKill())
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
	{
		auto LoadPrefabIconTextureFromFile = []()
		{
			UTexture2D* Texture = nullptr;

			auto PrefabIconPath = TEXT("LGUI/Resources/Icons/Prefab_40x.png");
			auto PrefabIconFullPath = FPaths::Combine(FPaths::ProjectPluginsDir(), PrefabIconPath);
			if (!FPaths::FileExists(PrefabIconFullPath))
			{
				PrefabIconFullPath = FPaths::Combine(FPaths::EnginePluginsDir(), PrefabIconPath);
			}

			if (!FPaths::FileExists(*PrefabIconFullPath))
			{
				return Texture;
			}

			TArray<uint8> CompressedData;
			if (!FFileHelper::LoadFileToArray(CompressedData, *PrefabIconFullPath))
			{
				return Texture;
			}

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(CompressedData.GetData(), CompressedData.Num()))
			{
				TArray<uint8> UncompressedRGBA;

				if (ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, UncompressedRGBA))
				{
					Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_R8G8B8A8);
					if (Texture != nullptr)
					{
						void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
						FMemory::Memcpy(TextureData, UncompressedRGBA.GetData(), UncompressedRGBA.Num());
						Texture->PlatformData->Mips[0].BulkData.Unlock();
						Texture->UpdateResource();
						Texture->AddToRoot();
					}
				}
			}

			return Texture;
		};
		
		static auto PrefabIconTexture = LoadPrefabIconTextureFromFile();
		if (PrefabIconTexture != nullptr && PrefabIconTexture->Resource != nullptr)
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

			FCanvasTriangleItem CanvasTriangle(Triangles, PrefabIconTexture->Resource);
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