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

AActor* ActorSerializer::DeserializeActorRecursiveForUseInRuntime(USceneComponent* Parent, const FLGUIActorSaveDataForBuild& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForUseInRuntime]Find class: '%s' at index: %d, but is not a Actor class, use default"), *(ActorClass->GetFName().ToString()), SaveData.ActorClass);
			ActorClass = AActor::StaticClass();
		}

		auto NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
		if (LoadedRootActor == nullptr)
		{
			LoadedRootActor = NewActor;
			LGUIManagerActor->BeginPrefabSystemProcessingActor(LoadedRootActor);
		}
		LGUIManagerActor->AddActorForPrefabSystem(NewActor, LoadedRootActor, 0);
		CreatedActors.Add(NewActor);
		LoadPropertyForRuntime(NewActor, SaveData.ActorPropertyData, GetActorExcludeProperties(true, true));

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		auto RootComp = NewActor->GetRootComponent();
		if (RootComp)//if this actor have default root component
		{
			//actor's default root component dont need mannually register
			LoadPropertyForRuntime(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
		}
		else
		{
			if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
			{
				if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass, Prefab.Get()))
				{
					if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
					{
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForUseInRuntime]Find class: '%s' at index: %d, but is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()), RootCompSaveData.ComponentClass);
						CompClass = USceneComponent::StaticClass();
					}
					RootComp = NewObject<USceneComponent>(NewActor, CompClass, RootCompSaveData.ComponentName, RF_Transactional);
					NewActor->SetRootComponent(RootComp);
					LoadPropertyForRuntime(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
					if (!RootComp->IsDefaultSubobject())
					{
						RegisterComponent(NewActor, RootComp);
					}
				}
			}
		}
		TArray<USceneComponent*> ThisActorSceneComponents;//all SceneComponent of this actor, when a SceneComponent need to attach to a parent, search from this list
		if (RootComp)
		{
			if (auto PrimitiveComp = Cast<UPrimitiveComponent>(RootComp))
			{
				PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
			}
			RootComp->RecreatePhysicsState();
			RootComp->MarkRenderStateDirty();
			ThisActorSceneComponents.Add(RootComp);
		}

		TArray<SceneComponentToParentIDStruct> NeedReparent_SceneComponents;//SceneComponent collection of this actor that need to reattach to parent
		int ComponentCount = SaveData.ComponentPropertyData.Num();
		for (int i = 1; i < ComponentCount; i++)//start from 1, skip RootComponent
		{
			auto CompData = SaveData.ComponentPropertyData[i];
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass, Prefab.Get()))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = UActorComponent::StaticClass();
					UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForUseInRuntime]Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				auto Comp = NewObject<UActorComponent>(NewActor, CompClass, CompData.ComponentName, RF_Transactional);
				LoadPropertyForRuntime(Comp, CompData.PropertyData, GetComponentExcludeProperties());
				if (!Comp->IsDefaultSubobject())
				{
					RegisterComponent(NewActor, Comp);
				}

				if (auto PrimitiveComp = Cast<UPrimitiveComponent>(Comp))
				{
					PrimitiveComp->BodyInstance.FixupData(PrimitiveComp);
				}
				if (auto SceneComp = Cast<USceneComponent>(Comp))
				{
					SceneComp->RecreatePhysicsState();
					SceneComp->MarkRenderStateDirty();

					SceneComponentToParentIDStruct ParentNameStruct;
					ParentNameStruct.Comp = SceneComp;
					ParentNameStruct.ParentCompID = CompData.SceneComponentParentID;
					NeedReparent_SceneComponents.Add(ParentNameStruct);
					ThisActorSceneComponents.Add(SceneComp);
				}
			}
			else
			{
				auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer::DeserializeActorRecursiveForUseInRuntime]Error prefab:%s. \nComponent Class of index:%d not found!"), *(Prefab->GetPathName()), (CompData.ComponentClass));
				UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg); 
#if WITH_EDITOR
				LGUIUtils::EditorNotification(FText::FromString(ErrorMsg)); 
