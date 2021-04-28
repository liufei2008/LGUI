﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorSerializer.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "Utils/BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"

using namespace LGUIPrefabSystem;

void ActorSerializer::DeserializeActorRecursiveForConvert(const FLGUIActorSaveData& SaveData, FLGUIActorSaveDataForBuild& ResultSaveData, int32& id)
{
	ResultSaveData.InitFromActorSaveData(SaveData);
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			ActorClass = AActor::StaticClass();
			UE_LOG(LGUI, Error, TEXT("Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
		}

		LoadPropertyForConvert(ActorClass, SaveData.ActorPropertyData, ResultSaveData.ActorPropertyData, GetActorExcludeProperties());

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		FLGUIComponentSaveDataForBuild RootCompSaveDataForBuild;
		RootCompSaveDataForBuild.InitFromComponentSaveData(RootCompSaveData);
		if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
		{
			if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass))
			{
				if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = USceneComponent::StaticClass();
					UE_LOG(LGUI, Error, TEXT("Class:%s is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				LoadPropertyForConvert(CompClass, RootCompSaveData.PropertyData, RootCompSaveDataForBuild.PropertyData, GetComponentExcludeProperties());
				ResultSaveData.ComponentPropertyData.Add(RootCompSaveDataForBuild);
			}
		}

		int ComponentCount = SaveData.ComponentPropertyData.Num();
		for (int i = 1; i < ComponentCount; i++)//start from 1, skip RootComponent
		{
			auto CompData = SaveData.ComponentPropertyData[i];
			FLGUIComponentSaveDataForBuild CompDataForBuild;
			CompDataForBuild.InitFromComponentSaveData(CompData);
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = UActorComponent::StaticClass();
					UE_LOG(LGUI, Error, TEXT("Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				LoadPropertyForConvert(CompClass, CompData.PropertyData, CompDataForBuild.PropertyData, GetComponentExcludeProperties());
				ResultSaveData.ComponentPropertyData.Add(CompDataForBuild);
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[DeserializeActorRecursive]Component Class of index:%d not found!"), (CompData.ComponentClass));
			}
		}

		id++;

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			FLGUIActorSaveDataForBuild ChildSaveDataForBuild;
			DeserializeActorRecursiveForConvert(ChildSaveData, ChildSaveDataForBuild, id);
			ResultSaveData.ChildActorData.Add(ChildSaveDataForBuild);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[DeserializeActorRecursive]Actor Class of index:%d not found!"), (SaveData.ActorClass));
	}
}

void ActorSerializer::LoadPropertyForConvert(UClass* TargetClass, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<UProperty>(TargetClass);
	int excludePropertyCount = ExcludeProperties.Num();
	int propertyIndex = 0;
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = Cast<UObjectPropertyBase>(propertyItem))
		{
			if (objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
			{
				continue;
			}
		}
		auto propertyName = propertyItem->GetFName();
		if (excludePropertyCount != 0)
		{
			int32 index;
			if (ExcludeProperties.Find(propertyName, index))
			{
				ExcludeProperties.RemoveAt(index);
				excludePropertyCount--;
				continue;
			}
		}
		if (LoadCommonPropertyForConvert(propertyItem, ItemType_Normal, propertyIndex, PropertyData, ResultPropertyData))
		{
			propertyIndex++;
		}
	}
}

