// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIEditorUtils.h"
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "LGUIEditorUtils"

TMap<FString, UTexture2D*> LGUIEditorUtils::IconPathToTextureMap;
void LGUIEditorUtils::DrawThumbnailIcon(const FString& IconPath, int32 X, int32 Y, uint32 Width, uint32 Height, FCanvas* Canvas)
{
	auto LoadSpriteIconTextureFromFile = [IconPath]()
	{
		UTexture2D* Texture = nullptr;

		auto SpriteIconFullPath = FPaths::Combine(FPaths::ProjectPluginsDir(), IconPath);
		if (!FPaths::FileExists(SpriteIconFullPath))
		{
			SpriteIconFullPath = FPaths::Combine(FPaths::EnginePluginsDir(), IconPath);
		}

		if (!FPaths::FileExists(*SpriteIconFullPath))
		{
			return Texture;
		}

		TArray<uint8> CompressedData;
		if (!FFileHelper::LoadFileToArray(CompressedData, *SpriteIconFullPath))
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

	UTexture2D* SpriteIconTexture = nullptr;
	if (!IconPathToTextureMap.Contains(IconPath))
	{
		IconPathToTextureMap.Add(IconPath, LoadSpriteIconTextureFromFile());
	}
	SpriteIconTexture = IconPathToTextureMap[IconPath];
	if (SpriteIconTexture != nullptr && SpriteIconTexture->Resource != nullptr)
	{
		const float Scale = 0.3f;
		float triangleWidth = Width * Scale, triangleHeight = Height * Scale;
		float textureWidth = SpriteIconTexture->GetSurfaceWidth();
		float textureHeight = SpriteIconTexture->GetSurfaceHeight();
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

		FCanvasTriangleItem CanvasTriangle(Triangles, SpriteIconTexture->Resource);
		CanvasTriangle.BlendMode = ESimpleElementBlendMode::SE_BLEND_Translucent;
		Canvas->DrawItem(CanvasTriangle);
	}
}

#undef LOCTEXT_NAMESPACE
