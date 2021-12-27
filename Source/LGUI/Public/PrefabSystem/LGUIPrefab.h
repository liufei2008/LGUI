﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LGUIPrefab.generated.h"

/**
 * Current prefab system version
 */
#define LGUI_CURRENT_PREFAB_VERSION 3
/**
 * Version 2: Support ActorGuid (start from 4.26)
 * Version 3: Use UE's build-in FArchive to serialize/deserialize. 
		Compare to version2: 1. About 2~3 times faster when deserialize.
							 2. Smaller disc space take.
							 3. Support CoreRedirects.
							 4. Support object flags.
							 5. Support all object serialization and reference, inlude default sub object and component.
 */
#define LGUI_PREFAB_VERSION_BuildinFArchive 3

class ULGUIPrefab;
class ULGUIPrefabOverrideParameterObject;
class ULGUIPrefabHelperObject;

USTRUCT(NotBlueprintType)
struct LGUI_API FLGUISubPrefabData
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")ULGUIPrefab* PrefabAsset;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TWeakObjectPtr<ULGUIPrefabOverrideParameterObject> OverrideParameterObject;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TArray<uint8> OverrideParameterData;
};

/**
 * Similar to Unity3D's Prefab. store actor and it's hierarchy and serailize to asset, deserialize and restore when need.
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, EditInlineNew)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	ULGUIPrefab();
	/** put actural UObject in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UObject*> ReferenceAssetList;
	/** put actural UClass in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UClass*> ReferenceClassList;
	/** put actural FName in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FName> ReferenceNameList;
#pragma region Before Prefab-Version 3
	/** put actural FString in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FString> ReferenceStringList;
	/** put actural FText in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FText> ReferenceTextList;
#pragma endregion Before Prefab-Version 3

#if WITH_EDITORONLY_DATA
	/** serialized data for editor use, this data contains editor-only property include property's name, will compare property name when deserialize form this */
	UPROPERTY()
		TArray<uint8> BinaryData;
	/** The time point when create this prefab. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		FDateTime CreateTime;
#endif
	/** Prefab system's version when creating this prefab */
	UPROPERTY()
		uint16 PrefabVersion;
	/** Engine's major version when creating this prefab */
	UPROPERTY()
		uint16 EngineMajorVersion;
	/** Engine's minor version when creating this prefab */
	UPROPERTY()
		uint16 EngineMinorVersion;
	/** 
	 * serialized data for publish, not contain property name and editor only property. much more faster than BinaryData when deserialize
	 */
	UPROPERTY()
		TArray<uint8> BinaryDataForBuild;
	/** This property contains this prefab's overrideable parameters data, can use LGUIObjectReader to reproduce ULGUIPrefabOverrideParameterObject. */
	UPROPERTY()
		TArray<uint8> OverrideParameterData;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced, Transient)
		class UThumbnailInfo* ThumbnailInfo;
	UPROPERTY(Transient)
		bool ThumbnailDirty = false;
	//UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
	UPROPERTY()
		ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
#endif
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);
	AActor* LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
#if WITH_EDITOR
public:
	void MakeAgentObjectsInPreviewWorld();
	void ClearAgentObjectsInPreviewWorld();
	void RefreshAgentObjectsInPreviewWorld();
	/** Refresh it. Note this will use agent data to serialize, so if the prefab editor is opened for this prefab, then we should not use this function, or modifyed value in prefab editor will lose */
	void RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab);

	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void WillNeverCacheCookedPlatformDataAgain()override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void PostInitProperties()override;
	virtual void PostCDOContruct()override;
	virtual bool PreSaveRoot(const TCHAR* Filename)override;
	virtual void PostSaveRoot(bool bCleanupIsRequired)override;
	virtual void PreSave(const class ITargetPlatform* TargetPlatform)override;
	virtual void PostRename(UObject* OldOuter, const FName OldName)override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostLoad()override;
	virtual void BeginDestroy()override;
	virtual void FinishDestroy()override;

	/**
	 * LoadPrefab for edit/modify, will keep reference of source prefab.
	 */
	AActor* LoadPrefabForEdit(UWorld* InWorld, USceneComponent* InParent
		, TMap<FGuid, TWeakObjectPtr<UObject>>& InOutMapGuidToObject, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
		, const TArray<uint8>& InOverrideParameterData, TWeakObjectPtr<ULGUIPrefabOverrideParameterObject>& OutOverrideParameterObject
	);
	void SavePrefab(AActor* RootActor
		, TMap<TWeakObjectPtr<UObject>, FGuid>& InOutMapObjectToGuid, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
		, TWeakObjectPtr<ULGUIPrefabOverrideParameterObject> InOverrideParameterObject, TArray<uint8>& OutOverrideParameterData
		, bool InForEditorOrRuntimeUse = true
	);
	void SavePrefabForRuntime(AActor* RootActor, TMap<TWeakObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap);
	/**
	 * LoadPrefab in editor, will not keep reference of source prefab, So we can't apply changes after modify it.
	 */
	AActor* LoadPrefabInEditor(UWorld* InWorld, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
#endif
};
