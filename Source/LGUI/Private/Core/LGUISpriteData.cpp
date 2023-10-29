// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUISpriteData.h"
#include "LGUI.h"
#include "Core/LGUISettings.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIDynamicSpriteAtlasData.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Engine.h"
#include "Utils/LGUIUtils.h"
#include "Core/Actor/LGUIManager.h"
#include "RHI.h"
#include "Rendering/Texture2DResource.h"
#include "TextureCompiler.h"
#include "Utils/LGUIUtils.h"
#include "RenderingThread.h"

#define LOCTEXT_NAMESPACE "LGUISpriteData"

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

bool ULGUISpriteData::InsertTexture(FLGUIDynamicSpriteAtlasData* InAtlasData)
{
#if WITH_EDITOR
	int32 spaceBetweenSprites = ULGUISettings::GetAtlasTexturePadding(packingTag);
#else
	static int32 spaceBetweenSprites = ULGUISettings::GetAtlasTexturePadding(packingTag);
#endif
	auto SizeX = InAtlasData->atlasTexture->GetSizeX();
	check(SizeX != 0);
	float atlasTextureSizeInv = 1.0f / SizeX;

	auto method = rbp::MaxRectsBinPack::RectBestAreaFit;
#if WITH_EDITOR
	FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
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
	if (spriteTexture->GetResource() != nullptr && atlasTexture->GetResource() != nullptr)
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
		RegionData->SpriteTextureResource = (FTexture2DResource*)spriteTexture->GetResource();
		RegionData->AtlasTextureResource = (FTexture2DResource*)atlasTexture->GetResource();
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

	auto atlasData = ULGUIDynamicSpriteAtlasManager::FindOrAdd(packingTag);
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
		UE_LOG(LGUI, Log, TEXT("[%s].%d Insert texture:%s expend size to %d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(spriteTexture->GetPathName()), newTextureSize);
		if (newTextureSize > WARNING_ATLAS_SIZE)
		{
			auto warningMsg = FText::Format(LOCTEXT("PackageSprite_AtlasSize_Warning", "{0} Trying to insert texture:{1}, result to expend size to:{2} larger than the preferred maximun texture size:{3}!\
\nTry reduce some sprite texture size, or use UITexture to render some large texture, or use different packingTag to split your atlasTexture.\
\nAlso remember to dispose unused atlas by call function DisposeAtlasByPackingTag from LGUIDynamicSpriteAtlasManager.\
")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
				, FText::FromString(spriteTexture->GetPathName())
				, newTextureSize, WARNING_ATLAS_SIZE);
			UE_LOG(LGUI, Warning, TEXT("%s"), *warningMsg.ToString());
#if WITH_EDITOR
			LGUIUtils::EditorNotification(warningMsg);
#endif
		}
		if ((uint32)newTextureSize > GetMax2DTextureDimension())
		{
			auto warningMsg = FText::Format(LOCTEXT("PackageSprite_AtlasSize_Error", "{0} Trying to insert texture:{1}, result too large size that not supported! Maximun texture size is:{2}.")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
				, FText::FromString(spriteTexture->GetPathName()), GetMax2DTextureDimension());
			UE_LOG(LGUI, Error, TEXT("%s"), *warningMsg.ToString());
#if WITH_EDITOR
			LGUIUtils::EditorNotification(warningMsg);
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

void ULGUISpriteData::ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv)
{
	spriteInfo.ApplyUV(InPackedRect.x, InPackedRect.y, InPackedRect.width, InPackedRect.height, InAtlasTextureSizeInv, InAtlasTextureSizeInv);
	spriteInfo.ApplyBorderUV(InAtlasTextureSizeInv, InAtlasTextureSizeInv);
	isInitialized = false;
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
#if WITH_EDITOR
				FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
			}
		}
		else if (
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderLeft) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderRight) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderTop) ||
			propertyName == GET_MEMBER_NAME_CHECKED(FLGUISpriteInfo, borderBottom)
			)
		{
			//sprite data, apply border
			if (spriteTexture != nullptr)
			{
#if WITH_EDITOR
				FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
				spriteInfo.width = spriteTexture->GetSizeX();
				spriteInfo.height = spriteTexture->GetSizeY();
				if (isInitialized)
				{
					float atlasTextureSizeInv = 1.0f / GetAtlasTexture()->GetSizeX();
					spriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
				}
			}
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingTag))
		{
			this->ReloadTexture();
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, spriteTexture))
		{
			this->ReloadTexture();
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, useEdgePixelPadding))
		{
			this->ReloadTexture();
		}
		else if (propertyName == GET_MEMBER_NAME_CHECKED(ULGUISpriteData, packingAtlas))
		{
			this->ReloadTexture();
		}

		ULGUIManagerWorldSubsystem::RefreshAllUI();
	}
}
bool ULGUISpriteData::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(ULGUISpriteData, packingTag))
		{
			return IsValid(packingAtlas);
		}
	}
	return Super::CanEditChange(InProperty);
}
void ULGUISpriteData::MarkAllSpritesNeedToReinitialize()
{
	ULGUIDynamicSpriteAtlasManager::ResetAtlasMap();
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

#if WITH_EDITOR
	if (IsValid(packingAtlas))
	{
		packingAtlas->MarkNotInitialized();
	}
#endif

#if WITH_EDITOR
	FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
	atlasTexture = spriteTexture;
	auto SizeX = atlasTexture->GetSizeX();
	auto SizeY = atlasTexture->GetSizeY();
	check(SizeX != 0 && SizeY != 0);
	float atlasTextureWidthInv = 1.0f / SizeX;
	float atlasTextureHeightInv = 1.0f / SizeY;
	spriteInfo.ApplyUV(0, 0, SizeX, SizeY, atlasTextureWidthInv, atlasTextureHeightInv);
	spriteInfo.ApplyBorderUV(atlasTextureWidthInv, atlasTextureHeightInv);
}

