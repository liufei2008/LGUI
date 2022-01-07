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
	virtual void Destroyed() override;

public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
	void RevertPrefab();
#if WITH_EDITOR
	//UPROPERTY(VisibleAnywhere, Category = "LGUI")TArray<FLGUIPrefabOverrideParameterData> ObjectOverrideParameterArray;//@todo: auto revert with override parameters
	void LoadPrefab(USceneComponent* InParent);
	void SavePrefab();
	//delete this prefab actor
	void DeleteThisInstance();
	void MoveActorToPrefabFolder();
	void CheckPrefabVersion();
#endif

#if WITH_EDITORONLY_DATA
public:
	FColor IdentityColor = FColor::Black;
	bool IsRandomColor = true;
	bool AutoDestroyLoadedActors = true;
	static TArray<FColor> AllColors;
private:
	static FName PrefabFolderName;
	TWeakPtr<SNotificationItem> NewVersionPrefabNotification;
	void OnNewVersionRevertPrefabClicked();
	void OnNewVersionDismissClicked();
#endif
};