#endif
			}
		}
		//SceneComponent reattach to parent
		for (int i = 0, count = NeedReparent_SceneComponents.Num(); i < count; i++)
		{
			auto& SceneCompStructData = NeedReparent_SceneComponents[i];
			if (SceneCompStructData.ParentCompID != -1)
			{
				SceneCompStructData.Comp->AttachToComponent(ThisActorSceneComponents[SceneCompStructData.ParentCompID], FAttachmentTransformRules::KeepRelativeTransform);
			}
		}
		//set parent
		if (Parent)
		{
			if (NewActor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || Parent->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))//for blueprint actor, we need to attach parent after actor finish spawn
			{
				BlueprintActorToParentActorStruct itemStruct;
				itemStruct.BlueprintActor = NewActor;
				itemStruct.ParentActor = Parent;
				BlueprintAndParentArray.Add(itemStruct);
			}
			if (RootComp)
			{
				RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		id++;
		MapIDToActor.Add(id, NewActor);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursiveForUseInRuntime(RootComp, ChildSaveData, id);
		}
		return NewActor;
	}
	else
	{
		auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer::DeserializeActorRecursiveForUseInRuntime]Error prefab:%s. \nActor Class of index:%d not found!"), *(Prefab->GetPathName()), (SaveData.ActorClass));
		UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);
#if WITH_EDITOR
		LGUIUtils::EditorNotification(FText::FromString(ErrorMsg));
#endif
		return nullptr;
	}
}

