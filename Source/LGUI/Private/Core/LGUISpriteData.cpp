// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/LGUISpriteData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIAtlasData.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Engine.h"


void FLGUISpriteInfo::ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal)
{
	width = InWidth;
	height = InHeight;

	uv0X = InX * texFullWidthReciprocal;
	uv0Y = (InY + InHeight) * texFullHeightReciprocal;
	uv3X = (InX + InWidth) * texFullWidthReciprocal;
	uv3Y = InY * texFullHeightReciprocal;
}
void FLGUISpriteInfo::ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal, const FVector4& uvRect)
{
	width = InWidth;
	height = InHeight;

	uv0X = InX * texFullWidthReciprocal + uvRect.X;
	uv0Y = (InY + InHeight) * texFullHeightReciprocal + uvRect.Y;
	uv3X = (InX + InWidth) * texFullWidthReciprocal * uvRect.Z + uvRect.X;
	uv3Y = InY * texFullHeightReciprocal * uvRect.W + uvRect.Y;
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
	float atlasTextureSizeInv = 1.0f / InAtlasData->atlasTexture->GetSizeX();

	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;
	int insertRectWidth = spriteTexture->GetSizeX() + spaceBetweenSprites + spaceBetweenSprites;
	int insertRectHeight = spriteTexture->GetSizeY() + spaceBetweenSprites + spaceBetweenSprites;

	auto packedRect = InAtlasData->atlasBinPack.Insert(insertRectWidth, insertRectHeight, method);
	if (packedRect.height <= 0)//means this area cannot fit the texture
	{
		return false;
	}
	else//this area can fit the texture, copy pixels
	{
		atlasTexture = InAtlasData->atlasTexture;
		//remove space
		packedRect.x += spaceBetweenSprites;
		packedRect.y += spaceBetweenSprites;
		packedRect.width -= spaceBetweenSprites + spaceBetweenSprites;
		packedRect.height -= spaceBetweenSprites + spaceBetweenSprites;
		//pixels
		CopySpriteTextureToAtlas(packedRect, spaceBetweenSprites);
		//add to sprite
		spriteInfo.ApplyUV(packedRect.x, packedRect.y, packedRect.width, packedRect.height, atlasTextureSizeInv, atlasTextureSizeInv);
		spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
		InAtlasData->spriteDataArray.Add(this);
		return true;
	}
}
void ULGUISpriteData::CopySpriteTextureToAtlas(rbp::Rect InPackedRect, int32 InAtlasTexturePadding)
{
	if (spriteTexture->Resource != nullptr && atlasTexture->Resource != nullptr)
	{
		FBox2D srcRegionBox(FVector2D(0, 0), FVector2D(InPackedRect.width, InPackedRect.height));
		FBox2D dstRegionBox(FVector2D(InPackedRect.x, InPackedRect.y), FVector2D(InPackedRect.x + InPackedRect.width, InPackedRect.y + InPackedRect.height));

		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* SpriteTextureResource;
			FTexture2DResource* AtlasTextureResource;
			FBox2D SrcRegionBox;
			FBox2D DstRegionBox;
			int32 SpaceBetweenSprites;
			rbp::Rect PackedRect;
		};
		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;
		RegionData->SpriteTextureResource = (FTexture2DResource*)spriteTexture->Resource;
		RegionData->AtlasTextureResource = (FTexture2DResource*)atlasTexture->Resource;
		RegionData->SrcRegionBox = srcRegionBox;
		RegionData->DstRegionBox = dstRegionBox;
		RegionData->SpaceBetweenSprites = InAtlasTexturePadding;
		RegionData->PackedRect = InPackedRect;

		ENQUEUE_RENDER_COMMAND(FLGUISpriteCopyTextureData)(
			[RegionData](FRHICommandListImmediate& RHICmdList)
		{
			auto spriteTextureRHIRef = RegionData->SpriteTextureResource->GetTexture2DRHI();
			auto atlasTextureRHIRef = RegionData->AtlasTextureResource->GetTexture2DRHI();
			auto srcRegionPosition = RegionData->SrcRegionBox.Min;
			auto srcRegionSize = RegionData->SrcRegionBox.GetSize();
			auto dstRegionPosition = RegionData->DstRegionBox.Min;
			auto packedRect = RegionData->PackedRect;
			auto spaceBetweenSprites = RegionData->SpaceBetweenSprites;
			//origin image
			FRHICopyTextureInfo CopyInfo;
			CopyInfo.SourcePosition = FIntVector(srcRegionPosition.X, srcRegionPosition.Y, 0);
			CopyInfo.Size = FIntVector(srcRegionSize.X, srcRegionSize.Y, 0);
			CopyInfo.DestPosition = FIntVector(dstRegionPosition.X, dstRegionPosition.Y, 0);
			RHICmdList.CopyTexture(
				spriteTextureRHIRef,
				atlasTextureRHIRef,
				CopyInfo
			);
			//pixel padding
			for (int paddingIndex = 0; paddingIndex < spaceBetweenSprites; paddingIndex++)
			{
				//Left
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(1, packedRect.height, 0);
				CopyInfo.DestPosition = FIntVector(packedRect.x - paddingIndex - 1, packedRect.y, 0);
				RHICmdList.CopyTexture(
					spriteTextureRHIRef,
					atlasTextureRHIRef,
					CopyInfo
				);
				//Right
				CopyInfo.SourcePosition = FIntVector(packedRect.width - 1, 0, 0);
				CopyInfo.Size = FIntVector(1, packedRect.height, 0);
				CopyInfo.DestPosition = FIntVector(packedRect.x + packedRect.width + paddingIndex, packedRect.y, 0);
				RHICmdList.CopyTexture(
					spriteTextureRHIRef,
					atlasTextureRHIRef,
					CopyInfo
				);
				//Top
				CopyInfo.SourcePosition = FIntVector(0, packedRect.height - 1, 0);
				CopyInfo.Size = FIntVector(packedRect.width, 1, 0);
				CopyInfo.DestPosition = FIntVector(packedRect.x, packedRect.y + packedRect.height + paddingIndex, 0);
				RHICmdList.CopyTexture(
					spriteTextureRHIRef,
					atlasTextureRHIRef,
					CopyInfo
				);
				//Bottom
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(packedRect.width, 1, 0);
				CopyInfo.DestPosition = FIntVector(packedRect.x, packedRect.y - paddingIndex - 1, 0);
				RHICmdList.CopyTexture(
					spriteTextureRHIRef,
					atlasTextureRHIRef,
					CopyInfo
				);
			}
			for (int paddingIndexY = 0; paddingIndexY < spaceBetweenSprites; paddingIndexY++)
			{
				for (int paddingIndexX = 0; paddingIndexX < spaceBetweenSprites; paddingIndexX++)
				{
					//LeftTop
					CopyInfo.SourcePosition = FIntVector(0, packedRect.height - 1, 0);
					CopyInfo.Size = FIntVector(1, 1, 0);
					CopyInfo.DestPosition = FIntVector(packedRect.x - spaceBetweenSprites + paddingIndexX, packedRect.y + packedRect.height + paddingIndexY, 0);
					RHICmdList.CopyTexture(
						spriteTextureRHIRef,
						atlasTextureRHIRef,
						CopyInfo
					);
					//RightTop
					CopyInfo.SourcePosition = FIntVector(packedRect.width - 1, packedRect.height - 1, 0);
					CopyInfo.Size = FIntVector(1, 1, 0);
					CopyInfo.DestPosition = FIntVector(packedRect.x + packedRect.width + paddingIndexX, packedRect.y + packedRect.height + paddingIndexY, 0);
					RHICmdList.CopyTexture(
						spriteTextureRHIRef,
						atlasTextureRHIRef,
						CopyInfo
					);
					//LeftBottom
					CopyInfo.SourcePosition = FIntVector(0, 0, 0);
					CopyInfo.Size = FIntVector(1, 1, 0);
					CopyInfo.DestPosition = FIntVector(packedRect.x - spaceBetweenSprites + paddingIndexX, packedRect.y - 1 - paddingIndexY, 0);
					RHICmdList.CopyTexture(
						spriteTextureRHIRef,
						atlasTextureRHIRef,
						CopyInfo
					);
					//RightBottom
					CopyInfo.SourcePosition = FIntVector(packedRect.width - 1, 0, 0);
					CopyInfo.Size = FIntVector(1, 1, 0);
					CopyInfo.DestPosition = FIntVector(packedRect.x + packedRect.width + paddingIndexX, packedRect.y - 1 - paddingIndexY, 0);
					RHICmdList.CopyTexture(
						spriteTextureRHIRef,
						atlasTextureRHIRef,
						CopyInfo
					);
				}
			}
			delete RegionData;
		});
	}
}

