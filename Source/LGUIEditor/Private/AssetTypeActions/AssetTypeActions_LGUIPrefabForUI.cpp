// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "AssetTypeActions_LGUIPrefabForUI.h"
#include "Misc/PackageName.h"
#include "EditorStyleSet.h"
#include "EditorFramework/AssetImportData.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "ContentBrowserModule.h"
#include "AssetNotifications.h"
#include "Algo/Transform.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabEditor/LGUIPrefabEditor.h"
#include "DataFactory/LGUIPrefabFactoryForUI.h"
#include "LGUIEditorModule.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_LGUIPrefabForUI"

FAssetTypeActions_LGUIPrefabForUI::FAssetTypeActions_LGUIPrefabForUI(EAssetTypeCategories::Type InAssetType)
: FAssetTypeActions_Base(), AssetType(InAssetType)
{

}

FText FAssetTypeActions_LGUIPrefabForUI::GetName() const
{
	return LOCTEXT("Name", "LGUI Prefab");
}

UClass* FAssetTypeActions_LGUIPrefabForUI::GetSupportedClass() const
{
	return ULGUIPrefabForUI::StaticClass();
}

void FAssetTypeActions_LGUIPrefabForUI::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor )
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

uint32 FAssetTypeActions_LGUIPrefabForUI::GetCategories()
{
	return AssetType;
}

bool FAssetTypeActions_LGUIPrefabForUI::CanFilter()
{
	return true;
}

#undef LOCTEXT_NAMESPACE
