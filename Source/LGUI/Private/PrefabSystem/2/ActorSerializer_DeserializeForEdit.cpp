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
#include "Core/Actor/LGUIManagerActor.h"

using namespace LGUIPrefabSystem;

#if WITH_EDITORONLY_DATA
AActor* ActorSerializer::DeserializeActorRecursiveForEdit(USceneComponent* Parent, const FLGUIActorSaveData& SaveData, int32& id)
{
	if (auto ActorClass = FindClassFromListByIndex(SaveData.ActorClass, Prefab.Get()))
	{
		if (!ActorClass->IsChildOf(AActor::StaticClass()))//if not the right class, use default
		{
			UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForEdit]Find class: '%s' at index: %d, but is not a Actor class, use default"), *(ActorClass->GetFName().ToString()), SaveData.ActorClass);
			ActorClass = AActor::StaticClass();
		}

		auto ActorGuidInPrefab = SaveData.GetActorGuid(FGuid::NewGuid());
		AActor* NewActor = this->GetExistingActorFunction != nullptr ? this->GetExistingActorFunction(ActorGuidInPrefab) : nullptr;
		bool UseExistingActor = NewActor != nullptr;
		if (UseExistingActor)
		{
			
		}
		else//not existing actor, create it
		{
			NewActor = TargetWorld->SpawnActorDeferred<AActor>(ActorClass, FTransform::Identity);
			CreatedActorsNeedToFinishSpawn.Add(NewActor);
			if (CreateNewActorFunction != nullptr)
			{
				CreateNewActorFunction(NewActor, ActorGuidInPrefab);
			}
		}
		if (!DeserializationSessionId.IsValid())
		{
			DeserializationSessionId = FGuid::NewGuid();
			LGUIManagerActor->BeginPrefabSystemProcessingActor(DeserializationSessionId);
		}
		LGUIManagerActor->AddActorForPrefabSystem(NewActor, DeserializationSessionId, 0);
		CreatedActors.Add(NewActor);
		CreatedActorsGuid.Add(ActorGuidInPrefab);
		LoadProperty(NewActor, SaveData.ActorPropertyData, GetActorExcludeProperties(true, true));

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
						UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForEdit]Find class: '%s' at index: %d, but is not a USceneComponent, use default"), *(CompClass->GetFName().ToString()), RootCompSaveData.ComponentClass);
						CompClass = USceneComponent::StaticClass();
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
		TInlineComponentArray<UActorComponent*> TempCompArray;
		if (UseExistingActor)
		{
			NewActor->GetComponents(TempCompArray);
			auto rootCompIndex = TempCompArray.IndexOfByKey(RootComp);
			if (rootCompIndex != INDEX_NONE)
			{
				TempCompArray.RemoveAt(rootCompIndex);
			}
		}
		for (int i = 1; i < ComponentCount; i++)//start from 1, skip RootComponent
		{
			auto CompData = SaveData.ComponentPropertyData[i];
			if (auto CompClass = FindClassFromListByIndex(CompData.ComponentClass, Prefab.Get()))
			{
				if (!CompClass->IsChildOf(UActorComponent::StaticClass()))//if not the right class, use default
				{
					CompClass = UActorComponent::StaticClass();
					UE_LOG(LGUI, Error, TEXT("[ActorSerializer::DeserializeActorRecursiveForEdit]Class:%s is not a UActorComponent, use default"), *(CompClass->GetFName().ToString()));
				}
				
				UActorComponent* Comp = nullptr;
				bool UseExistingComponent = false;
				if (UseExistingActor)
				{
					int foundCompIndex = TempCompArray.IndexOfByPredicate([CompClass, CompData](const UActorComponent* Item) {
						return Item->GetClass() == CompClass && Item->GetFName() == CompData.ComponentName;
						});
					if (foundCompIndex != INDEX_NONE)
					{
						UseExistingComponent = true;
						Comp = TempCompArray[foundCompIndex];
						TempCompArray.RemoveAtSwap(foundCompIndex);
					}
				}
				if (!UseExistingComponent)
				{
					Comp = NewObject<UActorComponent>(NewActor, CompClass, CompData.ComponentName, RF_Transactional);
				}
				LoadProperty(Comp, CompData.PropertyData, GetComponentExcludeProperties());
				if (!UseExistingComponent)
				{
					if (!Comp->IsDefaultSubobject())
					{
						RegisterComponent(NewActor, Comp);
					}
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
				UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForEdit]Component Class of index:%d not found!"), (CompData.ComponentClass));
			}
		}
		//clear excess components
		if (UseExistingActor)
		{
			for (auto item : TempCompArray)
			{
				item->ConditionalBeginDestroy();
			}
			TempCompArray.Empty();
		}
		//SceneComponent reattach to parent
		for (int i = 0; i < NeedReparent_SceneComponents.Num(); i++)
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
				BlueprintActorToParentActorStruct ItemStruct;
				ItemStruct.BlueprintActor = NewActor;
				ItemStruct.ParentActor = Parent;
				BlueprintAndParentArray.Add(ItemStruct);
			}
			if (RootComp)
			{
				RootComp->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
			}
		}

		id++;
		MapIDToActor.Add(id, NewActor);
		MapGuidToActor.Add(ActorGuidInPrefab, NewActor);

		for (auto ChildSaveData : SaveData.ChildActorData)
		{
			DeserializeActorRecursiveForEdit(RootComp, ChildSaveData, id);
		}
		return NewActor;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[ActorSerializer::DeserializeActorRecursiveForEdit]Actor Class of index:%d not found!"), (SaveData.ActorClass));
		return nullptr;
	}
}
#endif

#endif
