// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabTypeAction.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabTypeAction"

FLGUIPrefabTypeAction::FLGUIPrefabTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUIPrefabTypeAction::CanFilter()
{
	return true;
}

void FLGUIPrefabTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUIPrefabTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUIPrefabTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUIPrefabDataAsset", "LGUI Prefab");
}

UClass* FLGUIPrefabTypeAction::GetSupportedClass()const
{
	return ULGUIPrefab::StaticClass();
}

FColor FLGUIPrefabTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUIPrefabTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
