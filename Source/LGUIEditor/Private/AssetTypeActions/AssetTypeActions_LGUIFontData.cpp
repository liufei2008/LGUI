// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIFontData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIFontData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIFontData"

FAssetTypeActions_LGUIFontData::FAssetTypeActions_LGUIFontData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIFontData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIFontData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIFontData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIFontData::GetName()const
{
	return LOCTEXT("Name", "LGUI Font");
}

UClass* FAssetTypeActions_LGUIFontData::GetSupportedClass()const
{
	return ULGUIFontData::StaticClass();
}

FColor FAssetTypeActions_LGUIFontData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIFontData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
