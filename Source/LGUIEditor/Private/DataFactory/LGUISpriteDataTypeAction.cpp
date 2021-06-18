// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DataFactory/LGUISpriteDataTypeAction.h"
#include "ContentBrowserModule.h"
#include "Core/LGUISpriteData.h"

#define LOCTEXT_NAMESPACE "LGUISpriteDataTypeAction"

FLGUISpriteDataTypeAction::FLGUISpriteDataTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUISpriteDataTypeAction::CanFilter()
{
	return true;
}

void FLGUISpriteDataTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUISpriteDataTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUISpriteDataTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUISpriteDataAsset", "LGUI Sprite");
}

UClass* FLGUISpriteDataTypeAction::GetSupportedClass()const
{
	return ULGUISpriteData::StaticClass();
}

FColor FLGUISpriteDataTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUISpriteDataTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
