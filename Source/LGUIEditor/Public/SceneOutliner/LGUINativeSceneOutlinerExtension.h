// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Tickable.h"
#include "LGUINativeSceneOutlinerExtension.generated.h"

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
private:
	void OnPreSaveWorld(uint32 SaveFlags, class UWorld* World);
	void OnMapOpened(const FString& FileName, bool AsTemplate);
	void OnBeginPIE(const bool IsSimulating);
	void DelayRestoreHierarchy(const bool IsSimulating);
	void SaveSceneOutlinerTreeFolder();
	void RestoreSceneOutlinerTreeFolder();
	bool NeedToExpand(AActor* InActor);
	TArray<FName> UnexpandedActorArray;
	int frameCount = 0;
	bool needToRestore = false;
};