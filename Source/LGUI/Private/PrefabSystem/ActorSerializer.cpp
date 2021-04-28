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

ActorSerializer::ActorSerializer(ULGUIPrefab* InPrefab)
{
	Prefab = InPrefab;
}
ActorSerializer::ActorSerializer(UWorld* InTargetWorld)
{
	TargetWorld = TWeakObjectPtr<UWorld>(InTargetWorld);
}
#if WITH_EDITOR
AActor* ActorSerializer::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, TArray<AActor*>& AllLoadedActorArray)
{
	ActorSerializer serializer(InWorld);
	auto rootActor = serializer.DeserializeActor(Parent, InPrefab, false, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
	AllLoadedActorArray = serializer.CreatedActors;
	return rootActor;
}
AActor* ActorSerializer::LoadPrefabForEdit(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity)
{
	ActorSerializer serializer(InWorld);
	if (SetRelativeTransformToIdentity)
	{
		return serializer.DeserializeActor(Parent, InPrefab, true, FVector::ZeroVector, FQuat::Identity, FVector::OneVector);
	}
	else
	{
		return serializer.DeserializeActor(Parent, InPrefab);
	}
}
#endif
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, bool SetRelativeTransformToIdentity)
{
	ActorSerializer serializer(InWorld);
	AActor* result = nullptr;
	if (SetRelativeTransformToIdentity)
	{
		result = serializer.DeserializeActor(Parent, InPrefab, true);
	}
	else
	{
		result = serializer.DeserializeActor(Parent, InPrefab);
	}
	return result;
}
AActor* ActorSerializer::LoadPrefab(UWorld* InWorld, ULGUIPrefab* InPrefab, USceneComponent* Parent, FVector RelativeLocation, FQuat RelativeRotation, FVector RelativeScale)
{
	ActorSerializer serializer(InWorld);
	return serializer.DeserializeActor(Parent, InPrefab, true, RelativeLocation, RelativeRotation, RelativeScale);
}

AActor* ActorSerializer::DeserializeActor(USceneComponent* Parent, ULGUIPrefab* InPrefab, bool ReplaceTransform, FVector InLocation, FQuat InRotation, FVector InScale)
{
	if (!InPrefab)
	{
		UE_LOG(LGUI, Error, TEXT("Load Prefab, InPrefab is null!"));
		return nullptr;
	}
	if (!TargetWorld.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("Load Prefab, World is null!"));
		return nullptr;
	}

	//auto StartTime = FDateTime::Now();
#if WITH_EDITORONLY_DATA
	FLGUIPrefabSaveData SaveData;
	{
		auto LoadedData = InPrefab->BinaryData;
		if (LoadedData.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data is empty!"));
			return nullptr;
		}
		auto FromBinary = FMemoryReader(LoadedData, true);
		FromBinary.Seek(0);
		FromBinary << SaveData;
		FromBinary.FlushCache();
		FromBinary.Close();
		LoadedData.Empty();
	}
#else
	FLGUIActorSaveDataForBuild SaveDataForBuild;
	{
		auto& LoadedDataForBuild = InPrefab->BinaryDataForBuild;
		if (LoadedDataForBuild.Num() <= 0)
		{
			UE_LOG(LGUI, Warning, TEXT("Loaded data for build is empty!"));
			return nullptr;
		}
		auto FromBinaryForBuild = FMemoryReader(LoadedDataForBuild, true);
		FromBinaryForBuild.Seek(0);
		FromBinaryForBuild << SaveDataForBuild;
		FromBinaryForBuild.FlushCache();
		FromBinaryForBuild.Close();
	}
#endif
	Prefab = TWeakObjectPtr<ULGUIPrefab>(InPrefab);
#if WITH_EDITOR
	IsEditMode = !TargetWorld->IsGameWorld();
	if (IsEditMode)
	{
		ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(TargetWorld.Get());
	}
	else
#endif
	{
		ALGUIManagerActor::BeginPrefabSystemProcessingActor(TargetWorld.Get());
	}
	int32 id = 0;
	AActor* CreatedActor = nullptr;
#if WITH_EDITORONLY_DATA
	{
		CreatedActor = DeserializeActorRecursive(Parent, SaveData.SavedActor, id);
	}
