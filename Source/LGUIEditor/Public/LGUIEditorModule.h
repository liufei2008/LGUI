// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Modules/ModuleInterface.h"

class ULexWidget;
class FToolBarBuilder;
class FMenuBuilder;
DECLARE_LOG_CATEGORY_EXTERN(LGUIEditor, Log, All);

class FLGUIEditorModule : public IModuleInterface, public FGCObject
{
public:

	static const FName LexUIDynamicSpriteAtlasViewerTabName;
	static const FName LexUIWidgetInspectorTabName;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static FLGUIEditorModule& Get();
	
	TSharedRef<SWidget> MakeEditorToolsMenu(TFunction<ULexWidget*()> GetSelectedWidgetFunction, TFunction<void(FMenuBuilder&)> ExtendEditMenuFunction);
	TSharedPtr<class FUICommandList> PluginCommands;
	TArray<TSharedPtr<class FAssetTypeActions_Base>> AssetTypeActionsArray;

	const FSlateBrush* GetInteractionIconBrush(ULexWidget* Widget);
private:

	bool IsValidClassName(const FString& InName);

	void CreateUIElementSubMenu(FMenuBuilder& MenuBuilder, TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	void CreateUIExtensionSubMenu(FMenuBuilder& MenuBuilder, TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	void CreateUIBackBufferReaderSubMenu(FMenuBuilder& MenuBuilder, TFunction<ULexWidget*()> GetSelectedWidgetFunction);
	void CreateExtraPrefabsSubMenu(FMenuBuilder& MenuBuilder, TFunction<ULexWidget*()> GetSelectedActorFunction);

	TWeakObjectPtr<class ULexUIPrefabHelperObject> CurrentPrefabHelperObject;
private:
	TSharedRef<SDockTab> HandleSpawnDynamicSpriteAtlasViewerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> HandleSpawnLexUIInspectorTab(const FSpawnTabArgs& SpawnTabArgs);
	
	FDelegateHandle SequenceEditorHandle;
	FDelegateHandle OnInitializeSequenceHandle;
	static void OnInitializeSequence(class ULexUIPrefabSequence* Sequence);
	FDelegateHandle LexUIMaterialTrackEditorCreateTrackEditorHandle;
	TObjectPtr<class USequencerSettings> LexUIPrefabSequencerSettings = nullptr;

	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
};