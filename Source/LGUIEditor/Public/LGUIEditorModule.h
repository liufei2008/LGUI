// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
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

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FLGUIEditorModule& Get();

	TSharedRef<SWidget> MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool PreviewInViewport, bool EditorCameraControl, bool Others, bool UpgradeToLGUI3);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;
	void OnOutlinerSelectionChange();
private:

	void CreateUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIPostProcessSubMenu(FMenuBuilder& MenuBuilder);
	void BasicSetupSubMenu(FMenuBuilder& MenuBuilder);
	void ReplaceUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void ChangeTraceChannelSubMenu(FMenuBuilder& MenuBuilder);
	void AttachLayout(FMenuBuilder& MenuBuilder);
	void UseActiveViewportAsPreview();
	void ClearViewportPreview();
	void ToggleActiveViewportAsPreview();
	bool CanEditActorForPrefab();
	bool CanUnlinkActorForPrefab();
	bool CanDuplicateActor();
	bool CanPasteActor();
	bool CanBrowsePrefab();
	bool CanCreatePrefab();
	bool CanCheckPrefabOverrideParameter()const;
	bool CanReplaceUIElement();

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);

	void ToggleLGUIColumnInfo();
	bool LGUIColumnInfoChecked();
	void ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner);
	TWeakObjectPtr<class ALGUIPrefabHelperActor> CurrentPrefabHelperActor;
private:
	TSharedRef<SDockTab> HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
	bool bActiveViewportAsPreview = false;
	class FLGUINativeSceneOutlinerExtension* NativeSceneOutlinerExtension = nullptr;
	TSharedPtr<class SComboButton> PrefabOverrideDataViewerEntry;
	TSharedPtr<class SLGUIPrefabOverrideDataViewer> PrefabOverrideDataViewer;
};