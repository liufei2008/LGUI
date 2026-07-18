// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PrefabSystem/WidgetSerializerBase.h"
#include "LexUIPrefab.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectWriter.h"
#include "Serialization/ObjectReader.h"

namespace LexUIPrefabSystem
{
	struct FLexUICommonObjectSaveData
	{
	public:
		int32 ObjectClass = -1;
		uint32 ObjectFlags;

		/** The following two array stores default sub objects which belong to this object. Array must match index for specific component. When deserialize, use FName to find FGuid. */
		TArray<FGuid> DefaultSubObjectGuidArray;
		TArray<FName> DefaultSubObjectNameArray;
	};

	struct FLexUIObjectSaveData : FLexUICommonObjectSaveData
	{
	public:
		FName ObjectName;
		FGuid OuterObjectGuid;//outer object

		friend FArchive& operator<<(FArchive& Ar, FLexUIObjectSaveData& ObjectData)
		{
			Ar << ObjectData.ObjectClass;
			Ar << ObjectData.ObjectFlags;

			Ar << ObjectData.DefaultSubObjectGuidArray;
			Ar << ObjectData.DefaultSubObjectNameArray;

			Ar << ObjectData.ObjectName;
			Ar << ObjectData.OuterObjectGuid;
			return Ar;
		}

		friend void operator<<(FStructuredArchive::FSlot Slot, FLexUIObjectSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
			Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);

			Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
			Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);

