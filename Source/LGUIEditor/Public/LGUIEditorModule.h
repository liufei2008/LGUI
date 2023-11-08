// Copyright 2019-Present LexLiu. All Rights Reserved.

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
class FLGUINativeSceneOutlinerExtension;
DECLARE_LOG_CATEGORY_EXTERN(LGUIEditor, Log, All);

class FLGUIEditorModule : public IModuleInterface, public FGCObject
{
public:

	static const FName LGUIDynamicSpriteAtlasViewerName;
	static const FName LGUIPrefabSequenceTabName;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FLGUIEditorModule& Get();

	TSharedRef<SWidget> MakeEditorToolsMenu(bool InitialSetup, bool ComponentAction, bool OpenWindow, bool PreviewInViewport, bool EditorCameraControl, bool Others);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;
	void OnOutlinerSelectionChange();
	FLGUINativeSceneOutlinerExtension* GetNativeSceneOutlinerExtension()const;
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
	ECheckBoxState GetAutoUpdateLevelPrefab()const;
	bool CanCreatePrefab();
	bool CanCheckPrefabOverrideParameter()const;
	bool CanReplaceActor();
	bool CanAttachLayout();
	bool CanCreateActor();

	void AddEditorToolsToToolbarExtension(FToolBarBuilder& Builder);

	void ToggleLGUIColumnInfo();
	bool IsLGUIColumnInfoChecked();

	void ToggleAnchorTool();
	bool IsAnchorToolChecked();

	void ToggleDrawHelperFrame();
	bool IsDrawHelperFrameChecked();

	void ApplyLGUIColumnInfo(bool value, bool refreshSceneOutliner);
	TWeakObjectPtr<class ULGUIPrefabHelperObject> CurrentPrefabHelperObject;
private:
	TSharedRef<SDockTab> HandleSpawnDynamicSpriteAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> HandleSpawnLGUIPrefabSequenceTab(const FSpawnTabArgs& SpawnTabArgs);
	bool bActiveViewportAsPreview = false;
	FLGUINativeSceneOutlinerExtension* NativeSceneOutlinerExtension = nullptr;
	TSharedPtr<class SLGUIPrefabOverrideDataViewer> PrefabOverrideDataViewer = nullptr;
	void CheckPrefabOverrideDataViewerEntry();

	
	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	FName LGUIPrefabSequenceComponentName;
	static void OnInitializeSequence(class ULGUIPrefabSequence* Sequence);
	FDelegateHandle LGUIMaterialTrackEditorCreateTrackEditorHandle;
	class USequencerSettings* LGUIPrefabSequencerSettings = nullptr;

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
};