// Copyright 2019-present LexLiu. All Rights Reserved.

#include "LGUISDFFontDataTypeAction.h"
#include "LGUISDFFontData.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "LGUISDFFontDataTypeAction"

FLGUISDFFontDataTypeAction::FLGUISDFFontDataTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUISDFFontDataTypeAction::CanFilter()
{
	return true;
}

void FLGUISDFFontDataTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUISDFFontDataTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUISDFFontDataTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUISDFFontDataAsset", "LGUI SDF Font");
}

UClass* FLGUISDFFontDataTypeAction::GetSupportedClass()const
{
	return ULGUISDFFontData::StaticClass();
}

FColor FLGUISDFFontDataTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUISDFFontDataTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
