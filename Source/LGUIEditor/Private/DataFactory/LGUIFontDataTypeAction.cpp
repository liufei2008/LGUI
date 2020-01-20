// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIFontDataTypeAction.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIFontData.h"

#define LOCTEXT_NAMESPACE "LGUIFontDataTypeAction"

FLGUIFontDataTypeAction::FLGUIFontDataTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUIFontDataTypeAction::CanFilter()
{
	return true;
}

void FLGUIFontDataTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUIFontDataTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUIFontDataTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_FontDataAsset", "LGUIFont");
}

UClass* FLGUIFontDataTypeAction::GetSupportedClass()const
{
	return ULGUIFontData::StaticClass();
}

FColor FLGUIFontDataTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUIFontDataTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
