﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "UObject/GCObject.h"
#include "Toolkits/IToolkitHost.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LGUIPrefabPreviewScene.h"
#pragma once

class ULGUIPrefab;
class SLGUIPrefabEditorViewport;
class SLGUIPrefabEditorDetails;
class FLGUIPrefabEditorOutliner;
class SLGUIPrefabOverrideParameterEditor;
class SLGUIPrefabRawDataViewer;
class AActor;
class FLGUIPrefabPreviewScene;
class ULGUIPrefabHelperObject;
class ULGUIPrefabOverrideParameterHelperObject;
class ULGUIPrefabOverrideHelperObject;
struct FLGUISubPrefabData;

/**
 * 
 */
class FLGUIPrefabEditor : public FAssetEditorToolkit
	, public FGCObject
{
public:
	FLGUIPrefabEditor();
	~FLGUIPrefabEditor();

	// IToolkit interface
	virtual void RegisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<class FTabManager>& TabManager) override;
	// End of IToolkit interface

	// FAssetEditorToolkit
public:
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
private:
	virtual bool OnRequestClose()override;
	// End of FAssetEditorToolkit
public:
	/** FGCObject interface */
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName()const { return TEXT("LGUIPrefabEditor"); }

	bool CheckBeforeSaveAsset();

	void InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, ULGUIPrefab* InPrefab);

	/** Try to handle a drag-drop operation */
	FReply TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent);

	FLGUIPrefabPreviewScene& GetPreviewScene();
	UWorld* GetWorld();
	ULGUIPrefab* GetPrefabBeingEdited()const { return PrefabBeingEdited; }

	void DeleteActors(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray);

	static FLGUIPrefabEditor* GetEditorForPrefabIfValid(ULGUIPrefab* InPrefab);
	static ULGUIPrefabHelperObject* GetEditorPrefabHelperObjectForActor(AActor* InActor);
	static bool ActorIsRootAgent(AActor* InActor);
	bool RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab);

	bool GetSelectedObjectsBounds(FBoxSphereBounds& OutResult);
	FBoxSphereBounds GetAllObjectsBounds();
	bool ActorBelongsToSubPrefab(AActor* InSubPrefabActor);
	bool ActorIsSubPrefabRoot(AActor* InSubPrefabRootActor);
	FLGUISubPrefabData GetSubPrefabDataForActor(AActor* InSubPrefabActor);
	void GetInitialViewLocationAndRotation(FVector& OutLocation, FRotator& OutRotation);

	void OpenSubPrefab(AActor* InSubPrefabActor);
	void SelectSubPrefab(AActor* InSubPrefabActor);
	bool GetAnythingDirty()const;
	void CloseWithoutCheckDataDirty();

	ULGUIPrefabHelperObject* GetPrefabManagerObject()const { return PrefabHelperObject; }
private:
	ULGUIPrefab* PrefabBeingEdited = nullptr;
	ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
	static TArray<FLGUIPrefabEditor*> LGUIPrefabEditorInstanceCollection;

	TSharedPtr<SLGUIPrefabEditorViewport> ViewportPtr;
	TSharedPtr<SLGUIPrefabEditorDetails> DetailsPtr;
	TSharedPtr<FLGUIPrefabEditorOutliner> OutlinerPtr;
	TSharedPtr<SLGUIPrefabRawDataViewer> PrefabRawDataViewer;

	TWeakObjectPtr<AActor> CurrentSelectedActor;

	FLGUIPrefabPreviewScene PreviewScene;
private:

	void BindCommands();
	//void ExtendMenu();
	void ExtendToolbar();

	void OnApply();
	void OnOpenRawDataViewerPanel();

	TSharedRef<SDockTab> SpawnTab_Viewport(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Outliner(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args);

	bool IsFilteredActor(const AActor* Actor);
	void OnOutlinerPickedChanged(AActor* Actor);
	void OnOutlinerActorDoubleClick(AActor* Actor);
};
