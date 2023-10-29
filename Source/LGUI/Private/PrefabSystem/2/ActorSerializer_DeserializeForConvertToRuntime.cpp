// Copyright 2019-Present LexLiu. All Rights Reserved.

#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"

using namespace LGUIPrefabSystem;

#if WITH_EDITOR
void ActorSerializer::DeserializeActorRecursiveForConvertToRuntime(const FLGUIActorSaveData& SaveData, FLGUIActorSaveDataForBuild& ResultSaveData)
{
	ResultSaveData.InitFromActorSaveData(SaveData);
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForConvertToRuntime]Find class: '%s' at index: %d, but is not a Actor class, use default"), *(ActorClass->GetFName().ToString()), SaveData.ActorClass);
			ActorClass = AActor::StaticClass();
		}

		LoadPropertyForConvert(ActorClass, SaveData.ActorPropertyData, ResultSaveData.ActorPropertyData, GetActorExcludeProperties(true, true));

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		FLGUIComponentSaveDataForBuild RootCompSaveDataForBuild;
		RootCompSaveDataForBuild.InitFromComponentSaveData(RootCompSaveData);
		if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
		{
			if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass, Prefab.Get()))
			{
				if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
				{
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForConvertToRuntime]Find class: '%s' at index: %d, but is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()), RootCompSaveData.ComponentClass);
					CompClass = USceneComponent::StaticClass();
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
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass, Prefab.Get()))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForConvertToRuntime]Find class: '%s' at index: %d, but is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()), CompData.ComponentClass);
					CompClass = UActorComponent::StaticClass();
				}
				LoadPropertyForConvert(CompClass, CompData.PropertyData, CompDataForBuild.PropertyData, GetComponentExcludeProperties());
				ResultSaveData.ComponentPropertyData.Add(CompDataForBuild);
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForConvertToRuntime]Component Class of index:%d not found!"), (CompData.ComponentClass));
			}
		}

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			FLGUIActorSaveDataForBuild ChildSaveDataForBuild;
			DeserializeActorRecursiveForConvertToRuntime(ChildSaveData, ChildSaveDataForBuild);
			ResultSaveData.ChildActorData.Add(ChildSaveDataForBuild);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForConvertToRuntime]Actor Class of index:%d not found!"), (SaveData.ActorClass));
	}
}

void ActorSerializer::LoadPropertyForConvert(UClass* TargetClass, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<FProperty>(TargetClass);
	int excludePropertyCount = ExcludeProperties.Num();
	int propertyIndex = 0;
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(propertyItem))
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

