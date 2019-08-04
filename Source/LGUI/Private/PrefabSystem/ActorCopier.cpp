// Copyright 2019 LexLiu. All Rights Reserved.

#include "PrefabSystem/ActorCopier.h"
#include "Engine/World.h"
#include "LGUI.h"

bool ActorCopier::CopyCommonProperty(UProperty* Property, uint8* Src, uint8* Dest, int cppArrayIndex, bool isInsideCppArray)
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
		if (Cast<UClassProperty>(Property) != nullptr || Cast<USoftClassProperty>(Property) != nullptr)
		{
			Property->CopyCompleteValue_InContainer(Dest, Src);
			return false;
		}
		else if (auto objProperty = Cast<UObjectPropertyBase>(Property))
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
					CopyProperty(object, targetObject, {});
					objProperty->SetObjectPropertyValue_InContainer(Dest, targetObject, cppArrayIndex);
				}
				else if (auto actor = Cast<AActor>(object))//Actor reference, need to remap
				{
					if (auto id = MapOriginActorToID.Find(actor))//if reference inside Actor, we need to store it, after all actor is created then assign the referenced actor property
					{
						UPropertyMapStruct TempStruct;
						TempStruct.ObjProperty = Property;
						TempStruct.id = *id;
						TempStruct.Dest = Dest;
						ObjectPropertyMapStructList.Add(TempStruct);
					}
					else//if reference outsize actor, just copy value
					{
						Property->CopyCompleteValue_InContainer(Dest, Src);
						return false;
					}
				}
			}
		}

		else if (auto arrProperty = Cast<UArrayProperty>(Property))
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
		else if (auto mapProperty = Cast<UMapProperty>(Property))
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
		else if (auto setProperty = Cast<USetProperty>(Property))
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
		else if (auto structProperty = Cast<UStructProperty>(Property))
		{
			auto OriginPtr = Property->ContainerPtrToValuePtr<uint8>(Src, cppArrayIndex);
			auto CopiedPtr = Property->ContainerPtrToValuePtr<uint8>(Dest, cppArrayIndex);
			for (TFieldIterator<UProperty> It(structProperty->Struct); It; ++It)
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
void ActorCopier::CopyCommonPropertyForChecked(UProperty* Property, uint8* Src, uint8* Dest)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return;//skip property with special flag
	if (auto arrProperty = Cast<UArrayProperty>(Property))
	{
		FScriptArrayHelper OriginArrayHelper(arrProperty, Src);
		FScriptArrayHelper CopiedArrayHelper(arrProperty, Dest);
		auto arrayCount = OriginArrayHelper.Num();
		CopiedArrayHelper.Resize(arrayCount);
		for (int i = 0; i < arrayCount; i++)
		{
			CopyCommonProperty(arrProperty->Inner, OriginArrayHelper.GetRawPtr(i), CopiedArrayHelper.GetRawPtr(i));
		}
	}
	else if (auto mapProperty = Cast<UMapProperty>(Property))
	{
		FScriptMapHelper OriginMapHelper(mapProperty, Src);
		FScriptMapHelper CopiedMapHelper(mapProperty, Dest);
		auto count = OriginMapHelper.Num();
		for (int i = 0; i < count; i++)
		{
			CopiedMapHelper.AddDefaultValue_Invalid_NeedsRehash();
			CopyCommonProperty(mapProperty->KeyProp, OriginMapHelper.GetKeyPtr(i), CopiedMapHelper.GetKeyPtr(i));//key
			CopyCommonProperty(mapProperty->ValueProp, OriginMapHelper.GetPairPtr(i), CopiedMapHelper.GetPairPtr(i));//value
		}
		CopiedMapHelper.Rehash();
	}
	else if (auto setProperty = Cast<USetProperty>(Property))
	{
		FScriptSetHelper OriginSetHelper(setProperty, Src);
		FScriptSetHelper CopiedSetHelper(setProperty, Dest);
		auto count = OriginSetHelper.Num();
		for (int i = 0; i < count; i++)
		{
			CopiedSetHelper.AddDefaultValue_Invalid_NeedsRehash();
			CopyCommonProperty(setProperty->ElementProp, OriginSetHelper.GetElementPtr(i), CopiedSetHelper.GetElementPtr(i));
		}
		CopiedSetHelper.Rehash();
	}
	else if (auto structProperty = Cast<UStructProperty>(Property))
	{
		for (TFieldIterator<UProperty> It(structProperty->Struct); It; ++It)
		{
			CopyCommonProperty(*It, Src, Dest);
		}
	}

	else
	{
		Property->CopyCompleteValue(Dest, Src);
	}
}
void ActorCopier::CopyProperty(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<UProperty>(Target->GetClass());
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
void ActorCopier::CopyPropertyChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	//collect origin object's property value
	TMap<FName, UProperty*> srcPropertyMap;
	auto srcPropertyField = TFieldRange<UProperty>(Origin->GetClass());
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
	auto targetPropertyField = TFieldRange<UProperty>(Target->GetClass());
	for (const auto targetProperty : targetPropertyField)
	{
		auto propertyName = targetProperty->GetFName();
		if (auto srcPropertyPtr = srcPropertyMap.Find(propertyName))
		{
			auto srcProperty = *srcPropertyPtr;
			if (srcProperty->StaticClass() == targetProperty->StaticClass())
			{
				srcPropertyMap.Remove(propertyName);
				//these two methods can do the same result, but logically the first method is the right one
				//CopyCommonPropertyForChecked(targetProperty, srcProperty->ContainerPtrToValuePtr<uint8>(Origin), targetProperty->ContainerPtrToValuePtr<uint8>(Target));
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

void ActorCopier::CopyPropertyForActor(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
{
	OutterArray.Add(Target);
	Outter = Target;

	auto propertyField = TFieldRange<UProperty>(Target->GetClass());
	int excludePropertyCount = ExcludeProperties.Num();
	for (const auto propertyItem : propertyField)
	{
		if (auto objProperty = Cast<UObjectPropertyBase>(propertyItem))
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

AActor* ActorCopier::CopySingleActor(AActor* OriginActor, USceneComponent* Parent)
{
	TArray<FName>EmptyExcludeProperties;

	auto ActorExcludeProperties = GetActorExcludeProperties();	
	auto SceneComponentExcludeProperties = GetComponentExcludeProperties();

	auto CopiedActor = OriginActor->GetWorld()->SpawnActorDeferred<AActor>(OriginActor->GetClass(), FTransform::Identity);
	CopyPropertyForActor(OriginActor, CopiedActor, ActorExcludeProperties);
	const auto& OriginComponents = OriginActor->GetComponents();

	auto OriginRootComp = OriginActor->GetRootComponent();
	if (!OriginRootComp)//not have root component
	{
		return CopiedActor;
	}
	auto CopiedRootComp = CopiedActor->GetRootComponent();
	bool rootCompNeedRegister = false;
	if (!CopiedRootComp)
	{
		CopiedRootComp = NewObject<USceneComponent>(CopiedActor, OriginRootComp->GetClass(), OriginRootComp->GetFName(), RF_NoFlags);
		rootCompNeedRegister = true;
	}
	CopyProperty(OriginRootComp, CopiedRootComp, SceneComponentExcludeProperties);
	if (rootCompNeedRegister)
	{
		CopiedActor->FinishAndRegisterComponent(CopiedRootComp);
		CopiedActor->SetRootComponent(CopiedRootComp);
	}

	TArray<USceneComponent*> ThisActorSceneComponents;//this Actor's SceneComponent array. when child SceneComponent need to attach to parent, search from this array
	ThisActorSceneComponents.Add(CopiedRootComp);
	TArray<SceneComponentToParentNameStruct> NeedReparentSceneComponents;//need to reasign parent SceneComponent

	for (auto OriginComp : OriginComponents)
	{
		if (OriginComp == OriginRootComp)continue;//skip RootComponent
		if (OriginComp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		auto CopiedComp = NewObject<UActorComponent>(CopiedActor, OriginComp->GetClass(), OriginComp->GetFName(), RF_NoFlags);
		CopyProperty(OriginComp, CopiedComp, SceneComponentExcludeProperties);
		CopiedActor->FinishAndRegisterComponent(CopiedComp);

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
	//set parent
	if (Parent)
	{
		if (Parent->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || CopiedActor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			ActorAndParentStruct tempStruct;
			tempStruct.Actor = CopiedActor;
			tempStruct.Parent = Parent;
			BlueprintActorAndParentArray.Add(tempStruct);
		}
		CopiedRootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	}
	return CopiedActor;
}

AActor* ActorCopier::CopyActorRecursive(AActor* Actor, USceneComponent* Parent, int32& copiedActorId)
{
	auto CopiedActor = CopySingleActor(Actor, Parent);

	copiedActorId += 1;
	MapIDToCopiedActor.Add(copiedActorId, CopiedActor);

	CreatedActors.Add(CopiedActor);
	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		CopyActorRecursive(ChildActor, CopiedActor->GetRootComponent(), copiedActorId);
	}
	return CopiedActor;
}
AActor* ActorCopier::CopyActorInternal(AActor* RootActor, USceneComponent* Parent)
{
	if (!IsValid(RootActor))
	{
		UE_LOG(LGUI, Error, TEXT("CopyActor, RootActor is not valid!"));
		return nullptr;
	}
	
	int32 originActorId = 0, copiedActorId = 0;
	GenerateActorIDRecursive(RootActor, originActorId);
	auto Result = CopyActorRecursive(RootActor, Parent, copiedActorId);

	//reassign actor reference
	for (auto item : ObjectPropertyMapStructList)
	{
		if (auto refObj = MapIDToCopiedActor.Find(item.id))
		{
			auto objProperty = (UObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *refObj);
		}
	}
	//finish Actor Spawn
	for (auto Actor : CreatedActors)
	{
		if (Actor->IsValidLowLevel() && !Actor->IsPendingKill())//check, incase some actor is destoyed when BeginPlay
		{
			Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay execute
		}
	}
	//blueprint actor need to reassign parent after FinishSpawning
	for (auto ActorAndParentData : BlueprintActorAndParentArray)
	{
		if (ActorAndParentData.Actor->IsValidLowLevel() && !ActorAndParentData.Actor->IsPendingKill() && ActorAndParentData.Parent->IsValidLowLevel() && !ActorAndParentData.Parent->IsPendingKill())
		{
			ActorAndParentData.Actor->GetRootComponent()->AttachToComponent(ActorAndParentData.Parent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	return Result;
}
AActor* ActorCopier::DuplicateActor(AActor* RootActor, USceneComponent* Parent)
{
	ActorCopier copier;
	return copier.CopyActorInternal(RootActor, Parent);
}
void ActorCopier::CopyComponentValue(UActorComponent* SrcComp, UActorComponent* TargetComp)
{
	ActorCopier copier;
	return copier.CopyComponentValueInternal(SrcComp, TargetComp);
}
void ActorCopier::CopyComponentValueInternal(UActorComponent* SrcComp, UActorComponent* TargetComp)
{
	auto SceneComponentExcludeProperties = GetComponentExcludeProperties();
	if (SrcComp->GetClass() == TargetComp->GetClass())//copy for same type
	{
		CopyProperty(SrcComp, TargetComp, SceneComponentExcludeProperties);
	}
	else//check and copy
	{
		CopyPropertyChecked(SrcComp, TargetComp, SceneComponentExcludeProperties);
	}
}
void ActorCopier::GenerateActorIDRecursive(AActor* Actor, int32& id)
{
	id += 1;
	MapOriginActorToID.Add(Actor, id);

	TArray<AActor*> ChildrenActors;
	Actor->GetAttachedActors(ChildrenActors);
	for (auto ChildActor : ChildrenActors)
	{
		GenerateActorIDRecursive(ChildActor, id);
	}
}
TArray<FName> ActorCopier::GetActorExcludeProperties()
{
	return {
		//"Instigator",
		"RootComponent",//this will result in the copied actor have same RootComponent to original actor, and crash. so we need to skip it
		"BlueprintCreatedComponents",
		"InstanceComponents",
#if WITH_EDITORONLY_DATA
		"FolderPath",
#endif
	};
}
TArray<FName> ActorCopier::GetComponentExcludeProperties()
{
	return {
		"AttachParent",
	};
}




AActor* ActorCopier::ReplaceActorClass(AActor* TargetActor, TSubclassOf<AActor> NewActorClass)
{
	ActorCopier copier;
	return copier.ReplaceActorClassInternal(TargetActor, NewActorClass);
}
AActor* ActorCopier::ReplaceActorClassInternal(AActor* TargetActor, TSubclassOf<AActor> NewActorClass)
{
	if (!IsValid(TargetActor))
	{
		UE_LOG(LGUI, Error, TEXT("CopyActor, RootActor is not valid!"));
		return nullptr;
	}

	int32 originActorId = 0, copiedActorId = 0;
	GenerateActorIDRecursive(TargetActor, originActorId);
	auto Result = CopySingleActorAndReplaceClass(TargetActor, NewActorClass);

	//reassign actor reference
	for (auto item : ObjectPropertyMapStructList)
	{
		if (auto refObj = MapIDToCopiedActor.Find(item.id))
		{
			auto objProperty = (UObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *refObj);
		}
	}
	//finish Actor Spawn
	for (auto Actor : CreatedActors)
	{
		if (Actor->IsValidLowLevel() && !Actor->IsPendingKill())//check, incase some actor is destoyed when BeginPlay
		{
			Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay execute
		}
	}
	//blueprint actor need to reassign parent after FinishSpawning
	for (auto ActorAndParentData : BlueprintActorAndParentArray)
	{
		if (ActorAndParentData.Actor->IsValidLowLevel() && !ActorAndParentData.Actor->IsPendingKill() && ActorAndParentData.Parent->IsValidLowLevel() && !ActorAndParentData.Parent->IsPendingKill())
		{
			ActorAndParentData.Actor->GetRootComponent()->AttachToComponent(ActorAndParentData.Parent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	return Result;
}
AActor* ActorCopier::CopySingleActorAndReplaceClass(AActor* OriginActor, TSubclassOf<AActor> NewActorClass)
{
	TArray<FName>EmptyExcludeProperties;

	auto ActorExcludeProperties = GetActorExcludeProperties();
	auto SceneComponentExcludeProperties = GetComponentExcludeProperties();

	auto CopiedActor = OriginActor->GetWorld()->SpawnActorDeferred<AActor>(NewActorClass, FTransform::Identity);
	CopyPropertyChecked(OriginActor, CopiedActor, ActorExcludeProperties);
	const auto& OriginComponents = OriginActor->GetComponents();

	auto OriginRootComp = OriginActor->GetRootComponent();
	if (!OriginRootComp)//not have root component
	{
		return CopiedActor;
	}
	auto CopiedRootComp = CopiedActor->GetRootComponent();
	bool rootCompNeedRegister = false;
	if (!CopiedRootComp)
	{
		CopiedRootComp = NewObject<USceneComponent>(CopiedActor, OriginRootComp->GetClass(), OriginRootComp->GetFName(), RF_NoFlags);
		rootCompNeedRegister = true;
	}
	CopyPropertyChecked(OriginRootComp, CopiedRootComp, SceneComponentExcludeProperties);
	if (rootCompNeedRegister)
	{
		CopiedActor->FinishAndRegisterComponent(CopiedRootComp);
		CopiedActor->SetRootComponent(CopiedRootComp);
	}

	TArray<USceneComponent*> ThisActorSceneComponents;//this Actor's SceneComponent array. when child SceneComponent need to attach to parent, search from this array
	ThisActorSceneComponents.Add(CopiedRootComp);
	TArray<SceneComponentToParentNameStruct> NeedReparentSceneComponents;//need to reasign parent SceneComponent

	for (auto OriginComp : OriginComponents)
	{
		if (OriginComp == OriginRootComp)continue;//skip RootComponent
		if (OriginComp->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient component
		auto CopiedComp = NewObject<UActorComponent>(CopiedActor, OriginComp->GetClass(), OriginComp->GetFName(), RF_NoFlags);
		CopyPropertyChecked(OriginComp, CopiedComp, SceneComponentExcludeProperties);
		CopiedActor->FinishAndRegisterComponent(CopiedComp);

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