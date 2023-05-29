// Copyright 2019-present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUISDFFontData.h"
#include "Core/LGUISDFFontData.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUISDFFontData"

FAssetTypeActions_LGUISDFFontData::FAssetTypeActions_LGUISDFFontData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUISDFFontData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUISDFFontData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUISDFFontData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUISDFFontData::GetName()const
{
	return LOCTEXT("Name", "LGUI SDF Font");
}

UClass* FAssetTypeActions_LGUISDFFontData::GetSupportedClass()const
{
	return ULGUISDFFontData::StaticClass();
}

FColor FAssetTypeActions_LGUISDFFontData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUISDFFontData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
