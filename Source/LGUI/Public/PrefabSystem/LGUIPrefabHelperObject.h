// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefab.h"
#include "LGUIPrefabHelperObject.generated.h"

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

	/** Prefab object asset, null means this is a level prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULGUIPrefab> PrefabAsset = nullptr;
	/** Root actor of this prefab, null means this is a level prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<AActor> LoadedRootActor = nullptr;
	/** Map from guid to object, include all subprefab's object. Note object guid is not equals to subprefab's same object's guid. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
	/** Map to sub prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
#if WITH_EDITORONLY_DATA
	/** Broken actor-subprefab collection, only for level's subprefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TSet<TObjectPtr<AActor>> MissingPrefab;
#endif

#if WITH_EDITOR
	virtual void BeginDestroy()override;
	/** Make this prefab as manager object, will register some editor callbacks */
	void MarkAsManagerObject();
	bool GetIsManagerObject()const { return bIsMarkedAsManagerObject; }
#if WITH_EDITORONLY_DATA
	/** The root agent actor in prefab editor's outliner named [RootAgent] */
	UPROPERTY(Transient)TWeakObjectPtr<AActor> RootAgentActorForPrefabEditor = nullptr;
#endif
	bool IsInsidePrefabEditor() { return RootAgentActorForPrefabEditor.IsValid(); }

	void LoadPrefab(UWorld* InWorld, USceneComponent* InParent);

	static void SetActorPropertyInOutliner(AActor* Actor, bool InListed);

	ULGUIPrefab* GetSubPrefabAsset(AActor* InSubPrefabActor);
	void SavePrefab();
	void ClearLoadedPrefab();
	bool IsActorBelongsToSubPrefab(const AActor* InActor);
	/**
	 * Actor was belongs to a sub prefab, but the prefab asset is missing..
	 * Only call this function after IsActorBelongsToSubPrefab.
	 */
	bool IsActorBelongsToMissingSubPrefab(const AActor* InActor);
	bool ActorIsSubPrefabRootActor(const AActor* InActor);
	bool IsActorBelongsToThis(const AActor* InActor);
	void ClearInvalidObjectAndGuid();
	void AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, bool InIncludeRootTransform);
	FLGUISubPrefabData GetSubPrefabData(AActor* InSubPrefabActor);
	AActor* GetSubPrefabRootActor(AActor* InSubPrefabActor);
	/** For parent prefab. When parent prefab want to apply override parameter to subprefab, but the parameter belongs to subprefab's subprefab, then we need to mark override parameter for subprefab. */
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, const TArray<FName>& InPropertyNames);
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName);

	/** If sub prefab changed, then update parent prefab */
	bool RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab, AActor* InSubPrefabRootActor = nullptr);

	void CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject);

	void RevertPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInDst, const FLGUISubPrefabData& SubPrefabData, int RawArrayIndex = 0, bool IsInsideRawArray = false);
	void ApplyPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInDst, const FLGUISubPrefabData& SubPrefabData, int RawArrayIndex = 0, bool IsInsideRawArray = false);

	void RevertPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames);
	void RevertPrefabOverride(UObject* InObject, FName InPropertyName);
	void RevertAllPrefabOverride(UObject* InObject);
	void ApplyPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames);
	void ApplyPrefabOverride(UObject* InObject, FName InPropertyName);
	void ApplyAllOverrideToPrefab(UObject* InObject);

	void RefreshSubPrefabVersion(AActor* InSubPrefabRootActor);

	void MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, const TMap<FGuid, TObjectPtr<UObject>>& InSubMapGuidToObject, const TArray<FLGUIPrefabOverrideParameterData>& InObjectOverrideParameterArray);
	void RemoveSubPrefabByRootActor(AActor* InPrefabRootActor);
	void RemoveSubPrefab(AActor* InPrefabActor);
	ULGUIPrefab* GetPrefabAssetBySubPrefabObject(UObject* InObject);
	bool GetAnythingDirty()const;
	void SetNothingDirty();
	void SetAnythingDirty();
	void CheckPrefabVersion();
	void DismissAllVersionNotifications() { OnNewVersionDismissAllClicked(); }
	FSimpleMulticastDelegate OnSubPrefabNewVersionUpdated;
	/**
	 * @return	true if anything changed
	 */
	bool CleanupInvalidSubPrefab();
	void SetCanNotifyAttachment(bool value) { bCanNotifyAttachment = value; }
private:
	bool bIsMarkedAsManagerObject = false;
	bool bAnythingDirty = false;
	bool bCanCollectProperty = true;
	bool bCanNotifyAttachment = false;

	void OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent);
	void OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain);
	void TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty);

	void OnLevelActorAttached(AActor* Actor, const AActor* AttachTo);
	void OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom);
	void OnLevelActorDeleted(AActor* Actor);

	struct FAttachmentActorStruct
	{
		TWeakObjectPtr<AActor> Actor = nullptr;
		TWeakObjectPtr<AActor> DetachFrom = nullptr;
		TWeakObjectPtr<AActor> AttachTo = nullptr;
	};
	FAttachmentActorStruct AttachmentActor;
	void CheckAttachment();
	UWorld* GetPrefabWorld()const;

	/**
	 * For Level prefab only. Object link could still exist if delete subprefab's root actor by UE's delete function.
	 * @return true if anything change
	 */
	bool CleanupInvalidLinkToSubPrefabObject();

	struct FNotificationContainer
	{
		TWeakObjectPtr<AActor> SubPrefabRootActor;
		TWeakPtr<SNotificationItem> Notification;
	};
	TArray<FNotificationContainer> NewVersionPrefabNotificationArray;
	void OnNewVersionUpdateClicked(AActor* InPrefabRootActor);
	void OnNewVersionDismissClicked(AActor* InPrefabRootActor);
	void OnNewVersionUpdateAllClicked();
	void OnNewVersionDismissAllClicked();
#endif
};