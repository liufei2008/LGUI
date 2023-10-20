// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PrefabSystem/LGUIPrefab.h"

#if WITH_EDITOR

class ALGUIManagerActor;
namespace LGUIPrefabSystem
{
	enum class ELGUIPropertyType : uint8
	{
		//common data, use common serialization method
		PT_Common,
		//array,map,set
		PT_Container,
		//struct container
		PT_StructContainer,
		//the property is an object type which reference a actor/ asset/ class/ 
		PT_Reference,

		PT_int8,
		PT_int16,
		PT_int32,
		PT_int64,
		PT_uint8,
		PT_uint16,
		PT_uint32,
		PT_uint64,

		PT_bool,
		PT_float,
		PT_double,

		PT_Vector2,
		PT_Vector3,
		PT_Vector4,
		PT_Quat,
		PT_Color,
		PT_LinearColor,
		PT_Rotator,

		//the property is UObject and maked as Instanced
		PT_InstancedObject,

		PT_Name,
		PT_String,
		PT_Text,

		//not valid
		PT_None,
	};

	struct LGUIPrefabSerializerHelper
	{
		static void SerializeToArchiveByPropertyType(FArchive& Ar, ELGUIPropertyType& PropertyType, TArray<uint8>& Data)
		{
			int count = 0;//if property is reference type, then count means index of the reference
			switch (PropertyType)
			{
			case ELGUIPropertyType::PT_int8:
			case ELGUIPropertyType::PT_uint8:
			case ELGUIPropertyType::PT_bool:
				count = 1; break;
			case ELGUIPropertyType::PT_int16:
			case ELGUIPropertyType::PT_uint16:
				count = 2; break;
			case ELGUIPropertyType::PT_int32:
			case ELGUIPropertyType::PT_uint32:
			case ELGUIPropertyType::PT_float:
			case ELGUIPropertyType::PT_Color:
				count = 4; break;
			case ELGUIPropertyType::PT_int64:
			case ELGUIPropertyType::PT_uint64:
			case ELGUIPropertyType::PT_double:
			case ELGUIPropertyType::PT_Vector2:
				count = 8; break;

			case ELGUIPropertyType::PT_Vector3:
			case ELGUIPropertyType::PT_Rotator:
				count = 12; break;
			case ELGUIPropertyType::PT_Vector4:
			case ELGUIPropertyType::PT_Quat:
			case ELGUIPropertyType::PT_LinearColor:
				count = 16; break;

			case ELGUIPropertyType::PT_Container:
				count = 4; break;
			case ELGUIPropertyType::PT_Common:
				Ar << Data; return;

			case ELGUIPropertyType::PT_Reference:
			case ELGUIPropertyType::PT_String:
			case ELGUIPropertyType::PT_Name:
			case ELGUIPropertyType::PT_Text:
				count = 4; break;
			case ELGUIPropertyType::PT_InstancedObject:
				count = 4; break;

			case ELGUIPropertyType::PT_StructContainer:
				return;
			}

			if (Ar.IsLoading())
			{
				Data.AddUninitialized(count);
				Ar.Serialize(Data.GetData(), count);
			}
			else
			{
				Ar.Serialize(Data.GetData(), count);
			}
		}
	};

