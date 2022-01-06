// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUISpriteData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUISpriteData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUISpriteData"

FAssetTypeActions_LGUISpriteData::FAssetTypeActions_LGUISpriteData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUISpriteData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUISpriteData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUISpriteData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUISpriteData::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUISpriteDataAsset", "LGUI Sprite");
}

UClass* FAssetTypeActions_LGUISpriteData::GetSupportedClass()const
{
	return ULGUISpriteData::StaticClass();
}

FColor FAssetTypeActions_LGUISpriteData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUISpriteData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
