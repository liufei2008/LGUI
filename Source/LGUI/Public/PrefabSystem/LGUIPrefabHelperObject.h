// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "LGUIPrefab.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperObject.generated.h"


//helper component for Load/Edit prefab
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input), ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable)
class LGUI_API ULGUIPrefabHelperObject : public USceneComponent
{
	GENERATED_BODY()

public:	
	ULGUIPrefabHelperObject();

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		bool bIsInsidePrefabEditor = true;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TWeakObjectPtr<AActor> LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<TWeakObjectPtr<AActor>> AllLoadedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<FGuid, TWeakObjectPtr<UObject>> MapGuidToObject;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TWeakObjectPtr<ULGUIPrefabOverrideParameterObject> PrefabOverrideParameterObject;
#endif
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
#if WITH_EDITOR
	virtual void BeginDestroy()override;
	static void SetActorPropertyInOutliner(AActor* Actor, bool InListed);

	//make sub prefab's actors as normal actor
	void UnlinkSubPrefab(AActor* InSubPrefabActor);
	ULGUIPrefab* GetSubPrefabAsset(AActor* InSubPrefabActor);
	void RevertPrefab();
	void SavePrefab();
	void LoadPrefab(UWorld* InWorld, USceneComponent* InParent);
#endif
};