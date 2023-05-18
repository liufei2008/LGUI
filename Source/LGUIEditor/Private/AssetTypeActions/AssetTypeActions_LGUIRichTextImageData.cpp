// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIRichTextImageData.h"
#include "ContentBrowserModule.h"
#include "Core/LGUIRichTextImageData.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIRichTextImageData"

FAssetTypeActions_LGUIRichTextImageData::FAssetTypeActions_LGUIRichTextImageData(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIRichTextImageData::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIRichTextImageData::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIRichTextImageData::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIRichTextImageData::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUIRichTextImageDataAsset", "LGUI RichText Image Data");
}

UClass* FAssetTypeActions_LGUIRichTextImageData::GetSupportedClass()const
{
	return ULGUIRichTextImageData::StaticClass();
}

FColor FAssetTypeActions_LGUIRichTextImageData::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIRichTextImageData::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
