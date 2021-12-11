// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LGUIPrefabPreviewScene.h"
#pragma once

class ULGUIPrefab;
class FLGUIPrefabPreviewManager;
class SLGUIPrefabEditorViewport;
class SLGUIPrefabEditorDetailTab;
class FLGUIPrefabEditorOutliner;
class AActor;

/**
 * 
 */
class FLGUIPrefabEditor : public FAssetEditorToolkit
{
public:
	FLGUIPrefabEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FText GetToolkitName() const override;
	virtual FText GetToolkitToolTipText() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FString GetDocumentationLink() const override;
	virtual void OnToolkitHostingStarted(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void OnToolkitHostingFinished(const TSharedRef<class IToolkit>& Toolkit) override;
	virtual void SaveAsset_Execute()override;
	// End of FAssetEditorToolkit

	bool CheckBeforeSaveAsset();

	void InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, ULGUIPrefab* InPrefab);

	/** Try to handle a drag-drop operation */
	FReply TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent);

	FLGUIPrefabPreviewScene& GetPreviewScene() { return PreviewScene; };
	UWorld* GetWorld();
	ULGUIPrefab* GetPrefabBeingEdited()const { return PrefabBeingEdited; }

protected:
	ULGUIPrefab* PrefabBeingEdited = nullptr;
	TMap<FGuid, UObject*> MapGuidToObject;
	TSharedPtr<SLGUIPrefabEditorViewport> ViewportPtr;
	TSharedPtr<SLGUIPrefabEditorDetailTab> DetailsTabPtr;
	TSharedPtr<FLGUIPrefabEditorOutliner> OutlinerPtr;

	TWeakObjectPtr<AActor> CurrentSelectedActor;

	FLGUIPrefabPreviewScene PreviewScene;
private:
	//void BindCommands();
	//void ExtendMenu();
	void ExtendToolbar();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Outliner(const FSpawnTabArgs& Args);

	bool IsFilteredActor(const AActor* Actor);
	void OnOutlinerPickedChanged(AActor* Actor);
	void OnOutlinerActorDoubleClick(AActor* Actor);


	AActor* LoadedRootActor = nullptr;
	TArray<AActor*> AllLoadedActorArray;
	TArray<FGuid> AllLoadedActorGuidArrayInPrefab;
};