	//store FProperty data
	struct FLGUIPropertyData
	{
	public:
		//property's name
		//TArray/TMap/TSet container's element dont need to store element's name
		FName Name;
		//property's data
		//if property is Array/Map/Set container, then the data is container's size
		TArray<uint8> Data;
		//container data. for Array/Map/Set/Struct... which have nested data
		//if is TMap's element, store data as key/value/key/value...
		TArray<FLGUIPropertyData> ContainerData;
		//is this Property marked editor only?
		bool IsEditorOnly = false;
		ELGUIPropertyType PropertyType = ELGUIPropertyType::PT_Common;
		friend FArchive& operator<<(FArchive& Ar, FLGUIPropertyData& PropertyData)
		{
			Ar << PropertyData.Name;
			Ar << PropertyData.IsEditorOnly;
			Ar << PropertyData.PropertyType;
			LGUIPrefabSerializerHelper::SerializeToArchiveByPropertyType(Ar, PropertyData.PropertyType, PropertyData.Data);
			if (PropertyData.PropertyType == ELGUIPropertyType::PT_Container
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_StructContainer
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_InstancedObject
				)
			{
				Ar << PropertyData.ContainerData;
			}
			return Ar;
		}
		static bool GetPropertyDataFromArray(FName InPropertyName, const TArray<FLGUIPropertyData>& InArray, FLGUIPropertyData& Result)
		{
			int count = InArray.Num();
			for (int i = 0; i < count; i++)
			{
				auto item = InArray[i];
				if (item.Name == InPropertyName)
				{
					Result = item;
					return true;
				}
			}
			return false;
		}
		static void SetPropertyDataToArray(FName InPropertyName, TArray<FLGUIPropertyData>& InArray, const FLGUIPropertyData& Value)
		{
			int count = InArray.Num();
			for (int i = 0; i < count; i++)
			{
				auto item = InArray[i];
				if (item.Name == InPropertyName)
				{
					InArray[i] = Value;
					return;
				}
			}
		}
	};
	//ActorComponent serialize and save data
	struct FLGUIComponentSaveData
	{
	public:
		int32 ComponentClass = -1;//-1 means not exist. if RootComponent is -1 means this actor dont have RootComponent
		FName ComponentName;
		int32 SceneComponentParentID = -1;//-1 means the the SceneComponent dont have parent
		TArray<FLGUIPropertyData> PropertyData;
		USceneComponent* SceneComponentParent = nullptr;//helper
		friend FArchive& operator<<(FArchive& Ar, FLGUIComponentSaveData& ComponentData)
		{
			Ar << ComponentData.ComponentClass;
			Ar << ComponentData.ComponentName;
			Ar << ComponentData.SceneComponentParentID;
			Ar << ComponentData.PropertyData;
			return Ar;
		}
	};
	//Actor serialize and save data
	struct FLGUIActorSaveData
	{
	public:
		int32 ActorClass;
		int32 ActorID;//Actor id. if some property reference actor, use id to find
		TArray<FLGUIPropertyData> ActorPropertyData;
		TArray<FLGUIComponentSaveData> ComponentPropertyData;
		TArray<FLGUIActorSaveData> ChildActorData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveData& ActorData)
		{
			Ar << ActorData.ActorID;
			Ar << ActorData.ActorClass;
			Ar << ActorData.ActorPropertyData;
			Ar << ActorData.ComponentPropertyData;
			Ar << ActorData.ChildActorData;
			return Ar;
		}

#if WITH_EDITOR
		FGuid GetActorGuid(FGuid fallbackGuid) const;
		void SetActorGuid(FGuid guid);
#endif
	};
	struct FLGUIPrefabSaveData
	{
	public:
		FLGUIActorSaveData SavedActor;

		friend FArchive& operator<<(FArchive& Ar, FLGUIPrefabSaveData& GameData)
		{
			Ar << GameData.SavedActor;
			return Ar;
		}
	};

	//same as FLGUIPropertyData, except not store property name
	struct FLGUIPropertyDataForBuild
	{
	public:
		TArray<uint8> Data;
		TArray<FLGUIPropertyDataForBuild> ContainerData;
		ELGUIPropertyType PropertyType = ELGUIPropertyType::PT_Common;
		friend FArchive& operator<<(FArchive& Ar, FLGUIPropertyDataForBuild& PropertyData)
		{
			Ar << PropertyData.PropertyType;
			LGUIPrefabSerializerHelper::SerializeToArchiveByPropertyType(Ar, PropertyData.PropertyType, PropertyData.Data);
			if (PropertyData.PropertyType == ELGUIPropertyType::PT_Container
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_StructContainer
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_InstancedObject
				)
			{
				Ar << PropertyData.ContainerData;
			}
			return Ar;
		}
		void InitFromPropertySaveData(const FLGUIPropertyData& InData)
		{
			Data = InData.Data;
			PropertyType = InData.PropertyType;
		}
	};
	struct FLGUIComponentSaveDataForBuild
	{
	public:
		int32 ComponentClass;
		FName ComponentName;
		int32 SceneComponentParentID = -1;
		TArray<FLGUIPropertyDataForBuild> PropertyData;
		friend FArchive& operator<<(FArchive& Ar, FLGUIComponentSaveDataForBuild& ComponentData)
		{
			Ar << ComponentData.ComponentClass;
			Ar << ComponentData.ComponentName;
			Ar << ComponentData.SceneComponentParentID;
			Ar << ComponentData.PropertyData;
			return Ar;
		}
		void InitFromComponentSaveData(const FLGUIComponentSaveData& InData)
		{
			ComponentClass = InData.ComponentClass;
			ComponentName = InData.ComponentName;
			SceneComponentParentID = InData.SceneComponentParentID;
		}
	};
	struct FLGUIActorSaveDataForBuild
	{
	public:

		int32 ActorClass;
		int32 ActorID;//Actor id. if some property reference actor, use id to find
		TArray<FLGUIPropertyDataForBuild> ActorPropertyData;
		TArray<FLGUIComponentSaveDataForBuild> ComponentPropertyData;
		TArray<FLGUIActorSaveDataForBuild> ChildActorData;

		friend FArchive& operator<<(FArchive& Ar, FLGUIActorSaveDataForBuild& ActorData)
		{
			Ar << ActorData.ActorID;
			Ar << ActorData.ActorClass;
			Ar << ActorData.ActorPropertyData;
			Ar << ActorData.ComponentPropertyData;
			Ar << ActorData.ChildActorData;
			return Ar;
		}
		void InitFromActorSaveData(const FLGUIActorSaveData& InData)
		{
			ActorClass = InData.ActorClass;
			ActorID = InData.ActorID;
		}
	};


	/** This is just a record of old prefab system, will be removed in future release. Use LGUIPrefabSystem3::ActorSerializer3 instead. */
	/*
	serialize/deserialize actor, include hierarchy, property, reference
	not yet supported property type:
	UDelegateProperty(DynamicDelegate)
	FMulticastDelegateProperty(DynamicMulticastDelegate)
	FInterfaceProperty(TScriptInterface<T>)
	ActorComponent as reference property

	for Blueprint Actor: after finish spawn, all properties will be override by Blueprint, so try not serialize blueprint actor
	*/
	class LGUI_API ActorSerializer
	{
	private:
		ActorSerializer(ULGUIPrefab* InPrefab);
		ActorSerializer(UWorld* InTargetWorld);
	public:
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
		/**
		 * @param CallbackBeforeAwake	This callback function will execute before Awake event, parameter "Actor" is the loaded root actor.
		 */
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale, TFunction<void(AActor*)> CallbackBeforeAwake = nullptr);
#if WITH_EDITOR
		enum class EPrefabEditMode :uint8
		{
			NotEditable,
			EditInLevel,
		};
		static void SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
			, const TArray<AActor*>& InExistingActorArray, const TArray<FGuid>& InExistingActorGuidInPrefab
			, TArray<AActor*>& OutSerializedActors, TArray<FGuid>& OutSerializedActorsGuid);
		/**
		 * LoadPrefab for edit/modify, will keep reference of source prefab.
		 */
		static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent
			, TFunction<AActor* (FGuid)> InGetExistingActorFunction
			, TFunction<void (AActor*, FGuid)> InCreateNewActorFunction
			, TArray<AActor*>& OutCreatedActors, TArray<FGuid>& OutActorsGuid);
		/**
		 * LoadPrefab in editor, will not keep reference of source prefab, So we can't apply changes after modify it.
		 */
		static AActor* LoadPrefabInEditor(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
		static void RenewActorGuidForDuplicate(ULGUIPrefab* InPrefab);

		static FLGUIActorSaveData CreateActorSaveData(ULGUIPrefab* InPrefab);
		static void ConvertForBuildData(ULGUIPrefab* InPrefab);//convert saved data to build data

		static void SetActorGUIDNew(AActor* Actor);
		static void SetActorGUID(AActor* Actor, FGuid Guid);
#endif
		static void RegisterComponent(AActor* Actor, UActorComponent* Comp);

	private:
		TWeakObjectPtr<UWorld> TargetWorld = nullptr;//world that need to spawn actor

		TWeakObjectPtr<ULGUIPrefab> Prefab = nullptr;

		TMap<int32, AActor*> MapIDToActor;
#if WITH_EDITORONLY_DATA
		EPrefabEditMode editMode = EPrefabEditMode::NotEditable;

		TArray<AActor*> SkippingActors;

		TMap<FGuid, AActor*> MapGuidToActor;
		TMap<AActor*, FGuid> MapActorToGuid;

		TArray<AActor*> CreatedActorsNeedToFinishSpawn;
		TArray<FGuid> CreatedActorsGuid;//collect for created actors
		TArray<AActor*> ExistingActors;//prefab instance actors in level
		TArray<FGuid> ExistingActorsGuid;//actor's guid 
		TFunction<AActor* (FGuid)> GetExistingActorFunction = nullptr;
		TFunction<void(AActor*, FGuid)> CreateNewActorFunction = nullptr;
		int32 SubPrefabGuidSearchStartIndex = 0;

		TArray<AActor*> SerializedActors;//collect for created actors
		TArray<FGuid> SerializedActorsGuid;//collect for created actors

		TMap<FGuid, FGuid> DuplicateGuidFromOriginToNew;

		TMap<FGuid, int32> ActorGuidToIDForConvert;
#endif

#if WITH_EDITOR
		void GenerateActorIDRecursive(AActor* Actor);
		void CollectSkippingActorsRecursive(AActor* Actor);
#endif
		struct UPropertyMapStruct
		{
			FProperty* ObjProperty;//FObjectProperty
			int32 id;//property's UObject's id
			FGuid guid;//actor's guid
			uint8* Dest;//FObjectProperty's container address
			int32 cppArrayIndex = 0;//if is c++ array's element
		};
		TArray<UPropertyMapStruct> ObjectMapStructList;
		TArray<AActor*> CreatedActors;//collect for created actors

		//SceneComponent that belong to same actor, use this struct to store parent's name and then reparent it
		struct SceneComponentToParentIDStruct
		{
			USceneComponent* Comp;
			int32 ParentCompID;
		};

		//for blueprint actor to reattach parent after all actor finish spawn
		struct BlueprintActorToParentActorStruct
		{
			AActor* BlueprintActor;
			USceneComponent* ParentActor;
		};
		TArray<BlueprintActorToParentActorStruct> BlueprintAndParentArray;
		//for create instanced object, use this as outer
		TArray<UObject*> OutterArray;
		UObject* Outter = nullptr;

		/** Mark of this deserialization session. If nested prefab, this is still the root prefab's value. */
		FGuid DeserializationSessionId = FGuid();
		ALGUIManagerActor* LGUIManagerActor = nullptr;
#if WITH_EDITOR
		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors);
		//serialize and save FProperty
		void SaveProperty(UObject* Target, TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties);
		void SaveCommonProperty(FProperty* Property, int itemType, uint8* Dest, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);

		AActor* DeserializeActorRecursiveForEdit(USceneComponent* Parent, const FLGUIActorSaveData& SaveData, int32& id);
		AActor* DeserializeActorRecursiveForUseInEditor(USceneComponent* Parent, const FLGUIActorSaveData& SaveData, int32& id);
		void DeserializeActorRecursiveForConvertToRuntime(const FLGUIActorSaveData& SaveData, FLGUIActorSaveDataForBuild& ResultSaveData);
		void GenerateActorIDRecursiveForConvertToRuntime(const FLGUIActorSaveData& SaveData, int32& id);
		void RenewActorGuidRecursiveForDuplicate(FLGUIActorSaveData& SaveData);
		void DeserializeActorRecursiveForDuplicate(FLGUIActorSaveData& SaveData);
		void RenewActorGuidForDuplicate_Implement();

		//load and deserialize FProperty
		void LoadProperty(UObject* Target, const TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties);
		void LoadPropertyForConvert(UClass* TargetClass, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, TArray<FName> ExcludeProperties);
		void RemapActorGuidPropertyForDuplicate(UClass* TargetClass, TArray<FLGUIPropertyData>& PropertyData);
		bool LoadCommonProperty(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);
		bool LoadCommonPropertyForConvert(FProperty* Property, int itemType, int containerItemIndex, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);
		bool RemapActorGuidCommonPropertyForDuplicate(FProperty* Property, int itemType, int containerItemIndex, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);
