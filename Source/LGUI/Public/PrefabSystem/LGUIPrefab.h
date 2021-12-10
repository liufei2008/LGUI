// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefab.generated.h"

/**
 * Current prefab system version
 */
#define LGUI_CURRENT_PREFAB_VERSION 3
/**
 * Version 2: Support ActorGuid (start from 4.26)
 * Version 3: Use UE's build-in FArchive to serialize/deserialize. 
		Compare to version2: 1. About 2~3 times faster when deserialize.
							 2. Support all object serialization, inlude blueprint's default sub object.
 */
#define LGUI_PREFAB_VERSION_BuildinFArchive 3

/**
 * Similar to Unity3D's Prefab. store actor and it's hierarchy and serailize to asset, deserialize and restore when need.
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, EditInlineNew)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
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
#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced, Transient)
		class UThumbnailInfo* ThumbnailInfo;
	UPROPERTY(Transient)
		bool ThumbnailDirty = false;
	/** This actor is an agent existing in a preview world, for cook prefab asset. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")AActor* AgentRootActor = nullptr;
#endif
public:
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);

#if WITH_EDITOR
public:
	void MakeAgentActorsInPreviewWorld();
	void ClearAgentActorsInPreviewWorld();

	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void WillNeverCacheCookedPlatformDataAgain()override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void PostSaveRoot(bool bCleanupIsRequired)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void BeginDestroy()override;

	/**
	 * LoadPrefab for edit/modify, will keep reference of source prefab.
	 */
	AActor* LoadPrefabForEdit(UWorld* InWorld, USceneComponent* InParent
		, TMap<FGuid, UObject*>& InOutMapGuidToObject
	);
	void SavePrefab(AActor* RootActor
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid);
	void SavePrefabForRuntime(AActor* RootActor);
	/**
	 * LoadPrefab in editor, will not keep reference of source prefab, So we can't apply changes after modify it.
	 */
	AActor* LoadPrefabInEditor(UWorld* InWorld, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
#endif
};
