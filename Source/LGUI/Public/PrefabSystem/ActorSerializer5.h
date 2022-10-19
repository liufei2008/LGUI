// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PrefabSystem/ActorSerializerBase.h"
#include "LGUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

class ULGUIPrefabOverrideParameterObject;
class UUIItem;

namespace LGUIPrefabSystem5
{
	struct FLGUICommonObjectSaveData
	{
	public:
		int32 ObjectClass = -1;
		FGuid ObjectGuid;//use id to find object.
		uint32 ObjectFlags;
		TArray<uint8> PropertyData;

		/** The following two array stores default sub objects which belong to this actor. Array must match index for specific component. When deserialize, use FName to find FGuid. */
		TArray<FGuid> DefaultSubObjectGuidArray;
		TArray<FName> DefaultSubObjectNameArray;
	};

	struct FLGUIObjectSaveData : FLGUICommonObjectSaveData
	{
	public:
		FName ObjectName;
		FGuid OuterObjectGuid;//outer object

		friend FArchive& operator<<(FArchive& Ar, FLGUIObjectSaveData& ObjectData)
		{
			Ar << ObjectData.ObjectClass;
			Ar << ObjectData.ObjectGuid;
			Ar << ObjectData.ObjectFlags;
			Ar << ObjectData.PropertyData;

			Ar << ObjectData.DefaultSubObjectGuidArray;
			Ar << ObjectData.DefaultSubObjectNameArray;

			Ar << ObjectData.ObjectName;
			Ar << ObjectData.OuterObjectGuid;
			return Ar;
		}

		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIObjectSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
			Record << SA_VALUE(TEXT("ObjectGuid"), Data.ObjectGuid);
			Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
			Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData);

			Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
			Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