#endif
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
		AActor* DeserializeActorRecursiveForUseInRuntime(USceneComponent* Parent, const FLGUIActorSaveDataForBuild& SaveData, int32& id);
		void LoadPropertyForRuntime(UObject* Target, const TArray<FLGUIPropertyDataForBuild>& PropertyData, TArray<FName> ExcludeProperties);
		bool LoadCommonPropertyForRuntime(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyDataForBuild>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);

		//find id from list, if not will create
		int32 FindOrAddAssetIdFromList(UObject* AssetObject);
		int32 FindOrAddStringFromList(FString string);
		int32 FindOrAddNameFromList(FName name);
		int32 FindOrAddTextFromList(FText text);
		int32 FindOrAddClassFromList(UClass* uclass);
		//find object by id
		UObject* FindAssetFromListByIndex(int32 Id, ULGUIPrefab* InPrefab);
		FString FindStringFromListByIndex(int32 Id, ULGUIPrefab* InPrefab);
		FName FindNameFromListByIndex(int32 Id, ULGUIPrefab* InPrefab);
		FText FindTextFromListByIndex(int32 Id, ULGUIPrefab* InPrefab);
		UClass* FindClassFromListByIndex(int32 Id, ULGUIPrefab* InPrefab);

		void LogForBitConvertFail(bool success, FProperty* Property);

		FString GetValueAsString(const FLGUIPropertyData& ItemPropertyData);

		static const int ItemType_Normal = 0;
		static const int ItemType_Array = 1;
		static const int ItemType_Map = 2;
		static const int ItemType_Set = 3;

		TFunction<void(AActor*)> CallbackBeforeAwake = nullptr;

	public:
#if WITH_EDITORONLY_DATA
		static FName Name_ActorGuid;
#endif

		static TArray<FName> GetActorExcludeProperties(bool instigator, bool actorGuid);
		static TArray<FName> GetComponentExcludeProperties();
	};
}

#endif