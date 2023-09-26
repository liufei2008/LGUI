// Copyright 2019-Present LexLiu. All Rights Reserved.

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
		TArray<uint8> PropertyData;

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
			Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);
			Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData);

			Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
			Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

			Record << SA_VALUE(TEXT("ObjectName"), Data.ObjectName);
			Record << SA_VALUE(TEXT("OuterObjectGuid"), Data.OuterObjectGuid);
		}
	};

	struct FLGUIPrefabOverrideParameterSaveData
	{
	public:
		TArray<uint8> OverrideParameterData;
		TArray<uint8> OverrideObjectReferenceParameterData;
		TArray<FName> OverrideParameterNames;
		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabOverrideParameterSaveData& Data)
		{
			Ar << Data.OverrideParameterData;
			Ar << Data.OverrideObjectReferenceParameterData;
			Ar << Data.OverrideParameterNames;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabOverrideParameterSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("OverrideParameterData"), Data.OverrideParameterData);
			Record << SA_VALUE(TEXT("OverrideObjectReferenceParameterData"), Data.OverrideObjectReferenceParameterData);
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
				Ar << ActorData.PropertyData;

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
				Record << SA_VALUE(TEXT("PropertyData"), Data.PropertyData);

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
		/**
		 * For object reference property, include Actor or Component or other dynamic created object reference.
		 *		It should only save the "reference property", but finally I just save all property data in it. Because I can't filter out "reference property".
		 *		I tryed do this in "ShouldSkipProperty" in LGUIObjectWriter/Reader, but if "reference property" is inside struct and the struct inside TArray, then when LGUIObjectReader go into TArray, the struct is reset, so other "value property" in struct all gone.
		 */
		TMap<FGuid, TArray<uint8>> SavedObjectReferences;

		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedActor;
			Ar << GameData.SavedObjects;
			Ar << GameData.MapSceneComponentToParent;
			Ar << GameData.SavedObjectReferences;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLGUIPrefabSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("SavedActor"), Data.SavedActor);
			Record << SA_VALUE(TEXT("SavedObjects"), Data.SavedObjects);
			Record << SA_VALUE(TEXT("MapSceneComponentToParent"), Data.MapSceneComponentToParent);
			Record << SA_VALUE(TEXT("SavedObjectReferences"), Data.SavedObjectReferences);
		}
	};

	struct FDuplicateActorDataContainer;

	/*
	 * serialize/deserialize actor with hierarchy.
	 * Version 6 note:
	 *		The key is UE add a function: CustomPreSpawnInitalization for Actor and PropertyInitCallback for Object, which gives us opportunity to set property values before finish construction.
	 *		So what I planed to achieve is two steps deserialization:
	 *			1. first step to set all value properties right inside the callback function.
	 *			2. second step is after all objects are generated (so we get all UObject's pointers), do a deserialization process to set all UObject properties.
	 *		In order to do these, I make some change in LGUIObjectWriter/Reader to filter value-property and object-reference-property, not precisely correct but ok to use.
	 *		BUT!!! Here is a condition that make it fail:
	 *			If a Actor is inside a Struct and Struct inside a TArray, things run good at first step (just values, no object-reference(no actor)), and comes second step, when ObjectReader reach TArray, the array items value all reset, after the TArray process only the Actor property get correct value, other property in Struct are reset.
	 *
	 *		Finally, I remove these changes in LGUIObjectWriter/Reader, so the two steps not filter properties as I want. This could double the time of set-properties.
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
			, AActor* InParentRootActor
			, int32& InOutActorIndex
			, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
			, const TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TArray<AActor*>&)>& InOnSubPrefabFinishDeserializeFunction
			, const TFunction<void(FGuid, UObject*)>& InOnSubPrefabDeserializeObjectFunction
		);

	private:
		bool bSetHierarchyIndexForRootComponent = false;//need to set hierarchyindex to last for root component?
		//bool bUseDeltaSerialization = false;//true means only serialize property that not default value. Why comment this?: If parent-prefab override sub-prefab's value to default then DeltaSerialization will not store override parameter

		struct FComponentDataStruct
		{
			UActorComponent* Component = nullptr;
			FGuid SceneComponentParentGuid;
		};
		TArray<FComponentDataStruct> CreatedComponents;

		TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubPrefabMap;
		TArray<FComponentDataStruct> SubPrefabRootComponents;
		//when restore actor data, some object could be created by default (default component or object), then we collect it
		TSet<UObject*> AlreadyRestoredObject;
		TArray<AActor*> CreatedActors;//collection for all actors, include sub-prefab
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
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors, TMap<FGuid, TArray<uint8>>& SavedObjectReferences);
		void SerializeObjectArray(TMap<FGuid, FLGUIObjectSaveData>& ObjectSaveDataArray, TMap<FGuid, TArray<uint8>>& SavedObjectReferences, TMap<FGuid, FGuid>& MapSceneComponentToParent);
		void SerializeActorToData(AActor* RootActor, FLGUIPrefabSaveData& OutData);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorFromData(FLGUIPrefabSaveData& SaveData, USceneComponent* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		AActor* GenerateActorRecursive(FLGUIActorSaveData& SavedActors, TMap<FGuid, FLGUIObjectSaveData>& InSavedObjects, USceneComponent* Parent, FGuid ParentGuid);
		void GenerateObjectArray(TMap<FGuid, FLGUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapSceneComponentToParent);

		/** Loaded root actor when deserialize. If nested prefab, this is still the parent prefab's root actor */
		AActor* LoadedRootActor = nullptr;
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
		TFunction<void(AActor*, const TMap<FGuid, TObjectPtr<UObject>>&, const TArray<AActor*>&)> OnSubPrefabFinishDeserializeFunction = nullptr;
		/**
		 * @param	FGuid	object's guid in SubPrefab.
		 * @param	UObject*	object in SubPrefab.
		 */
		TFunction<void(FGuid, UObject*)> OnSubPrefabDeserializeObjectFunction = nullptr;

		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	bool	is SceneComponent
		 */
		TFunction<void(UObject*, TArray<uint8>&, bool)> WriterOrReaderFunction = nullptr;
		/** For dynamic created object reference. */
		TFunction<void(UObject*, TArray<uint8>&, bool)> WriterOrReaderFunctionForObjectReference = nullptr;
		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	TArray<FName>&	Member properties to filter
		 */
		TFunction<void(UObject*, TArray<uint8>&, const TArray<FName>&)> WriterOrReaderFunctionForSubPrefabOverride = nullptr;
		/** For dynamic created object reference. */
		TFunction<void(UObject*, TArray<uint8>&, const TArray<FName>&)> WriterOrReaderFunctionForSubPrefabOverrideForObjectReference = nullptr;
	};

	struct FDuplicateActorDataContainer
	{
		FLGUIPrefabSaveData ActorData;
		ActorSerializer Serializer;
	};
}