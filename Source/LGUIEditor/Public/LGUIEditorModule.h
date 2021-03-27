// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

#include "SlateBasics.h"
#include "UnrealEd.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"

#include "LGUIHeaders.h"

class FToolBarBuilder;
class FMenuBuilder;
DECLARE_LOG_CATEGORY_EXTERN(LGUIEditor, Log, All);

class FLGUIEditorModule : public IModuleInterface
{
public:

	static const FName LGUIAtlasViewerName;

	static FLGUIEditorModule* Instance;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	TSharedRef<SWidget> MakeEditorToolsMenu(bool IsSceneOutlineMenu);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;
private:

	void CreateUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIPostProcessSubMenu(FMenuBuilder& MenuBuilder);
	void BasicSetupSubMenu(FMenuBuilder& MenuBuilder);
	void ReplaceUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void ChangeTraceChannelSubMenu(FMenuBuilder& MenuBuilder);
	void AttachLayout(FMenuBuilder& MenuBuilder);

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);
	void UseActiveViewportAsPreview();
	void ClearViewportPreview();

	bool CanEditActorForPrefab();
private:
	TSharedRef<SDockTab> HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
};