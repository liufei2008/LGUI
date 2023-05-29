// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIRichTextCustomStyleData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIRichTextCustomStyleData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIRichTextCustomStyleData"

FAssetTypeActions_LGUIRichTextCustomStyleData::FAssetTypeActions_LGUIRichTextCustomStyleData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIRichTextCustomStyleData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIRichTextCustomStyleData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIRichTextCustomStyleData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIRichTextCustomStyleData::GetName()const
{
	return LOCTEXT("Name", "LGUI RichText Custom Style Data");
}

UClass* FAssetTypeActions_LGUIRichTextCustomStyleData::GetSupportedClass()const
{
	return ULGUIRichTextCustomStyleData::StaticClass();
}

FColor FAssetTypeActions_LGUIRichTextCustomStyleData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIRichTextCustomStyleData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
