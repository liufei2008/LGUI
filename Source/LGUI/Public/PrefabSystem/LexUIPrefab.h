// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Misc/NetworkVersion.h"
#include "Engine/EngineBaseTypes.h"
#include "LexUIPrefabInstanceScene.h"
#include "LexUIPrefab.generated.h"

#define LEXUIPREFAB_SERIALIZER_NEWEST_INCLUDE "PrefabSystem/WidgetSerializer.h"
#define LEXUIPREFAB_SERIALIZER_NEWEST_NAMESPACE LexUIPrefabSystem

class ULexWidget;

enum class ELexUIPrefabVersion : uint16
{
	/** Version 2: Support ActorGuid (start from 4.26). */
	OldVersion = 2,
	/**
	 * Version 3: Use UE's build-in FArchive to serialize/deserialize.
	 *		Compare to version2: 1. About 2~3 times faster when deserialize.
	 *							 2. Smaller disc space.
	 *							 3. Support CoreRedirects.
	 *							 4. Support object flags.
	 *							 5. Support all object serialization and reference, include default sub object and component.
	 */
	BuiltinFArchive = 3,
	/** Support nested default sub object. */
	NestedDefaultSubObject = 4,
	/** Support UObject name. */
	ObjectName = 5,
	/** Support common actor types, not just UI actor. */
	CommonActor = 6,
	/** Support new actor under sub-prefab's actor. */
	ActorAttachToSubPrefab = 7,
	/**
	 * This version is mainly to solve the case:
	 *		There are Prefabs, A is origin prefab, B contains A, C contains B,
	 *		open A and add a new object O to A (new Actor or ActorComponent or other UObject), apply A then close,
	 *		then open C and modify property on object O, apply C then close,
	 *		open C again, here error happens, because O is not exist in B yet, so B will always create new guid for O, then pass to C as sub-prefab, so when open C again, the modified property on O will not serialize, because sub-prefab's guid on O is changed.
	 * Solution:
	 *		Use a map data D, map from object's unique id (sub-prefab's root actor's guid and new created object's origin guid --origin guid means the object's guid in root prefab) to created guid,
	 *		when load sub-prefab, if not find guid then create a new guid and store it in data D, next time when load sub-prefab if still don't find the guid (because B create a new guid for it) then search in data D and use existing guid,
	 *		so the guid can persist.
	 */
	NewObjectOnNestedPrefab = 8,
	/**
	 * Serialize FText as reference, to solve problem about FText serialization from 5.7 to 5.8
	 * This version also use WidgetSerializer, just change LexUIObjectReaderAndWriter's FArchive<<FText to serialize FText as reference, so it is not compatible with previous version.
	 * Note: This version is not compatible with previous version, so if you want to use this version, you need to re-create all prefab assets.
	 */
	FTextAsReference = 9,

	/** new version must be added before this line. */
	MAX_NO_USE,
	NEWEST = MAX_NO_USE - 1,
};

/**
 * Current prefab system version
 */
#define LEXUI_CURRENT_PREFAB_VERSION (uint16)ELexUIPrefabVersion::NEWEST

class ULexUIPrefab;
class ULexUIPrefabHelperObject;

USTRUCT(NotBlueprintType)
struct LGUI_API FLexUIPrefabOverrideParameterData
{
	GENERATED_BODY()
public:
	FLexUIPrefabOverrideParameterData() {};

	UPROPERTY(EditAnywhere, Category = "LGUI")
		TWeakObjectPtr<UObject> Object;
	/** UObject's member property name */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FName> MemberPropertyNames;
};

/** Unique id for newly created object in sub-prefab, just for store data here. Check description on ELexUIPrefabVersion.NewObjectOnNestedPrefab */
USTRUCT(NotBlueprintType)
struct FLexUISubPrefabObjectUniqueId
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FGuid RootWidgetGuidInParentPrefab;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FGuid ObjectGuidInOriginPrefab;

	bool operator==(const FLexUISubPrefabObjectUniqueId& other)const
	{
		return this->RootWidgetGuidInParentPrefab == other.RootWidgetGuidInParentPrefab && this->ObjectGuidInOriginPrefab == other.ObjectGuidInOriginPrefab;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLexUISubPrefabObjectUniqueId& other)
	{
		return HashCombine(GetTypeHash(other.RootWidgetGuidInParentPrefab), GetTypeHash(other.ObjectGuidInOriginPrefab));
	}
};