			Record << SA_VALUE(TEXT("ObjectName"), Data.ObjectName);
			Record << SA_VALUE(TEXT("OuterObjectGuid"), Data.OuterObjectGuid);
		}
	};

	//ActorComponent serialize and save data
	struct FLGUIComponentSaveData : FLGUICommonObjectSaveData
	{
	public:
		FName ComponentName;
		FGuid SceneComponentParentGuid = FGuid();//invalid guid means the the SceneComponent dont have parent. @todo: For Blueprint-Actor's BlueprintCreatedComponent, leave this to invalid, because that component's parent is managed by blueprint
		FGuid OuterObjectGuid;//outer object

		friend FArchive& operator<<(FArchive& Ar, FLGUIComponentSaveData& ComponentData)
		{
			Ar << ComponentData.ObjectClass;
			Ar << ComponentData.ObjectGuid;
			Ar << ComponentData.ObjectFlags;
			Ar << ComponentData.PropertyData;

			Ar << ComponentData.ComponentName;
			Ar << ComponentData.SceneComponentParentGuid;
			Ar << ComponentData.OuterObjectGuid;

			Ar << ComponentData.DefaultSubObjectGuidArray;
			Ar << ComponentData.DefaultSubObjectNameArray;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIComponentSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
			Record << SA_VALUE(TEXT("ObjectGuid"), Data.ObjectGuid);
			Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
			Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData);

			Record << SA_VALUE(TEXT("ComponentName"), Data.ComponentName);
			Record << SA_VALUE(TEXT("SceneComponentParentGuid"), Data.SceneComponentParentGuid);
			Record << SA_VALUE(TEXT("OuterObjectGuid"), Data.OuterObjectGuid);

			Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
			Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);
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
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabOverrideParameterRecordData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("ObjectGuid"), Data.ObjectGuid);
			Record << SA_VALUE(TEXT("OverrideParameterData"), Data.OverrideParameterData);
			Record << SA_VALUE(TEXT("OverrideParameterNameSet"), Data.OverrideParameterNameSet);
		}
	};

	//Actor serialize and save data
	struct FLGUIActorSaveData : FLGUICommonObjectSaveData
	{
	public:
		bool bIsPrefab = false;
		int32 PrefabAssetIndex;
		TArray<FLGUIPrefabOverrideParameterRecordData> ObjectOverrideParameterArray;//override sub prefab's parameter
		TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;//sub prefab's object use a different guid in parent prefab. So multiple same sub prefab can exist in same parent prefab.

		FGuid RootComponentGuid;

		TArray<FLGUIActorSaveData> ChildActorData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveData& ActorData)
		{
			Ar << ActorData.bIsPrefab;
			if (ActorData.bIsPrefab)
			{
				Ar << ActorData.PrefabAssetIndex;
				Ar << ActorData.ObjectGuid;//sub prefab's root actor's guid
				Ar << ActorData.ObjectOverrideParameterArray;
				Ar << ActorData.MapObjectGuidFromParentPrefabToSubPrefab;
			}
			else
			{
				Ar << ActorData.ObjectGuid;
				Ar << ActorData.ObjectClass;
				Ar << ActorData.ObjectFlags;
				Ar << ActorData.PropertyData;

				Ar << ActorData.RootComponentGuid;

				Ar << ActorData.DefaultSubObjectGuidArray;
				Ar << ActorData.DefaultSubObjectNameArray;

				Ar << ActorData.ChildActorData;
			}
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIActorSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("bIsPrefab"), Data.bIsPrefab);
			if (Data.bIsPrefab)
			{
				Record << SA_VALUE(TEXT("PrefabAssetIndex"), Data.PrefabAssetIndex);
				Record << SA_VALUE(TEXT("ObjectGuid"), Data.ObjectGuid);
				Record << SA_VALUE(TEXT("ObjectOverrideParameterArray"), Data.ObjectOverrideParameterArray);
				Record << SA_VALUE(TEXT("MapObjectGuidFromParentPrefabToSubPrefab"), Data.MapObjectGuidFromParentPrefabToSubPrefab);
			}
			else
			{
				Record << SA_VALUE(TEXT("ObjectGuid"), Data.ObjectGuid);
				Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
				Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
				Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData);

				Record << SA_VALUE(TEXT("RootComponentGuid"), Data.RootComponentGuid);

				Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
				Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

				Record << SA_VALUE(TEXT("ChildActorData"), Data.ChildActorData);
			}
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
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("SavedActor"), Data.SavedActor);
			Record << SA_VALUE(TEXT("SavedObjects"), Data.SavedObjects);
			Record << SA_VALUE(TEXT("SavedComponents"), Data.SavedComponents);
		}
	};


	/*
	 * serialize/deserialize actor with hierarchy
	 */
	class LGUI_API ActorSerializer : public LGUIPrefabSystem::ActorSerializerBase
	{
	public:
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * Replace asset and class then load prefab.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, const TMap<UObject*, UObject*>& InReplaceAssetMap, const TMap<UClass*, UClass*>& InReplaceClassMap, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * LoadPrefab and keep reference of objects.
		 */
		static AActor* LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
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
		static AActor* DuplicateActor(AActor* OriginRootActor, USceneComponent* Parent);
		/** Prepare one data and duplicate multiple times */
		static bool PrepareDataForDuplicate(AActor* RootActor, FLGUIPrefabSaveData& OutData, ActorSerializer& OutSerializer);
		static AActor* DuplicateActorWithPreparedData(FLGUIPrefabSaveData& InData, ActorSerializer& OutSerializer, USceneComponent* InParent);
		/**
		 * Editor version, duplicate actor with hierarchy, will also concern sub prefab.
		 */
		static AActor* DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
			, const TMap<AActor*, FLGUISubPrefabData>& InSubPrefabMap
			, const TMap<UObject*, FGuid>& InMapObjectToGuid
			, TMap<AActor*, FLGUISubPrefabData>& OutDuplicatedSubPrefabMap
			, TMap<FGuid, UObject*>& OutMapGuidToObject
		);

		static AActor* LoadSubPrefab(
			UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, AActor* InParentRootActor
			, int32& InOutActorIndex
			, TMap<FGuid, UObject*>& InMapGuidToObject
			, TFunction<void(AActor*, const TMap<FGuid, UObject*>&, const TArray<AActor*>&)> InOnSubPrefabFinishDeserializeFunction
		);

	private:
		bool bSetHierarchyIndexForRootComponent = false;//need to set hierarchyindex to last for root component?
		//bool bUseDeltaSerialization = false;//true means only serialize property that not default value. Why comment this?: If parent-prefab override sub-prefab's value to default then DeltaSerialization will not store override parameter

		struct ComponentDataStruct
		{
			UActorComponent* Component = nullptr;
			FGuid SceneComponentParentGuid;
		};
		TArray<ComponentDataStruct> CreatedComponents;

		TMap<AActor*, FLGUISubPrefabData> SubPrefabMap;

		TArray<AActor*> CreatedActors;//collect for created actors

		void CollectActorRecursive(AActor* Actor);

		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors);
		void SerializeObjectArray(TArray<FLGUIObjectSaveData>& ObjectSaveDataArray, TArray<FLGUIComponentSaveData>& ComponentSaveDataArray);
		void SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		AActor* DeserializeActorRecursive(FLGUIActorSaveData& SavedActors);
		void PreGenerateActorRecursive(FLGUIActorSaveData& SavedActors, USceneComponent* Parent);
		void PreGenerateObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);
		void DeserializeObjectArray(const TArray<FLGUIObjectSaveData>& SavedObjects, const TArray<FLGUIComponentSaveData>& SavedComponents);

		/** Loaded root actor when deserialize. If nested prefab, this is still the parent prefab's root actor */
		AActor* LoadedRootActor = nullptr;
		int32 ActorIndexInPrefab = 0;
		bool bIsSubPrefab = false;
		/** A temperary string for log if is loading or saving prefab (not duplicate). */
		FString PrefabAssetPath;

		TFunction<void()> CallbackBeforeDeserialize = nullptr;
		TFunction<void(AActor*)> CallbackBeforeAwake = nullptr;

		/**
		 * @param	AActor*		SubPrefab's root actor
		 * @param	const TMap<FGuid, UObject*>&	SubPrefab's map guid to all object
		 * @param	const TArray<AActor*>&		SubPrefab's all created actor
		 */
		TFunction<void(AActor*, const TMap<FGuid, UObject*>&, const TArray<AActor*>&)> OnSubPrefabFinishDeserializeFunction = nullptr;

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
	};
}