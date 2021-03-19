// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
		TArray<TWeakObjectPtr<AActor>> UnexpandedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TSoftObjectPtr<AActor>> UnexpandedSoftActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<AActor>> TemporarilyHiddenActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TSoftObjectPtr<AActor>> TemporarilyHiddenSoftActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FName> ExpandedFolderArray;
};

UCLASS()
class LGUIEDITOR_API ULGUINativeSceneOutlinerExtension : public UObject, public FTickableGameObject
{
public:
	GENERATED_BODY()
	void Init();
public:
	//begin TickableEditorObject interface
	virtual void Tick(float DeltaTime)override;
	virtual bool IsTickable() const { return true; }
	virtual bool IsTickableInEditor()const { return true; }
	virtual TStatId GetStatId() const override;
	//end TickableEditorObject interface
	static bool active;
private:
	void OnPreSaveWorld(uint32 SaveFlags, class UWorld* World);
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	void OnPreBeginPIE(const bool IsSimulating);
	void OnBeginPIE(const bool IsSimulating);
	void OnEndPIE(const bool IsSimulating);
	void SaveSceneOutlinerState();
	void RestoreSceneOutlinerState();
	void RestoreSceneOutlinerStateForTreeItem(SceneOutliner::FTreeItemPtr& Item, ALGUIEditorLevelDataStorageActor* storageActor);
	void SetDelayRestore(bool RestoreTemporarilyHidden);
	int frameCount = 0;
	bool needToRestore = false;
	bool shouldRestoreTemporarilyHidden = false;
	ALGUIEditorLevelDataStorageActor* FindOrCreateDataStorageActor();
};