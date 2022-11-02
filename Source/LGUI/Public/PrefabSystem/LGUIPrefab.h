﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Misc/NetworkVersion.h"
#include "LGUIPrefab.generated.h"

enum class ELGUIPrefabVersion : uint16
{
	/** Version 2: Support ActorGuid (start from 4.26). */
	OldVersion = 2,
	/**
	 * Version 3: Use UE's build-in FArchive to serialize/deserialize.
	 *		Compare to version2: 1. About 2~3 times faster when deserialize.
	 *							 2. Smaller disc space take.
	 *							 3. Support CoreRedirects.
	 *							 4. Support object flags.
	 *							 5. Support all object serialization and reference, inlude default sub object and component.
	 */
	BuildinFArchive = 3,
	/** Support nested default sub object. */
	NestedDefaultSubObject = 4,
	/** Support UObject name. */
	ObjectName = 5,

	/** new version must be added before this line. */
	MAX_NO_USE,
	NEWEST = MAX_NO_USE - 1,
};

/**
 * Current prefab system version
 */
#define LGUI_CURRENT_PREFAB_VERSION (uint16)ELGUIPrefabVersion::NEWEST

class ULGUIPrefab;
class ULGUIPrefabHelperObject;

USTRUCT(NotBlueprintType)
struct LGUI_API FLGUIPrefabOverrideParameterData
{
	GENERATED_BODY()
public:
	FLGUIPrefabOverrideParameterData() {};

	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<UObject> Object;
	/** UObject's member property name */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TSet<FName> MemberPropertyName;
};

USTRUCT(NotBlueprintType)
struct LGUI_API FLGUISubPrefabData
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")ULGUIPrefab* PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TArray<FLGUIPrefabOverrideParameterData> ObjectOverrideParameterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TMap<FGuid, UObject*> MapGuidToObject;
#if WITH_EDITORONLY_DATA
	/** For level editor, to tell if this prefab is latest version. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")FDateTime TimePointWhenSavePrefab;
#endif
public:
	void AddMemberProperty(UObject* InObject, FName InPropertyName);
	void AddMemberProperty(UObject* InObject, const TSet<FName>& InPropertyNameSet);
	void RemoveMemberProperty(UObject* InObject, FName InPropertyName);
	void RemoveMemberProperty(UObject* InObject);
	/** 
	 * Check parameters, remove invalid.
	 * @return true if anything changed.
	 */
	bool CheckParameters();
};

USTRUCT(NotBlueprintType)
struct FLGUIPrefabDataForPrefabEditor
{
	GENERATED_BODY()
public:
	UPROPERTY()
		FVector ViewLocationForPrefabEditor = FVector::ZeroVector;
	UPROPERTY()
		FRotator ViewRotationForPrefabEditor = FRotator::ZeroRotator;
	UPROPERTY()
		FIntPoint CanvasSize = FIntPoint(1920, 1080);
	UPROPERTY()
		bool bNeedCanvas = true;//do we need LGUICanvas component? default is true
	UPROPERTY()
		uint8 CanvasRenderMode = 0;//default LGUICanvas's render mode is ELGUIRenderMode::ScreenSpaceOverlay
	UPROPERTY()
		TEnumAsByte<EViewModeIndex> ViewMode = EViewModeIndex::VMI_Lit;//editor viewport's viewmode
	UPROPERTY()
		TSet<FGuid> UnexpendActorSet;
};

/**
 * Similar to Unity3D's Prefab. Store actor and it's hierarchy and serailize to asset, deserialize and restore when needed.
 * If you don't want to package the prefab for runtime (only use in editor), you can put the prefab in a folder named "EditorOnly".
 */
UCLASS(ClassGroup = (LGUI), BlueprintType)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	ULGUIPrefab();
	friend class FLGUIPrefabCustomization;
	friend class ULGUIPrefabFactory;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		bool bIsPrefabVariant = false;
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
#endif

#if WITH_EDITORONLY_DATA
public:
	/** serialized data for editor use, this data contains editor-only property include property's name, will compare property name when deserialize form this */
	UPROPERTY()
		TArray<uint8> BinaryData;
	/** The time point when create/save this prefab. */
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
	UPROPERTY()
		uint16 EnginePatchVersion;
