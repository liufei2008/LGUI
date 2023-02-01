// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIEmojiData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIEmojiData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIEmojiData"

FAssetTypeActions_LGUIEmojiData::FAssetTypeActions_LGUIEmojiData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIEmojiData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIEmojiData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIEmojiData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIEmojiData::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUIEmojiDataAsset", "LGUI Emoji data");
}

UClass* FAssetTypeActions_LGUIEmojiData::GetSupportedClass()const
{
	return ULGUIEmojiData::StaticClass();
}

FColor FAssetTypeActions_LGUIEmojiData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIEmojiData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
