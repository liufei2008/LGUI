// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"
#include "UObject/ObjectVersion.h"

class ALGUIManagerActor;

namespace LGUIPrefabSystem
{
	/*
	 * serialize/deserialize actor with hierarchy
	 */
	class LGUI_API ActorSerializerBase
	{
		friend class FLGUIObjectReader;
		friend class FLGUIObjectWriter;
		friend class FLGUIDuplicateObjectReader;
		friend class FLGUIDuplicateObjectWriter;
		friend class FLGUIOverrideParameterObjectWriter;
		friend class FLGUIOverrideParameterObjectReader;
		friend class FLGUIDuplicateOverrideParameterObjectWriter;
		friend class FLGUIDuplicateOverrideParameterObjectReader;

	public:
		virtual ~ActorSerializerBase() {}

		TMap<UObject*, TArray<uint8>> SaveOverrideParameterToData(TArray<FLGUIPrefabOverrideParameterData> InData);
		void RestoreOverrideParameterFromData(TMap<UObject*, TArray<uint8>>& InData, TArray<FLGUIPrefabOverrideParameterData> InNameSetData);

		virtual void SetupArchive(FArchive& InArchive);

		//Actor that belongs to this prefab. All UObjects which get outer of these actor can be serailized
		TArray<AActor*> WillSerializeActorArray;
		//Common UObjects that need to serialize. Outer object should stay at lower index then sub object, so when deserialize the outer object will created ealier, then the sub object can use the correct outer.
		TArray<UObject*> WillSerializeObjectArray;
		bool ObjectBelongsToThisPrefab(UObject* InObject);

		const TSet<FName>& GetSceneComponentExcludeProperties();
		bool CollectObjectToSerailize(UObject* Object, FGuid& OutGuid);
		//Check object and it's up outer to tell if it is trash
		bool ObjectIsTrash(UObject* InObject);
		//find id from list, if not then create
		int32 FindOrAddAssetIdFromList(UObject* AssetObject);
		int32 FindOrAddClassFromList(UClass* Class);
		int32 FindOrAddNameFromList(const FName& Name);
		//find object by id
		UObject* FindAssetFromListByIndex(int32 Id);
		UClass* FindClassFromListByIndex(int32 Id);
		FName FindNameFromListByIndex(int32 Id);
		TArray<UObject*> ReferenceAssetList;
		TArray<UClass*> ReferenceClassList;
		TArray<FName> ReferenceNameList;
		ALGUIManagerActor* LGUIManagerActor = nullptr;

		bool bOverrideVersions = false;
		uint16 PrefabVersion = 0;
		FPackageFileVersion ArchiveVersion;
		int32 ArchiveLicenseeVer = 0;
		FEngineVersionBase ArEngineVer;
		uint32 ArEngineNetVer = 0;
		uint32 ArGameNetVer = 0;
		TMap<FGuid, TObjectPtr<UObject>> MapGuidToObject;
		TMap<UObject*, FGuid> MapObjectToGuid;

	protected:
		UWorld* TargetWorld = nullptr;//world that need to spawn actor
		bool bIsEditorOrRuntime = true;
		static bool CanUseUnversionedPropertySerialization();
	};
}