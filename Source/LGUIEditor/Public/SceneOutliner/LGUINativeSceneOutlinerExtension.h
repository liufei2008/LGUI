// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "LGUINativeSceneOutlinerExtension.generated.h"

USTRUCT()
struct LGUIEDITOR_API FLGUISceneOutlinerStateArrayStruct
{
	GENERATED_BODY()
	UPROPERTY() TArray<FName> UnexpandedActorArray;
};
USTRUCT()
struct LGUIEDITOR_API FLGUISceneOutlinerStateMapStruct
{
	GENERATED_BODY()
	UPROPERTY() FString readme;
	UPROPERTY() FString readmeCH;
	UPROPERTY() TMap<FString, FLGUISceneOutlinerStateArrayStruct> WorldNameToUnexpandedActor;
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
	void SaveSceneOutlinerTreeFolder();
	void RestoreSceneOutlinerTreeFolder();
	void SetDelayRestore();
	bool NeedToExpand(AActor* InActor);
	const FString& GetLGUIDataFilePath();
	TArray<FName> UnexpandedActorArray;
	FLGUISceneOutlinerStateMapStruct SceneOutlinerStateStruct;
	int frameCount = 0;
	bool needToRestore = false;
};