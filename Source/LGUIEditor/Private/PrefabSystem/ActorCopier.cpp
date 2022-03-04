// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "ActorCopier.h"
#include "Engine/World.h"
#include "LGUI.h"
#include "LGUIEditorModule.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/2/ActorSerializer.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
using namespace LGUIPrefabSystem;
bool ActorCopier::CopyCommonProperty(FProperty* Property, uint8* Src, uint8* Dest, int cppArrayIndex, bool isInsideCppArray)
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
void ActorCopier::CopyCommonPropertyForChecked(FProperty* Property, uint8* Src, uint8* Dest)
{
	if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_DisableEditOnInstance))return;//skip property with special flag
	if (auto arrProperty = CastField<FArrayProperty>(Property))
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
	else if (auto mapProperty = CastField<FMapProperty>(Property))
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
	else if (auto setProperty = CastField<FSetProperty>(Property))
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
	else if (auto structProperty = CastField<FStructProperty>(Property))
	{
		for (TFieldIterator<FProperty> It(structProperty->Struct); It; ++It)
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
void ActorCopier::CopyPropertyChecked(UObject* Origin, UObject* Target, TArray<FName> ExcludeProperties)
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
				CopyCommonPropertyForChecked(targetProperty, srcProperty->ContainerPtrToValuePtr<uint8>(Origin), targetProperty->ContainerPtrToValuePtr<uint8>(Target));
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

	auto propertyField = TFieldRange<FProperty>(Target->GetClass());
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

	auto ActorExcludeProperties = ActorSerializer::GetActorExcludeProperties(false, true);
	auto SceneComponentExcludeProperties = ActorSerializer::GetComponentExcludeProperties();

	auto CopiedActor = TargetWorld->SpawnActorDeferred<AActor>(OriginActor->GetClass(), FTransform::Identity);
	DuplicatingActorCollection.Add(CopiedActor);
	if (CopiedRootActor == nullptr)
	{
		CopiedRootActor = CopiedActor;
#if WITH_EDITORONLY_DATA
		if (IsEditMode)
		{
			ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(TargetWorld.Get(), CopiedRootActor);
		}
		else
#endif
		{
			ALGUIManagerActor::BeginPrefabSystemProcessingActor(TargetWorld.Get(), CopiedRootActor);
		}
	}
#if WITH_EDITORONLY_DATA
	if (IsEditMode)
	{
		ULGUIEditorManagerObject::AddActorForPrefabSystem(CopiedActor);
	}
	else
#endif
	{
		ALGUIManagerActor::AddActorForPrefabSystem(CopiedActor, CopiedRootActor, 0);
	}
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
		CopiedRootComp = NewObject<USceneComponent>(CopiedActor, OriginRootComp->GetClass(), OriginRootComp->GetFName(), RF_Transactional);
		rootCompNeedRegister = true;
	}
	CopyProperty(OriginRootComp, CopiedRootComp, SceneComponentExcludeProperties);
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
		auto CopiedComp = NewObject<UActorComponent>(CopiedActor, OriginComp->GetClass(), OriginComp->GetFName(), RF_Transactional);
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
			SceneComp->MarkRenderStateDirty();
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
	CopiedRootComp->UpdateComponentToWorld();
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
	auto StartTime = FDateTime::Now();
	TargetWorld = RootActor->GetWorld();
#if WITH_EDITORONLY_DATA
	IsEditMode = !TargetWorld->IsGameWorld();
#endif
	
	int32 originActorId = 0, copiedActorId = 0;
	GenerateActorIDRecursive(RootActor, originActorId);
	auto Result = CopyActorRecursive(RootActor, Parent, copiedActorId);

	//reassign actor reference
	for (auto item : ObjectPropertyMapStructList)
	{
		if (auto refObj = MapIDToCopiedActor.Find(item.id))
		{
			auto objProperty = (FObjectPropertyBase*)item.ObjProperty;
			objProperty->SetObjectPropertyValue_InContainer(item.Dest, *refObj);
		}
	}
	//finish Actor Spawn
	for (auto Actor : CreatedActors)
	{
		if (Actor->IsValidLowLevel())//check, incase some actor is destoyed when BeginPlay
		{
			Actor->FinishSpawning(FTransform::Identity, true);//BeginPlay execute
		}
	}
	//blueprint actor need to reassign parent after FinishSpawning
	for (auto ActorAndParentData : BlueprintActorAndParentArray)
	{
		if (ActorAndParentData.Actor->IsValidLowLevel() && ActorAndParentData.Parent->IsValidLowLevel())
		{
			ActorAndParentData.Actor->GetRootComponent()->AttachToComponent(ActorAndParentData.Parent, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}

#if WITH_EDITOR
	if (IsEditMode)
	{
		for (auto item : DuplicatingActorCollection)
		{
			ULGUIEditorManagerObject::RemoveActorForPrefabSystem(item);
		}
		if (CopiedRootActor != nullptr)//if any error hanppens then CopiedRootActor could be nullptr, so check it
		{
			ULGUIEditorManagerObject::EndPrefabSystemProcessingActor(TargetWorld.Get(), CopiedRootActor);
		}
	}
	else
#endif
	{
		for (auto item : DuplicatingActorCollection)
		{
			ALGUIManagerActor::RemoveActorForPrefabSystem(item, CopiedRootActor);
		}
		if (CopiedRootActor != nullptr)//if any error hanppens then CopiedRootActor could be nullptr, so check it
		{
			ALGUIManagerActor::EndPrefabSystemProcessingActor(TargetWorld.Get(), CopiedRootActor);
		}
	}

	auto TimeSpan = FDateTime::Now() - StartTime;
	auto Name =
#if WITH_EDITOR
		RootActor->GetActorLabel();
#else
		RootActor->GetPathName();
#endif
	UE_LOG(LGUIEditor, Log, TEXT("Take %fs duplicating actor: %s"), TimeSpan.GetTotalSeconds(), *Name);
	return Result;
}
AActor* ActorCopier::DuplicateActor(AActor* RootActor, USceneComponent* Parent)
{
	if (!IsValid(RootActor))
	{
		UE_LOG(LGUIEditor, Error, TEXT("[ActorCopier::CopyActorInternal]RootActor is not valid!"));
		return nullptr;
	}
	if (!RootActor->GetWorld())
	{
		UE_LOG(LGUIEditor, Error, TEXT("[ActorCopier::CopyActorInternal]RootActor is not valid!"));
		return nullptr;
	}
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
	auto SceneComponentExcludeProperties = ActorSerializer::GetComponentExcludeProperties();
	SceneComponentExcludeProperties.Add(TEXT("CreationMethod"));
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
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
