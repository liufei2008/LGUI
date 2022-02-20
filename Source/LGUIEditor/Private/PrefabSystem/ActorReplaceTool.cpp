// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "ActorReplaceTool.h"
#include "Engine/World.h"
#include "LGUI.h"
#include "LGUIEditorModule.h"
#include "Utils/LGUIUtils.h"
#include "EngineUtils.h"
#include "PrefabSystem/2/ActorSerializer.h"

using namespace LGUIPrefabSystem;
bool ActorReplaceTool::CopyCommonProperty(FProperty* Property, uint8* Src, uint8* Dest, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return true;//skip property with special flag
	if (Property->ArrayDim > 1 && isInsideCppArray == false)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			if (!CopyCommonProperty(Property, Src, Dest, i, true))
				break;
		}
	}
	else
	{
		if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
		{
			Property->CopyCompleteValue_InContainer(Dest, Src);
			return false;
		}
		else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
		{
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Src, cppArrayIndex))
			{
				if (object->IsAsset() || object->IsA(UClass::StaticClass()))
				{
					objProperty->SetObjectPropertyValue_InContainer(Dest, object, cppArrayIndex);
				}
				else if (Property->HasAnyPropertyFlags(CPF_InstancedReference))
				{
					UObject* targetObject = objProperty->GetObjectPropertyValue_InContainer(Dest, cppArrayIndex);
					if (targetObject == nullptr)
					{
						targetObject = NewObject<UObject>(Outter, object->GetClass());
					}
					if (object->GetClass()->IsChildOf(USceneComponent::StaticClass()))
					{
						CopyProperty(object, targetObject, ActorSerializer::GetComponentExcludeProperties());
					}
					else
					{
						CopyProperty(object, targetObject, {});
					}
					objProperty->SetObjectPropertyValue_InContainer(Dest, targetObject, cppArrayIndex);
				}
				else if (auto actor = Cast<AActor>(object))//Actor reference, need to remap
				{
					if (actor == OriginActor)//reference origin actor
					{
						objProperty->SetObjectPropertyValue_InContainer(Dest, CopiedActor, cppArrayIndex);
					}
					else
					{
						objProperty->CopyCompleteValue_InContainer(Dest, Src);
					}
				}
			}
		}

		else if (auto arrProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper OriginArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			FScriptArrayHelper CopiedArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto arrayCount = OriginArrayHelper.Num();
			CopiedArrayHelper.Resize(arrayCount);
			for (int i = 0; i < arrayCount; i++)
			{
				CopyCommonProperty(arrProperty->Inner, OriginArrayHelper.GetRawPtr(i), CopiedArrayHelper.GetRawPtr(i));
			}
		}
		else if (auto mapProperty = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper OriginMapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			FScriptMapHelper CopiedMapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = OriginMapHelper.Num();
			for (int i = 0; i < count; i++)
			{
				CopiedMapHelper.AddDefaultValue_Invalid_NeedsRehash();
				CopyCommonProperty(mapProperty->KeyProp, OriginMapHelper.GetKeyPtr(i), CopiedMapHelper.GetKeyPtr(i));//key
				CopyCommonProperty(mapProperty->ValueProp, OriginMapHelper.GetPairPtr(i), CopiedMapHelper.GetPairPtr(i));//value
			}
			CopiedMapHelper.Rehash();
		}
		else if (auto setProperty = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper OriginSetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			FScriptSetHelper CopiedSetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Dest, cppArrayIndex));
			auto count = OriginSetHelper.Num();
			for (int i = 0; i < count; i++)
			{
				CopiedSetHelper.AddDefaultValue_Invalid_NeedsRehash();
				CopyCommonProperty(setProperty->ElementProp, OriginSetHelper.GetElementPtr(i), CopiedSetHelper.GetElementPtr(i));
			}
			CopiedSetHelper.Rehash();
		}
		else if (auto structProperty = CastField<FStructProperty>(Property))
		{
			auto OriginPtr = Property->ContainerPtrToValuePtr<uint8>(Src, cppArrayIndex);
			auto CopiedPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
			for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
			{
				CopyCommonProperty(*It, OriginPtr, CopiedPtr);
			}
		}

		else
		{
			Property->CopyCompleteValue_InContainer(Dest, Src);
			return false;
		}
	}
	return true;
}
void ActorReplaceTool::CopyProperty(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : propertyField)
	{
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
		CopyCommonProperty(propertyItem, (uint8*)Origin, (uint8*)Target);
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
void ActorReplaceTool::CopyPropertyChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	//collect origin object's property value
	TMap<FName, FProperty*> srcPropertyMap;
	auto srcPropertyField = TFieldRange<FProperty>(Origin->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : srcPropertyField)
	{
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
		srcPropertyMap.Add(propertyName, propertyItem);
	}
	//copy to target property
	auto targetPropertyField = TFieldRange<FProperty>(Target->GetClass());
	for (const auto targetProperty : targetPropertyField)
	{
		auto propertyName = targetProperty->GetFName();
		if (auto srcPropertyPtr = srcPropertyMap.Find(propertyName))
		{
			auto srcProperty = *srcPropertyPtr;
			if (srcProperty->StaticClass() == targetProperty->StaticClass())
			{
				srcPropertyMap.Remove(propertyName);
				CopyCommonProperty(targetProperty, (uint8*)Origin, (uint8*)Target);//because we have compared the property's name, so this is correct too
			}
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

void ActorReplaceTool::CopyPropertyForActorChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	//collect origin object's property value
	TMap<FName, FProperty*> srcPropertyMap;
	auto srcPropertyField = TFieldRange<FProperty>(Origin->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : srcPropertyField)
	{
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
		srcPropertyMap.Add(propertyName, propertyItem);
	}
	//copy to target property
	auto targetPropertyField = TFieldRange<FProperty>(Target->GetClass());
	for (const auto targetProperty : targetPropertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(targetProperty))
		{
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Origin))
			{
				if (object->GetClass()->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
				{
					continue;
				}
			}
		}

		auto propertyName = targetProperty->GetFName();
		if (auto srcPropertyPtr = srcPropertyMap.Find(propertyName))
		{
			auto srcProperty = *srcPropertyPtr;
			if (srcProperty->StaticClass() == targetProperty->StaticClass())
			{
				srcPropertyMap.Remove(propertyName);
				CopyCommonProperty(targetProperty, (uint8*)Origin, (uint8*)Target);//because we have compared the property's name, so this is correct too
			}
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






AActor* ActorReplaceTool::ReplaceActorClass(AActor* TargetActor, TSubclassOf<AActor> NewActorClass)
{
	ActorReplaceTool copier;
	
	TArray<AActor*> childrenList;
	TargetActor->GetAttachedActors(childrenList);
	auto OrignParent = TargetActor->GetRootComponent()->GetAttachParent();
	auto resultActor = copier.ReplaceActorClassInternal(TargetActor, NewActorClass);
	for (auto child : childrenList)
	{
		child->AttachToActor(resultActor, FAttachmentTransformRules::KeepRelativeTransform);
	}
	resultActor->AttachToComponent(OrignParent, FAttachmentTransformRules::KeepRelativeTransform);
	LGUIUtils::DestroyActorWithHierarchy(TargetActor);

	return resultActor;
}
AActor* ActorReplaceTool::ReplaceActorClassInternal(AActor* TargetActor, TSubclassOf<AActor> NewActorClass)
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LGUIEditor, Error, TEXT("CopyActor, RootActor is not valid!"));
		return nullptr;
	}

	OriginActor = TargetActor;
	auto Result = CopySingleActorAndReplaceClass(TargetActor, NewActorClass);
	Result->FinishSpawning(FTransform::Identity, true);

	auto ActorExcludeProperties = ActorSerializer::GetActorExcludeProperties(false, true);
	auto SceneComponentExcludeProperties = ActorSerializer::GetComponentExcludeProperties();
	//iterate all actors in world and replace actor referece
	for (TActorIterator<AActor> ActorItr(TargetActor->GetWorld()); ActorItr; ++ActorItr)
	{
		auto itemActor = *ActorItr;
		CheckPropertyForActor(itemActor, ActorExcludeProperties);
		const auto& Components = itemActor->GetComponents();
		for (auto Comp : Components)
		{
			if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
			CheckProperty(Comp, SceneComponentExcludeProperties);
		}
	}

	return Result;
}
AActor* ActorReplaceTool::CopySingleActorAndReplaceClass(AActor* TargetActor, TSubclassOf<AActor> NewActorClass)
{
	TArray<FName>EmptyExcludeProperties;

	auto ActorExcludeProperties = ActorSerializer::GetActorExcludeProperties(false, true);
	auto SceneComponentExcludeProperties = ActorSerializer::GetComponentExcludeProperties();

	CopiedActor = TargetActor->GetWorld()->SpawnActorDeferred<AActor>(NewActorClass, FTransform::Identity);
	CopyPropertyForActorChecked(TargetActor, CopiedActor, ActorExcludeProperties);
	const auto& OriginComponents = TargetActor->GetComponents();

	auto OriginRootComp = TargetActor->GetRootComponent();
	if (!OriginRootComp)//not have root component
	{
		return CopiedActor;
	}
	auto CopiedRootComp = CopiedActor->GetRootComponent();
	bool rootCompNeedRegister = false;
	if (!CopiedRootComp)
	{
		CopiedRootComp = NewObject<USceneComponent>(CopiedActor, OriginRootComp->GetClass(), OriginRootComp->GetFName(), OriginRootComp->GetFlags());
		rootCompNeedRegister = true;
	}
	CopyPropertyChecked(OriginRootComp, CopiedRootComp, SceneComponentExcludeProperties);
	if (rootCompNeedRegister)
	{
		LGUIPrefabSystem::ActorSerializer::RegisterComponent(CopiedActor, CopiedRootComp);
		CopiedActor->SetRootComponent(CopiedRootComp);
	}

	TArray<USceneComponent*> ThisActorSceneComponents;//this Actor's SceneComponent array. when child SceneComponent need to attach to parent, search from this array
	ThisActorSceneComponents.Add(CopiedRootComp);
	TArray<SceneComponentToParentNameStruct> NeedReparentSceneComponents;//need to reasign parent SceneComponent

	for (auto OriginComp : OriginComponents)
	{
		if (OriginComp == OriginRootComp)continue;//skip RootComponent
		if (OriginComp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		auto CopiedComp = NewObject<UActorComponent>(CopiedActor, OriginComp->GetClass(), OriginComp->GetFName(), OriginComp->GetFlags());
		CopyProperty(OriginComp, CopiedComp, SceneComponentExcludeProperties);
		LGUIPrefabSystem::ActorSerializer::RegisterComponent(CopiedActor, CopiedComp);

		if (auto SceneComp = Cast<USceneComponent>(CopiedComp))
		{
			SceneComponentToParentNameStruct ParentNameStruct;
			ParentNameStruct.Comp = SceneComp;
			ParentNameStruct.ParentName = ((USceneComponent*)OriginComp)->GetAttachParent()->GetFName();
			NeedReparentSceneComponents.Add(ParentNameStruct);
			ThisActorSceneComponents.Add(SceneComp);
			SceneComp->RecreatePhysicsState();
		}
	}
	//assign child SceneComponent's parent
	for (auto ParentNameStructData : NeedReparentSceneComponents)
	{
		auto parentName = ParentNameStructData.ParentName;
		int count = ThisActorSceneComponents.Num();
		for (int i = 0; i < count; i++)
		{
			auto ItemComponent = ThisActorSceneComponents[i];
			if (parentName == ItemComponent->GetFName())
			{
				ParentNameStructData.Comp->AttachToComponent(ItemComponent, FAttachmentTransformRules::KeepRelativeTransform);
				i = count;//break loop
			}
		}
	}
	return CopiedActor;
}



void ActorReplaceTool::CheckPropertyForActor(UObject* Origin, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<FProperty>(Origin->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = CastField<FObjectPropertyBase>(propertyItem))
		{
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Origin))
			{
				if (object->GetClass()->IsChildOf(UActorComponent::StaticClass()))//ignore ActorComponent, just need to handle normal properties
				{
					continue;
				}
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
		CheckCommonProperty(propertyItem, (uint8*)Origin);
	}
}
void ActorReplaceTool::CheckProperty(UObject* Origin, TArray<FName> ExcludeProperties)
{
	auto propertyField = TFieldRange<FProperty>(Origin->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : propertyField)
	{
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
		CheckCommonProperty(propertyItem, (uint8*)Origin);
	}
}
bool ActorReplaceTool::CheckCommonProperty(FProperty* Property, uint8* Src, int cppArrayIndex, bool isInsideCppArray)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return true;//skip property with special flag
	if (Property->ArrayDim > 1 && isInsideCppArray == false)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			if (!CheckCommonProperty(Property, Src, i, true))
				break;
		}
	}
	else
	{
		if (CastField<FClassProperty>(Property) != nullptr || CastField<FSoftClassProperty>(Property) != nullptr)
		{
			return false;
		}
		else if (auto objProperty = CastField<FObjectPropertyBase>(Property))
		{
			if (auto object = objProperty->GetObjectPropertyValue_InContainer(Src, cppArrayIndex))
			{
				if (object->IsAsset() || object->IsA(UClass::StaticClass()))
				{
					
				}
				else if (Property->HasAnyPropertyFlags(CPF_InstancedReference))
				{
					if (object->GetClass()->IsChildOf(USceneComponent::StaticClass()))
					{
						CheckProperty(object, ActorSerializer::GetComponentExcludeProperties());
					}
					else
					{
						CheckProperty(object, {});
					}
				}
				else if (auto actor = Cast<AActor>(object))//Actor reference, need to remap
				{
					if (actor == OriginActor)
					{
						objProperty->SetObjectPropertyValue_InContainer(Src, CopiedActor, cppArrayIndex);
					}
				}
			}
		}

		else if (auto arrProperty = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper OriginArrayHelper(arrProperty, arrProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			auto arrayCount = OriginArrayHelper.Num();
			for (int i = 0; i < arrayCount; i++)
			{
				CheckCommonProperty(arrProperty->Inner, OriginArrayHelper.GetRawPtr(i));
			}
		}
		else if (auto mapProperty = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper OriginMapHelper(mapProperty, mapProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			auto count = OriginMapHelper.Num();
			for (int i = 0; i < count; i++)
			{
				CheckCommonProperty(mapProperty->KeyProp, OriginMapHelper.GetKeyPtr(i));//key
				CheckCommonProperty(mapProperty->ValueProp, OriginMapHelper.GetPairPtr(i));//value
			}
		}
		else if (auto setProperty = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper OriginSetHelper(setProperty, setProperty->ContainerPtrToValuePtr<void>(Src, cppArrayIndex));
			auto count = OriginSetHelper.Num();
			for (int i = 0; i < count; i++)
			{
				CheckCommonProperty(setProperty->ElementProp, OriginSetHelper.GetElementPtr(i));
			}
		}
		else if (auto structProperty = CastField<FStructProperty>(Property))
		{
			auto OriginPtr = Property->ContainerPtrToValuePtr<uint8>(Src, cppArrayIndex);
			for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
			{
				CheckCommonProperty(*It, OriginPtr);
			}
		}

		else
		{
			return false;
		}
	}
	return true;
}