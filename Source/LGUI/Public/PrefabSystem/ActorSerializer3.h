﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

class ULGUIPrefabOverrideParameterObject;
class UUIItem;

namespace LGUIPrefabSystem3
{
	//@todo: could use int as object's id instead of FGuid, because object's id only need to be unique inside single prefab, not even in nested prefab.

	struct FLGUIObjectSaveData
	{
	public:
		int32 ObjectClass = -1;
		FGuid ObjectGuid;//use id to find object.
		uint32 ObjectFlags;
		TArray<uint8> PropertyData;
		FGuid OuterObjectGuid;//outer object
		friend FArchive& operator<<(FArchive& Ar, FLGUIObjectSaveData& ObjectData)
		{
			Ar << ObjectData.ObjectClass;
			Ar << ObjectData.ObjectGuid;
			Ar << ObjectData.ObjectFlags;
			Ar << ObjectData.PropertyData;
			Ar << ObjectData.OuterObjectGuid;
			return Ar;
		}
	};

	//ActorComponent serialize and save data
	struct FLGUIComponentSaveData
	{
	public:
		int32 ComponentClass;
		FName ComponentName;
		FGuid ComponentGuid;
		uint32 ObjectFlags;
		FGuid SceneComponentParentGuid = FGuid();//invalid guid means the the SceneComponent dont have parent. @todo: For Blueprint-Actor's BlueprintCreatedComponent, leave this to invalid, because that component's parent is managed by blueprint
		FGuid OuterObjectGuid;//outer object
		TArray<uint8> PropertyData;
		friend FArchive& operator<<(FArchive& Ar, FLGUIComponentSaveData& ComponentData)
		{
			Ar << ComponentData.ComponentClass;
			Ar << ComponentData.ComponentName;
			Ar << ComponentData.ComponentGuid;
			Ar << ComponentData.ObjectFlags;
			Ar << ComponentData.SceneComponentParentGuid;
			Ar << ComponentData.OuterObjectGuid;
			Ar << ComponentData.PropertyData;
			return Ar;
		}
	};