USTRUCT(NotBlueprintType)
struct LGUI_API FLexUISubPrefabData
{
	GENERATED_BODY()
public:
	FLexUISubPrefabData();
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TObjectPtr<ULexUIPrefab> PrefabAsset = nullptr;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TArray<FLexUIPrefabOverrideParameterData> ObjectOverrideParameterArray;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;
	/** Check description on ELexUIPrefabVersion.NewObjectOnNestedPrefab */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TMap<FLexUISubPrefabObjectUniqueId, FGuid> MapObjectIdToNewlyCreatedId;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
#if WITH_EDITORONLY_DATA
	/** For level editor, combine all create time (include all sub prefab) to create this MD5, to tell if this prefab is latest version. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")FString OverallVersionMD5;
	/** Temporary color for quick identify in editor */
	FLinearColor EditorIdentifyColor;
#endif
public:
	void AddMemberProperty(UObject* InObject, FName InPropertyName);
	void AddMemberProperty(UObject* InObject, const TArray<FName>& InPropertyNames);
	void RemoveMemberProperty(UObject* InObject, FName InPropertyName);
	void RemoveMemberProperty(UObject* InObject);
	/** 
	 * Check parameters, remove invalid.
	 * @return true if anything changed.
	 */
	bool CheckParameters();
};

USTRUCT(NotBlueprintType)
struct FLexUIPrefabDataForPrefabEditor
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FVector ViewLocation = FVector::ZeroVector;
	UPROPERTY()
	FRotator ViewRotation = FRotator::ZeroRotator;
	UPROPERTY()
	FVector ViewOrbitLocation = FVector::ZeroVector;
	UPROPERTY()
	FIntPoint CanvasSize = FIntPoint(1920, 1080);
	UPROPERTY()
	uint8 CanvasRenderMode = 3;//default LexCanvas's render mode is ELexUIRenderMode::WorldSpace_LexUI
	UPROPERTY()
	TEnumAsByte<EViewModeIndex> ViewMode = EViewModeIndex::VMI_Lit;//editor viewport's view-mode
	UPROPERTY()
	uint8 ViewportType = 2;//ELevelViewportType::LVT_OrthoYZ
	UPROPERTY()
	TSet<FGuid> UnexpandedWidgetSet;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FLexUIPrefab_LoadPrefabCallback, ULexWidget*, LoadedRootWidget);

/**
 * Similar to Unity3D's Prefab. Store actor and it's hierarchy then serialize to asset, deserialize and restore when needed.
 * If you don't want to package the prefab for runtime (only use in editor), you can put the prefab in a folder named "EditorOnly".
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, DisplayName="LexUI Prefab")
class LGUI_API ULexUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	ULexUIPrefab();
	friend class ULexUIPrefabHelperObject;
	friend class FLexUIPrefabCustomization;
	friend class ULexUIPrefabFactory;

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		bool bIsPrefabVariant = false;
public:
	/** put actual UObject in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<TObjectPtr<UObject>> ReferenceAssetList;
	/** put actual UClass in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<TObjectPtr<UClass>> ReferenceClassList;
	/** put actual FName in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FName> ReferenceNameList;
	/** put actual FText in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
	TArray<FText> ReferenceTextList;
#endif

#if WITH_EDITORONLY_DATA
public:
	/** serialized data for editor use, this data contains editor-only property include property's name, will compare property name when deserialize form this */
	UPROPERTY()
		TArray<uint8> BinaryData;
	/** The time point when create/save this prefab. Use UtcNow from prefab version 6. */
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
	UPROPERTY()int32 ArchiveVersionUE5 = -1;
	UPROPERTY()int32 ArchiveLicenseeVer = (int32)EUnrealEngineObjectLicenseeUEVersion::VER_LIC_NONE;
	UPROPERTY()uint32 ArEngineNetVer = (uint32)FEngineNetworkCustomVersion::ReplayDormancy;
	UPROPERTY()uint32 ArGameNetVer = 0;
#endif
	UPROPERTY()int32 ArchiveVersion_ForBuild = (int32)EUnrealEngineObjectUE4Version::VER_UE4_CORRECT_LICENSEE_FLAG;//this default version is the time when LGUIPrefab support FArchive version
	UPROPERTY()int32 ArchiveVersionUE5_ForBuild = -1;
	UPROPERTY()int32 ArchiveLicenseeVer_ForBuild = (int32)EUnrealEngineObjectLicenseeUEVersion::VER_LIC_NONE;
	UPROPERTY()uint32 ArEngineNetVer_ForBuild = (uint32)FEngineNetworkCustomVersion::ReplayDormancy;
	UPROPERTY()uint32 ArGameNetVer_ForBuild = 0;

	/** build version for ReferenceAssetList */
	UPROPERTY()
		TArray<TObjectPtr<UObject>> ReferenceAssetListForBuild;
	/** build version for ReferenceClassList */
	UPROPERTY()
		TArray<TObjectPtr<UClass>> ReferenceClassListForBuild;
	/** build version for ReferenceNameList */
	UPROPERTY()
		TArray<FName> ReferenceNameListForBuild;
	/** build version for ReferenceTextList */
	UPROPERTY()
	TArray<FText> ReferenceTextListForBuild;
	/**
	 * serialized data for publish, not contain property name and editor only property. much more faster than BinaryData when deserialize
	 */
	UPROPERTY()
		TArray<uint8> BinaryDataForBuild;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced, Transient)
		TObjectPtr<class UThumbnailInfo> ThumbnailInfo;
	UPROPERTY(Transient)
		bool bThumbnailDirty = false;
	UPROPERTY()
		FLexUIPrefabDataForPrefabEditor PrefabDataForPrefabEditor;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", DuplicateTransient)
		TObjectPtr<ULexUIPrefabHelperObject> PrefabHelperObject = nullptr;
	TUniquePtr<FLexUIPrefabInstanceScene> PrefabInstanceScene;