void ULGUISpriteData::PackageSprite()
{
	if (spriteTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon)
	{
		ApplySpriteTextureSetting(spriteTexture);
	}

	auto atlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
	atlasData->EnsureAtlasTexture(packingTag);
	atlasTexture = atlasData->atlasTexture;
PACK_AND_INSERT:
	if (InsertTexture(atlasData))
	{

	}
	else//all area cannot fit the texture, then expend texture size
	{
		int32 newTextureSize = atlasData->ExpendTextureSize(packingTag);
		UE_LOG(LGUI, Log, TEXT("[PackageSprite]Insert texture:%s expend size to %d"), *(spriteTexture->GetPathName()), newTextureSize);
		if (newTextureSize > WARNING_ATLAS_SIZE)
		{
			UE_LOG(LGUI, Warning, TEXT("[PackageSprite]Trying to insert texture:%s, result to expend size to:%d larger than the preferred maximun texture size:%d! Try reduce some sprite texture size, or use UITexture to render some large texture, or use different packingTag to splite your atlasTexture."), *(spriteTexture->GetPathName()), newTextureSize, WARNING_ATLAS_SIZE);
		}
		goto PACK_AND_INSERT;
	}
}

void ULGUISpriteData::CheckSpriteTexture()
{
	if (spriteTexture == nullptr)
	{
		spriteTexture = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
	}
}

