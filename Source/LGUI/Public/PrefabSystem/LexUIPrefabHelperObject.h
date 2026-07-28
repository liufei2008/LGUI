// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Components/SceneComponent.h"
#include "LexUIPrefab.h"
#include "LexUIPrefabHelperObject.generated.h"

class AActor;

/**
 * helper object for manage prefab's load/save
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintType, NotBlueprintable)
class LGUI_API ULexUIPrefabHelperObject : public UObject
{
	GENERATED_BODY()

public:	
	ULexUIPrefabHelperObject();

	/** Prefab object asset, null means this is a level prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULexUIPrefab> PrefabAsset = nullptr;
	/** Root widget of this prefab, null means this is a level prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<ULexWidget> LoadedRootWidget = nullptr;
	/** Map from guid to object, include all sub-prefab's object. Note object guid is not equals to sub-prefab's same object's guid. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
	/** Map to sub prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData> SubPrefabMap;
#if WITH_EDITORONLY_DATA
	/** Broken widget-sub-prefab collection, only for level's sub-prefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TSet<TObjectPtr<ULexWidget>> MissingPrefab;
#endif

#if WITH_EDITOR
	virtual void BeginDestroy()override;
#if WITH_EDITORONLY_DATA
	TWeakObjectPtr<UWorld> PrefabInstanceWorld = nullptr;
#endif

	void Init(ULexUIPrefab* InPrefab, class FLexUIPrefabInstanceScene* InPrefabInstanceScene);
	
	ULexUIPrefab* GetSubPrefabAsset(ULexWidget* InSubPrefabWidget);
	void SavePrefab();
	void ClearLoadedPrefab();
	bool IsWidgetBelongsToSubPrefab(const ULexWidget* InWidget);
	/**
	 * Actor was belongs to a sub prefab, but the prefab asset is missing..
	 * Only call this function after IsActorBelongsToSubPrefab.
	 */
	bool IsWidgetBelongsToMissingSubPrefab(const ULexWidget* InWidget);
	bool IsSubPrefabRootWidget(const ULexWidget* InWidget);
	bool IsWidgetBelongsToThis(const ULexWidget* InWidget);
	bool ClearInvalidObjectAndGuid();
	void AddMemberPropertyToSubPrefab(ULexWidget* InSubPrefabWidget, UObject* InObject, FName InPropertyName);
	void RemoveMemberPropertyFromSubPrefab(ULexWidget* InSubPrefabWidget, UObject* InObject, FName InPropertyName);
	void RemoveAllMemberPropertyFromSubPrefab(ULexWidget* InSubPrefabActor, bool InIncludeRootTransform);
	FLexUISubPrefabData GetSubPrefabData(ULexWidget* InSubPrefabWidget);
	ULexWidget* GetSubPrefabRootWidget(ULexWidget* InSubPrefabWidget);
	/** For parent prefab. When parent prefab want to apply override parameter to subprefab, but the parameter belongs to subprefab's subprefab, then we need to mark override parameter for subprefab. */
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, const TArray<FName>& InPropertyNames);
	void MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName);

	/** If sub prefab changed, then update parent prefab */
	bool RefreshOnSubPrefabDirty(ULexUIPrefab* InSubPrefab, ULexWidget* InSubPrefabRootWidget = nullptr);

	void CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject);

	void RevertPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInDst, const FLexUISubPrefabData& SubPrefabData, int RawArrayIndex = 0, bool IsInsideRawArray = false);
	void ApplyPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInDst, const FLexUISubPrefabData& SubPrefabData, int RawArrayIndex = 0, bool IsInsideRawArray = false);
	FName GetExtraRelatedPropertyForApplyOrRevert(UObject* InObject, FName InPropertyName);
	void AfterObjectPropertyApplyOrRevert(UObject* InObject, FName InPropertyName);

	void RevertPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames);
	void RevertAllPrefabOverride(UObject* InObject);
	void ApplyPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames);
	void ApplyAllOverrideToPrefab(UObject* InObject);

	void RefreshSubPrefabVersion(ULexWidget* InSubPrefabRootWidget);

	void MakePrefabAsSubPrefab(ULexUIPrefab* InPrefab, ULexWidget* InWidget, const TMap<FGuid, TObjectPtr<UObject>>& InSubMapGuidToObject, const TArray<FLexUIPrefabOverrideParameterData>& InObjectOverrideParameterArray);
	void BreakPrefabVariant();
	void RemoveSubPrefabByRootWidget(ULexWidget* InPrefabRootWidget);
	void RemoveSubPrefabByAnyWidgetOfSubPrefab(ULexWidget* InPrefabWidget);
	ULexUIPrefab* GetPrefabAssetBySubPrefabObject(UObject* InObject);
	bool GetAnythingDirty()const;
	void SetAnythingDirty();
	void CheckPrefabVersion();
	FSimpleMulticastDelegate OnSubPrefabNewVersionUpdated;
	/**
	 * @return	true if anything changed
	 */
	bool CleanupInvalidSubPrefab();
private:
	bool bAnythingDirty = false;
	bool bCanCollectProperty = true;
	bool bCanNotifyComponentCreateDelete = true;
	bool bAlreadyShowMessageAtThisFrame = false;

	void OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent);
	void OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain);
	void TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty);

	UWorld* GetPrefabWorld()const;

	/**
	 * For Level prefab only. Object link could still exist if delete sub-prefab's root widget by UE's delete function.
	 * @return true if anything change
	 */
	bool CleanupInvalidLinkToSubPrefabObject();

public:
	static ULexUIPrefabHelperObject* GetPrefabHelperObject_WhichManageThisWidget(ULexWidget* InWidget);
#endif
};