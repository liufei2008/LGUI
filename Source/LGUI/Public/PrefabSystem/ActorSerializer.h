// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LGUIPrefab.h"

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
	};

	struct LGUIHelperFunction
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
			LGUIHelperFunction::SerializeToArchiveByPropertyType(Ar, PropertyData.PropertyType, PropertyData.Data);
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
			LGUIHelperFunction::SerializeToArchiveByPropertyType(Ar, PropertyData.PropertyType, PropertyData.Data);
			if (PropertyData.PropertyType == ELGUIPropertyType::PT_Container
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_StructContainer
				|| PropertyData.PropertyType == ELGUIPropertyType::PT_InstancedObject
				)
			{
				Ar << PropertyData.ContainerData;
			}
			return Ar;
		}
		void FromPropertyData(const FLGUIPropertyData& InData)
		{
			Data = InData.Data;
			PropertyType = InData.PropertyType;
			for (auto ItemData : InData.ContainerData)
			{
				if (!ItemData.IsEditorOnly)
				{
					FLGUIPropertyDataForBuild ItemNewData;
					ItemNewData.FromPropertyData(ItemData);
					ContainerData.Add(ItemNewData);
				}
			}
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
		void FromComponentSaveData(const FLGUIComponentSaveData& InData)
		{
			ComponentClass = InData.ComponentClass;
			ComponentName = InData.ComponentName;
			SceneComponentParentID = InData.SceneComponentParentID;
			for (auto ItemData : InData.PropertyData)
			{
				if (!ItemData.IsEditorOnly)
				{
					FLGUIPropertyDataForBuild ItemNewData;
					ItemNewData.FromPropertyData(ItemData);
					PropertyData.Add(ItemNewData);
				}
			}
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
		void FromActorSaveData(const FLGUIActorSaveData& InData)
		{
			ActorClass = InData.ActorClass;
			ActorID = InData.ActorID;
			for (auto ItemData : InData.ActorPropertyData)
			{
				if (!ItemData.IsEditorOnly)
				{
					FLGUIPropertyDataForBuild NewItemData;
					NewItemData.FromPropertyData(ItemData);
					ActorPropertyData.Add(NewItemData);
				}
			}
			for (auto ItemData : InData.ComponentPropertyData)
			{
				FLGUIComponentSaveDataForBuild NewItemData;
				NewItemData.FromComponentSaveData(ItemData);
				ComponentPropertyData.Add(NewItemData);
			}
			for (auto ItemData : InData.ChildActorData)
			{
				FLGUIActorSaveDataForBuild NewItemData;
				NewItemData.FromActorSaveData(ItemData);
				ChildActorData.Add(NewItemData);
			}
		}
	};


	DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIPrefabSystem_DeserializeActorDelegate, bool);

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
	public:
		ActorSerializer(UWorld* InTargetWorld);
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
		static AActor* LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale);
		static void SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab);
		//serialize actor
		void SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab);
		//deserialize actor
		AActor* DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform = false, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector, bool ForceUseEditorData = false);
#if WITH_EDITOR
		static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, TArray<AActor*>& AllLoadedActorArray);
		static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity = true);
		//static AActor* LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector InLocation = FVector::ZeroVector, FQuat InRotation = FQuat::Identity, FVector InScale = FVector::OneVector);
#endif
#if WITH_EDITOR
		static FLGUIActorSaveData CreateActorSaveData(ULGUIPrefab* InPrefab);
#endif
		static void ConvertForBuildData(const FLGUIActorSaveData InSaveData, ULGUIPrefab* InPrefab);//convert saved data to build data
		static void RegisterComponent(AActor* Actor, UActorComponent* Comp);
	private:

		TWeakObjectPtr<UWorld> TargetWorld = nullptr;

		TWeakObjectPtr<ULGUIPrefab> Prefab = nullptr;

		TArray<AActor*> DeserializingActorCollection;//collect for deserializing actor

#if WITH_EDITORONLY_DATA
		bool IsEditMode = false;
		bool IsLoadForEdit = false;
#endif

		TMap<AActor*, int32> MapActorToID;
		void GenerateActorIDRecursive(AActor* Actor, int32& id);
		TMap<int32, AActor*> MapIDToActor;
		struct UPropertyMapStruct
		{
			FProperty* ObjProperty;//FObjectProperty
			int32 id;//property's UObject's id
			uint8* Dest;//FObjectProperty's container address
			int32 cppArrayIndex = 0;//if is c++ array's element
		};
		TArray<UPropertyMapStruct> ObjectMapStructList;
		TArray<AActor*> CreatedActors;

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

		FLGUIActorSaveData SerializeSingleActor(AActor* Actor);
		//serialize actor
		void SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& SavedActors);
		//deserialize actor
		AActor* DeserializeActorRecursive(USceneComponent* Parent, FLGUIActorSaveData& SaveData, int32& id);
		AActor* DeserializeActorRecursive(USceneComponent* Parent, FLGUIActorSaveDataForBuild& SaveData, int32& id);

		//serialize and save FProperty
		void SaveProperty(UObject* Target, TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties);
		//load and deserialize FProperty
		void LoadProperty(UObject* Target, const TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties);
		void LoadProperty(UObject* Target, const TArray<FLGUIPropertyDataForBuild>& PropertyData, TArray<FName> ExcludeProperties);

		//find id from list, if not will create
		int32 FindAssetIdFromList(UObject* AssetObject);
		int32 FindStringIdFromList(FString string);
		int32 FindNameIdFromList(FName name);
		int32 FindTextIdFromList(FText text);
		int32 FindClassIdFromList(UClass* uclass);
		//find object by id
		UObject* FindAssetFromListByIndex(int32 Id);
		FString FindStringFromListByIndex(int32 Id);
		FName FindNameFromListByIndex(int32 Id);
		FText FindTextFromListByIndex(int32 Id);
		UClass* FindClassFromListByIndex(int32 Id);

		void SaveCommonProperty(FProperty* Property, int itemType, uint8* Dest, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);
		bool LoadCommonProperty(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);
		bool LoadCommonProperty(FProperty* Property, int itemType, int containerItemIndex, uint8* Dest, const TArray<FLGUIPropertyDataForBuild>& PropertyData, int cppArrayIndex = 0, bool isInsideCppArray = false);

		TArray<FName> GetActorExcludeProperties();
		TArray<FName> GetComponentExcludeProperties();

		static const int ItemType_Normal = 0;
		static const int ItemType_Array = 1;
		static const int ItemType_Map = 2;
		static const int ItemType_Set = 3;
	};
}