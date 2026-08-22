// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUISpriteData.h"
#include "LGUI.h"
#include "Core/LexUISettings.h"
#include "Core/Components/LexSpriteBase.h"
#include "Core/LexUIDynamicSpriteAtlasData.h"
#include "Core/LexUIStaticSpriteAtlasData.h"
#include "UObject/UObjectIterator.h"
#include "Engine/Engine.h"
#include "Utils/LexUIUtils.h"
#include "Core/LexUIManager.h"
#include "RHI.h"
#include "TextureCompiler.h"
#include "RenderingThread.h"

#define LOCTEXT_NAMESPACE "LGUISpriteData"

bool FLexUISpriteInfo::ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal)
{
#if WITH_EDITOR
	PosX = InX;
	PosY = InY;
#endif
	auto NewMinUV = FVector2f(InX * texFullWidthReciprocal, InY * texFullHeightReciprocal);
	auto NewMaxUV = FVector2f((InX + InWidth) * texFullWidthReciprocal, (InY + InHeight) * texFullHeightReciprocal);
	
	if (Width == InWidth && Height == InHeight && MinUV == NewMinUV && MaxUV == NewMaxUV)
		return false;
	
	Width = InWidth;
	Height = InHeight;
	MinUV = NewMinUV;
	MaxUV = NewMaxUV;
	return true;
}
bool FLexUISpriteInfo::ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal, const FVector4f& uvRect)
{
#if WITH_EDITOR
	PosX = InX;
	PosY = InY;
#endif
	auto NewMinUV = FVector2f(InX * texFullWidthReciprocal + uvRect.X, InY * texFullHeightReciprocal + uvRect.Y);
	auto NewMaxUV = FVector2f((InX + InWidth) * texFullWidthReciprocal * uvRect.Z + uvRect.X, (InY + InHeight) * texFullHeightReciprocal * uvRect.W + uvRect.Y);
	
	if (Width == InWidth && Height == InHeight && MinUV == NewMinUV && MaxUV == NewMaxUV)
		return false;
	
	Width = InWidth;
	Height = InHeight;
	MinUV = NewMinUV;
	MaxUV = NewMaxUV;
	return true;
}
bool FLexUISpriteInfo::HasBorder()const
{
	return Border.Left != 0 || Border.Right != 0 || Border.Top != 0 || Border.Bottom != 0;
}
bool FLexUISpriteInfo::HasPadding()const
{
	return Padding.Left != 0 || Padding.Right != 0 || Padding.Top != 0 || Padding.Bottom != 0;
}
bool FLexUISpriteInfo::ApplyBorderUV(float texFullWidthReciprocal, float texFullHeightReciprocal)
{
	auto NewBorderMinUV = FVector2f(MinUV.X + Border.Left * texFullWidthReciprocal, MinUV.Y + Border.Top * texFullHeightReciprocal);
	auto NewBorderMaxUV = FVector2f(MaxUV.X - Border.Right * texFullWidthReciprocal, MaxUV.Y - Border.Bottom * texFullHeightReciprocal);
	if (BorderMinUV == NewBorderMinUV && BorderMaxUV == NewBorderMaxUV)
		return false;
	BorderMinUV = NewBorderMinUV;
	BorderMaxUV = NewBorderMaxUV;
	return true;
}

void FLexUISpriteInfo::ApplyFlip(FLexUISpriteInfo& Result, bool bFlipH, bool bFlipV)
{
	if (bFlipH)
	{
		Swap(Result.MinUV.X, Result.MaxUV.X);
		Swap(Result.BorderMinUV.X, Result.BorderMaxUV.X);
		Swap(Result.Border.Left, Result.Border.Right);
		Swap(Result.Padding.Left, Result.Padding.Right);
	}
	if (bFlipV)
	{
		Swap(Result.MinUV.Y, Result.MaxUV.Y);
		Swap(Result.BorderMinUV.Y, Result.BorderMaxUV.Y);
		Swap(Result.Border.Top, Result.Border.Bottom);
		Swap(Result.Padding.Top, Result.Padding.Bottom);
	}
}

ULexUISpriteData::ULexUISpriteData()
{
	// PackingAtlas = LoadObject<ULexUIStaticSpriteAtlasData>(NULL, TEXT("/LGUI/DefaultSpriteAtlasData"));
}