#else
	{
		CreatedActor = DeserializeActorRecursiveForBuild(Parent, SaveDataForBuild, id);
	}
#endif
	if (CreatedActor != nullptr)
	{
		if (USceneComponent* rootComp = CreatedActor->GetRootComponent())
		{
			rootComp->UpdateComponentToWorld();
			if (ReplaceTransform)
			{
				rootComp->SetRelativeLocationAndRotation(InLocation, InRotation);
				rootComp->SetRelativeScale3D(InScale);
			}
		}
	}

	//reassign actor reference after all actor are created
	for (auto item : ObjectMapStructList)
	{
		if (auto obj = MapIDToActor.Find(item.id))
		{
			auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *obj, item.cppArrayIndex);
		}
	}
	//finish Actor Spawn
	for (auto Actor : CreatedActors)
	{
		if (Actor->IsValidLowLevel() && !Actor->IsPendingKill())//check, incase some actor is destroyed by other actor when BeginPlay
		{
#if WITH_EDITORONLY_DATA
			if (!IsEditMode)
			{
				Actor->bIsEditorPreviewActor = false;//make this to false, or BeginPlay won't be called
			}
#endif
			Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay is called at this point
		}
	}
	//assign parent for blueprint actor after actor spawn, otherwise blueprint actor will attach to world root
	for (auto item : BlueprintAndParentArray)
	{
		if (item.BlueprintActor->IsValidLowLevel() && !item.BlueprintActor->IsPendingKill() && item.ParentActor->IsValidLowLevel() && !item.ParentActor->IsPendingKill())
		{
			item.BlueprintActor->GetRootComponent()->AttachToComponent(item.ParentActor, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

#if WITH_EDITORONLY_DATA
	if (IsEditMode)
	{
		for (auto item : DeserializingActorCollection)
		{
			ULGUIEditorManagerObject::RemoveActorForPrefabSystem(item);
		}
		ULGUIEditorManagerObject::EndPrefabSystemProcessingActor();
	}
	else
#endif
	{
		for (auto item : DeserializingActorCollection)
		{
			ALGUIManagerActor::RemoveActorForPrefabSystem(item);
		}
		ALGUIManagerActor::EndPrefabSystemProcessingActor(TargetWorld.Get());
	}
	//UE_LOG(LGUI, Display, TEXT("Dserialize Prefab Duration:%s"), *((FDateTime::Now() - StartTime).ToString()));
	return CreatedActor;
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
				auto id = FindClassIdFromList((UClass*)object);
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
					auto id = FindClassIdFromList(object->GetClass());
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
					if (auto id = MapActorToID.Find(actor))
					{
						ItemPropertyData.Data = BitConverter::GetBytes(*id);
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
			auto id = FindStringIdFromList(stringValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_String;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto nameProperty = CastField<FNameProperty>(Property))
		{
			auto nameValue = nameProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindNameIdFromList(nameValue);
			ItemPropertyData.Data = BitConverter::GetBytes(id);
			ItemPropertyData.PropertyType = ELGUIPropertyType::PT_Name;
			PropertyData.Add(ItemPropertyData);
		}
		else if (auto textProperty = CastField<FTextProperty>(Property))
		{
			auto textValue = textProperty->GetPropertyValue_InContainer(Dest, cppArrayIndex);
			auto id = FindTextIdFromList(textValue);
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
				ExcludeProperties.RemoveAt(index);
				excludePropertyCount--;
				continue;
			}
		}
		SaveCommonProperty(propertyItem, ItemType_Normal, (uint8*)Target, PropertyData);
	}
}
//serialize single Actor
FLGUIActorSaveData ActorSerializer::SerializeSingleActor(AActor* Actor)
{
	FLGUIActorSaveData ActorRecord;
	ActorRecord.ActorClass = FindClassIdFromList(Actor->GetClass());

	SaveProperty(Actor, ActorRecord.ActorPropertyData, GetActorExcludeProperties());
	auto& Components = Actor->GetComponents();

	TArray<USceneComponent*> AllSceneComponentOfThisActor;
	auto RootComp = Actor->GetRootComponent();
	FLGUIComponentSaveData RootCompData;
	if (RootComp)
	{
		RootCompData.ComponentClass = FindClassIdFromList(RootComp->GetClass());
		RootCompData.ComponentName = RootComp->GetFName();
		SaveProperty(RootComp, RootCompData.PropertyData, GetComponentExcludeProperties());
		AllSceneComponentOfThisActor.Add(RootComp);
	}
	ActorRecord.ComponentPropertyData.Add(RootCompData);//add a data seat for RootComponent, so when deserialize we dont need to check again

	for (auto Comp : Components)
	{
		if (Comp == RootComp)continue;//skip RootComponent
		if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		if (Comp->GetClass()->IsChildOf(ULGUIPrefabHelperComponent::StaticClass()))continue;//skip LGUIPrefabHelperComponent, this component is only for editor helper
																							//if (Comp->IsInBlueprint())continue;//skip Blueprint component
		FLGUIComponentSaveData CompData;
		CompData.ComponentClass = FindClassIdFromList(Comp->GetClass());
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
		ActorRecord.ComponentPropertyData.Add(CompData);
	}
	for (int i = 0, count = ActorRecord.ComponentPropertyData.Num(); i < count; i++)
	{
		auto& componentPropertyData = ActorRecord.ComponentPropertyData[i];
		if (componentPropertyData.SceneComponentParent)
		{
			int32 parentCompIndex;
			if (AllSceneComponentOfThisActor.Find(componentPropertyData.SceneComponentParent, parentCompIndex))
			{
				ActorRecord.ComponentPropertyData[i].SceneComponentParentID = parentCompIndex;
			}
		}
	}

	return ActorRecord;
}
void ActorSerializer::GenerateActorIDRecursive(AActor* Actor, int32& id)
{
	id += 1;
	MapActorToID.Add(Actor, id);

	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		GenerateActorIDRecursive(ChildActor, id);
	}
}
void ActorSerializer::SerializeActorRecursive(AActor* Actor, FLGUIActorSaveData& ActorSaveData)
{
	ActorSaveData = SerializeSingleActor(Actor);
	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	TArray<FLGUIActorSaveData> ChildSaveDataList;
	for (auto ChildActor : ChildrenActors)
	{
		FLGUIActorSaveData ChildActorSaveData;
		SerializeActorRecursive(ChildActor, ChildActorSaveData);
		ChildSaveDataList.Add(ChildActorSaveData);
	}
	ActorSaveData.ChildActorData = ChildSaveDataList;
}
void ActorSerializer::SavePrefab(AActor* RootActor, ULGUIPrefab* InPrefab)
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
	serializer.SerializeActor(RootActor, InPrefab);
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

	int32 id = 0;
	GenerateActorIDRecursive(RootActor, id);

	FLGUIActorSaveData ActorSaveData;
	SerializeActorRecursive(RootActor, ActorSaveData);

	FLGUIPrefabSaveData SaveData;
	SaveData.SavedActor = ActorSaveData;

