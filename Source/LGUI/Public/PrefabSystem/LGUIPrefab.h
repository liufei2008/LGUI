// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	UPROPERTY(VisibleAnywhere, Category = "LGUI")ULGUIPrefab* PrefabAsset;
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
		uint8 CanvasRenderMode = 0;//default is ELGUIRenderMode::ScreenSpaceOverlay
};

//@todo: prefab variant

/**
 * Similar to Unity3D's Prefab. Store actor and it's hierarchy and serailize to asset, deserialize and restore when needed.
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, NotBlueprintable)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	ULGUIPrefab();
	friend class FLGUIPrefabCustomization;

#if WITH_EDITORONLY_DATA
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
	UPROPERTY()int32 ArchiveVersion = (int32)EUnrealEngineObjectUE4Version::VER_UE4_CORRECT_LICENSEE_FLAG;//this default version is the time when LGUIPrefab support FArchive version
	UPROPERTY()int32 ArchiveVersionUE5 = -1;
	UPROPERTY()int32 ArchiveLicenseeVer = (int32)EUnrealEngineObjectLicenseeUEVersion::VER_LIC_NONE;
	UPROPERTY()uint32 ArEngineNetVer = (uint32)EEngineNetworkVersionHistory::HISTORY_REPLAY_DORMANCY;
	UPROPERTY()uint32 ArGameNetVer = 0;

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
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefab(UObject* WorldContextObject, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject"), Category = LGUI)
		AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FRotator Rotation, FVector Scale);
	AActor* LoadPrefabWithTransform(UObject* WorldContextObject, USceneComponent* InParent, FVector Location, FQuat Rotation, FVector Scale);
	AActor* LoadPrefab(UWorld* InWorld, USceneComponent* InParent, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab and keep reference of source objects.
	 */
	AActor* LoadPrefabWithExistingObjects(UWorld* InWorld, USceneComponent* InParent
		, TMap<FGuid, UObject*>& InOutMapGuidToObject, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
		, bool InSetHierarchyIndexForRootComponent = true
	);
	bool IsPrefabBelongsToThisSubPrefab(ULGUIPrefab* InPrefab, bool InRecursive);
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
	virtual void PostRename(UObject* OldOuter, const FName OldName)override;
	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams)override;
	virtual void PostDuplicate(bool bDuplicateForPIE)override;
	virtual void PostLoad()override;
	virtual void BeginDestroy()override;
	virtual void FinishDestroy()override;
	virtual void PostEditUndo()override;

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
