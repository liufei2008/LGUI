// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailsView.h"
#include "PropertyHandle.h"

class FToolBarBuilder;
class FMenuBuilder;
DECLARE_LOG_CATEGORY_EXTERN(LGUIEditor, Log, All);

class FLGUIEditorModule : public IModuleInterface
{
public:

	static const FName LGUIAtlasViewerName;
	static const FName LGUIPrefabSequenceTabName;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FLGUIEditorModule& Get();

	TSharedRef<SWidget> MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool OpenWindow, bool PreviewInViewport, bool EditorCameraControl, bool Others, bool UpgradeToLGUI3);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;
	void OnOutlinerSelectionChange();
private:

	bool IsValidClassName(const FString& InName);

	void CreateUIElementSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder);
	void CreateUIPostProcessSubMenu(FMenuBuilder& MenuBuilder);
	void CreateCommonActorSubMenu(FMenuBuilder& MenuBuilder);
	void CreateExtraPrefabsSubMenu(FMenuBuilder& MenuBuilder);
	void BasicSetupSubMenu(FMenuBuilder& MenuBuilder);
	void ReplaceActorSubMenu(FMenuBuilder& MenuBuilder);
	void ChangeTraceChannelSubMenu(FMenuBuilder& MenuBuilder);
	void AttachLayout(FMenuBuilder& MenuBuilder);
	void UseActiveViewportAsPreview();
	void ClearViewportPreview();
	void ToggleActiveViewportAsPreview();
	bool CanUnpackActorForPrefab();
	bool CanBrowsePrefab();
	bool CanUpdateLevelPrefab();
	bool CanCreatePrefab();
	bool CanCheckPrefabOverrideParameter()const;
	bool CanReplaceActor();
	bool CanAttachLayout();
	bool CanCreateActor();

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);

	void ToggleLGUIColumnInfo();
	bool LGUIColumnInfoChecked();

	void ToggleDrawHelperFrame();
	bool GetDrawHelperFrameChecked();

	void ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner);
	TWeakObjectPtr<class ULGUIPrefabHelperObject> CurrentPrefabHelperObject;
private:
	TSharedRef<SDockTab> HandleSpawnAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> HandleSpawnLGUIPrefabSequenceTab(const FSpawnTabArgs& SpawnTabArgs);
	bool bActiveViewportAsPreview = false;
	class FLGUINativeSceneOutlinerExtension* NativeSceneOutlinerExtension = nullptr;
	TSharedPtr<class SLGUIPrefabOverrideDataViewer> PrefabOverrideDataViewer = nullptr;
	void CheckPrefabOverrideDataViewerEntry();

	
	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	FName LGUIPrefabSequenceComponentName;
	static void OnInitializeSequence(class ULGUIPrefabSequence* Sequence);
};