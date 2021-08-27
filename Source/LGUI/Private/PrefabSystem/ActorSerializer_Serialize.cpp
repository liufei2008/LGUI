// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

void ActorSerializer::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& OutActorSaveData)
{
	OutActorSaveData.ActorClass = FindOrAddClassFromList(Actor->GetClass());

	SaveProperty(Actor, OutActorSaveData.ActorPropertyData, GetActorExcludeProperties(true, false));
#if WITH_EDITOR
	if (serializeMode == EPrefabSerializeMode::RecreateFromExisting)//no need to replace guid
	{
	}
	else if (serializeMode == EPrefabSerializeMode::CreateNew)
	{
		OutActorSaveData.SetActorGuid(FGuid::NewGuid());//create mode use new guid
	}
	else if (serializeMode == EPrefabSerializeMode::Apply)//apply mode use existing guid
	{
		//replace guid
		int foundActorIndex = ExistingActors.Find(Actor);
		if (foundActorIndex != INDEX_NONE)
		{
			auto guid = ExistingActorsGuid[foundActorIndex];
			ExistingActors.RemoveAtSwap(foundActorIndex);
			ExistingActorsGuid.RemoveAtSwap(foundActorIndex);
			OutActorSaveData.SetActorGuid(guid);
		}
	}
	SerializedActors.Add(Actor);
	SerializedActorsGuid.Add(OutActorSaveData.GetActorGuid(Actor->GetActorGuid()));
#endif
	auto& Components = Actor->GetComponents();

	TArray<USceneComponent*> AllSceneComponentOfThisActor;
	auto RootComp = Actor->GetRootComponent();
	FLGUIComponentSaveData RootCompData;
	if (RootComp)
	{
		RootCompData.ComponentClass = FindOrAddClassFromList(RootComp->GetClass());
		RootCompData.ComponentName = RootComp->GetFName();
		SaveProperty(RootComp, RootCompData.PropertyData, GetComponentExcludeProperties());
		AllSceneComponentOfThisActor.Add(RootComp);
	}
	OutActorSaveData.ComponentPropertyData.Add(RootCompData);//add a data seat for RootComponent, so when deserialize we dont need to check again

	for (auto Comp : Components)
	{
		if (Comp == RootComp)continue;//skip RootComponent
		if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		if (Comp->GetClass()->IsChildOf(ULGUIPrefabHelperComponent::StaticClass()))continue;//skip LGUIPrefabHelperComponent, this component is only for editor helper
																							//if (Comp->IsInBlueprint())continue;//skip Blueprint component
		FLGUIComponentSaveData CompData;
		CompData.ComponentClass = FindOrAddClassFromList(Comp->GetClass());
		CompData.ComponentName = Comp->GetFName();
		if (auto SceneComp = Cast<USceneComponent>(Comp))
		{
			if (auto parentComp = SceneComp->GetAttachParent())
			{
				CompData.SceneComponentParent = parentComp;
			}
			AllSceneComponentOfThisActor.Add(SceneComp);
		}
		SaveProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
		OutActorSaveData.ComponentPropertyData.Add(CompData);
	}
	for (int i = 0, count = OutActorSaveData.ComponentPropertyData.Num(); i < count; i++)
	{
		auto& componentPropertyData = OutActorSaveData.ComponentPropertyData[i];
		if (componentPropertyData.SceneComponentParent)
		{
			int32 parentCompIndex;
			if (AllSceneComponentOfThisActor.Find(componentPropertyData.SceneComponentParent, parentCompIndex))
			{
				OutActorSaveData.ComponentPropertyData[i].SceneComponentParentID = parentCompIndex;
			}
		}
	}

	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	TArray<FLGUIActorSaveData> ChildSaveDataList;
	for (auto ChildActor : ChildrenActors)
	{
		FLGUIActorSaveData ChildActorSaveData;
		SerializeActorRecursive(ChildActor, ChildActorSaveData);
		ChildSaveDataList.Add(ChildActorSaveData);
	}
	OutActorSaveData.ChildActorData = ChildSaveDataList;
}
void ActorSerializer::SaveProperty(UObject* Target, TArray<FLGUIPropertyData>& PropertyData, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
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
				ExcludeProperties.RemoveAtSwap(index);
				excludePropertyCount--;
				continue;
			}
		}
		SaveCommonProperty(propertyItem, ItemType_Normal, (uint8*)Target, PropertyData);
	}
}
void ActorSerializer::SaveCommonProperty(FProperty* Property, int itemType, uint8* Dest, TArray<FLGUIPropertyData>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return;//skip property with these flag
	FLGUIPropertyData ItemPropertyData;
	if (Property->IsEditorOnlyProperty())
		ItemPropertyData.IsEditorOnly = true;
	switch (itemType)
	{
	case ItemType_Normal://normal property stored as Name-Value
	{
		ItemPropertyData.Name = Property->GetFName();
	}
	break;
	}
	if (Property->ArrayDim > 1 && isInsideCppArray == false)
	{
		ItemPropertyData.Data = BitConverter::GetBytes(Property->ArrayDim);
		ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			SaveCommonProperty(Property, ItemType_Array, Dest, ItemPropertyData.ContainerData, i, true);
		}
		PropertyData.Add(ItemPropertyData);
	}
	else
	{
		if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
		{
			auto classProperty = (FObjectPropertyBase*)Property;
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			if (auto object = classProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex))
			{
				auto id = FindOrAddClassFromList((UClass*)object);
				ItemPropertyData.Data = BitConverter::GetBytes(id);
			}
			else
			{
				ItemPropertyData.Data = BitConverter::GetBytes(-1);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
		{
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Reference;
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex))
			{
				if (object->IsAsset() || object->IsA(UClass::StaticClass()))
				{
					auto id = FindAssetIdFromList(object);
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					PropertyData.Add(ItemPropertyData);
				}
				else if (Property->HasAnyPropertyFlags(CPF_InstancedReference))
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_InstancedObject;
					auto id = FindOrAddClassFromList(object->GetClass());
					ItemPropertyData.Data = BitConverter::GetBytes(id);
					if (object->GetClass()->IsChildOf(USceneComponent::StaticClass()))
					{
						SaveProperty(object, ItemPropertyData.ContainerData, GetComponentExcludeProperties());
					}
					else
					{
						SaveProperty(object, ItemPropertyData.ContainerData, {});
					}
					PropertyData.Add(ItemPropertyData);
				}
				else if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//Actor need remap
				{
					auto actor = Cast<AActor>(object);
					if (auto guid = MapActorToGuid.Find(actor))
					{
						ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Common;//mark the property as PT_Common, because PT_Reference only allow 4 byte on data array
						ItemPropertyData.Data = BitConverter::GetBytes(*guid);
						PropertyData.Add(ItemPropertyData);
					}
					else//if reference actor is not a child of prefab's root actor, then save index as -1, means null value
					{
						ItemPropertyData.Data = BitConverter::GetBytes(-1);
						PropertyData.Add(ItemPropertyData);
					}
				}
				else
				{
					ItemPropertyData.Data = BitConverter::GetBytes(-1);
					PropertyData.Add(ItemPropertyData);
				}
			}
			else
			{
				ItemPropertyData.Data = BitConverter::GetBytes(-1);
				PropertyData.Add(ItemPropertyData);
			}
		}

		else if (auto interfaceProperty = CastField<FInterfaceProperty>(Property))
		{
			//UE_LOG(LGUI, Error, TEXT("[ActorSerializerNotHandled]FInterfaceProperty:%s"), *(Property->GetFName().ToString()));
		}

		else if (auto arrProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto arrayCount = ArrayHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(arrayCount);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < arrayCount; i++)
			{
				SaveCommonProperty(arrProperty->Inner, ItemType_Array, ArrayHelper.GetRawPtr(i), ItemPropertyData.ContainerData);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto mapProperty = CastField<FMapProperty>(Property))//map element's data stored as key/value/key/value...
		{
			FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = MapHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(count);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < count; i++)
			{
				SaveCommonProperty(mapProperty->KeyProp, ItemType_Map, MapHelper.GetKeyPtr(i), ItemPropertyData.ContainerData);//key
				SaveCommonProperty(mapProperty->ValueProp, ItemType_Map, MapHelper.GetPairPtr(i), ItemPropertyData.ContainerData);//value
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto setProperty = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = SetHelper.Num();
			ItemPropertyData.Data = BitConverter::GetBytes(count);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Container;
			for (int i = 0; i < count; i++)
			{
				SaveCommonProperty(setProperty->ElementProp, ItemType_Set, SetHelper.GetElementPtr(i), ItemPropertyData.ContainerData);
			}
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto structProperty = CastField<FStructProperty>(Property))
		{
			auto structName = structProperty->Struct->GetFName();
			bool isSimpleData = false;
			if (structName == TEXT("Vector2D"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector2; isSimpleData = true;
			}
			else if (structName == TEXT("Vector"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector3; isSimpleData = true;
			}
			else if (structName == TEXT("Vector4"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Vector4; isSimpleData = true;
			}
			else if (structName == TEXT("Quat"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Quat; isSimpleData = true;
			}
			else if (structName == TEXT("Color"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Color; isSimpleData = true;
			}
			else if (structName == TEXT("LinearColor"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_LinearColor; isSimpleData = true;
			}
			else if (structName == TEXT("Rotator"))
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Rotator; isSimpleData = true;
			}
			if (isSimpleData)
			{
				auto data = Property->ContainerPtrToValuePtr<uint8>((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data.Reserve(Property->ElementSize);
				for (int i = 0; i < Property->ElementSize; i++)
				{
					ItemPropertyData.Data.Add(data[i]);
				}
			}
			else
			{
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_StructContainer;
				auto structPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
				for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
				{
					SaveCommonProperty(*It, ItemType_Normal, structPtr, ItemPropertyData.ContainerData);
				}
			}
			PropertyData.Add(ItemPropertyData);
		}

		else if (auto strProperty = CastField<FStrProperty>(Property))
		{
			auto stringValue = strProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindOrAddStringFromList(stringValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_String;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto nameProperty = CastField<FNameProperty>(Property))
		{
			auto nameValue = nameProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindOrAddNameFromList(nameValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Name;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto textProperty = CastField<FTextProperty>(Property))
		{
			auto textValue = textProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindOrAddTextFromList(textValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Text;
			PropertyData.Add(ItemPropertyData);
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
			if (auto boolProperty = CastField<FBoolProperty>(Property))//some bool is declared as bit field, so we need to handle it specially
			{
				auto value = boolProperty->GetPropertyValue_InContainer((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data = BitConverter::GetBytes(value);
				ItemPropertyData.PropertyType = ELGUIPropertyType::PT_bool;
			}
			else
			{
				if (CastField<FInt8Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int8;
				}
				else if (CastField<FInt16Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int16;
				}
				else if (CastField<FIntProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int32;
				}
				else if (CastField<FInt64Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_int64;
				}
				else if (CastField<FByteProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint8;
				}
				else if (CastField<FUInt16Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint16;
				}
				else if (CastField<FUInt32Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint32;
				}
				else if (CastField<FUInt64Property>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint64;
				}
				else if (CastField<FFloatProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_float;
				}
				else if (CastField<FDoubleProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_double;
				}
				else if (CastField<FEnumProperty>(Property) != nullptr)
				{
					ItemPropertyData.PropertyType = ELGUIPropertyType::PT_uint8;
				}
				auto data = Property->ContainerPtrToValuePtr<uint8>((void*)Dest, cppArrayIndex);
				ItemPropertyData.Data.Reserve(Property->ElementSize);
				for (int i = 0; i < Property->ElementSize; i++)
				{
					ItemPropertyData.Data.Add(data[i]);
				}
			}
			PropertyData.Add(ItemPropertyData);
		}
	}
}

