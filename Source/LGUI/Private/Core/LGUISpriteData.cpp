// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/LGUISpriteData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIAtlasData.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Engine.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "RHI.h"
#include "Rendering/Texture2DResource.h"


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
	uv0Y = (InY + InHeight) * texFullHeightReciprocal * uvRect.W + uvRect.Y;
	uv3X = (InX + InWidth) * texFullWidthReciprocal * uvRect.Z + uvRect.X;
	uv3Y = InY * texFullHeightReciprocal + uvRect.Y;
}
bool FLGUISpriteInfo::HasBorder()const
{
	return (borderLeft | borderRight | borderTop | borderBottom) != 0;
}
bool FLGUISpriteInfo::HasPadding()const
{
	return (paddingLeft | paddingRight | paddingTop | paddingBottom) != 0;
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
		RegionData->SpaceBetweenSprites = useEdgePixelPadding ? InAtlasTexturePadding : 0;
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
			if (spaceBetweenSprites > 0)
			{
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
			}
			delete RegionData;
		});
	}
}

bool ULGUISpriteData::PackageSprite()
{
	CheckAndApplySpriteTextureSetting(spriteTexture);

	auto atlasData = ULGUIAtlasManager::FindOrAdd(packingTag);
	atlasData->EnsureAtlasTexture(packingTag);
	atlasTexture = atlasData->atlasTexture;
PACK_AND_INSERT:
	if (InsertTexture(atlasData))
	{
		return true;
	}
	else//all area cannot fit the texture, then expend texture size
	{
		int32 newTextureSize = atlasData->GetWillExpendTextureSize();
		UE_LOG(LGUI, Log, TEXT("[PackageSprite]Insert texture:%s expend size to %d"), *(spriteTexture->GetPathName()), newTextureSize);
		if (newTextureSize > WARNING_ATLAS_SIZE)
		{
			FString warningMsg = FString::Printf(TEXT("[ULGUISpriteData::PackageSprite]Trying to insert texture:%s, result to expend size to:%d larger than the preferred maximun texture size:%d!\
\nTry reduce some sprite texture size, or use UITexture to render some large texture, or use different packingTag to split your atlasTexture.\
\nAlso remember to dispose unused atlas by call function DisposeAtlasByPackingTag from LGUIAtlasManager.\
")
				, *(spriteTexture->GetPathName()), newTextureSize, WARNING_ATLAS_SIZE);
			UE_LOG(LGUI, Warning, TEXT("%s"), *warningMsg);
#if WITH_EDITOR
			LGUIUtils::EditorNotification(FText::FromString(warningMsg));
#endif
		}
		if ((uint32)newTextureSize >= GetMax2DTextureDimension())
		{
			FString warningMsg = FString::Printf(TEXT("[ULGUISpriteData::PackageSprite]Trying to insert texture:%s, result too large size that not supported! Maximun texture size is:%d.")
, *(spriteTexture->GetPathName()), GetMax2DTextureDimension());
			UE_LOG(LGUI, Error, TEXT("%s"), *warningMsg);
#if WITH_EDITOR
			LGUIUtils::EditorNotification(FText::FromString(warningMsg));
#endif
			return false;
		}
		atlasData->ExpendTextureSize(packingTag);
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
		auto propertyName = PropertyChangedEvent.Property->GetFName();
		if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteTexture))
		{
			if (spriteTexture != nullptr)
			{
				CheckAndApplySpriteTextureSetting(spriteTexture);
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
			}
		}
		if (
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderLeft) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderRight) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderTop) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderBottom)
			)
		{
			//sprite data, apply border
			if (spriteTexture != nullptr)
			{
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
				if (isInitialized)
				{
					float atlasTextureSizeInv = 1.0f / GetAtlasTexture()->GetSizeX();
					spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
				}
			}
		}
		if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingTag))
		{
			this->ReloadTexture();
		}
		if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteTexture))
		{
			this->ReloadTexture();
		}
		if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, useEdgePixelPadding))
		{
			this->ReloadTexture();
		}

		ULGUIEditorManagerObject::RefreshAllUI();
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

void ULGUISpriteData::CheckAndApplySpriteTextureSetting(UTexture2D* InSpriteTexture)
{
	if (
		InSpriteTexture->CompressionSettings != TextureCompressionSettings::TC_EditorIcon
		|| InSpriteTexture->LODGroup != TextureGroup::TEXTUREGROUP_UI
		|| InSpriteTexture->SRGB != true
		)
	{
		InSpriteTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
		InSpriteTexture->LODGroup = TextureGroup::TEXTUREGROUP_UI;
		InSpriteTexture->SRGB = true;
		InSpriteTexture->UpdateResource();
		InSpriteTexture->MarkPackageDirty();
	}
}