#if WITH_EDITORONLY_DATA
	UPROPERTY()int32 ArchiveVersion = (int32)EUnrealEngineObjectUE4Version::VER_UE4_CORRECT_LICENSEE_FLAG;//this default version is the time when LGUIPrefab support FArchive version
	UPROPERTY()int32 ArchiveLicenseeVer = (int32)EUnrealEngineObjectLicenseeUE4Version::VER_LIC_NONE;
	UPROPERTY()uint32 ArEngineNetVer = (uint32)EEngineNetworkVersionHistory::HISTORY_REPLAY_DORMANCY;
	UPROPERTY()uint32 ArGameNetVer = 0;
#endif
	UPROPERTY()int32 ArchiveVersion_ForBuild = (int32)EUnrealEngineObjectUE4Version::VER_UE4_CORRECT_LICENSEE_FLAG;//this default version is the time when LGUIPrefab support FArchive version
	UPROPERTY()int32 ArchiveLicenseeVer_ForBuild = (int32)EUnrealEngineObjectLicenseeUE4Version::VER_LIC_NONE;
	UPROPERTY()uint32 ArEngineNetVer_ForBuild = (uint32)EEngineNetworkVersionHistory::HISTORY_REPLAY_DORMANCY;
	UPROPERTY()uint32 ArGameNetVer_ForBuild = 0;

	/** build version for ReferenceAssetList */
	UPROPERTY()
		TArray<UObject*> ReferenceAssetListForBuild;
	/** build version for ReferenceClassList */
	UPROPERTY()
		TArray<UClass*> ReferenceClassListForBuild;
	/** build version for ReferenceNameList */
	UPROPERTY()
		TArray<FName> ReferenceNameListForBuild;
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
	UPROPERTY()
		FLGUIPrefabDataForPrefabEditor PrefabDataForPrefabEditor;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		ULGUIPrefabHelperObject* PrefabHelperObject = nullptr;
#endif
public:
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param SetRelativeTransformToIdentity Set created root actor's transform to zero after load.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param Location Set created root actor's location after load.
	 * @param Rotation Set created root actor's rotation after load.
	 * @param Scale Set created root actor's scale after load.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param InReplaceAssetMap Replace source asset to dest before load the prefab.
	 * @param InReplaceClassMap Replace source class to dest before load the prefab.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefabWithReplacement(UObject* WorldContextObject, USceneComponent* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap);
	AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LGUILifeCycleBehaviour will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param SetRelativeTransformToIdentity Set created root actor's transform to zero after load.
	 */
	AActor* LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab and keep reference of source objects.
	 */
	AActor* LoadPrefabWithExistingObjects(UWorld* InWorld, USceneComponent* InParent
		, TMap<FGuid, UObject*>& InOutMapGuidToObject, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
		, bool InSetHierarchyIndexForRootComponent = true
	);
	bool IsPrefabBelongsToThisSubPrefab(ULGUIPrefab* InPrefab, bool InRecursive);
#if WITH_EDITOR
	void CopyDataTo(ULGUIPrefab* TargetPrefab);
	bool GetIsPrefabVariant()const { return bIsPrefabVariant; }
#endif
private:
#if WITH_EDITOR
	TWeakObjectPtr<AActor> ContainerActor;//container actor for UI or common actor
	AActor* GetContainerActor();
public:
	void MakeAgentObjectsInPreviewWorld();
	void ClearAgentObjectsInPreviewWorld();
	void RefreshAgentObjectsInPreviewWorld();
	ULGUIPrefabHelperObject* GetPrefabHelperObject();

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
	virtual void PostEditUndo()override;
	virtual bool IsEditorOnly()const override;

	void SavePrefab(AActor* RootActor
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
		, bool InForEditorOrRuntimeUse = true
	);
	/**
	 * @todo: There is a more efficient way for dealing with sub prefab in runtime: break sub prefab and store all actors (with override parameters) in root prefab.
	 */
	void SavePrefabForRuntime(AActor* RootActor, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap);
	/**
	 * LoadPrefab in editor, will not keep reference of source prefab, So we can't apply changes after modify it.
	 */
	AActor* LoadPrefabInEditor(UWorld* InWorld, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
	AActor* LoadPrefabInEditor(UWorld* InWorld, USceneComponent* Parent, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap, TMap<FGuid, UObject*>& OutMapGuidToObject, bool SetRelativeTransformToIdentity = true);
#endif
};