bool ActorSerializer::LoadCommonPropertyForConvert(UProperty* Property, int itemType, int itemPropertyIndex, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return false;//skip property with these flags
	if (Property->IsEditorOnlyProperty())return false;//Build data dont have EditorOnly property
	FLGUIPropertyData ItemPropertyData;
	FLGUIPropertyDataForBuild ItemPropertyDataForBuild;
	bool HaveData = false;

	switch (itemType)
	{
	case ItemType_Normal:
	{
		if (FLGUIPropertyData::GetPropertyDataFromArray(Property->GetFName(), PropertyData, ItemPropertyData))
		{
			HaveData = true;
		}
		else
		{
			ResultPropertyData.Add(ItemPropertyDataForBuild);//have not data, empty
			HaveData = false;
			return true;
		}
	}
	break;
	case ItemType_Array:
	case ItemType_Map:
	case ItemType_Set:
	{
		if (itemPropertyIndex < PropertyData.Num())
		{
			ItemPropertyData = PropertyData[itemPropertyIndex];
			HaveData = true;
		}
	}
	break;
	}

	bool bitConvertSuccess = false;
	if (HaveData)
	{
		ItemPropertyDataForBuild.InitFromPropertySaveData(ItemPropertyData);
		if (Property->ArrayDim > 1 && isInsideCppArray == false)
		{
			for (int i = 0; i < Property->ArrayDim; i++)
			{
				LoadCommonPropertyForConvert(Property, ItemType_Array, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData, i, true);
			}
			ResultPropertyData.Add(ItemPropertyDataForBuild);
			return true;
		}
		else
		{
			if (Cast<UClassProperty>(Property) != nullptr || Cast<USoftClassProperty>(Property) != nullptr)
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto objProperty = Cast<UObjectPropertyBase>(Property))
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
				{
					ResultPropertyData.Add(ItemPropertyDataForBuild);
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						if (auto newObjClass = FindClassFromListByIndex(index))
						{
							LoadPropertyForConvert(newObjClass, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData, {});
						}
					}
					ResultPropertyData.Add(ItemPropertyDataForBuild);
					return true;
				}
				return true;
			}

			else if (auto interfaceProperty = Cast<UInterfaceProperty>(Property))
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]UInterfaceProperty:%s"), *(Property->GetFName().ToString()));
			}

			else if (auto arrProperty = Cast<UArrayProperty>(Property))
			{
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//array count
				for (int i = 0; i < ArrayCount; i++)
				{
					LoadCommonPropertyForConvert(arrProperty->Inner, ItemType_Array, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto mapProperty = Cast<UMapProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess) * 2;//Map element's data is stored as key,value,key,value...
				for (int i = 0; i < count; i++)
				{
					LoadCommonPropertyForConvert(mapProperty->KeyProp, ItemType_Map, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);//key
					i++;
					LoadCommonPropertyForConvert(mapProperty->ValueProp, ItemType_Map, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);//value. PairPtr of map is the real ptr of value
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto setProperty = Cast<USetProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//Set count
				for (int i = 0; i < count; i++)
				{
					LoadCommonPropertyForConvert(setProperty->ElementProp, ItemType_Set, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto structProperty = Cast<UStructProperty>(Property))
			{
				bool isSimpleProperty = false;
				switch (ItemPropertyData.PropertyType)
				{
				case ELGUIPropertyType::PT_Vector2:
				case ELGUIPropertyType::PT_Vector3:
				case ELGUIPropertyType::PT_Vector4:
				case ELGUIPropertyType::PT_Quat:
				case ELGUIPropertyType::PT_Color:
				case ELGUIPropertyType::PT_LinearColor:
				case ELGUIPropertyType::PT_Rotator:
					isSimpleProperty = true; break;
				}
				if (isSimpleProperty)
				{

				}
				else
				{
					int propertyIndex = 0;
					for (TFieldIterator<UProperty> It(structProperty->Struct); It; ++It)
					{
						if (LoadCommonPropertyForConvert(*It, ItemType_Normal, propertyIndex, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData))
						{
							propertyIndex++;
						}
					}
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}

			else if (auto strProperty = Cast<UStrProperty>(Property))//store string as reference
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto nameProperty = Cast<UNameProperty>(Property))
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto textProperty = Cast<UTextProperty>(Property))
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto delegateProperty = Cast<UDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]delegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else if (auto multicastDelegateProperty = Cast<UMulticastDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]multicastDelegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
		}
		return false;
	}
	return false;
}

#if WITH_EDITOR
void ActorSerializer::ConvertForBuildData(ULGUIPrefab* InPrefab)
{
	FLGUIPrefabSaveData SaveData;
	auto FromBinary = FMemoryReader(InPrefab->BinaryData);
	FromBinary.Seek(0);
	FromBinary << SaveData;
	FromBinary.FlushCache();
	FromBinary.Close();

	FLGUIActorSaveDataForBuild ActorSaveDataForBuild;
	int32 id = 0;
	ActorSerializer serializer(InPrefab);
	serializer.DeserializeActorRecursiveForConvert(SaveData.SavedActor, ActorSaveDataForBuild, id);

	FBufferArchive ToBinaryForBuild;
	ToBinaryForBuild << ActorSaveDataForBuild;
	if (ToBinaryForBuild.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Save binary for build length is 0!"));
		return;
	}
	InPrefab->BinaryDataForBuild = ToBinaryForBuild;

	ToBinaryForBuild.FlushCache();
	ToBinaryForBuild.Empty();
}
#endif