void ULGUISpriteData::ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv, UTexture2D* InAtlasTexture)
{
	this->atlasTexture = InAtlasTexture;
	isInitialized = true;
	spriteInfo.ApplyUV(InPackedRect.x, InPackedRect.y, InPackedRect.width, InPackedRect.height, InAtlasTextureSizeInv, InAtlasTextureSizeInv);
	spriteInfo.ApplyBorderUV(InAtlasTextureSizeInv, InAtlasTextureSizeInv);
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
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
			}
		}
		if (
			propertyName == TEXT("borderLeft") ||
			propertyName == TEXT("borderRight") ||
			propertyName == TEXT("borderTop") ||
			propertyName == TEXT("borderBottom")
			)
		{
			//sprite data, apply border
			if (spriteTexture != nullptr)
			{
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
				if (isInitialized)
				{
					float atlasTextureSizeInv = 1.0f / InitAndGetAtlasTexture()->GetSizeX();
					spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
				}
			}
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
			//ULGUIAtlasManager::InitCheck();
			//if (!isInitialized)
			{
				PackageSprite();
			}
		}
		isInitialized = true;
	}
}

UTexture2D* ULGUISpriteData::InitAndGetAtlasTexture()
{
	InitSpriteData();
	return atlasTexture;
}
const FLGUISpriteInfo& ULGUISpriteData::InitAndGetSpriteInfo()
{
	InitSpriteData();
	return spriteInfo;
}
bool ULGUISpriteData::HavePackingTag()const
{
	return !packingTag.IsNone();
}
const FName& ULGUISpriteData::GetPackingTag()const
{
	return packingTag;
}

UTexture2D* ULGUISpriteData::GetAtlasTexture()const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetAtlasTexture]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	return atlasTexture;
}
const FLGUISpriteInfo& ULGUISpriteData::GetSpriteInfo()const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetSpriteInfo]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	return spriteInfo;
}
void ULGUISpriteData::GetSpriteSize(int32& width, int32& height)const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetSpriteSize]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	width = spriteInfo.width;
	height = spriteInfo.height;
}
void ULGUISpriteData::GetSpriteBorderSize(int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom)const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetSpriteBorderSize]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	borderLeft = spriteInfo.borderLeft;
	borderRight = spriteInfo.borderRight;
	borderTop = spriteInfo.borderTop;
	borderBottom = spriteInfo.borderBottom;
}
void ULGUISpriteData::GetSpriteUV(float& UV0X, float& UV0Y, float& UV3X, float& UV3Y)const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetSpriteUV]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	UV0X = spriteInfo.uv0X;
	UV0Y = spriteInfo.uv0Y;
	UV3X = spriteInfo.uv3X;
	UV3Y = spriteInfo.uv3Y;
}
void ULGUISpriteData::GetSpriteBorderUV(float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y)const
{
	if (!isInitialized)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::GetSpriteBorderUV]always remember to call InitSpriteData() before this function to initialize this SpriteData"));
	}
	borderUV0X = spriteInfo.buv0X;
	borderUV0Y = spriteInfo.buv0Y;
	borderUV3X = spriteInfo.buv3X;
	borderUV3Y = spriteInfo.buv3Y;
}

ULGUISpriteData* ULGUISpriteData::CreateLGUISpriteData(UObject* WorldContextObject, UTexture2D* inSpriteTexture, FVector2D inHorizontalBorder /* = FVector2D::ZeroVector */, FVector2D inVerticalBorder /* = FVector2D::ZeroVector */, FName inPackingTag /* = TEXT("Main") */)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(inSpriteTexture))
	{
		return nullptr;
	}
	// check size
	if (inSpriteTexture)
	{
		int32 atlasPadding = 0;
		auto lguiSetting = GetDefault<ULGUISettings>()->defaultAtlasSetting.spaceBetweenSprites;
		if (inSpriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || inSpriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			UE_LOG(LGUI, Error, TEXT("Target texture width or height is too large! Consider use UITexture to render this texture."));
			return nullptr;
		}
		// Apply setting for sprite creation
		//inSpriteTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		inSpriteTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		inSpriteTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		inSpriteTexture->SRGB = true;
		inSpriteTexture->UpdateResource();
		inSpriteTexture->MarkPackageDirty();
	}

	ULGUISpriteData* result = NewObject<ULGUISpriteData>(world);
	if (inSpriteTexture)
	{
		result->spriteTexture = inSpriteTexture;
		auto& spriteInfo = result->spriteInfo;
		spriteInfo.width = inSpriteTexture->GetSurfaceWidth();
		spriteInfo.height = inSpriteTexture->GetSurfaceHeight();
		spriteInfo.borderLeft = (uint16)inHorizontalBorder.X;
		spriteInfo.borderRight = (uint16)inHorizontalBorder.Y;
		spriteInfo.borderTop = (uint16)inVerticalBorder.X;
		spriteInfo.borderBottom = (uint16)inVerticalBorder.Y;
	}
	return result;
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
		spriteData->renderSpriteArray.RemoveSingle(InUISprite);
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
