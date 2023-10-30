// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabLevelManagerActor.generated.h"

class ULGUIPrefabHelperObject;

/**
 * Wraper or container for ULGUIPrefabHelperObject. One level should only have one LGUIPrefabLevelManagerActor.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, NotPlaceable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabLevelManagerActor : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ALGUIPrefabLevelManagerActor();

#if WITH_EDITOR
	virtual void BeginPlay()override;
	virtual void PostInitProperties()override;
	virtual void PostActorCreated()override;
	virtual void BeginDestroy() override;
	virtual void Destroyed()override;
private:
	FDelegateHandle OnSubPrefabNewVersionUpdatedDelegateHandle;
	FDelegateHandle BeginPIEDelegateHandle;
	void CollectWhenCreate();
	void CleanupWhenDestroy();
	static TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALGUIPrefabLevelManagerActor>> MapLevelToManagerActor;
public:
	static FName PrefabFolderName;
	static ALGUIPrefabLevelManagerActor* GetInstance(ULevel* InLevel, bool CreateIfNotExist = true);
	static ALGUIPrefabLevelManagerActor* GetInstanceByPrefabHelperObject(ULGUIPrefabHelperObject* InHelperObject);
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIPrefabHelperObject> PrefabHelperObject = nullptr;
#endif
};
