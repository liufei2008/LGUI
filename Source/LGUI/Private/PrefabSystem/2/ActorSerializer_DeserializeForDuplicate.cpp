// Copyright 2019-2022 LexLiu. All Rights Reserved.

#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Core/Actor/LGUIManagerActor.h"

using namespace LGUIPrefabSystem;

#if WITH_EDITORONLY_DATA
void ActorSerializer::RenewActorGuidRecursiveForDuplicate(FLGUIActorSaveData& SaveData)
{
	auto OriginGuid = SaveData.GetActorGuid(FGuid());
	auto NewGuid = FGuid::NewGuid();
	DuplicateGuidFromOriginToNew.Add(OriginGuid, NewGuid);
	SaveData.SetActorGuid(NewGuid);
	for (auto& ChildSaveData : SaveData.ChildActorData)
	{
		RenewActorGuidRecursiveForDuplicate(ChildSaveData);
	}
}
void ActorSerializer::DeserializeActorRecursiveForDuplicate(FLGUIActorSaveData& SaveData)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		RemapActorGuidPropertyForDuplicate(ActorClass, SaveData.ActorPropertyData);

		int ComponentCount = SaveData.ComponentPropertyData.Num();
		for (int i = 0; i < ComponentCount; i++)
		{
			auto& CompData = SaveData.ComponentPropertyData[i];
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass, Prefab.Get()))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForDuplicate]Find class: '%s' at index: %d, but is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()), CompData.ComponentClass);
					CompClass = UActorComponent::StaticClass();
				}
				RemapActorGuidPropertyForDuplicate(CompClass, CompData.PropertyData);
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForDuplicate]Component Class of index:%d not found!"), (CompData.ComponentClass));
			}
		}

		for (auto& ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursiveForDuplicate(ChildSaveData);
		}
	}
}
void ActorSerializer::RemapActorGuidPropertyForDuplicate(UClass* TargetClass, TArray<FLGUIPropertyData>& PropertyData)
{
	auto propertyField = TFieldRange<FProperty>(TargetClass);
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
		if (RemapActorGuidCommonPropertyForDuplicate(propertyItem, ItemType_Normal, propertyIndex, PropertyData))
		{
			propertyIndex++;
		}
	}

	OutterArray.RemoveAt(OutterArray.Num() - 1);
	if (OutterArray.Num() >= 1)
	{
		Outter = OutterArray[OutterArray.Num() - 1];
	}
	else
	{
		Outter = nullptr;
	}
}
bool ActorSerializer::RemapActorGuidCommonPropertyForDuplicate(FProperty* Property, int itemType, int itemPropertyIndex, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return false;//skip property with these flags
	FLGUIPropertyData ItemPropertyData;
	bool HaveData = false;

	switch (itemType)
	{
	case ItemType_Normal:
	{
		if (FLGUIPropertyData::GetPropertyDataFromArray(Property->GetFName(), PropertyData, ItemPropertyData))
		{
			HaveData = true;
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
		if (Property->ArrayDim > 1 && isInsideCppArray == false)
		{
			for (int i = 0; i < Property->ArrayDim; i++)
			{
				RemapActorGuidCommonPropertyForDuplicate(Property, ItemType_Array, i, ItemPropertyData.ContainerData, i, true);
			}
			return true;
		}
		else
		{
			if (auto objProperty = CastField<FObjectPropertyBase>(Property))
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);

					if (index <= -1)
					{
						return true;
					}
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//if is Actor
				{
					FGuid guid = FGuid();
					if (ItemPropertyData.Data.Num() == 16)
					{
						guid = BitConverter::ToGuid(ItemPropertyData.Data, bitConvertSuccess);
						LogForBitConvertFail(bitConvertSuccess, Property);
						if (DuplicateGuidFromOriginToNew.Contains(guid))
						{
							auto newGuid = DuplicateGuidFromOriginToNew[guid];
							ItemPropertyData.Data = BitConverter::GetBytes(newGuid);
							FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
						}
					}
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						if (auto newObjClass = FindClassFromListByIndex(index, Prefab.Get()))
						{
							RemapActorGuidPropertyForDuplicate(newObjClass, ItemPropertyData.ContainerData);
							FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
						}
					}
					return true;
				}
				return true;
			}

			else if (auto arrProperty = CastField<FArrayProperty>(Property))
			{
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//array count
				LogForBitConvertFail(bitConvertSuccess, Property);
				for (int i = 0; i < ArrayCount; i++)
				{
					RemapActorGuidCommonPropertyForDuplicate(arrProperty->Inner, ItemType_Array, i, ItemPropertyData.ContainerData);
				}
				FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
				return true;
			}
			else if (auto mapProperty = CastField<FMapProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess) * 2;//Map element's data is stored as key,value,key,value...
				int mapIndex = 0;
				for (int i = 0; i < count; i++)
				{
					RemapActorGuidCommonPropertyForDuplicate(mapProperty->KeyProp, ItemType_Map, i, ItemPropertyData.ContainerData);//key
					i++;
					RemapActorGuidCommonPropertyForDuplicate(mapProperty->ValueProp, ItemType_Map, i, ItemPropertyData.ContainerData);//value. PairPtr of map is the real ptr of value
					mapIndex++;
				}
				FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
				return true;
			}
			else if (auto setProperty = CastField<FSetProperty>(Property))
			{
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//Set count
				for (int i = 0; i < count; i++)
				{
					RemapActorGuidCommonPropertyForDuplicate(setProperty->ElementProp, ItemType_Set, i, ItemPropertyData.ContainerData);
				}
				FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
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
						if (RemapActorGuidCommonPropertyForDuplicate(*It, ItemType_Normal, propertyIndex, ItemPropertyData.ContainerData))
						{
							propertyIndex++;
						}
					}
					FLGUIPropertyData::SetPropertyDataToArray(Property->GetFName(), PropertyData, ItemPropertyData);
				}
				return true;
			}
		}
		return false;
	}
	return false;
}
#endif

#endif
