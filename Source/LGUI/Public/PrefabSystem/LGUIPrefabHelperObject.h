// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperObject.generated.h"

class ULGUIPrefab;
class ULGUIPrefabOverrideParameterObject;
struct FLGUISubPrefabData;
class AActor;

/**
 * helper object for manage prefab's load/save
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable)
class LGUI_API ULGUIPrefabHelperObject : public UObject
{
	GENERATED_BODY()

public:	
	ULGUIPrefabHelperObject();

	virtual void PostInitProperties()override;
	virtual bool IsEditorOnly() const { return false; }

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<AActor*> AllLoadedActorArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<FGuid, UObject*> MapGuidToObject;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FDateTime TimePointWhenSavePrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		bool bIsInsidePrefabEditor = true;
#endif

#if WITH_EDITOR
	void LoadPrefab(UWorld* InWorld, USceneComponent* InParent);

	virtual void BeginDestroy()override;
	virtual void PostEditUndo()override;

	static void SetActorPropertyInOutliner(AActor* Actor, bool InListed);

	//make sub prefab's actors as normal actor
	void UnpackSubPrefab(AActor* InSubPrefabActor);
	void UnpackPrefab(AActor* InPrefabActor);
	ULGUIPrefab* GetSubPrefabAsset(AActor* InSubPrefabActor);
	void SavePrefab();
	void ClearLoadedPrefab();
	bool IsActorBelongsToSubPrefab(const AActor* InActor);
	bool ActorIsSubPrefabRootActor(const AActor* InActor);
	bool IsActorBelongsToThis(const AActor* InActor, bool InCludeSubPrefab);
	void AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject);
	FLGUISubPrefabData GetSubPrefabData(AActor* InSubPrefabActor);
	/** For parent prefab. When parent prefab want to apply override parameter to subprefab, but the parameter belongs to subprefab's subprefab, then we need to mark override parameter for subprefab. */
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName);
#endif
};