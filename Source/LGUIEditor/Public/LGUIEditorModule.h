// Copyright 2019-2020 LexLiu. All Rights Reserved.

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

	static const FName LGUIEditorToolsTabName;
	static const FName LGUIEventComponentSelectorName;
	static const FName LGUIEventFunctionSelectorName;
	static const FName LGUIAtlasViewerName;

	static FLGUIEditorModule* Instance;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	TSharedRef<SWidget> MakeEditorToolsMenu(bool IsSceneOutlineMenu);
	TSharedPtr<class FUICommandList> PluginCommands;
	void RefreshSceneOutliner();
private:

	void CreateUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder);
	void BasicSetupSubMenu(FMenuBuilder& MenuBuilder);
	void ReplaceUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void ChangeTraceChannelSubMenu(FMenuBuilder& MenuBuilder);

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);

	TSharedRef<SDockTab> HandleSpawnEditorToolsTab(const FSpawnTabArgs& SpawnTabArgs);

	void EditorToolButtonClicked();
	bool CanEditActorForPrefab();
private:
	TSharedRef<SDockTab> HandleSpawnEventComponentSelectorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> HandleSpawnEventFunctionSelectorTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
};