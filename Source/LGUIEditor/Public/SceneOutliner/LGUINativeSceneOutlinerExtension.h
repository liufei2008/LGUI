// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "SceneOutlinerFwd.h"
#include "LGUINativeSceneOutlinerExtension.generated.h"


USTRUCT()
struct LGUIEDITOR_API FLGUISceneOutlinerStateArrayStruct
{
	GENERATED_BODY()
	UPROPERTY() TArray<FName> ExpandedFolderArray;
	UPROPERTY() TArray<FName> UnexpandedActorArray;
	UPROPERTY() TArray<FName> TemporarilyHiddenInEditorActorArray;
};
USTRUCT()
struct LGUIEDITOR_API FLGUISceneOutlinerStateMapStruct
{
	GENERATED_BODY()
	UPROPERTY() FString readme;
	UPROPERTY() FString readmeCH;
	UPROPERTY() TMap<FString, FLGUISceneOutlinerStateArrayStruct> WorldNameToUnexpandedActor;
};

//UCLASS(NotBlueprintable, NotBlueprintType, notplaceable)
//class LGUI_API ALGUIEditorLevelDataStorageActor : public AActor
//{
//	GENERATED_BODY()
//public:
//	UPROPERTY(VisibleAnywhere, Category = "LGUI")
//		TArray<TWeakObjectPtr<AActor>> ExpandedActors;
//	UPROPERTY(VisibleAnywhere, Category = "LGUI")
//		TArray<FName> ExpandedFolders;
//	UPROPERTY(VisibleAnywhere, Category = "LGUI")
//		TArray<TWeakObjectPtr<ULevel>> ExpandedLevels;
//
//	UPROPERTY(VisibleAnywhere, Category = "LGUI")
//		TArray<TWeakObjectPtr<AActor>> TemporarilyHiddenInEditorActors;
//};

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
	void RestoreSceneOutlinerStateForTreeItem(SceneOutliner::FTreeItemPtr& Item);
	void SetDelayRestore(bool RestoreTemporarilyHidden);
	const FString& GetLGUIDataFilePath();
	TArray<FName> ExpandedFolderArray;
	TArray<FName> UnexpandedActorArray;
	TArray<FName> TemporarilyHiddenInEditorActorArray;
	FLGUISceneOutlinerStateMapStruct SceneOutlinerStateStruct;
	int frameCount = 0;
	bool needToRestore = false;
	bool shouldRestoreTemporarilyHidden = false;
	//ALGUIEditorLevelDataStorageActor* FindOrCreateDataStorageActor();
};