bool ActorSerializer::LoadCommonPropertyForConvert(FProperty* Property, int itemType, int itemPropertyIndex, const TArray<FLGUIPropertyData>& PropertyData, TArray<FLGUIPropertyDataForBuild>& ResultPropertyData, int cppArrayIndex, bool isInsideCppArray)
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
			ItemPropertyDataForBuild.PropertyType = ELGUIPropertyType::PT_None;
			ResultPropertyData.Add(ItemPropertyDataForBuild);
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
		else
		{
			ItemPropertyDataForBuild.PropertyType = ELGUIPropertyType::PT_None;
			ResultPropertyData.Add(ItemPropertyDataForBuild);
			HaveData = false;
			return true;
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
			if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
			{
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
				{
					if (Prefab->PrefabVersion < 2)
					{

					}
					else//convert guid to index id
					{
						FGuid guid = FGuid();
						if (ItemPropertyData.Data.Num() == 16)
						{
							guid = BitConverter::ToGuid(ItemPropertyData.Data, bitConvertSuccess);
							LogForBitConvertFail(bitConvertSuccess, Property);
							check(ActorGuidToIDForConvert.Contains(guid));
							ItemPropertyDataForBuild.Data = BitConverter::GetBytes(ActorGuidToIDForConvert[guid]);
						}
					}
					ResultPropertyData.Add(ItemPropertyDataForBuild);
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						if (auto newObjClass = FindClassFromListByIndex(index, Prefab.Get()))
						{
							LoadPropertyForConvert(newObjClass, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData, {});
						}
					}
					ResultPropertyData.Add(ItemPropertyDataForBuild);
					return true;
				}
				return true;
			}

			else if (auto interfaceProperty = CastField<FInterfaceProperty>(Property))
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]FInterfaceProperty:%s"), *(Property->GetFName().ToString()));
			}

			else if (auto arrProperty = CastField<FArrayProperty>(Property))
			{
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//array count
				LogForBitConvertFail(bitConvertSuccess, Property);
				for (int i = 0; i < ArrayCount; i++)
				{
					LoadCommonPropertyForConvert(arrProperty->Inner, ItemType_Array, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto mapProperty = CastField<FMapProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess) * 2;//Map element's data is stored as key,value,key,value...
				LogForBitConvertFail(bitConvertSuccess, Property);
				for (int i = 0; i < count; i++)
				{
					LoadCommonPropertyForConvert(mapProperty->KeyProp, ItemType_Map, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);//key
					i++;
					LoadCommonPropertyForConvert(mapProperty->ValueProp, ItemType_Map, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);//value. PairPtr of map is the real ptr of value
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto setProperty = CastField<FSetProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//Set count
				LogForBitConvertFail(bitConvertSuccess, Property);
				for (int i = 0; i < count; i++)
				{
					LoadCommonPropertyForConvert(setProperty->ElementProp, ItemType_Set, i, ItemPropertyData.ContainerData, ItemPropertyDataForBuild.ContainerData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto structProperty = CastField<FStructProperty>(Property))
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
					for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
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

			else if (auto strProperty = CastField<FStrProperty>(Property))//store string as reference
			{
				if (ItemPropertyData.PropertyType != ELGUIPropertyType::PT_String//prev property is not string
					&& Prefab->PrefabVersion >= 1//string type is not valid in version below 1
					)
				{
					auto stringValue = GetValueAsString(ItemPropertyData);
					auto id = FindOrAddStringFromList(stringValue);
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_String;
					ItemPropertyDataForBuild.InitFromPropertySaveData(ItemPropertyData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto nameProperty = CastField<FNameProperty>(Property))
			{
				if(ItemPropertyData.PropertyType != ELGUIPropertyType::PT_Name//prev property is not name
					&& Prefab->PrefabVersion >= 1//name type is not valid in version below 1
					)
				{
					auto nameValue = FName(*GetValueAsString(ItemPropertyData));
					auto id = FindOrAddNameFromList(nameValue);
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Name;
					ItemPropertyDataForBuild.InitFromPropertySaveData(ItemPropertyData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto textProperty = CastField<FTextProperty>(Property))
			{
				if(ItemPropertyData.PropertyType != ELGUIPropertyType::PT_Text//prev property is not text
					&& Prefab->PrefabVersion >= 1//text type is not valid in version below 1
					)
				{
					auto textValue = FText::FromString(GetValueAsString(ItemPropertyData));
					auto id = FindOrAddTextFromList(textValue);
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Text;
					ItemPropertyDataForBuild.InitFromPropertySaveData(ItemPropertyData);
				}
				ResultPropertyData.Add(ItemPropertyDataForBuild);
				return true;
			}
			else if (auto delegateProperty = CastField<FDelegateProperty>(Property))//blueprint dynamic delegate
			{
				//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]delegateProperty:%s"), *(Property->GetFName().ToString()));
			}
			else if (auto multicastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))//blueprint dynamic delegate
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
	serializer.GenerateActorIDRecursiveForConvertToRuntime(SaveData.SavedActor, id);
	serializer.DeserializeActorRecursiveForConvertToRuntime(SaveData.SavedActor, ActorSaveDataForBuild);

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
void ActorSerializer::GenerateActorIDRecursiveForConvertToRuntime(const FLGUIActorSaveData& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		id++;
		ActorGuidToIDForConvert.Add(SaveData.GetActorGuid(FGuid()), id);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			FLGUIActorSaveDataForBuild ChildSaveDataForBuild;
			GenerateActorIDRecursiveForConvertToRuntime(ChildSaveData, id);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::GenerateActorIDRecursiveForConvertToRuntime]Actor Class of index:%d not found!"), (SaveData.ActorClass));
	}
}
#endif

#endif