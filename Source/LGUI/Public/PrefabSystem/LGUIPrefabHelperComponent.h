// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "LGUIPrefab.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperComponent.generated.h"


class ALGUIPrefabActor;

//helper component for PrefabSystem. for manage a prefab actor in level. only use this in editor
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input), ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable)
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
		void SavePrefab(bool InCreateOrApply, bool InIncludeOtherPrefabAsSubPrefab);
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
	bool IsRootPrefab()const;
	void RestoreSubPrefabs();
	FGuid GetGuidByActor(AActor* InActor, bool InIncludeSubPrefabs);
	int32 GetActorIndexFromCreatedActors(AActor* InActor, bool InIncludeSubPrefabs);
	bool GetActorAndGuidsFromCreatedActors(AActor* InActor, bool InRemoveFromList, bool InIncludeSubPrefabs, FGuid& OutRemovedActorGuid);
	bool IsInsideSubPrefab(AActor* InActor);

	void RemoveEditorTickDelegate();
	void EditorTick(float DeltaTime);
private:
	//Clear AllLoadedActorArray, remove it if not under root actor
	void CleanupLoadedActors();
#endif
#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(Transient)AActor* ParentActorForEditor;//@todo: remove this

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllLoadedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FGuid> AllLoadedActorsGuidArrayInPrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ALGUIPrefabActor* ParentPrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ALGUIPrefabActor*> SubPrefabs;
	FColor IdentityColor;

	FDelegateHandle EditorTickDelegateHandle;
private:
	static FName PrefabFolderName;
#endif
};