void ULGUISpriteData::InitSpriteData()
{
	if (!isInitialized)
	{
		if (IsValid(packingAtlas))
		{
#if WITH_EDITOR
			//add it again as check if it exist in packingAtlas
			packingAtlas->AddSpriteData(this);
#endif
			if (packingAtlas->InitCheck())
			{
				atlasTexture = packingAtlas->GetAtlasTexture();
				//no need to set spriteInfo because it is already set when do static pack
				return;
			}
			else
			{
				UE_LOG(LGUI, Error, TEXT("[%s].%d PackingAtlas:%s pack error, will fallback to use PackingTag!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(packingAtlas->GetPathName()));
			}
		}
		if (spriteTexture == nullptr)
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d SpriteData:%s spriteTexture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
			return;
		}
		if (!packingTag.IsNone())//need to pack to atlas
		{
#if WITH_EDITOR
			FTextureCompilingManager::Get().FinishCompilation({ spriteTexture });
#endif
			if (PackageSprite())
			{
				isInitialized = true;
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[%s].%d PackageSprite fail. Will automatically clear packingTag to make it valid."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
				packingTag = NAME_None;
				this->MarkPackageDirty();
				isInitialized = false;
			}
		}
		else//no need to pack to atlas, so spriteTextire self is the atlas
		{
			atlasTexture = spriteTexture;
			auto SizeX = atlasTexture->GetSizeX();
			auto SizeY = atlasTexture->GetSizeY();
			check(SizeX != 0 && SizeY != 0);
			float atlasTextureWidthInv = 1.0f / SizeX;
			float atlasTextureHeightInv = 1.0f / SizeY;
			//spriteInfo.ApplyUV(0, 0, atlasTexture->GetSizeX(), atlasTexture->GetSizeY(), atlasTextureWidthInv, atlasTextureHeightInv);
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
	return !IsValid(packingAtlas) && packingTag.IsNone();
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
		UE_LOG(LGUI, Error, TEXT("[%s].%d Input texture not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	// check size
	if (inSpriteTexture)
	{
		int32 atlasPadding = 0;
		auto lguiSetting = GetDefault<ULGUISettings>()->defaultAtlasSetting.spaceBetweenSprites;
		if (inSpriteTexture->GetSizeX() + atlasPadding * 2 > WARNING_ATLAS_SIZE || inSpriteTexture->GetSizeY() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			auto warningMsg = FText::Format(LOCTEXT("CreateLGUISpriteData_Size_Warning", "{0} Target texture width or height is too large! Consider use UITexture to render this texture.")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
			UE_LOG(LGUI, Warning, TEXT("%s"), *warningMsg.ToString());
#if WITH_EDITOR
			LGUIUtils::EditorNotification(warningMsg);
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
		spriteInfo.width = inSpriteTexture->GetSizeX();
		spriteInfo.height = inSpriteTexture->GetSizeY();
		spriteInfo.borderLeft = (uint16)inHorizontalBorder.X;
		spriteInfo.borderRight = (uint16)inHorizontalBorder.Y;
		spriteInfo.borderTop = (uint16)inVerticalBorder.X;
		spriteInfo.borderBottom = (uint16)inVerticalBorder.Y;
	}
	return result;
}

void ULGUISpriteData::AddUISprite(TScriptInterface<class IUISpriteRenderableInterface> InUISprite)
{
	if (IsValid(packingAtlas))
	{
		InitSpriteData();
#if WITH_EDITOR
		//packingAtlas only need to collect sprite in editor
		packingAtlas->AddRenderSprite(InUISprite);
#endif
	}
	else if (!packingTag.IsNone())
	{
		InitSpriteData();
		auto& spriteArray = ULGUIDynamicSpriteAtlasManager::FindOrAdd(packingTag)->renderSpriteArray;
		spriteArray.AddUnique(InUISprite.GetObject());
	}
}
void ULGUISpriteData::RemoveUISprite(TScriptInterface<class IUISpriteRenderableInterface> InUISprite)
{
	if (IsValid(packingAtlas))
	{
#if WITH_EDITOR
		//packingAtlas only need to collect sprite in editor
		packingAtlas->RemoveRenderSprite(InUISprite);
#endif
	}
	else if (!packingTag.IsNone())
	{
		if (auto spriteData = ULGUIDynamicSpriteAtlasManager::Find(packingTag))
		{
			spriteData->renderSpriteArray.RemoveSingle(InUISprite.GetObject());
		}
	}
}
bool ULGUISpriteData::ReadPixel(const FVector2D& InUV, FColor& OutPixel)const
{
	if (packingAtlas != nullptr)
	{
		return packingAtlas->ReadPixel(InUV, OutPixel);
	}
	return false;
}
bool ULGUISpriteData::SupportReadPixel()const
{
	return packingAtlas != nullptr;
}

ULGUISpriteData* ULGUISpriteData::GetDefaultWhiteSolid()
{
	static auto defaultWhiteSolid = LoadObject<ULGUISpriteData>(NULL, TEXT("/LGUI/LGUIPreset_WhiteSolid"));
	if (defaultWhiteSolid == nullptr)
	{
		auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load default sprite error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
		return nullptr;
	}
	return defaultWhiteSolid;
}

#undef LOCTEXT_NAMESPACE