#if WITH_EDITORONLY_DATA
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

	Prefab->DataCount = Prefab->BinaryData.Num();
#endif
	Prefab->EngineMajorVersion = ENGINE_MAJOR_VERSION;
	Prefab->EngineMinorVersion = ENGINE_MINOR_VERSION;
	Prefab->PrefabVersion = LGUI_PREFAB_VERSION;

	//ConvertForBuildData(InPrefab);

	Prefab->MarkPackageDirty();
	UE_LOG(LGUI, Log, TEXT("SerializeActor, prefab:%s duration:%s"), *(Prefab->GetPathName()), *((FDateTime::Now() - StartTime).ToString()));
}
#if WITH_EDITOR
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
#endif

TArray<FName> ActorSerializer::GetActorExcludeProperties()
{
	return {
		"Instigator",
		"RootComponent",//this will result in the copied actor have same RootComponent to original actor, and crash. so we need to skip it
		"BlueprintCreatedComponents",
		"InstanceComponents",
#if WITH_EDITORONLY_DATA
		"FolderPath",
#endif
	};
}
TArray<FName> ActorSerializer::GetComponentExcludeProperties()
{
	return {
		"AttachParent",
	};
}


int32 ActorSerializer::FindAssetIdFromList(UObject* AssetObject)
{
	if (!AssetObject)return -1;
	auto& ReferenceAssetList = Prefab->ReferenceAssetList;
	int32 resultIndex;
	if (ReferenceAssetList.Find(AssetObject, resultIndex))
	{
		return resultIndex;//return index if found
	}
	else//add to list if not found
	{
		ReferenceAssetList.Add(AssetObject);
		return ReferenceAssetList.Num() - 1;
	}
}
int32 ActorSerializer::FindStringIdFromList(FString string)//put string into list to avoid same string sotred multiple times
{
	auto& ReferenceStringList = Prefab->ReferenceStringList;
	int32 resultIndex;
	if (ReferenceStringList.Find(string, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceStringList.Add(string);
		return ReferenceStringList.Num() - 1;
	}
}
int32 ActorSerializer::FindNameIdFromList(FName name)
{
	auto& ReferenceNameList = Prefab->ReferenceNameList;
	int32 resultIndex;
	if (ReferenceNameList.Find(name, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceNameList.Add(name);
		return ReferenceNameList.Num() - 1;
	}
}
int32 ActorSerializer::FindTextIdFromList(FText text)
{
	auto& ReferenceTextList = Prefab->ReferenceTextList;
	int32 Count = ReferenceTextList.Num();
	for (int32 i = 0; i < Count; i++)
	{
		if (ReferenceTextList[i].CompareTo(text, ETextComparisonLevel::Default) == 0)
		{
			return i;
		}
	}

	ReferenceTextList.Add(text);
	return ReferenceTextList.Num() - 1;
}
int32 ActorSerializer::FindClassIdFromList(UClass* uclass)
{
	if (!uclass)return -1;
	auto& ReferenceClassList = Prefab->ReferenceClassList;
	int32 resultIndex;
	if (ReferenceClassList.Find(uclass, resultIndex))
	{
		return resultIndex;
	}
	else
	{
		ReferenceClassList.Add(uclass);
		return ReferenceClassList.Num() - 1;
	}
}

UObject* ActorSerializer::FindAssetFromListByIndex(int32 Id)
{
	auto& ReferenceAssetList = Prefab->ReferenceAssetList;
	int32 count = ReferenceAssetList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceAssetList[Id];
}
FString ActorSerializer::FindStringFromListByIndex(int32 Id)
{
	auto& ReferenceStringList = Prefab->ReferenceStringList;
	int32 count = ReferenceStringList.Num();
	if (Id >= count || Id < 0)
	{
		return FString();
	}
	return ReferenceStringList[Id];
}
FName ActorSerializer::FindNameFromListByIndex(int32 Id)
{
	auto& ReferenceNameList = Prefab->ReferenceNameList;
	int32 count = ReferenceNameList.Num();
	if (Id >= count || Id < 0)
	{
		return FName();
	}
	return ReferenceNameList[Id];
}
FText ActorSerializer::FindTextFromListByIndex(int32 Id)
{
	auto& ReferenceTextList = Prefab->ReferenceTextList;
	int32 count = ReferenceTextList.Num();
	if (Id >= count || Id < 0)
	{
		return FText();
	}
	return ReferenceTextList[Id];
}
UClass* ActorSerializer::FindClassFromListByIndex(int32 Id)
{
	auto& ReferenceClassList = Prefab->ReferenceClassList;
	int32 count = ReferenceClassList.Num();
	if (Id >= count || Id < 0)
	{
		return nullptr;
	}
	return ReferenceClassList[Id];
}
void ActorSerializer::RegisterComponent(AActor* Actor, UActorComponent* Comp)
{
	switch (Comp->CreationMethod)
	{
	default:
	case EComponentCreationMethod::SimpleConstructionScript:
	case EComponentCreationMethod::UserConstructionScript:
	{
		Actor->FinishAndRegisterComponent(Comp);
	}
	break;
	case EComponentCreationMethod::Instance:
	{
		Comp->RegisterComponent();
		Actor->AddInstanceComponent(Comp);
	}
	break;
	//@todo: should also consider EComponentCreationMethod::Native
	}
}
