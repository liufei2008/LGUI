// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "AssetTypeAction/LGUIStaticMeshCacheTypeAction.h"
#include "ContentBrowserModule.h"
#include "Extensions/UIStaticMesh.h"

#define LOCTEXT_NAMESPACE "LGUIStaticMeshCacheTypeAction"

FLGUIStaticMeshCacheTypeAction::FLGUIStaticMeshCacheTypeAction(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FLGUIStaticMeshCacheTypeAction::CanFilter()
{
	return true;
}

void FLGUIStaticMeshCacheTypeAction::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FLGUIStaticMeshCacheTypeAction::GetCategories()
{
	return assetType;
}

FText FLGUIStaticMeshCacheTypeAction::GetName()const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUIStaticMeshCacheDataAsset", "LGUI UIStaticMeshCache");
}

UClass* FLGUIStaticMeshCacheTypeAction::GetSupportedClass()const
{
	return ULGUIStaticMeshCacheData::StaticClass();
}

FColor FLGUIStaticMeshCacheTypeAction::GetTypeColor()const
{
	return FColor::White;
}

bool FLGUIStaticMeshCacheTypeAction::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
