// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabHelperActor.generated.h"

class ULGUIPrefabHelperObject;
struct FLGUIPrefabOverrideParameterData;

UCLASS(NotBlueprintable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabHelperActor : public AActor
{
	GENERATED_BODY()
	
	
public:	
	// Sets default values for this actor's properties
	ALGUIPrefabHelperActor();

	virtual void BeginPlay()override;
	virtual void PostInitProperties()override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Destroyed()override;
	virtual void BeginDestroy() override;

public:
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FDateTime TimePointWhenSavePrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		AActor* LoadedRootActor = nullptr;
#endif
#if WITH_EDITOR
	void MarkPrefabVersionAsLatest();
	void LoadPrefab(USceneComponent* InParent);
	bool IsValidPrefabHelperActor();
	void MoveActorToPrefabFolder();
	void CheckPrefabVersion();
	void RevertPrefab();
#endif

#if WITH_EDITORONLY_DATA
public:
	bool bAutoDestroyLoadedActors = true;
	static FName PrefabFolderName;
private:
	TWeakPtr<SNotificationItem> NewVersionPrefabNotification;
	void OnNewVersionRevertPrefabClicked();
	void OnNewVersionDismissClicked();
#endif
};

/**
 * Wraper or container for ULGUIPrefabHelperObject. One level should only have one LGUIPrefabManagerActor.
 * @todo: check if there are multiple LGUIPrefabManagerActor in one level
 */
UCLASS(NotBlueprintable, NotBlueprintType, HideCategories = (Rendering, Actor, Input))
class LGUI_API ALGUIPrefabManagerActor : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	ALGUIPrefabManagerActor();

#if WITH_EDITOR
	virtual void PostInitProperties()override;
	virtual void BeginDestroy() override;
private:
	static TMap<ULevel*, ALGUIPrefabManagerActor*> MapLevelToManagerActor;
public:
	static ALGUIPrefabManagerActor* GetPrefabManagerActor(ULevel* InLevel);
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
#endif
};
