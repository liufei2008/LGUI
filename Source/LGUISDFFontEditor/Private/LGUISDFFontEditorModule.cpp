// Copyright 2019-present LexLiu. All Rights Reserved.

#include "LGUISDFFontEditorModule.h"
#include "LGUISDFFontDataTypeAction.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "LGUISDFFontEditor"
DEFINE_LOG_CATEGORY(LGUISDFFontEditor)

void FLGUISDFFontEditorModule::StartupModule()
{		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	//register asset
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		//register AssetCategory
		EAssetTypeCategories::Type LGUIAssetCategoryBit = AssetTools.FindAdvancedAssetCategory(FName(TEXT("LGUI")));
		if (LGUIAssetCategoryBit == EAssetTypeCategories::Misc)
		{
			LGUIAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("LGUI")), LOCTEXT("LGUIAssetCategory", "LGUI"));
		}

		SDFFontDataTypeAction = MakeShareable(new FLGUISDFFontDataTypeAction(LGUIAssetCategoryBit));
		AssetTools.RegisterAssetTypeActions(SDFFontDataTypeAction.ToSharedRef());
	}
}

void FLGUISDFFontEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetTools")))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.UnregisterAssetTypeActions(SDFFontDataTypeAction.ToSharedRef());
	}
}
	
IMPLEMENT_MODULE(FLGUISDFFontEditorModule, LGUISDFFontEditor)

#undef LOCTEXT_NAMESPACE