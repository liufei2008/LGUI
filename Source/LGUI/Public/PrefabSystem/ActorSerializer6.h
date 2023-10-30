// Copyright 2019-Present LexLiu. All Rights Reserved.

#if WITH_EDITOR
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

namespace LGUIPrefabSystem6
{
	struct FLGUICommonObjectSaveData
	{
	public:
		int32 ObjectClass = -1;
		uint32 ObjectFlags;
		TArray<uint8> PropertyData_Deprecated;//@todo: delete this

		/** The following two array stores default sub objects which belong to this object. Array must match index for specific component. When deserialize, use FName to find FGuid. */
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
			Ar << ObjectData.ObjectFlags;

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
			Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
			Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData_Deprecated);

			Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
			Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

			Record << SA_VALUE(TEXT("ObjectName"), Data.ObjectName);
			Record << SA_VALUE(TEXT("OuterObjectGuid"), Data.OuterObjectGuid);
		}
	};

	struct FLGUIPrefabOverrideParameterSaveData
	{
	public:
		TArray<uint8> OverrideParameterData_Deprecated;//@todo: delete this
		TArray<uint8> OverrideParameterData;
		TArray<FName> OverrideParameterNames;
		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabOverrideParameterSaveData& Data)
		{
			Ar << Data.OverrideParameterData;
			Ar << Data.OverrideParameterNames;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabOverrideParameterSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("OverrideParameterData"), Data.OverrideParameterData_Deprecated);
			Record << SA_VALUE(TEXT("OverrideObjectReferenceParameterData"), Data.OverrideParameterData);
			Record << SA_VALUE(TEXT("OverrideParameterNameSet"), Data.OverrideParameterNames);
		}
	};

	//Actor serialize and save data
	struct FLGUIActorSaveData : FLGUICommonObjectSaveData
	{
	public:
		bool bIsPrefab = false;
		int32 PrefabAssetIndex;
		TMap<FGuid, FLGUIPrefabOverrideParameterSaveData> MapObjectGuidToSubPrefabOverrideParameter;//override sub prefab's parameter
		TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;//sub prefab's object use a different guid in parent prefab. So multiple same sub prefab can exist in same parent prefab.

		FGuid ActorGuid;
		FGuid RootComponentGuid;

		TArray<FLGUIActorSaveData> ChildrenActorDataArray;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveData& ActorData)
		{
			Ar << ActorData.bIsPrefab;
			if (ActorData.bIsPrefab)
			{
				Ar << ActorData.PrefabAssetIndex;
				Ar << ActorData.ActorGuid;//sub prefab's root actor's guid
				Ar << ActorData.MapObjectGuidToSubPrefabOverrideParameter;
				Ar << ActorData.MapObjectGuidFromParentPrefabToSubPrefab;
			}
			else
			{
				Ar << ActorData.ActorGuid;
				Ar << ActorData.ObjectClass;
				Ar << ActorData.ObjectFlags;

				Ar << ActorData.RootComponentGuid;

				Ar << ActorData.DefaultSubObjectGuidArray;
				Ar << ActorData.DefaultSubObjectNameArray;

				Ar << ActorData.ChildrenActorDataArray;
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
				Record << SA_VALUE(TEXT("ActorGuid"), Data.ActorGuid);
				Record << SA_VALUE(TEXT("SubPrefabOverrideParameterArray"), Data.MapObjectGuidToSubPrefabOverrideParameter);
				Record << SA_VALUE(TEXT("MapObjectGuidFromParentPrefabToSubPrefab"), Data.MapObjectGuidFromParentPrefabToSubPrefab);
			}
			else
			{
				Record << SA_VALUE(TEXT("ActorGuid"), Data.ActorGuid);
				Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
				Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
				Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData_Deprecated);

				Record << SA_VALUE(TEXT("RootComponentGuid"), Data.RootComponentGuid);

				Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
				Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

				Record << SA_VALUE(TEXT("ChildrenActorDataArray"), Data.ChildrenActorDataArray);
			}
		}
	};

	struct FLGUIPrefabSaveData
	{
	public:
		FLGUIActorSaveData SavedActor;
		TMap<FGuid, FLGUIObjectSaveData> SavedObjects;
		/** Key as child, value as parent. */
		TMap<FGuid, FGuid> MapSceneComponentToParent;
		/** Map guid to parameter data */
		TMap<FGuid, TArray<uint8>> SavedObjectData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedActor;
			Ar << GameData.SavedObjects;
			Ar << GameData.MapSceneComponentToParent;
			Ar << GameData.SavedObjectData;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("SavedActor"), Data.SavedActor);
			Record << SA_VALUE(TEXT("SavedObjects"), Data.SavedObjects);
			Record << SA_VALUE(TEXT("MapSceneComponentToParent"), Data.MapSceneComponentToParent);
			Record << SA_VALUE(TEXT("SavedObjectReferences"), Data.SavedObjectData);
		}
	};

	struct FDuplicateActorDataContainer;

	/*
	 * serialize/deserialize actor with hierarchy.
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
		 * LoadPrefab and keep reference of objects.
		 */
		static AActor* LoadPrefabWithExistingObjects(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& OutSubPrefabMap
			, bool InSetHierarchyIndexForRootComponent = true
		);

		/** Save prefab data for editor use. */
		static void SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
			, TMap<UObject*, FGuid>& OutMapObjectToGuid, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
			, bool InForEditorOrRuntimeUse
		);
		
		/**
		 * Duplicate actor with hierarchy
		 */
		static AActor* DuplicateActor(AActor* OriginRootActor, USceneComponent* Parent);
		/** Prepare one data and duplicate multiple times */
		static bool PrepareDataForDuplicate(AActor* RootActor, FDuplicateActorDataContainer& OutData);
		static AActor* DuplicateActorWithPreparedData(FDuplicateActorDataContainer& InData, USceneComponent* InParent);
		/**
		 * Editor version, duplicate actor with hierarchy, will also concern sub prefab.
		 */
		static AActor* DuplicateActorForEditor(AActor* OriginRootActor, USceneComponent* Parent
			, const TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& InSubPrefabMap
			, const TMap<UObject*, FGuid>& InMapObjectToGuid
			, TMap<TObjectPtr<AActor>, FLGUISubPrefabData>& OutDuplicatedSubPrefabMap
			, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject
		);

		static AActor* LoadSubPrefab(
			UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, const FGuid& InParentDeserializationSessionId
			, int32& InOutActorIndex
			, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
			, const TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TArray<AActor*>&, const TArray<UActorComponent*>&)>& InOnSubPrefabFinishDeserializeFunction
		);

		static void PostSetPropertiesOnActor(UActorComponent* InComp);
	private:
		bool bSetHierarchyIndexForRootComponent = false;//need to set hierarchyindex to last for root component?

		struct FComponentDataStruct
		{
			UActorComponent* Component = nullptr;
			FGuid SceneComponentParentGuid;
		};
		TArray<FComponentDataStruct> ComponentsInThisPrefab;
		//include components in sub-prefab and sub-prefab's sub-prefab...
		TArray<UActorComponent*> AllComponents;
		//collection for all actors, include sub-prefab
		TArray<AActor*> AllActors;

		TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
		TArray<FComponentDataStruct> SubPrefabRootComponents;

		void CollectActorRecursive(AActor* Actor);

		struct FSubPrefabObjectOverrideParameterData
		{
			UObject* Object = nullptr;
			TArray<uint8> ParameterDatas;
			TArray<FName> ParameterNames;
		};
		TArray<FSubPrefabObjectOverrideParameterData> SubPrefabOverrideParameters;

		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors, TMap<FGuid, TArray<uint8>>& SavedObjectData);
		void SerializeObjectArray(TMap<FGuid, FLGUIObjectSaveData>& ObjectSaveDataArray, TMap<FGuid, TArray<uint8>>& SavedObjectData, TMap<FGuid, FGuid>& MapSceneComponentToParent);
		void SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		AActor* GenerateActorRecursive(FLGUIActorSaveData& SavedActors, TMap<FGuid, FLGUIObjectSaveData>& InSavedObjects, USceneComponent* Parent, FGuid ParentGuid);
		void GenerateObjectArray(TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapSceneComponentToParent);

		/** Mark of this deserialization session. If nested prefab, this is still the root prefab's value. */
		FGuid DeserializationSessionId = FGuid();
		int32 ActorIndexInPrefab = 0;
		bool bIsSubPrefab = false;
		/** A temperary string for log if is loading or saving prefab (not duplicate). */
		FString PrefabAssetPath;
		
		struct FSubPrefabObjectOverideData
		{
			UObject* Object;
			TArray<uint8> Data;
			TArray<FName> Names;
		};
		/** Store sub-prefab's override data(object reference), after all object is generated then restore it. */
		TArray<FSubPrefabObjectOverideData> SubPrefabObjectOverrideData;

		TFunction<void(AActor*)> CallbackBeforeAwake = nullptr;

		/**
		 * @param	AActor*		SubPrefab's root actor
		 * @param	const TMap<FGuid, UObject*>&	SubPrefab's map guid to all object
		 * @param	const TArray<AActor*>&		SubPrefab's all created actor
		 */
		TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TArray<AActor*>&, const TArray<UActorComponent*>&)> OnSubPrefabFinishDeserializeFunction = nullptr;

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
		 * @param	TArray<FName>&	Member properties to filter
		 */
		TFunction<void(UObject*, TArray<uint8>&, const TArray<FName>&)> WriterOrReaderFunctionForSubPrefabOverride = nullptr;
	};

	struct FDuplicateActorDataContainer
	{
		FLGUIPrefabSaveData ActorData;
		ActorSerializer Serializer;
	};
}
#endif