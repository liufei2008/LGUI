﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Components/SceneComponent.h"
#include "LGUIPrefabHelperObject.generated.h"

class ULGUIPrefab;
class ULGUIPrefabOverrideParameterObject;
struct FLGUISubPrefabData;
struct FLGUIPrefabOverrideParameterData;
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

	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		ULGUIPrefab* PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		AActor* LoadedRootActor;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<FGuid, UObject*> MapGuidToObject;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;

#if WITH_EDITOR
	virtual void BeginDestroy()override;
	/** Make this prefab as manager object, will register some editor callbacks */
	void MarkAsManagerObject();
	bool GetIsManagerObject()const { return bIsMarkedAsManagerObject; }
#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)TWeakObjectPtr<AActor> RootAgentActorForPrefabEditor = nullptr;
#endif
	bool IsInsidePrefabEditor() { return RootAgentActorForPrefabEditor.IsValid(); }

	void LoadPrefab(UWorld* InWorld, USceneComponent* InParent);

	static void SetActorPropertyInOutliner(AActor* Actor, bool InListed);

	ULGUIPrefab* GetSubPrefabAsset(AActor* InSubPrefabActor);
	void SavePrefab();
	void ClearLoadedPrefab();
	bool IsActorBelongsToSubPrefab(const AActor* InActor);
	bool ActorIsSubPrefabRootActor(const AActor* InActor);
	bool IsActorBelongsToThis(const AActor* InActor);
	void ClearInvalidObjectAndGuid();
	void AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName);
	void RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabActor);
	FLGUISubPrefabData GetSubPrefabData(AActor* InSubPrefabActor);
	AActor* GetSubPrefabRootActor(AActor* InSubPrefabActor);
	/** For parent prefab. When parent prefab want to apply override parameter to subprefab, but the parameter belongs to subprefab's subprefab, then we need to mark override parameter for subprefab. */
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName);

	bool RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab, AActor* InSubPrefabRootActor = nullptr);

	void CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject);

	void RevertPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void RevertPrefabOverride(UObject* InObject, FName InPropertyName);
	void RevertAllPrefabOverride(UObject* InObject);
	void ApplyPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void ApplyPrefabOverride(UObject* InObject, FName InPropertyName);
	void ApplyAllOverrideToPrefab(UObject* InObject);

	void CheckPrefabHelperActor(AActor* InSubPrefabRootActor);

	void MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, const TMap<FGuid, UObject*>& InSubMapGuidToObject, const TArray<FLGUIPrefabOverrideParameterData>& InObjectOverrideParameterArray);
	void RemoveSubPrefabByRootActor(AActor* InPrefabRootActor);
	void RemoveSubPrefab(AActor* InPrefabActor);
	ULGUIPrefab* GetPrefabAssetBySubPrefabObject(UObject* InObject);
	bool GetAnythingDirty()const { return bAnythingDirty; }
	void SetNothingDirty() { bAnythingDirty = false; }
	void CheckPrefabVersion();
	void DismissAllVersionNotifications() { OnNewVersionDismissAllClicked(); }
	FSimpleMulticastDelegate OnSubPrefabNewVersionUpdated;
	/**
	 * @return	true if anything changed
	 */
	bool CleanupInvalidSubPrefab();
private:
	bool bIsMarkedAsManagerObject = false;
	bool bAnythingDirty = false;
	bool bCanCollectProperty = true;
	bool bCanNotifyDetachment = false;

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