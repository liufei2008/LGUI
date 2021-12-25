// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "Utils/LGUIUtils.h"

ULGUIPrefabHelperObject::ULGUIPrefabHelperObject()
{
	PrimaryComponentTick.bCanEverTick = false;
	bIsEditorOnly = true;
}


void ULGUIPrefabHelperObject::OnRegister()
{
	Super::OnRegister();
}
void ULGUIPrefabHelperObject::OnUnregister()
{
	Super::OnUnregister();
}

#if WITH_EDITOR
void ULGUIPrefabHelperObject::BeginDestroy()
{
	if (PrefabOverrideParameterObject.IsValid())
	{
		PrefabOverrideParameterObject->RemoveFromRoot();
		PrefabOverrideParameterObject->ConditionalBeginDestroy();
	}
	Super::BeginDestroy();
}
void ULGUIPrefabHelperObject::SetActorPropertyInOutliner(AActor* Actor, bool InListed)
{
	auto bEditable_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bEditable"));
	bEditable_Property->SetPropertyValue_InContainer(Actor, InListed);

	Actor->bHiddenEd = !InListed;
	Actor->bHiddenEdLayer = !InListed;
	Actor->bHiddenEdLevel = !InListed;
	Actor->bLockLocation = !InListed;

	auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
	bListedInSceneOutliner_Property->SetPropertyValue_InContainer(Actor, InListed);
}

void ULGUIPrefabHelperObject::LoadPrefab(UWorld* InWorld, USceneComponent* InParent)
{
	if (!LoadedRootActor.IsValid())
	{
		LoadedRootActor = PrefabAsset->LoadPrefabForEdit(InWorld
			, InParent
			, MapGuidToObject, SubPrefabMap
			, PrefabAsset->OverrideParameterData, PrefabOverrideParameterObject
		);
		if (PrefabOverrideParameterObject == nullptr)//old version prefab will not create it, so make sure it is created
		{
			PrefabOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>(LoadedRootActor.Get());
		}
		PrefabOverrideParameterObject->AddToRoot();
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapGuidToObject)
		{
			if (auto Actor = Cast<AActor>(KeyValue.Value.Get()))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}
		//Make subprefab's actor invisible
		TArray<AActor*> AllChildrenActorArray;
		LGUIUtils::CollectChildrenActors(LoadedRootActor.Get(), AllChildrenActorArray, true);
		for (auto ActorItem : AllChildrenActorArray)
		{
			if (!AllLoadedActorArray.Contains(ActorItem))
			{
				SetActorPropertyInOutliner(ActorItem, false);
			}
		}
	}
}

void ULGUIPrefabHelperObject::SavePrefab()
{
	if (PrefabAsset)
	{
		//fill OverrideParameterObject with default value
		if (PrefabOverrideParameterObject == nullptr)//this could be a newly created prefab, so the PrefabOverrideParameterObject could be null
		{
			PrefabOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>(LoadedRootActor.Get());
		}
		PrefabOverrideParameterObject->AddToRoot();
		PrefabOverrideParameterObject->SaveCurrentValueAsDefault();

		TMap<TWeakObjectPtr<UObject>, FGuid> MapObjectToGuid;
		for (auto KeyValue : MapGuidToObject)
		{
			if (KeyValue.Value.IsValid())
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		PrefabAsset->SavePrefab(LoadedRootActor.Get()
			, MapObjectToGuid, SubPrefabMap
			, PrefabOverrideParameterObject, PrefabAsset->OverrideParameterData
		);
		MapGuidToObject.Empty();
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
			if (auto Actor = Cast<AActor>(KeyValue.Key.Get()))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}
		PrefabAsset->RefreshAgentActorsInPreviewWorld();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ULGUIPrefabHelperObject::UnlinkSubPrefab(AActor* InSubPrefabActor)
{
	check(SubPrefabMap.Contains(InSubPrefabActor));
	SubPrefabMap.Remove(InSubPrefabActor);
	TArray<AActor*> ChildrenActors;
	LGUIUtils::CollectChildrenActors(InSubPrefabActor, ChildrenActors, false);
	for (auto Actor : ChildrenActors)
	{
		SetActorPropertyInOutliner(Actor, true);
	}
}

ULGUIPrefab* ULGUIPrefabHelperObject::GetSubPrefabAsset(AActor* InSubPrefabActor)
{
	check(SubPrefabMap.Contains(InSubPrefabActor));
	return SubPrefabMap[InSubPrefabActor].PrefabAsset;
}

void ULGUIPrefabHelperObject::RevertPrefab()
{
	if (IsValid(PrefabAsset))
	{
		AActor* OldParentActor = nullptr;
		bool haveRootTransform = true;
		FTransform OldTransform;
		FUIAnchorData OldAnchorData;
		//store root transform
		if (LoadedRootActor.IsValid())
		{
			OldParentActor = LoadedRootActor->GetAttachParentActor();
			if (auto RootComp = LoadedRootActor->GetRootComponent())
			{
				haveRootTransform = true;
				OldTransform = LoadedRootActor->GetRootComponent()->GetRelativeTransform();
				if (auto RootUIComp = Cast<UUIItem>(RootComp))
				{
					OldAnchorData = RootUIComp->GetAnchorData();
				}
			}
		}
		//Revert exsiting objects with parameters inside prefab
		{
			LoadedRootActor = nullptr;
			LoadPrefab(this->GetWorld(), OldParentActor->GetRootComponent());
		}

		if (LoadedRootActor.IsValid())
		{
			if (haveRootTransform)
			{
				if (auto RootComp = LoadedRootActor->GetRootComponent())
				{
					RootComp->SetRelativeTransform(OldTransform);
					if (auto RootUIItem = Cast<UUIItem>(RootComp))
					{
						RootUIItem->SetAnchorData(OldAnchorData);
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}
#endif
