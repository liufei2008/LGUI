// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/LGUISpriteData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIAtlasData.h"
#include "UObject/UObjectIterator.h"


void FLGUISpriteInfo::ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal)
{
	width = InWidth;
	height = InHeight;

	uv0X = InX * texFullWidthReciprocal;
	uv0Y = (InY + InHeight) * texFullHeightReciprocal;
	uv3X = (InX + InWidth) * texFullWidthReciprocal;
	uv3Y = InY * texFullHeightReciprocal;
}
bool FLGUISpriteInfo::HasBorder()const
{
	return borderLeft != 0 || borderRight != 0 || borderTop != 0 || borderBottom != 0;
}
void FLGUISpriteInfo::ApplyBorderUV(float texFullWidthReciprocal, float texFullHeightReciprocal)
{
	buv0X = uv0X + borderLeft * texFullWidthReciprocal;
	buv3X = uv3X - borderRight * texFullWidthReciprocal;
	buv0Y = uv0Y - borderBottom * texFullHeightReciprocal;
	buv3Y = uv3Y + borderTop * texFullHeightReciprocal;
}

bool ULGUISpriteData::InsertTexture(FLGUIAtlasData* InAtlasData)
{
#if WITH_EDITOR
	int32 spaceBetweenSprites = ULGUISettings::GetAtlasTexturePadding(packingTag);
#else
	static int32 spaceBetweenSprites = ULGUISettings::GetAtlasTexturePadding(packingTag);
#endif
	float atlasTextureSizeInv = 1.0f / InAtlasData->atlasBinPack.GetBinWidth();

	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;
	int insertRectWidth = spriteTexture->GetSurfaceWidth() + spaceBetweenSprites + spaceBetweenSprites;
	int insertRectHeight = spriteTexture->GetSurfaceHeight() + spaceBetweenSprites + spaceBetweenSprites;

	auto& areaItem = InAtlasData->atlasBinPack;
	auto packedRect = areaItem.Insert(insertRectWidth, insertRectHeight, method);
	if (packedRect.height <= 0)//means this area cannot fit the texture
	{
		return false;
	}
	else//this area can fit the texture, copy pixels
	{
		//remove space
		packedRect.x += spaceBetweenSprites;
		packedRect.y += spaceBetweenSprites;
		packedRect.width -= spaceBetweenSprites + spaceBetweenSprites;
		packedRect.height -= spaceBetweenSprites + spaceBetweenSprites;
		//pixels
		auto tempAtlasTexture = InAtlasData->atlasTexture;
		FBox2D srcRegionBox(FVector2D(0, 0), FVector2D(packedRect.width, packedRect.height));
		FBox2D dstRegionBox(FVector2D(packedRect.x, packedRect.y), FVector2D(packedRect.x + packedRect.width, packedRect.y + packedRect.height));

		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* SpriteTextureResource;
			FTexture2DResource* AtlasTextureResource;
			FBox2D SrcRegionBox;
			FBox2D DstRegionBox;
			int32 SpaceBetweenSprites;
			rbp::Rect PackedRect;
		};
		FUpdateTextureRegionsData* copyData = new FUpdateTextureRegionsData;
		copyData->SpriteTextureResource = (FTexture2DResource*)spriteTexture->Resource;
		copyData->AtlasTextureResource = (FTexture2DResource*)tempAtlasTexture->Resource;
		copyData->SrcRegionBox = srcRegionBox;
		copyData->DstRegionBox = dstRegionBox;
		copyData->SpaceBetweenSprites = spaceBetweenSprites;
		copyData->PackedRect = packedRect;

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
			FLGUISpriteCopyTextureData,
			FUpdateTextureRegionsData*, copyData, copyData,
			{
		auto spriteTextureRHIRef = copyData->SpriteTextureResource->GetTexture2DRHI();
		auto atlasTextureRHIRef = copyData->AtlasTextureResource->GetTexture2DRHI();
		auto srcRegionBox = copyData->SrcRegionBox;
		auto dstRegionBox = copyData->DstRegionBox;
		auto packedRect = copyData->PackedRect;
		auto spaceBetweenSprites = copyData->SpaceBetweenSprites;
		//origin image
		RHICmdList.CopySubTextureRegion(
			spriteTextureRHIRef,
			atlasTextureRHIRef,
			srcRegionBox,
			dstRegionBox
		);
		//pixel padding
		for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
		{
			//Left
			srcRegionBox.Min = FVector2D(0, 0);
			srcRegionBox.Max = FVector2D(1, packedRect.height);
			dstRegionBox.Min = FVector2D(packedRect.x - paddingIndex - 1, packedRect.y);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(1, packedRect.height);
			RHICmdList.CopySubTextureRegion(
				spriteTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//Right
			srcRegionBox.Min = FVector2D(packedRect.width - 1, 0);
			srcRegionBox.Max = FVector2D(packedRect.width, packedRect.height);
			dstRegionBox.Min = FVector2D(packedRect.x + packedRect.width + paddingIndex, packedRect.y);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(1, packedRect.height);
			RHICmdList.CopySubTextureRegion(
				spriteTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//Top
			srcRegionBox.Min = FVector2D(0, packedRect.height - 1);
			srcRegionBox.Max = FVector2D(packedRect.width, packedRect.height);
			dstRegionBox.Min = FVector2D(packedRect.x, packedRect.y + packedRect.height + paddingIndex);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(packedRect.width, 1);
			RHICmdList.CopySubTextureRegion(
				spriteTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//Bottom
			srcRegionBox.Min = FVector2D(0, 0);
			srcRegionBox.Max = FVector2D(packedRect.width, 1);
			dstRegionBox.Min = FVector2D(packedRect.x, packedRect.y - paddingIndex - 1);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(packedRect.width, 1);
			RHICmdList.CopySubTextureRegion(
				spriteTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
		}
		for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
		{
			//LeftTop
			srcRegionBox.Min = FVector2D(packedRect.x - spaceBetweenSprites, packedRect.y + packedRect.height - 1);
			srcRegionBox.Max = FVector2D(packedRect.x, packedRect.y + packedRect.height);
			dstRegionBox.Min = FVector2D(packedRect.x - spaceBetweenSprites, packedRect.y + packedRect.height + paddingIndex);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(spaceBetweenSprites, 1);
			RHICmdList.CopySubTextureRegion(
				atlasTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//RightTop
			srcRegionBox.Min = FVector2D(packedRect.x + packedRect.width, packedRect.y + packedRect.height - 1);
			srcRegionBox.Max = FVector2D(packedRect.x + packedRect.width + spaceBetweenSprites, packedRect.y + packedRect.height);
			dstRegionBox.Min = FVector2D(packedRect.x + packedRect.width, packedRect.y + packedRect.height + paddingIndex);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(spaceBetweenSprites, 1);
			RHICmdList.CopySubTextureRegion(
				atlasTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//LeftBottom
			srcRegionBox.Min = FVector2D(packedRect.x - spaceBetweenSprites, packedRect.y);
			srcRegionBox.Max = FVector2D(packedRect.x, packedRect.y + 1);
			dstRegionBox.Min = FVector2D(packedRect.x - spaceBetweenSprites, packedRect.y - 1 - paddingIndex);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(spaceBetweenSprites, 1);
			RHICmdList.CopySubTextureRegion(
				atlasTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
			//RightBottom
			srcRegionBox.Min = FVector2D(packedRect.x + packedRect.width, packedRect.y);
			srcRegionBox.Max = FVector2D(packedRect.x + packedRect.width + spaceBetweenSprites, packedRect.y + 1);
			dstRegionBox.Min = FVector2D(packedRect.x + packedRect.width, packedRect.y - 1 - paddingIndex);
			dstRegionBox.Max = dstRegionBox.Min + FVector2D(spaceBetweenSprites, 1);
			RHICmdList.CopySubTextureRegion(
				atlasTextureRHIRef,
				atlasTextureRHIRef,
				srcRegionBox,
				dstRegionBox
			);
		}
		delete copyData;
		}
		);
		//add to sprite
		atlasTexture = tempAtlasTexture;
		spriteInfo.ApplyUV(packedRect.x, packedRect.y, packedRect.width, packedRect.height, atlasTextureSizeInv, atlasTextureSizeInv);
		spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
		InAtlasData->spriteDataArray.Add(this);
		return true;
	}
}

void ULGUISpriteData::PackageSprite()
{
#if WITH_EDITOR
	int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#else
	static int32 defaultAtlasTextureSize = ULGUISettings::GetAtlasTextureInitialSize(packingTag);
#endif

	if (spriteTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon)
	{
		ApplySpriteTextureSetting(spriteTexture);
	}

	auto atlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
	
	if (atlasData->atlasTexture == nullptr)//means first time of this tag
	{
		atlasData->atlasBinPack.Init(defaultAtlasTextureSize, defaultAtlasTextureSize, 0, 0);
		CreateAtlasTexture(0, defaultAtlasTextureSize, atlasData);
	}
PACK_AND_INSERT:
	if (InsertTexture(atlasData))
	{

	}
	else//all area cannot fit the texture, then expend texture size
	{
		int32 oldTextureSize = atlasData->atlasBinPack.GetBinWidth();
		int32 newTextureSize = oldTextureSize * 2;
		UE_LOG(LGUI, Log, TEXT("[PackageSprite]Insert texture:%s expend size to %d"), *(spriteTexture->GetPathName()), newTextureSize);
		if (newTextureSize > WARNING_ATLAS_SIZE)
		{
			UE_LOG(LGUI, Warning, TEXT("[PackageSprite]Trying to insert texture:%s, result to expend size to:%d larger than the preferred maximun texture size:%d! Try reduce some sprite texture size, or use UITexture to render some large texture, or use different packingTag to splite your atlasTexture."), *(spriteTexture->GetPathName()), newTextureSize, WARNING_ATLAS_SIZE);
		}
		atlasData->atlasBinPack.ExpendSize(newTextureSize, newTextureSize);
		//create new texture
		CreateAtlasTexture(oldTextureSize, newTextureSize, atlasData);
		//scale down sprite uv
		for (ULGUISpriteData* spriteItem : atlasData->spriteDataArray)
		{
			spriteItem->atlasTexture = atlasData->atlasTexture;
			spriteItem->spriteInfo.ScaleUV(0.5f);
		}
		//tell UISprite to scale down uv
		for (auto itemSprite : atlasData->renderSpriteArray)
		{
			if (itemSprite.IsValid())
			{
				itemSprite->ApplyAtlasTextureScaleUp();
			}
		}

		goto PACK_AND_INSERT;
	}
}
void ULGUISpriteData::CreateAtlasTexture(int oldTextureSize, int newTextureSize, FLGUIAtlasData* InAtlasData)
{
#if WITH_EDITOR
	bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#else
	static bool atlasSRGB = ULGUISettings::GetAtlasTextureSRGB(packingTag);
	static auto filter = ULGUISettings::GetAtlasTextureFilter(packingTag);
#endif
	auto texture = UTexture2D::CreateTransient(newTextureSize, newTextureSize, PF_B8G8R8A8);
	texture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	texture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	texture->SRGB = atlasSRGB;
	texture->UpdateResource();
	texture->AddToRoot();
	auto oldTexture = InAtlasData->atlasTexture;
	InAtlasData->atlasTexture = texture;

	//copy texture
	if (oldTextureSize != 0 && oldTexture != nullptr)
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_THREEPARAMETER
		(
			FLGUISpriteCopyAtlasTexture,
			UTexture2D*, oldTexture, oldTexture,
			UTexture2D*, texture, texture,
			int32, oldTextureSize, oldTextureSize,
			{
				FBox2D regionBox(FVector2D(0, 0), FVector2D(oldTextureSize, oldTextureSize));
				RHICmdList.CopySubTextureRegion(
					((FTexture2DResource*)oldTexture->Resource)->GetTexture2DRHI(),
					((FTexture2DResource*)texture->Resource)->GetTexture2DRHI(),
					regionBox,
					regionBox
				);
				oldTexture->RemoveFromRoot();//ready for gc
			}
		);
	}
}
void ULGUISpriteData::CheckSpriteTexture()
{
	if (spriteTexture == nullptr)
	{
		spriteTexture = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
	}
}
#if WITH_EDITOR
void ULGUISpriteData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	CheckSpriteTexture();
	if (PropertyChangedEvent.Property != nullptr)
	{
		auto propertyName = PropertyChangedEvent.Property->GetName();
		if (propertyName == TEXT("spriteTexture"))
		{
			if (spriteTexture != nullptr)
			{
				if (spriteTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon)
				{
					ApplySpriteTextureSetting(spriteTexture);
				}
				spriteInfo.width = spriteTexture->GetSurfaceWidth();
				spriteInfo.height = spriteTexture->GetSurfaceHeight();
			}
		}
		//sprite data, apply border
		if (spriteTexture != nullptr)
		{
			if (isInitialized)
			{
				float atlasTextureSizeInv = 1.0f / GetAtlasTexture()->GetSurfaceWidth();
				spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
			}
			spriteInfo.width = spriteTexture->GetSurfaceWidth();
			spriteInfo.height = spriteTexture->GetSurfaceHeight();
		}
	}
}
void ULGUISpriteData::MarkAllSpritesNeedToReinitialize()
{
	ULGUIAtlasManager::ResetAtlasMap();
	for (TObjectIterator<ULGUISpriteData> SpriteItr; SpriteItr; ++SpriteItr)
	{
		SpriteItr->isInitialized = false;
	}
}
#endif

void ULGUISpriteData::ApplySpriteTextureSetting(UTexture2D* InSpriteTexture)
{
	InSpriteTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
	InSpriteTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
	InSpriteTexture->SRGB = true;
	InSpriteTexture->UpdateResource();
	InSpriteTexture->MarkPackageDirty();
}

void ULGUISpriteData::ReloadTexture()
{
	isInitialized = false;
}

void ULGUISpriteData::InitSpriteData()
{
	if (!isInitialized)
	{
		if (spriteTexture == nullptr)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::InitSpriteData]SpriteData:%s spriteTexture is null!"), *(this->GetPathName()));
			return;
		}
		if (packingTag.IsNone())
		{
			atlasTexture = spriteTexture;
		}
		else
		{
			PackageSprite();
		}
		isInitialized = true;
	}
}

UTexture2D* ULGUISpriteData::GetAtlasTexture()
{
	InitSpriteData();
	return atlasTexture;
}
const FLGUISpriteInfo* ULGUISpriteData::GetSpriteInfo()
{
	InitSpriteData();
	return &spriteInfo;
}
bool ULGUISpriteData::HavePackingTag()const
{
	return !packingTag.IsNone();
}
const FName& ULGUISpriteData::GetPackingTag()const
{
	return packingTag;
}

void ULGUISpriteData::AddUISprite(UUISpriteBase* InUISprite)
{
	InitSpriteData();
	auto& spriteArray = ULGUIAtlasManager::FindOrAdd(packingTag)->renderSpriteArray;
	spriteArray.AddUnique(InUISprite);
}
void ULGUISpriteData::RemoveUISprite(UUISpriteBase* InUISprite)
{
	if (auto spriteData = ULGUIAtlasManager::Find(packingTag))
	{
		spriteData->renderSpriteArray.Remove(InUISprite);
	}
}

ULGUISpriteData* ULGUISpriteData::GetDefaultWhiteSolid()
{
	static auto defaultWhiteSolid = LoadObject<ULGUISpriteData>(NULL, TEXT("/LGUI/LGUIPreset_WhiteSolid"));
	if (defaultWhiteSolid == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[LGUISpriteData::GetDefaultWhiteSolid]Load default sprite error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		return nullptr;
	}
	return defaultWhiteSolid;
}