	struct FLGUIPrefabOverrideParameterRecordData
	{
	public:
		FGuid ObjectGuid;
		TArray<uint8> OverrideParameterData;
		TSet<FName> OverrideParameterNameSet;
		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabOverrideParameterRecordData& Data)
		{
			Ar << Data.ObjectGuid;
			Ar << Data.OverrideParameterData;
			Ar << Data.OverrideParameterNameSet;
			return Ar;
		}
	};

	//Actor serialize and save data
	struct FLGUIActorSaveData
	{
	public:
		bool bIsPrefab = false;
		int32 PrefabAssetIndex;
		TArray<FLGUIPrefabOverrideParameterRecordData> ObjectOverrideParameterArray;//override sub prefab's parameter
		TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;//sub prefab's object use a different guid in parent prefab. So multiple same sub prefab can exist in same parent prefab.

		int32 ActorClass;
		FGuid ActorGuid;//use id to find actor
		uint32 ObjectFlags;
		TArray<uint8> ActorPropertyData;
		FGuid RootComponentGuid;
		/**
		 * The following two array stores default sub objects which belong to this actor. Array must match index for specific component. When deserialize, use FName to find FGuid.
		 * Common UObject/UActorComponent don't need these, because actor use "CollectDefaultSubobjects(xxx, true)" to find default sub objects, with nested objects, that should include all created default sub objects.
		 */
		TArray<FGuid> DefaultSubObjectGuidArray;
		TArray<FName> DefaultSubObjectNameArray;

		TArray<FLGUIActorSaveData> ChildActorData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveData& ActorData)
		{
			Ar << ActorData.bIsPrefab;
			if (ActorData.bIsPrefab)
			{
				Ar << ActorData.PrefabAssetIndex;
				Ar << ActorData.ActorGuid;//sub prefab's root actor's guid
				Ar << ActorData.ObjectOverrideParameterArray;
				Ar << ActorData.MapObjectGuidFromParentPrefabToSubPrefab;
			}
			else
			{
				Ar << ActorData.ActorGuid;
				Ar << ActorData.ActorClass;
				Ar << ActorData.ObjectFlags;
				Ar << ActorData.ActorPropertyData;
				Ar << ActorData.RootComponentGuid;
				Ar << ActorData.DefaultSubObjectGuidArray;
				Ar << ActorData.DefaultSubObjectNameArray;

				Ar << ActorData.ChildActorData;
			}
			return Ar;
		}
	};

	struct FLGUIPrefabSaveData
	{
	public:
		FLGUIActorSaveData SavedActor;
		TArray<FLGUIObjectSaveData> SavedObjects;
		TArray<FLGUIComponentSaveData> SavedComponents;

		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedActor;
			Ar << GameData.SavedObjects;
			Ar << GameData.SavedComponents;
			return Ar;
		}
	};


	/*
	 * serialize/deserialize actor with hierarchy
	 * Not support: LazyObject, SoftObject
	 */
	class LGUI_API ActorSerializer3
	{
	private:
		friend class FLGUIObjectReader;
		friend class FLGUIObjectWriter;
		friend class FLGUIDuplicateObjectReader;
		friend class FLGUIDuplicateObjectWriter;
		friend class FLGUIOverrideParameterObjectWriter;
		friend class FLGUIOverrideParameterObjectReader;

	public:
		ActorSerializer3();
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * LoadPrefab for edit/modify, will keep reference of source prefab.
		 */
		static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, TMap<FGuid, UObject*>& InOutMapGuidToObjects, TMap<AActor*, FLGUISubPrefabData>& OutSubPrefabMap
			, bool InSetHierarchyIndexForRootComponent = true
		);

		/** Save prefab data for editor use. */
		static void SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
			, TMap<UObject*, FGuid>& OutMapObjectToGuid, TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
			, bool InForEditorOrRuntimeUse
		);
		
		/**
		 * Duplicate actor with hierarchy
		 */
		static AActor* DuplicateActor(AActor* RootActor, USceneComponent* Parent);

		TMap<UObject*, TArray<uint8>> SaveOverrideParameterToData(TArray<FLGUIPrefabOverrideParameterData> InData);
		void RestoreOverrideParameterFromData(TMap<UObject*, TArray<uint8>>& InData, TArray<FLGUIPrefabOverrideParameterData> InNameSetData);
	private:
		static AActor* LoadSubPrefab(
			UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, TMap<FGuid, UObject*>& InMapGuidToObject
			, TFunction<void(AActor*, const TMap<FGuid, UObject*>&)> InOnSubPrefabFinishDeserializeFunction
			, bool InIsLoadForEdit
		);

		UWorld* TargetWorld = nullptr;//world that need to spawn actor
		bool bIsLoadForEdit = true;
		bool bIsEditorOrRuntime = true;
		bool bSetHierarchyIndexForRootComponent = false;//need to set hierarchyindex to last for root component?

		TMap<FGuid, UObject*> MapGuidToObject;
		struct ComponentDataStruct
		{
			UActorComponent* Component;
			FGuid SceneComponentParentGuid;
		};
		TArray<ComponentDataStruct> CreatedComponents;

		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;
		//Actor and ActorComponent that belongs to this prefab. All UObjects which get outer of these actor/component can be serailized
		TArray<UObject*> WillSerailizeActorArray;
		TArray<UObject*> WillSerailizeObjectArray;
		bool ObjectBelongsToThisPrefab(UObject* InObject);
		//Check object and it's up outer to tell if it is trash
		bool ObjectIsTrash(UObject* InObject);
		TMap<UObject*, FGuid> MapObjectToGuid;

		TArray<AActor*> CreatedActors;//collect for created actors
		TArray<FGuid> CreatedActorsGuid;//collect for created actor's guid

		void CollectActorRecursive(AActor* Actor);
		bool CollectObjectToSerailize(UObject* Object, FGuid& OutGuid);

		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors);
		void SerializeObjectArray(TArray<FLGUIObjectSaveData>& ObjectSaveDataArray, TArray<FLGUIComponentSaveData>& ComponentSaveDataArray);
		void SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		AActor* DeserializeActorRecursive(FLGUIActorSaveData& SavedActors);
		void PreGenerateActorRecursive(FLGUIActorSaveData& SavedActors, AActor* ParentActor);
		void PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);
		void DeserializeObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);

		//find id from list, if not will create
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

		const TSet<FName>& GetSceneComponentExcludeProperties();

		TFunction<void(AActor*)> CallbackBeforeAwake = nullptr;

		TFunction<void(AActor*, const TMap<FGuid, UObject*>&)> CallbackBeforeAwakeForSubPrefab = nullptr;

		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	bool	is SceneComponent
		 */
		TFunction<void(UObject*, TArray<uint8>&, bool)> WriterOrReaderFunction = nullptr;
		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	TSet<FName>&	Member properties to filter
		 */
		TFunction<void(UObject*, TArray<uint8>&, const TSet<FName>&)> WriterOrReaderFunctionForSubPrefab = nullptr;
		//duplicate actor
		AActor* SerializeActor_ForDuplicate(AActor* RootActor, USceneComponent* Parent);

		void SetupArchive(FArchive& InArchive);
	};
}