void ActorSerializer::LoadPropertyForRuntime(UObject* Target, const TArray<FLGUIPropertyDataForBuild>& PropertyData, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
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
		if (LoadCommonPropertyForRuntime(propertyItem, ItemType_Normal, propertyIndex, (uint8*)Target, PropertyData))
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

bool ActorSerializer::LoadCommonPropertyForRuntime(FProperty* Property, int itemType, int itemPropertyIndex, uint8* Dest, const TArray<FLGUIPropertyDataForBuild>& PropertyData, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return false;//skip property with these flags
	if (Property->IsEditorOnlyProperty())return false;//Build data dont have EditorOnly property
	FLGUIPropertyDataForBuild ItemPropertyData;
	bool HaveData = false;

	switch (itemType)
	{
	case ItemType_Normal:
	{
		if (itemPropertyIndex < PropertyData.Num())
		{
			ItemPropertyData = PropertyData[itemPropertyIndex];
			if(ItemPropertyData.PropertyType != ELGUIPropertyType::PT_None)
			{
				HaveData = true;
			}
			else
			{
				return true;
			}
		}
		else
		{
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
			if(ItemPropertyData.PropertyType != ELGUIPropertyType::PT_None)
			{
				HaveData = true;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
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
				LoadCommonPropertyForRuntime(Property, ItemType_Array, i, Dest, ItemPropertyData.ContainerData, i, true);
			}
			return true;
		}
		else
		{
			if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
			{
				int index = -2;
				if (ItemPropertyData.Data.Num() == 4)
				{
					index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
				}
				if (index <= -1)return true;
				if (auto asset = FindClassFromListByIndex(index, Prefab.Get()))
				{
					auto classProperty = (FObjectPropertyBase*)Property;
					classProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
				}
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
				if (index <= -1)
				{
					if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
					{
						objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
					}
					return true;
				}
				if (objProperty->PropertyClass->IsChildOf(AActor::StaticClass()))//is Actor
				{
					UPropertyMapStruct mapStruct;//store these data, after all actor created, reassign actor reference
					mapStruct.id = index;
					mapStruct.Dest = Dest;
					mapStruct.ObjProperty = objProperty;
					mapStruct.cppArrayIndex = cppArrayIndex;
					ObjectMapStructList.Add(mapStruct);
					return true;
				}
				else
				{
					if (Property->HasAnyPropertyFlags(CPF_InstancedReference))//is instanced object
					{
						UObject* newObj = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex);
						if (newObj == nullptr)
						{
							if (auto newObjClass = FindClassFromListByIndex(index, Prefab.Get()))
							{
								newObj = NewObject<UObject>(Outter, newObjClass);
							}
						}
						if (newObj != nullptr)
						{
							LoadPropertyForRuntime(newObj, ItemPropertyData.ContainerData, {});
							objProperty->SetObjectPropertyValue_InContainer(Dest, newObj, cppArrayIndex);
						}
					}
					else if (auto asset = FindAssetFromListByIndex(index, Prefab.Get()))//is asset
					{
						objProperty->SetObjectPropertyValue_InContainer(Dest, asset, cppArrayIndex);
					}
					else
					{
						if (!objProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))//if this property is ActorComponent, if clear the value then it will become ineditable
						{
							objProperty->ClearValue_InContainer(Dest, cppArrayIndex);
						}
					}
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
				FScriptArrayHelper ArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				int ArrayCount = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//array count
				LogForBitConvertFail(bitConvertSuccess, Property);
				ArrayHelper.Resize(ArrayCount);
				for (int i = 0; i < ArrayCount; i++)
				{
					LoadCommonPropertyForRuntime(arrProperty->Inner, ItemType_Array, i, ArrayHelper.GetRawPtr(i), ItemPropertyData.ContainerData);
				}
				return true;
			}
			else if (auto mapProperty = CastField<FMapProperty>(Property))
			{
				FScriptMapHelper MapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess) * 2;//Map element's data is stored as key,value,key,value...
				LogForBitConvertFail(bitConvertSuccess, Property);
				int mapIndex = 0;
				for (int i = 0; i < count; i++)
				{
					MapHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonPropertyForRuntime(mapProperty->KeyProp, ItemType_Map, i, MapHelper.GetKeyPtr(mapIndex), ItemPropertyData.ContainerData);//key
					i++;
					LoadCommonPropertyForRuntime(mapProperty->ValueProp, ItemType_Map, i, MapHelper.GetPairPtr(mapIndex), ItemPropertyData.ContainerData);//value. PairPtr of map is the real ptr of value
					mapIndex++;
				}
				MapHelper.Rehash();
				return true;
			}
			else if (auto setProperty = CastField<FSetProperty>(Property))
			{
				FScriptSetHelper SetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
				auto count = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);//Set count
				LogForBitConvertFail(bitConvertSuccess, Property);
				for (int i = 0; i < count; i++)
				{
					SetHelper.AddDefaultValue_Invalid_NeedsRehash();
					LoadCommonPropertyForRuntime(setProperty->ElementProp, ItemType_Set, i, SetHelper.GetElementPtr(i), ItemPropertyData.ContainerData);
				}
				SetHelper.Rehash();
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
					Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
				}
				else
				{
					auto structPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
					int propertyIndex = 0;
					for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
					{
						if (LoadCommonPropertyForRuntime(*It, ItemType_Normal, propertyIndex, structPtr, ItemPropertyData.ContainerData))
						{
							propertyIndex++;
						}
					}
				}
				return true;
			}

			else if (auto strProperty = CastField<FStrProperty>(Property))//string store as reference
			{
				if (ItemPropertyData.Data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
					auto stringValue = FindStringFromListByIndex(index, Prefab.Get());
					strProperty->SetPropertyValue_InContainer(Dest, stringValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto nameProperty = CastField<FNameProperty>(Property))
			{
				if (ItemPropertyData.Data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
					auto nameValue = FindNameFromListByIndex(index, Prefab.Get());
					nameProperty->SetPropertyValue_InContainer(Dest, nameValue, cppArrayIndex);
				}
				return true;
			}
			else if (auto textProperty = CastField<FTextProperty>(Property))
			{
				if (ItemPropertyData.Data.Num() == 4)
				{
					auto index = BitConverter::ToInt32(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
					auto textValue = FindTextFromListByIndex(index, Prefab.Get());
					textProperty->SetPropertyValue_InContainer(Dest, textValue, cppArrayIndex);
				}
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
				if (auto boolProperty = CastField<FBoolProperty>(Property))
				{
					auto value = BitConverter::ToBoolean(ItemPropertyData.Data, bitConvertSuccess);
					LogForBitConvertFail(bitConvertSuccess, Property);
					boolProperty->SetPropertyValue_InContainer((void*)Dest, value, cppArrayIndex);
				}
				else
				{
					if (Property->GetSize() != ItemPropertyData.Data.Num())
					{
						auto ErrorMsg = FString::Printf(TEXT("[ActorSerializer/LoadCommonPropertyForBuild]Error prefab:%s. \n	Load value of Property:%s (type:%s), but size not match, PropertySize:%d, dataSize:%d.")
							, *(Prefab->GetPathName()), *(Property->GetName()), *(Property->GetClass()->GetName()), Property->GetSize(), ItemPropertyData.Data.Num());
						UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg);
#if WITH_EDITOR
						LGUIUtils::EditorNotification(FText::FromString(ErrorMsg));
#endif
						return false;
					}
					Property->CopyCompleteValue(Property->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex), ItemPropertyData.Data.GetData());
				}
				return true;
			}
		}
		return false;
	}
	return false;
}

#endif
