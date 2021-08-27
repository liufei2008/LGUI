// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefab.generated.h"

//Version 2: Support ActorGuid (start from 4.26), nested prefab
#define LGUI_PREFAB_VERSION 2

/**
 * similar to Unity3D's Prefab. store actor and it's hierarchy and serailize to asset, deserialize and restore when need.
 * See property "UseBuildData" to get more information. 
 */
UCLASS(ClassGroup = (LGUI), BlueprintType, EditInlineNew)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	/** put actural UObject in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UObject*> ReferenceAssetList;
	/** put actural FString in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FString> ReferenceStringList;
	/** put actural FName in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FName> ReferenceNameList;
	/** put actural FText in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FText> ReferenceTextList;
	/** put actural UClass in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UClass*> ReferenceClassList;
#if WITH_EDITORONLY_DATA
	/** map actor guid to sub prefabs, actor as parent node to the sub prefab. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TMap<FGuid, ULGUIPrefab*> SubPrefabs;
	/** serialized data for editor use, this data contains editor-only property include property's name, will compare property name when deserialize form this */
	UPROPERTY()
		TArray<uint8> BinaryData;
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
	/** map actor index to sub prefabs, actor as parent node to the sub prefab. */
	UPROPERTY()
		TMap<int32, ULGUIPrefab*> SubPrefabsForBuild;
#if WITH_EDITORONLY_DATA
	UPROPERTY(Instanced, Transient)
		class UThumbnailInfo* ThumbnailInfo;
	UPROPERTY(Transient)
		bool ThumbnailDirty = false;
#endif

#if WITH_EDITOR
public:
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void WillNeverCacheCookedPlatformDataAgain()override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void PreSave(const class ITargetPlatform* TargetPlatform)override;
	void CopyTo(ULGUIPrefab* Other);
#endif
};