			Record << SA_VALUE(TEXT("ObjectName"), Data.ObjectName);
			Record << SA_VALUE(TEXT("OuterObjectGuid"), Data.OuterObjectGuid);
		}
	};

	struct FLexUIPrefabOverrideParameterSaveData
	{
	public:
		TArray<uint8> OverrideParameterData;
		TArray<FName> OverrideParameterNames;
		friend FArchive& operator<<(FArchive& Ar, FLexUIPrefabOverrideParameterSaveData& Data)
		{
			Ar << Data.OverrideParameterData;
			Ar << Data.OverrideParameterNames;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLexUIPrefabOverrideParameterSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("OverrideObjectReferenceParameterData"), Data.OverrideParameterData);
			Record << SA_VALUE(TEXT("OverrideParameterNameSet"), Data.OverrideParameterNames);
		}
	};

	struct FLexUISubPrefabObjectUniqueIdSaveData
	{
	public:
		FGuid RootWidgetGuidInParentPrefab;
		FGuid ObjectGuidInOriginPrefab;

		bool operator==(const FLexUISubPrefabObjectUniqueIdSaveData& other)const
		{
			return this->RootWidgetGuidInParentPrefab == other.RootWidgetGuidInParentPrefab && this->ObjectGuidInOriginPrefab == other.ObjectGuidInOriginPrefab;
		}
		friend FORCEINLINE uint32 GetTypeHash(const FLexUISubPrefabObjectUniqueIdSaveData& other)
		{
			return HashCombine(GetTypeHash(other.RootWidgetGuidInParentPrefab), GetTypeHash(other.ObjectGuidInOriginPrefab));
		}

		friend FArchive& operator<<(FArchive& Ar, FLexUISubPrefabObjectUniqueIdSaveData& Data)
		{
			Ar << Data.RootWidgetGuidInParentPrefab;
			Ar << Data.ObjectGuidInOriginPrefab;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLexUISubPrefabObjectUniqueIdSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("RootWidgetGuidInParentPrefab"), Data.RootWidgetGuidInParentPrefab);
			Record << SA_VALUE(TEXT("ObjectGuidInOriginPrefab"), Data.ObjectGuidInOriginPrefab);
		}
	};

	//Widget serialize and save data
	struct FLexUIWidgetSaveData : FLexUICommonObjectSaveData
	{
	public:
		bool bIsPrefab = false;
		int32 PrefabAssetIndex;
		TMap<FGuid, FLexUIPrefabOverrideParameterSaveData> MapObjectGuidToSubPrefabOverrideParameter;//override sub prefab's parameter
		TMap<FLexUISubPrefabObjectUniqueIdSaveData, FGuid> MapObjectIdToNewlyCreatedId;
		TMap<FGuid, FGuid> MapObjectGuidFromParentPrefabToSubPrefab;//sub prefab's object use a different guid in parent prefab. So multiple same sub prefab can exist in same parent prefab.

		FGuid WidgetGuid;

		friend FArchive& operator<<(FArchive& Ar, FLexUIWidgetSaveData& WidgetData)
		{
			Ar << WidgetData.bIsPrefab;
			if (WidgetData.bIsPrefab)
			{
				Ar << WidgetData.PrefabAssetIndex;
				Ar << WidgetData.WidgetGuid;//sub prefab's root actor's guid
				Ar << WidgetData.MapObjectGuidToSubPrefabOverrideParameter;
				Ar << WidgetData.MapObjectIdToNewlyCreatedId;
				Ar << WidgetData.MapObjectGuidFromParentPrefabToSubPrefab;
			}
			else
			{
				Ar << WidgetData.WidgetGuid;
				Ar << WidgetData.ObjectClass;
				Ar << WidgetData.ObjectFlags;

				Ar << WidgetData.DefaultSubObjectGuidArray;
				Ar << WidgetData.DefaultSubObjectNameArray;
			}
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLexUIWidgetSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("bIsPrefab"), Data.bIsPrefab);
			if (Data.bIsPrefab)
			{
				Record << SA_VALUE(TEXT("PrefabAssetIndex"), Data.PrefabAssetIndex);
				Record << SA_VALUE(TEXT("WidgetGuid"), Data.WidgetGuid);
				Record << SA_VALUE(TEXT("SubPrefabOverrideParameterArray"), Data.MapObjectGuidToSubPrefabOverrideParameter);
				Record << SA_VALUE(TEXT("MapObjectIdToNewlyCreatedId"), Data.MapObjectIdToNewlyCreatedId);
				Record << SA_VALUE(TEXT("MapObjectGuidFromParentPrefabToSubPrefab"), Data.MapObjectGuidFromParentPrefabToSubPrefab);
			}
			else
			{
				Record << SA_VALUE(TEXT("WidgetGuid"), Data.WidgetGuid);
				Record << SA_VALUE(TEXT("ObjectClass"), Data.ObjectClass);
				Record << SA_VALUE(TEXT("ObjectFlags"), Data.ObjectFlags);

				Record << SA_VALUE(TEXT("DefaultSubObjectGuidArray"), Data.DefaultSubObjectGuidArray);
				Record << SA_VALUE(TEXT("DefaultSubObjectNameArray"), Data.DefaultSubObjectNameArray);
			}
		}
	};

	struct FLexUIPrefabSaveData
	{
	public:
		TArray<FLexUIWidgetSaveData> SavedWidgets;
		TMap<FGuid, FLexUIObjectSaveData> SavedObjects;
		/** Key as child, value as parent. */
		TMap<FGuid, FGuid> MapWidgetToParent;
		/** Map guid to parameter data */
		TMap<FGuid, TArray<uint8>> SavedObjectData;

		friend FArchive& operator<<(FArchive& Ar, FLexUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedWidgets;
			Ar << GameData.SavedObjects;
			Ar << GameData.MapWidgetToParent;
			Ar << GameData.SavedObjectData;
			return Ar;
		}
		friend void operator<<(FStructuredArchive::FSlot Slot, FLexUIPrefabSaveData& Data)
		{
			FStructuredArchive::FRecord Record = Slot.EnterRecord();
			Record << SA_VALUE(TEXT("SavedWidgets"), Data.SavedWidgets);
			Record << SA_VALUE(TEXT("SavedObjects"), Data.SavedObjects);
			Record << SA_VALUE(TEXT("MapSceneComponentToParent"), Data.MapWidgetToParent);
			Record << SA_VALUE(TEXT("SavedObjectReferences"), Data.SavedObjectData);
		}
	};

	struct FDuplicateWidgetDataContainer;

	/*
	 * serialize/deserialize actor with hierarchy.
	 */
	class LGUI_API WidgetSerializer : public LexUIPrefabSystem::WidgetSerializerBase
	{
	public:
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "LexWidget" is the loaded root widget.
		 */
		static ULexWidget* LoadPrefab(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent, bool SetRelativeTransformToIdentity = true, TFunction<void(ULexWidget*)> CallbackBeforeAwake = nullptr);
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "LexWidget" is the loaded root widget.
		 */
		static ULexWidget* LoadPrefab(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(ULexWidget*)> CallbackBeforeAwake = nullptr);
		/**
		 * LoadPrefab and keep reference of objects.
		 */
		static ULexWidget* LoadPrefabWithExistingObjects(UWorld* InWorld, UObject* InOuter, ULexUIPrefab* InPrefab, ULexWidget* Parent
			, TMap<FGuid, TObjectPtr<UObject>>& InOutMapGuidToObjects, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutSubPrefabMap
		);

		/** Save prefab data for editor use.
		 * @return If save prefab success
		 */
		static bool SavePrefab(ULexWidget* RootWidget, ULexUIPrefab* InPrefab
			, TMap<UObject*, FGuid>& OutMapObjectToGuid, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& InSubPrefabMap
			, bool InForEditorOrRuntimeUse
		);
		
		/**
		 * Duplicate widget with hierarchy
		 */
		static ULexWidget* DuplicateWidget(UWorld* InWorld, UObject* InOwnerObject, ULexWidget* OriginRootWidget, ULexWidget* Parent);
		/** Prepare one data and duplicate multiple times */
		static bool PrepareDataForDuplicate(ULexWidget* RootWidget, FDuplicateWidgetDataContainer& OutData);
		static ULexWidget* DuplicateWidgetWithPreparedData(UWorld* InWorld, UObject* InOwnerObject, FDuplicateWidgetDataContainer& InData, ULexWidget* InParent);
		/**
		 * Editor version, duplicate widget with hierarchy, will also concern sub prefab.
		 */
		static ULexWidget* DuplicateWidgetForEditor(UWorld* InWorld, ULexWidget* OriginRootWidget, ULexWidget* Parent
			, const TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& InSubPrefabMap
			, const TMap<UObject*, FGuid>& InMapObjectToGuid
			, TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData>& OutDuplicatedSubPrefabMap
			, TMap<FGuid, TObjectPtr<UObject>>& OutMapGuidToObject
		);

		static ULexWidget* LoadSubPrefab(UWorld* InWorld,
			UObject* InOwnerObject, ULexUIPrefab* InPrefab, ULexWidget* Parent
			, TMap<FGuid, TObjectPtr<UObject>>& InMapGuidToObject
			, const TFunction<void(ULexWidget*, const TMap<FGuid, TObjectPtr<UObject>>&, const TMap<TObjectPtr<UObject>, FGuid>&, const TArray<ULexWidget*>&)>& InOnSubPrefabFinishDeserializeFunction
		);

	private:
		struct FWidgetAttachment
		{
			ULexWidget* Widget = nullptr;
			FGuid ParentGuid;
		};
		TArray<FWidgetAttachment> WidgetAttachmentArray;
		//collection for all widgets, include sub-prefab
		TArray<ULexWidget*> AllWidgetArray;

		TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData> SubPrefabMap;
		TArray<ULexWidget*> SubPrefabWidgetArray;
		//this collection will collect all widgets of this prefab, and root widget of sub prefab
		TArray<ULexWidget*> TrySerializeWidgetArray;
		//origin guid mean the object guid in it's origin prefab, not sub prefab
		TMap<TObjectPtr<UObject>, FGuid> MapObjectToOriginGuid;

		void CollectWidgetRecursive(ULexWidget* Widget);

		struct FSubPrefabObjectOverrideParameterData
		{
			UObject* Object = nullptr;
			TArray<uint8> ParameterDatas;
			TArray<FName> ParameterNames;
		};
		TArray<FSubPrefabObjectOverrideParameterData> SubPrefabOverrideParameters;

		//serialize widget
		bool SerializeWidget(ULexWidget* RootWidget, ULexUIPrefab* InPrefab);
		void SerializeWidgetArray(TMap<FGuid, FGuid>& MapWidgetToParent, TArray<FLexUIWidgetSaveData>& SavedWidgets, TMap<FGuid, TArray<uint8>>& SavedObjectData);
		void SerializeObjectArray(TMap<FGuid, FLexUIObjectSaveData>& ObjectSaveDataArray, TMap<FGuid, TArray<uint8>>& SavedObjectData);
		void SerializeWidgetToData(ULexWidget* RootWidget, FLexUIPrefabSaveData& OutData);
		//deserialize widget
		ULexWidget* DeserializeWidget(ULexWidget* Parent, ULexUIPrefab* InPrefab, const TFunction<void()>& InCallbackBeforeDeserialize, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		ULexWidget* DeserializeWidgetFromData(FLexUIPrefabSaveData& SaveData, ULexWidget* Parent, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale);
		ULexWidget* GenerateWidgetArray(TArray<FLexUIWidgetSaveData>& SavedWidgets, TMap<FGuid, FLexUIObjectSaveData>& InSavedObjects, TMap<FGuid, FGuid>& MapWidgetToParent, FGuid ParentGuid);
		void GenerateObjectArray(TMap<FGuid, FLexUIObjectSaveData>& SavedObjects, TMap<FGuid, FGuid>& MapWidgetToParent);

		bool bIsSubPrefab = false;
		/** A temporary string for log if loading or saving prefab (not duplicate). */
		FString PrefabAssetPath;
		
		struct FSubPrefabObjectOverideData
		{
			UObject* Object;
			TArray<uint8> Data;
			TArray<FName> Names;
		};
		/** Store sub-prefab's override data(object reference), after all object is generated then restore it. */
		TArray<FSubPrefabObjectOverideData> SubPrefabObjectOverrideData;

		TFunction<void(ULexWidget*)> CallbackBeforeAwake = nullptr;

		/**
		 * @param	ULexWidget*		SubPrefab's root widget
		 * @param	const TMap<FGuid, UObject*>&	SubPrefab's map guid to all object
		 * @param	const TArray<ULexWidget*>&		SubPrefab's all created widget
		 */
		TFunction<void(ULexWidget*, const TMap<FGuid, TObjectPtr<UObject>>&, const TMap<TObjectPtr<UObject>, FGuid>&, const TArray<ULexWidget*>&)> OnSubPrefabFinishDeserializeFunction = nullptr;

		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 */
		TFunction<void(UObject*, TArray<uint8>&)> WriterOrReaderFunction = nullptr;
		/**
		 * Writer and Reader for serialize or deserialize
		 * @param	UObject*	Object to serialize/deserialize
		 * @param	TArray<uint8>&	Data buffer
		 * @param	TArray<FName>&	Member properties to filter
		 */
		TFunction<void(UObject*, TArray<uint8>&, const TArray<FName>&)> WriterOrReaderFunctionForSubPrefabOverride = nullptr;
	};

	struct FDuplicateWidgetDataContainer
	{
		FLexUIPrefabSaveData WidgetData;
		WidgetSerializer Serializer;
	};
}