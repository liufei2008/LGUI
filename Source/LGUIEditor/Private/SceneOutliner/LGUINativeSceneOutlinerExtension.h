// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "SceneOutlinerFwd.h"
#include "LGUINativeSceneOutlinerExtension.generated.h"


/**
 * This actor is used by LGUI Editor to restore SceneOutliner actor's folder state and temp hidden state, only valid for editor
 */
UCLASS(NotBlueprintable, NotBlueprintType, notplaceable)
class LGUIEDITOR_API ALGUIEditorLevelDataStorageActor : public AActor
{
	GENERATED_BODY()
public:
	ALGUIEditorLevelDataStorageActor()
	{
		bIsEditorOnlyActor = true;
		bListedInSceneOutliner = false;
	}
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<AActor>> ExpandedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TSoftObjectPtr<AActor>> ExpandedSoftActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<AActor>> TemporarilyHiddenActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TSoftObjectPtr<AActor>> TemporarilyHiddenSoftActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FName> ExpandedFolderArray;
};

class LGUIEDITOR_API FLGUINativeSceneOutlinerExtension : public FTickableGameObject
{
public:
	FLGUINativeSceneOutlinerExtension();
	~FLGUINativeSceneOutlinerExtension();
public:
	//begin TickableEditorObject interface
	virtual void Tick(float DeltaTime)override;
	virtual bool IsTickable() const { return true; }
	virtual bool IsTickableInEditor()const { return true; }
	virtual TStatId GetStatId() const override;
	//end TickableEditorObject interface

	/**
	 * Temporary store state and then restore it.
	 */
	void Restore();
private:
	void OnPreSaveWorld(class UWorld* World, class FObjectPreSaveContext Context);
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	void OnPreBeginPIE(const bool IsSimulating);
	void OnBeginPIE(const bool IsSimulating);
	void OnEndPIE(const bool IsSimulating);
	void PreserveHierarchyStateChange();
	FDelegateHandle OnPreSaveWorldDelegateHandle;
	FDelegateHandle OnMapOpenedDelegateHandle;
	FDelegateHandle OnPreBeginPIEDelegateHandle;
	FDelegateHandle OnBeginPIEDelegateHandle;
	FDelegateHandle OnEndPIEDelegateHandle;
	FDelegateHandle OnLGUIEditorPreserveHierarchyStateChangeDelegateHandle;

	void SaveSceneOutlinerState();
	void SaveSceneOutlinerStateForPIE();
	void RestoreSceneOutlinerState();
	void SetDelayRestore(bool RestoreTemporarilyHidden, bool RestoreUseFName);
	void OnIterateTreeItem(const TFunction<void(STreeView<FSceneOutlinerTreeItemPtr>&, FSceneOutlinerTreeItemPtr&)>& Function);
	float delayRestoreTime = 0;
	bool needToRestore = false;
	bool shouldRestoreTemporarilyHidden = false;
	bool shouldRestoreUseFNameData = false;
	TArray<FName> ExpandedActorArray;
	TArray<FName> ExpandedFolderArray;
	ALGUIEditorLevelDataStorageActor* FindDataStorageActor(bool CreateIfNotExist = true);
};