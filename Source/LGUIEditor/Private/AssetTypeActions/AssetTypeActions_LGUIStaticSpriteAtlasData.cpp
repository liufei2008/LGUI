// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIStaticSpriteAtlasData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIStaticSpriteAtlasData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIStaticSpriteAtlasData"

FAssetTypeActions_LGUIStaticSpriteAtlasData::FAssetTypeActions_LGUIStaticSpriteAtlasData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIStaticSpriteAtlasData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIStaticSpriteAtlasData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIStaticSpriteAtlasData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIStaticSpriteAtlasData::GetName()const
{
	return LOCTEXT("Name", "LGUI Static Sprite Atlas Data");
}

UClass* FAssetTypeActions_LGUIStaticSpriteAtlasData::GetSupportedClass()const
{
	return ULGUIStaticSpriteAtlasData::StaticClass();
}

FColor FAssetTypeActions_LGUIStaticSpriteAtlasData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIStaticSpriteAtlasData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
