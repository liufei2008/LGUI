// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabHelperActor.generated.h"

class ULGUIPrefabHelperObject;
struct FLGUIPrefabOverrideParameterData;

UCLASS(ClassGroup = (LGUI), NotBlueprintable, NotBlueprintType, NotPlaceable, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabHelperActor : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALGUIPrefabHelperActor();

	virtual void BeginPlay()override;
	virtual void Destroyed()override;
	virtual void BeginDestroy() override;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIPrefab> PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<AActor> LoadedRootActor = nullptr;
#endif
#if WITH_EDITOR
	void LoadPrefab(USceneComponent* InParent);
	void MoveActorToPrefabFolder();
#endif

#if WITH_EDITORONLY_DATA
public:
	bool bAutoDestroyLoadedActors = true;
	static FName PrefabFolderName;
#endif
};

/**
 * Wraper or container for ULGUIPrefabHelperObject. One level should only have one LGUIPrefabManagerActor.
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, NotPlaceable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabManagerActor : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ALGUIPrefabManagerActor();

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
	static TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALGUIPrefabManagerActor>> MapLevelToManagerActor;
public:
	static ALGUIPrefabManagerActor* GetPrefabManagerActor(ULevel* InLevel, bool CreateIfNotExist = true);
	static ALGUIPrefabManagerActor* GetPrefabManagerActorByPrefabHelperObject(ULGUIPrefabHelperObject* InHelperObject);
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIPrefabHelperObject> PrefabHelperObject = nullptr;
#endif
};
