// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "LGUIPrefab.generated.h"

/**
* similar to Unity3D's Prefab. store actor and it's hierarchy and serailize to asset, deserialize and restore when need
*/
UCLASS(BlueprintType)
class LGUI_API ULGUIPrefab : public UObject
{
	GENERATED_BODY()

public:
	//put actural UObject in this array, and store index in prefab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UObject*> ReferenceAssetList;
	//put actural FString in this array, and store index in prefab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FString> ReferenceStringList;
	//put actural FName in this array, and store index in prefab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<FName> ReferenceNameList;
	//put actural FText in this array, and store index in prefab
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FText> ReferenceTextList;
	//put actural UClass in this array, and store index in prefab
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		TArray<UClass*> ReferenceClassList;

#if WITH_EDITORONLY_DATA
	/*
	use build data?
	after serialize, two data array will be saved --BinaryData and BinaryDataForBuild. build game will only use BinaryDataForBuild. UseBuildData can test BinaryDataForBuild in editor, if anything wrong happens, you can recreate prefab by click "RecreateThis"
	BinaryData contains EditorOnly property, when deserialize from it, will compare property name
	BinaryDataForBuild dont contains EditorOnly property, when deserialize form it, will set property value by memory order, much faster than BinaryData
	*/
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseBuildData = false;
	//serialized data for editor use, this data contains property's name, will compare property name when deserialize form this
	UPROPERTY()
		TArray<uint8> BinaryData;
	//BinaryData's length
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint64 DataCount;
#endif
	//Engine's major version when creating this prefab
	UPROPERTY()
		uint16 EngineMajorVersion;
	//Engine's minor version when creating this prefab
	UPROPERTY()
		uint16 EngineMinorVersion;
	//serialized data for publish, not contain property name and editor only property. much more faster than BinaryData when deserialize
	UPROPERTY()
		TArray<uint8> BinaryDataForBuild;
#if WITH_EDITORONLY_DATA
	//BinaryDataForBuild's length
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint64 DataCountForBuild;
	UPROPERTY(Instanced, Transient)
		class UThumbnailInfo* ThumbnailInfo;
	UPROPERTY(Transient)
		bool ThumbnailDirty = false;
#endif
};