bool ULexUISpriteData::PackSprite()
{
	CheckAndApplySpriteTextureSetting(SpriteTexture);

	auto AtlasData = ULexUIDynamicSpriteAtlasManager::FindOrAdd(PackingTag);
	AtlasData->EnsureAtlasTexture();
#if WITH_EDITOR
	FTextureCompilingManager::Get().FinishCompilation({ SpriteTexture });
#endif
	if (AtlasData->PackSprite(this))
	{
		return true;
	}
	else//all area cannot fit the texture, then show a warning
	{
		auto WarningMsg = FText::Format(LOCTEXT("PackageSprite_AtlasSize_Warning", "{0} Can't pack sprite texture:{1}!"
"\nTry reduce sprite texture size, or maybe don't use it as sprite."
"\nAlso remember to dispose unused atlas by call function DisposeAtlasByPackingTag from {2}."
)
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
			, FText::FromString(SpriteTexture->GetPathName())
			, FText::FromString(ULexUIDynamicSpriteAtlasManager::StaticClass()->GetName())
			);
		UE_LOG(LGUI, Warning, TEXT("%s"), *WarningMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(WarningMsg, false);
#endif
		return false;
	}
}

void ULexUISpriteData::CheckSpriteTexture()
{
	if (SpriteTexture == nullptr)
	{
		SpriteTexture = FLexUIUtils::GetDefaultWhiteTexture();
	}
}

bool ULexUISpriteData::ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv)
{
	bool bBaseInfoDirty = SpriteInfo.ApplyUV(InPackedRect.x, InPackedRect.y, InPackedRect.width, InPackedRect.height, InAtlasTextureSizeInv, InAtlasTextureSizeInv);
	bool bBorderInfoDirty = SpriteInfo.ApplyBorderUV(InAtlasTextureSizeInv, InAtlasTextureSizeInv);
	bIsInitialized = false;
	return bBaseInfoDirty || bBorderInfoDirty;
}
#if WITH_EDITOR
void ULexUISpriteData::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	auto PropertyName = PropertyAboutToChange->GetFName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, PackingAtlas))
	{
		if (IsValid(PackingAtlas))
		{
			PackingAtlas->RemoveSpriteData(this);
			PackingAtlas->MarkAtlasPackDirty();
		}
	}
}

void ULexUISpriteData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	CheckSpriteTexture();
	if (PropertyChangedEvent.Property != nullptr)
	{
		auto PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, SpriteTexture))
		{
			if (PackingType == ELexUISpritePackingType::Static && PackingAtlas)
			{
				PackingAtlas->MarkAtlasPackDirty();
			}
			if (SpriteTexture != nullptr)
			{
				CheckAndApplySpriteTextureSetting(SpriteTexture);
#if WITH_EDITOR
				FTextureCompilingManager::Get().FinishCompilation({ SpriteTexture });
#endif
				SpriteInfo.Width = SpriteTexture->GetSizeX();
				SpriteInfo.Height = SpriteTexture->GetSizeY();
			}
			this->ReloadTexture();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, PackingTag))
		{
			this->ReloadTexture();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, bUseEdgePixelPadding))
		{
			if (PackingType == ELexUISpritePackingType::Static && PackingAtlas)
			{
				PackingAtlas->MarkAtlasPackDirty();
			}
			this->ReloadTexture();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, PackingAtlas))
		{
			if (IsValid(PackingAtlas))
			{
				if (!PackingAtlas->ContainsSpriteData(this))
				{
					PackingAtlas->AddSpriteData(this);
					PackingAtlas->MarkAtlasPackDirty();
				}
			}
			if (auto DynamicSpriteAtlasData = ULexUIDynamicSpriteAtlasManager::Find(PackingTag))
			{
				DynamicSpriteAtlasData->CheckSprite();
			}
			this->ReloadTexture();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexUISpriteData, PackingType))
		{
			if (PackingType == ELexUISpritePackingType::Static && PackingAtlas)
			{
				PackingAtlas->MarkAtlasPackDirty();
			}
			this->ReloadTexture();
		}

		ULexUIManagerWorldSubsystem::RefreshAllUI();
	}
}

void ULexUISpriteData::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	FString PropertyPath;
	auto PropNode = PropertyChangedEvent.PropertyChain.GetHead();
	while (PropNode != nullptr)
	{
		PropertyPath += PropNode->GetValue()->GetName();
		PropNode = PropNode->GetNextNode();
		if (PropNode)
		{
			PropertyPath += ".";
		}
	}
	if (SpriteTexture != nullptr)
	{
#if WITH_EDITOR
		FTextureCompilingManager::Get().FinishCompilation({ SpriteTexture });
#endif
		SpriteInfo.Width = SpriteTexture->GetSizeX();
		SpriteInfo.Height = SpriteTexture->GetSizeY();
		if (bIsInitialized)
		{
			float atlasTextureSizeInv = 1.0f / GetAtlasTexture()->GetSizeX();
			SpriteInfo.ApplyUV(SpriteInfo.PosX, SpriteInfo.PosY, SpriteInfo.Width, SpriteInfo.Height, atlasTextureSizeInv, atlasTextureSizeInv);
			SpriteInfo.ApplyBorderUV(atlasTextureSizeInv, atlasTextureSizeInv);
		}
	}
}