#endif
public:
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LexUIBehaviour and LGUIPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param InCallbackBeforeAwake This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
	 * @param SetRelativeTransformToIdentity Set created root actor's transform to zero after load.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InCallbackBeforeAwake,SetRelativeTransformToIdentity", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InCallbackBeforeAwake"), Category = LGUI)
		ULexWidget* LoadPrefab(UObject* WorldContextObject, ULexWidget* InParent, const FLexUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LexUIBehaviour and LexUIPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param Location Set created root actor's location after load.
	 * @param Rotation Set created root actor's rotation after load.
	 * @param Scale Set created root actor's scale after load.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InCallbackBeforeAwake", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InCallbackBeforeAwake"), Category = LGUI)
		ULexWidget* LoadPrefabWithTransform(UObject* WorldContextObject, ULexWidget* InParent, FVector Location, FRotator Rotation, FVector Scale, const FLexUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake);
	ULexWidget* LoadPrefabWithTransform(UObject* WorldContextObject, ULexWidget* InParent, FVector Location, FQuat Rotation, FVector Scale, const TFunction<void(ULexWidget*)>& InCallbackBeforeAwake);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LexUIBehaviour and LexUIPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent widget that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param InReplaceAssetMap Replace source asset to dest before load the prefab.
	 * @param InReplaceClassMap Replace source class to dest before load the prefab.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "InCallbackBeforeAwake", UnsafeDuringActorConstruction = "true", WorldContext = "WorldContextObject", AutoCreateRefTerm = "InCallbackBeforeAwake"), Category = LGUI)
		ULexWidget* LoadPrefabWithReplacement(UObject* WorldContextObject, ULexWidget* InParent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, const FLexUIPrefab_LoadPrefabCallback& InCallbackBeforeAwake);
	/**
	 * LoadPrefab to create actor.
	 * Awake function in LexUIBehaviour and LexUIPrefabInterface will be called right after LoadPrefab is done.
	 * @param InParent Parent scene component that the created root actor will be attached to. Can be null so the created root actor will not attach to anyone.
	 * @param SetRelativeTransformToIdentity Set created root actor's transform to zero after load.
	 * @param InCallbackBeforeAwake This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
	 */
	ULexWidget* LoadPrefab(UWorld* InWorld, ULexWidget* InParent, const TFunction<void(ULexWidget*)>& InCallbackBeforeAwake = nullptr, bool SetRelativeTransformToIdentity = false);
	/**
	 * LoadPrefab and keep reference of source objects.
	 */
	ULexWidget* LoadPrefabWithExistingObjects(UWorld* InWorld, UObject* InOuter, ULexWidget* InParent
		, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObject, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutSubPrefabMap
	);
	bool IsPrefabBelongsToThisSubPrefab(ULexUIPrefab* InPrefab, bool InRecursive);
#if WITH_EDITOR
	void CopyDataTo(ULexUIPrefab* TargetPrefab);
	bool GetIsPrefabVariant()const { return bIsPrefabVariant; }
	FString GenerateOverallVersionMD5();
#endif
private:
#if WITH_EDITOR
	void SetRootWidgetNameFromPrefab();
public:
	FLexUIPrefabInstanceScene* GetPrefabInstanceScene();
	void ClearPrefabInstanceScene();
	void EnsureInstanceObjects();
	ULexUIPrefabHelperObject* GetPrefabHelperObject();

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
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;

	bool SavePrefab(ULexWidget* RootWidget
		, TMap<UObject*, FGuid>& InOutMapObjectToGuid, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& InSubPrefabMap
		, bool InForEditorOrRuntimeUse = true
	);
	void RecreatePrefab();
	/**
	 * @todo: There is a more efficient way for dealing with sub prefab in runtime: break sub prefab and store all actors (with override parameters) in root prefab.
	 */
	//void SavePrefabForRuntime(AActor* RootActor, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap);
	/**
	 * LoadPrefab in editor, will not keep reference of source prefab, So we can't apply changes after modify it.
	 */
	ULexWidget* LoadPrefabInEditor(UWorld* InWorld, UObject* InOuter, ULexWidget* Parent);
	ULexWidget* LoadPrefabInEditor(UWorld* InWorld, UObject* InOuter, ULexWidget* Parent, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutSubPrefabMap, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject, bool SetRelativeTransformToIdentity = true);
#endif
};
