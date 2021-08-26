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

#if WITH_EDITOR
AActor* ActorSerializer::DeserializeActorRecursiveForRecreate(USceneComponent* Parent, const FLGUIActorSaveData& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			ActorClass = AActor::StaticClass();
			UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DeserializeActorRecursiveForRecreate]Class:%s is not a Actor, use default"), *(ActorClass->GetFName().ToString()));
		}

		auto NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
		auto guidInPrefab = SaveData.GetActorGuid(FGuid::NewGuid());
		bool skipGuid = false;
		if (CreatedActorsGuid.Contains(guidInPrefab))
		{
			UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForRecreate] Get duplicated ActorGuid, means this prefab may created by previours PrefabSystem. This issue will be fixed during this operation."));
			skipGuid = true;
		}
		LoadProperty(NewActor, SaveData.ActorPropertyData, GetActorExcludeProperties(true, skipGuid));
		CreatedActorsGuid.Add(NewActor->GetActorGuid());
		ULGUIEditorManagerObject::AddActorForPrefabSystem(NewActor);

		auto RootCompSaveData = SaveData.ComponentPropertyData[0];
		auto RootComp = NewActor->GetRootComponent();
		if (RootComp)//if this actor have default root component
		{
			//actor's default root component dont need mannually register
			LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
		}
		else
		{
			if (RootCompSaveData.ComponentClass != -1)//have RootComponent data
			{
				if (auto CompClass = FindClassFromListByIndex(RootCompSaveData.ComponentClass, Prefab.Get()))
				{
					if (!CompClass->IsChildOf(USceneComponent::StaticClass()))//if not the right class, use default
					{
						CompClass = USceneComponent::StaticClass();
						UE_LOG(LGUI, Error, TEXT("Class:%s is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()));
					}
					RootComp = NewObject<USceneComponent>(NewActor, CompClass, RootCompSaveData.ComponentName, RF_Transactional);
					NewActor->SetRootComponent(RootComp);
					LoadProperty(RootComp, RootCompSaveData.PropertyData, GetComponentExcludeProperties());
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
					UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DeserializeActorRecursiveForRecreate]Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				auto Comp = NewObject<UActorComponent>(NewActor, CompClass, CompData.ComponentName, RF_Transactional);
				LoadProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
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
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForRecreate]Component Class of index:%d not found!"), (CompData.ComponentClass));
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
			DeserializeActorRecursiveForRecreate(RootComp, ChildSaveData, id);
		}
		return NewActor;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForRecreate]Actor Class of index:%d not found!"), (SaveData.ActorClass));
		return nullptr;
	}
}
#endif