bool ULexUISpriteData::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}

void ULexUISpriteData::BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)
{
}

void ULexUISpriteData::MarkAllSpritesNeedToReinitialize()
{
	ULexUIDynamicSpriteAtlasManager::ResetAtlasMap();
	for (TObjectIterator<ULexUISpriteData> SpriteItr; SpriteItr; ++SpriteItr)
	{
		SpriteItr->bIsInitialized = false;
	}
}
#endif

void ULexUISpriteData::CheckAndApplySpriteTextureSetting(UTexture2D* InSpriteTexture)
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

void ULexUISpriteData::ReloadTexture()
{
	bIsInitialized = false;

#if WITH_EDITOR
	if (PackingType == ELexUISpritePackingType::Static && IsValid(PackingAtlas))
	{
		PackingAtlas->MarkAtlasPackDirty();
		PackingAtlas->MarkNotInitialized();
	}
#endif

#if WITH_EDITOR
	FTextureCompilingManager::Get().FinishCompilation({ SpriteTexture });
#endif
	AtlasTexture = SpriteTexture;
	auto SizeX = AtlasTexture->GetSizeX();
	auto SizeY = AtlasTexture->GetSizeY();
	check(SizeX != 0 && SizeY != 0);
	float atlasTextureWidthInv = 1.0f / SizeX;
	float atlasTextureHeightInv = 1.0f / SizeY;
	SpriteInfo.ApplyUV(0, 0, SizeX, SizeY, atlasTextureWidthInv, atlasTextureHeightInv);
	SpriteInfo.ApplyBorderUV(atlasTextureWidthInv, atlasTextureHeightInv);
}

