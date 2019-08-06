// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "LGUIPrefab.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperComponent.generated.h"


//helper component for PrefabSystem. for manage a prefab actor in level. only use this in editor
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input), ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUIPrefabHelperComponent : public USceneComponent
{
	GENERATED_BODY()

public:	

	ULGUIPrefabHelperComponent();
	virtual void BeginPlay() override;
	virtual void OnRegister()override;

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void LoadPrefab();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SavePrefab();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void RevertPrefab();
	//delete this prefab actor
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void DeleteThisInstance();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPrefabAsset(ULGUIPrefab* InPrefab)
	{
		PrefabAsset = InPrefab;
	}
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPrefab* GetPrefabAsset()
	{
		return PrefabAsset;
	}
	UFUNCTION(BlueprintCallable, Category = LGUI)
		AActor* GetLoadedRootActor()
	{
		return LoadedRootActor;
	}

#if WITH_EDITOR
	UPROPERTY(Transient)AActor* ParentActorForEditor;
#endif
public:
	//Donot change this unless you know what you doing
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset;
	//Donot change this unless you know what you doing
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllLoadedActorArray;
};