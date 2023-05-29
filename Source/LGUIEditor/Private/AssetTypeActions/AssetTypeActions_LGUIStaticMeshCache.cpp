// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions/AssetTypeActions_LGUIStaticMeshCache.h"
#include "ContentBrowserModule.h"
#include "Extensions/UIStaticMesh.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIStaticMeshCache"

FAssetTypeActions_LGUIStaticMeshCache::FAssetTypeActions_LGUIStaticMeshCache(EAssetTypeCategories::Type InAssetType)
	: assetType(InAssetType)
{

}

bool FAssetTypeActions_LGUIStaticMeshCache::CanFilter()
{
	return true;
}

void FAssetTypeActions_LGUIStaticMeshCache::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);
}

uint32 FAssetTypeActions_LGUIStaticMeshCache::GetCategories()
{
	return assetType;
}

FText FAssetTypeActions_LGUIStaticMeshCache::GetName()const
{
	return LOCTEXT("Name", "LGUI StaticMesh Cache");
}

UClass* FAssetTypeActions_LGUIStaticMeshCache::GetSupportedClass()const
{
	return ULGUIStaticMeshCacheData::StaticClass();
}

FColor FAssetTypeActions_LGUIStaticMeshCache::GetTypeColor()const
{
	return FColor::White;
}

bool FAssetTypeActions_LGUIStaticMeshCache::HasActions(const TArray<UObject*>& InObjects)const
{
	return true;
}


#undef LOCTEXT_NAMESPACE