void ULGUISpriteData::ReloadTexture()
{
	isInitialized = false;

	atlasTexture = spriteTexture;
	float atlasTextureWidthInv = 1.0f / atlasTexture->GetSurfaceWidth();
	float atlasTextureHeightInv = 1.0f / atlasTexture->GetSurfaceHeight();
	spriteInfo.ApplyUV(0, 0, atlasTexture->GetSurfaceWidth(), atlasTexture->GetSurfaceHeight(), atlasTextureWidthInv, atlasTextureHeightInv);
	spriteInfo.ApplyBorderUV(atlasTextureWidthInv, atlasTextureHeightInv);
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
		if (!packingTag.IsNone())//need to pack to atlas
		{
			if (PackageSprite())
			{
				isInitialized = true;
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ULGUISpriteData::InitSpriteData]PackageSprite fail. Will automatically clear packingTag to make it valid."));
				packingTag = NAME_None;
				this->MarkPackageDirty();
				isInitialized = false;
			}
		}
		else//no need to pack to atlas, so spriteTexture self is the atlas
		{
			atlasTexture = spriteTexture;
			float atlasTextureWidthInv = 1.0f / atlasTexture->GetSurfaceWidth();
			float atlasTextureHeightInv = 1.0f / atlasTexture->GetSurfaceHeight();
			//spriteInfo.ApplyUV(0, 0, atlasTexture->GetSurfaceWidth(), atlasTexture->GetSurfaceHeight(), atlasTextureWidthInv, atlasTextureHeightInv);
			spriteInfo.ApplyBorderUV(atlasTextureWidthInv, atlasTextureHeightInv);
			isInitialized = true;
		}
	}
}

UTexture2D* ULGUISpriteData::GetAtlasTexture()
{
	InitSpriteData();
	return atlasTexture;
}
const FLGUISpriteInfo& ULGUISpriteData::GetSpriteInfo()
{
	InitSpriteData();
	return spriteInfo;
}

bool ULGUISpriteData::IsIndividual()const
{
	return packingTag.IsNone();
}

bool ULGUISpriteData::HavePackingTag()const
{
	return !packingTag.IsNone();
}
const FName& ULGUISpriteData::GetPackingTag()const
{
	return packingTag;
}

ULGUISpriteData* ULGUISpriteData::CreateLGUISpriteData(UObject* Outer, UTexture2D* inSpriteTexture, FVector2D inHorizontalBorder /* = FVector2D::ZeroVector */, FVector2D inVerticalBorder /* = FVector2D::ZeroVector */, FName inPackingTag /* = TEXT("Main") */)
{
	if (!IsValid(inSpriteTexture))
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUISpriteData::CreateLGUISpriteData]Input texture not valid!"));
		return nullptr;
	}
	// check size
	if (inSpriteTexture)
	{
		int32 atlasPadding = 0;
		auto lguiSetting = GetDefault<ULGUISettings>()->defaultAtlasSetting.spaceBetweenSprites;
		if (inSpriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || inSpriteTexture->GetSurfaceHeight() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			FString warningMsg = FString::Printf(TEXT("[ULGUISpriteData::CreateLGUISpriteData]Target texture width or height is too large! Consider use UITexture to render this texture."));
			UE_LOG(LGUI, Warning, TEXT("%s"), *warningMsg);
#if WITH_EDITOR
			LGUIUtils::EditorNotification(FText::FromString(warningMsg));
#endif
		}
		// Apply setting for sprite creation
		//inSpriteTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		CheckAndApplySpriteTextureSetting(inSpriteTexture);
	}

	ULGUISpriteData* result = NewObject<ULGUISpriteData>(IsValid(Outer) ? Outer : GetTransientPackage());
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
	if (!packingTag.IsNone())
	{
		InitSpriteData();
		auto& spriteArray = ULGUIAtlasManager::FindOrAdd(packingTag)->renderSpriteArray;
		spriteArray.AddUnique(InUISprite);
	}
}
void ULGUISpriteData::RemoveUISprite(UUISpriteBase* InUISprite)
{
	if (!packingTag.IsNone())
	{
		if (auto spriteData = ULGUIAtlasManager::Find(packingTag))
		{
			spriteData->renderSpriteArray.RemoveSingle(InUISprite);
		}
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
