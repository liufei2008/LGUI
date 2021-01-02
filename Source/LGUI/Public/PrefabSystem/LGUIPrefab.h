// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefab.generated.h"

/**
 * similar to Unity3D's Prefab. store actor and it's hierarchy and serailize to asset, deserialize and restore when need.
 * See property "UseBuildData" to get more information. 
 */
UCLASS(BlueprintType)
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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FText> ReferenceTextList;
	/** put actural UClass in this array, and store index in prefab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UClass*> ReferenceClassList;

#if WITH_EDITORONLY_DATA
	/**
	 * use build data?
	 * after serialize, two data array will be saved --EditorData(BinaryData) and BinaryDataForBuild. build game will only use BinaryDataForBuild. UseBuildData can test BinaryDataForBuild in editor, if anything wrong happens, you can recreate prefab by click "RecreateThis".
	 *
	 * EditorData contains all properties including EditorOnly properties, when deserialize from it, LGUIPrefab will compare property name and type and then set the property value.
	 *
	 * BinaryDataForBuild don't contains EditorOnly property, when deserialize from it, LGUIPrefab will set property value by memory order, which is much faster than EditorData.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseBuildData = false;
	/** serialized data for editor use, this data contains property's name, will compare property name when deserialize form this */
	UPROPERTY()
		TArray<uint8> BinaryData;
	/** BinaryData's length */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint64 DataCount;
#endif
	/** Engine's major version when creating this prefab */
	UPROPERTY()
		uint16 EngineMajorVersion;
	/** Engine's minor version when creating this prefab */
	UPROPERTY()
		uint16 EngineMinorVersion;
	/** serialized data for publish, not contain property name and editor only property. much more faster than BinaryData when deserialize */
	UPROPERTY()
		TArray<uint8> BinaryDataForBuild;
#if WITH_EDITORONLY_DATA
	/** BinaryDataForBuild's length */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint64 DataCountForBuild;
	UPROPERTY(Instanced, Transient)
		class UThumbnailInfo* ThumbnailInfo;
	UPROPERTY(Transient)
		bool ThumbnailDirty = false;
#endif
};