void ULexUISpriteData::InitSpriteData()
{
	if (!bIsInitialized)
	{
#if WITH_EDITOR
		if (IsRunningCookCommandlet())
		{
			bIsInitialized = true;
			return;
		}
#endif
		if (PackingType == ELexUISpritePackingType::Static)
		{
			if (IsValid(PackingAtlas))
			{
#if WITH_EDITOR
				if (!PackingAtlas->ContainsSpriteData(this))
				{
					PackingAtlas->AddSpriteData(this);
					PackingAtlas->MarkAtlasPackDirty();
				}
#endif
				AtlasTexture = PackingAtlas->GetAtlasTexture(AtlasTextureIndex);
				if (AtlasTexture)
				{
					bIsInitialized = true;
				}
				else
				{
					UE_LOG(LGUI, Warning, TEXT("[%s].%d SpriteData:%s AtlasTexture is null! "), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
				}
			}
		}
		else
		{
			if (SpriteTexture == nullptr)
			{
				UE_LOG(LGUI, Error, TEXT("[%s].%d SpriteData:%s SpriteTexture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(this->GetPathName()));
				return;
			}
			if (!PackingTag.IsNone())//need to pack to atlas
			{
#if WITH_EDITOR
				FTextureCompilingManager::Get().FinishCompilation({ SpriteTexture });
#endif
				if (PackSprite())
				{
					bIsInitialized = true;
				}
				else
				{
					auto WarningMsg = FString::Printf(TEXT("[%s].%d Pack Sprite fail. Will automatically clear PackingTag to make it valid."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
					UE_LOG(LGUI, Warning, TEXT("%s"), *WarningMsg);
#if WITH_EDITOR
					FLexUIUtils::EditorNotification(FText::FromString(WarningMsg), false);
#endif
					PackingTag = NAME_None;
					this->MarkPackageDirty();
					bIsInitialized = false;
				}
			}
			else//no need to pack to atlas, so spriteTexture self is the atlas
			{
				AtlasTexture = SpriteTexture;
				auto SizeX = AtlasTexture->GetSizeX();
				auto SizeY = AtlasTexture->GetSizeY();
				check(SizeX != 0 && SizeY != 0);
				float atlasTextureWidthInv = 1.0f / SizeX;
				float atlasTextureHeightInv = 1.0f / SizeY;
				//spriteInfo.ApplyUV(0, 0, AtlasTexture->GetSizeX(), AtlasTexture->GetSizeY(), atlasTextureWidthInv, atlasTextureHeightInv);
				SpriteInfo.ApplyBorderUV(atlasTextureWidthInv, atlasTextureHeightInv);
				bIsInitialized = true;
			}
		}
	}
}

UTexture2D* ULexUISpriteData::GetAtlasTexture()
{
	InitSpriteData();
	return AtlasTexture;
}
const FLexUISpriteInfo& ULexUISpriteData::GetSpriteInfo()
{
	InitSpriteData();
	return SpriteInfo;
}

bool ULexUISpriteData::IsIndividual()const
{
	return !IsValid(PackingAtlas) && PackingTag.IsNone();
}

bool ULexUISpriteData::HavePackingTag()const
{
	return !PackingTag.IsNone();
}
const FName& ULexUISpriteData::GetPackingTag()const
{
	return PackingTag;
}

ULexUISpriteData* ULexUISpriteData::CreateLexUISpriteData(UObject* Outer, UTexture2D* InSpriteTexture, FMargin InBorder, FName InPackingTag /* = TEXT("Main") */)
{
	if (!IsValid(InSpriteTexture))
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Input texture not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return nullptr;
	}
	// check size
	int32 atlasPadding = ULexUISettings::GetAtlasTexturePadding(InPackingTag);
	if (InSpriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || InSpriteTexture->GetSurfaceHeight() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
	{
		auto warningMsg = FText::Format(LOCTEXT("CreateLexUISpriteData_Size_Warning", "{0} Target texture width or height is too large! Consider use LexImage to render this texture.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Warning, TEXT("%s"), *warningMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(warningMsg, false);
#endif
	}
	// Apply setting for Sprite creation
	CheckAndApplySpriteTextureSetting(InSpriteTexture);

	ULexUISpriteData* result = NewObject<ULexUISpriteData>(IsValid(Outer) ? Outer : GetTransientPackage());
	result->PackingTag = InPackingTag;
	result->SpriteTexture = InSpriteTexture;
	auto& spriteInfo = result->SpriteInfo;
	spriteInfo.Width = InSpriteTexture->GetSizeX();
	spriteInfo.Height = InSpriteTexture->GetSizeY();
	spriteInfo.Border = InBorder;
	return result;
}

void ULexUISpriteData::AddUISprite(TScriptInterface<class ILexUISpriteRenderInterface> InUISprite)
{
	if (PackingType == ELexUISpritePackingType::Static)
	{
		if (IsValid(PackingAtlas))
		{
#if WITH_EDITOR
			//packingAtlas only need to collect Sprite in editor
			PackingAtlas->AddRenderSprite(InUISprite);
#endif
		}
	}
	else
	{
		if (!PackingTag.IsNone())
		{
			auto& spriteArray = ULexUIDynamicSpriteAtlasManager::FindOrAdd(PackingTag)->RenderSpriteArray;
			spriteArray.AddUnique(InUISprite.GetObject());
		}
	}
}
void ULexUISpriteData::RemoveUISprite(TScriptInterface<class ILexUISpriteRenderInterface> InUISprite)
{
	if (IsValid(PackingAtlas))
	{
#if WITH_EDITOR
		//packingAtlas only need to collect Sprite in editor
		PackingAtlas->RemoveRenderSprite(InUISprite);
#endif
	}
	else if (!PackingTag.IsNone())
	{
		if (auto spriteData = ULexUIDynamicSpriteAtlasManager::Find(PackingTag))
		{
			spriteData->RenderSpriteArray.RemoveSingle(InUISprite.GetObject());
		}
	}
}
bool ULexUISpriteData::ReadPixel(const FVector2D& InUV, FColor& OutPixel)const
{
	if (PackingAtlas != nullptr)
	{
		return PackingAtlas->ReadPixel(AtlasTextureIndex, InUV, OutPixel);
	}
	return false;
}
bool ULexUISpriteData::SupportReadPixel()const
{
	return PackingAtlas != nullptr;
}

ULexUISpriteData* ULexUISpriteData::GetDefaultWhiteSolid()
{
	static auto defaultWhiteSolid = LoadObject<ULexUISpriteData>(NULL, TEXT("/LGUI/LexUIPreset_WhiteSolid"));
	if (defaultWhiteSolid == nullptr)
	{
		auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load default Sprite error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(errMsg, false, 10);
#endif
		return nullptr;
	}
	return defaultWhiteSolid;
}
ULexUISpriteData* ULexUISpriteData::GetDefaultFrameRect()
{
	static auto defaultFrameRect = LoadObject<ULexUISpriteData>(NULL, TEXT("/LGUI/LexUIPreset_Rect_Sprite"));
	if (defaultFrameRect == nullptr)
	{
		auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load default sprite error! Missing some content of LexUI plugin, reinstall this plugin may fix the issue.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(errMsg, false, 10);
#endif
		return nullptr;
	}
	return defaultFrameRect;
}


#undef LOCTEXT_NAMESPACE
