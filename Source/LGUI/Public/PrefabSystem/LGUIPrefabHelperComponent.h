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
		void LoadPrefab(USceneComponent* InParent = nullptr);
	void LoadSubPrefab(USceneComponent* InParent, TMap<FGuid, FGuid> InGuidFromPrefabToInstance);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SavePrefab(bool InIncludeOtherPrefabAsSubPrefab);
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
	FGuid GetGuidByActor(AActor* InActor);
	bool IsActorBelongsToSubPrefab(AActor* InActor);
	bool IsActorBelongsToPrefab(AActor* InActor);
	ULGUIPrefabHelperComponent* GetSubPrefabWhichManageTheActor(AActor* InActor);
	bool IsSubPrefabRootActor(AActor* InActor);
	void CleanupPrefabAndActor();

	void RemoveEditorTickDelegate();
	void EditorTick(float DeltaTime);
private:
#endif
#if WITH_EDITORONLY_DATA
public:
	UPROPERTY(Transient)AActor* ParentActorForEditor;//When drag a prefab from content browser, this is the selected parent

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	/** All loaded actor, include sub prefab's actor */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllLoadedActorArray;
	/** All loaded actor's guid which stored in prefab, include sub prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<FGuid> AllLoadedActorGuidArrayInPrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ALGUIPrefabActor* ParentPrefab;
	/** SubPrefab's root actor to prefab map */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<AActor*, ALGUIPrefabActor*> SubPrefabs;
	FColor IdentityColor = FColor::Black;
	bool IsRandomColor = true;
	static TArray<FColor> AllColors;

	FDelegateHandle EditorTickDelegateHandle;
private:
	static FName PrefabFolderName;
#endif
};