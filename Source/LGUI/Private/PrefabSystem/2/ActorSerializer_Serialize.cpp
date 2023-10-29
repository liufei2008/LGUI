// Copyright 2019-Present LexLiu. All Rights Reserved.

#if WITH_EDITOR
#include "PrefabSystem/2/ActorSerializer.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "BitConverter.h"
#include "Engine/World.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BufferArchive.h"
#include "Components/PrimitiveComponent.h"
#include "UObject/TextProperty.h"
#include "Runtime/Launch/Resources/Version.h"

using namespace LGUIPrefabSystem;

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITOR
void ActorSerializer::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& OutActorSaveData)
{
	OutActorSaveData.ActorClass = FindOrAddClassFromList(Actor->GetClass());

	SaveProperty(Actor, OutActorSaveData.ActorPropertyData, GetActorExcludeProperties(true, false));
	check(MapActorToGuid.Contains(Actor));//this should not happen, because we already call GenerateActorIDRecursive
	auto ActorGuid = MapActorToGuid[Actor];
	OutActorSaveData.SetActorGuid(ActorGuid);

	SerializedActors.Add(Actor);
	SerializedActorsGuid.Add(ActorGuid);
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
		if (!SkippingActors.Contains(ChildActor))
		{
			FLGUIActorSaveData ChildActorSaveData;
			SerializeActorRecursive(ChildActor, ChildActorSaveData);
			ChildSaveDataList.Add(ChildActorSaveData);
		}
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
					auto id = FindOrAddAssetIdFromList(object);
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


void ActorSerializer::SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab
	, const TArray<AActor*>& InExistingActorArray, const TArray<FGuid>& InExistingActorGuidInPrefab
	, TArray<AActor*>& OutSerializedActors, TArray<FGuid>& OutSerializedActorsGuid)
{
	if (!RootActor || !InPrefab)
	{
		UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]RootActor Or InPrefab is null!"));
		return;
	}
	if (!RootActor->GetWorld())
	{
		UE_LOG(LGUI, Error, TEXT("[ActorSerializer::SerializeActor]Cannot get World from RootActor!"));
		return;
	}
	ActorSerializer serializer(RootActor->GetWorld());
	serializer.ExistingActors = InExistingActorArray;
	serializer.ExistingActorsGuid = InExistingActorGuidInPrefab;
	serializer.SerializeActor(RootActor, InPrefab);
	OutSerializedActors = serializer.SerializedActors;
	OutSerializedActorsGuid = serializer.SerializedActorsGuid;
}
void ActorSerializer::SerializeActor(AActor* RootActor, ULGUIPrefab* InPrefab)
{
	Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);
	//clear old reference data
	Prefab->ReferenceAssetList.Empty();
	Prefab->ReferenceStringList.Empty();
	Prefab->ReferenceNameList.Empty();
	Prefab->ReferenceTextList.Empty();
	Prefab->ReferenceClassList.Empty();

	auto StartTime = FDateTime::Now();

	CollectSkippingActorsRecursive(RootActor);

	GenerateActorIDRecursive(RootActor);

	FLGUIActorSaveData ActorSaveData;
	SerializeActorRecursive(RootActor, ActorSaveData);

	FLGUIPrefabSaveData SaveData;
	SaveData.SavedActor = ActorSaveData;

	FBufferArchive ToBinary;
	ToBinary << SaveData;

	if (ToBinary.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Save binary length is 0!"));
		return;
	}

	Prefab->BinaryData = ToBinary;
	Prefab->ThumbnailDirty = true;

	ToBinary.FlushCache();
	ToBinary.Empty();
	Prefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
	Prefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
	Prefab->EnginePatchVersion = ENGINE_PATCH_VERSION;
	Prefab->PrefabVersion = LGUI_CURRENT_PREFAB_VERSION;
	Prefab->CreateTime = FDateTime::Now();

	Prefab->MarkPackageDirty();
	UE_LOG(LGUI, Log, TEXT("[ActorSerializer::SerializeActor] prefab:%s duration:%s"), *(Prefab->GetPathName()), *((FDateTime::Now() - StartTime).ToString()));
}

FLGUIActorSaveData ActorSerializer::CreateActorSaveData(ULGUIPrefab* InPrefab)
{
	FLGUIPrefabSaveData SaveData;
	auto LoadedData = InPrefab->BinaryData;
	if (LoadedData.Num() <= 0)
	{
		UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
		return SaveData.SavedActor;
	}
	auto FromBinary = FMemoryReader(LoadedData, true);
	FromBinary.Seek(0);
	FromBinary << SaveData;
	FromBinary.FlushCache();
	FromBinary.Close();
	LoadedData.Empty();
	return SaveData.SavedActor;
}

void ActorSerializer::GenerateActorIDRecursive(AActor* Actor)
{
    if (!MapActorToGuid.Contains(Actor))
    {
        MapActorToGuid.Add(Actor, Actor->GetActorGuid());
    }

	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		if (!SkippingActors.Contains(ChildActor))
		{
			GenerateActorIDRecursive(ChildActor);
		}
	}
}

void ActorSerializer::CollectSkippingActorsRecursive(AActor* Actor)
{
	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		CollectSkippingActorsRecursive(ChildActor);
	}
}

void ActorSerializer::SetActorGUIDNew(AActor* Actor)
{
	auto guidProp = FindFieldChecked<FStructProperty>(AActor::StaticClass(), Name_ActorGuid);
	auto newGuid = FGuid::NewGuid();
	guidProp->CopyCompleteValue(guidProp->ContainerPtrToValuePtr<void>(Actor), &newGuid);
}
void ActorSerializer::SetActorGUID(AActor* Actor, FGuid Guid)
{
	auto guidProp = FindFieldChecked<FStructProperty>(AActor::StaticClass(), Name_ActorGuid);
	guidProp->CopyCompleteValue(guidProp->ContainerPtrToValuePtr<void>(Actor), &Guid);
}

#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif

#endif
