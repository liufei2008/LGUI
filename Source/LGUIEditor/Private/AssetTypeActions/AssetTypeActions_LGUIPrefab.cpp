// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIPrefab.h"
#include "Misc/PackageName.h"
#include "EditorStyleSet.h"
#include "EditorFramework/AssetImportData.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "AssetNotifications.h"
#include "Algo/Transform.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "LGUIEditorModule.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabTypeAction"

FAssetTypeActions_LGUIPrefab::FAssetTypeActions_LGUIPrefab(EAssetTypeCategories::Type InAssetType)
: FAssetTypeActions_Base(), AssetType(InAssetType)
{

}

FText FAssetTypeActions_LGUIPrefab::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_LGUIPrefabDataAsset", "LGUI Prefab");
}

UClass* FAssetTypeActions_LGUIPrefab::GetSupportedClass() const
{
	return ULGUIPrefab::StaticClass();
}

void FAssetTypeActions_LGUIPrefab::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
{
	//FAssetTypeActions_Base::OpenAssetEditor(InObjects, EditWithinLevelEditor);

	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		if (auto LGUIPrefab = Cast<ULGUIPrefab>(*ObjIt))
		{
			TSharedRef<FLGUIPrefabEditor> NewPrefabEditor(new FLGUIPrefabEditor());
			NewPrefabEditor->InitPrefabEditor(Mode, EditWithinLevelEditor, LGUIPrefab);
		}
	}
}

uint32 FAssetTypeActions_LGUIPrefab::GetCategories()
{
	return AssetType;
}

bool FAssetTypeActions_LGUIPrefab::CanFilter()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
