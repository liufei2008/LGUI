// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
#if WITH_EDITOR
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
		ULGUIPrefab* GetPrefabAsset()const { return PrefabAsset; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		AActor* GetLoadedRootActor()const { return LoadedRootActor; }
	void MoveActorToPrefabFolder();

	void RemoveEditorTickDelegate();
	void EditorTick(float DeltaTime);
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)AActor* ParentActorForEditor;
	//Donot change this unless you know what you doing
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset;
	//Donot change this unless you know what you doing
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllLoadedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FGuid> AllLoadedActorsGuidArrayInPrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Instanced)
		ULGUIPrefab* PrefabInstance;
	FColor IdentityColor;

	FDelegateHandle EditorTickDelegateHandle;
private:
	static FName PrefabFolderName;
#endif
};