// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Utils/LGUIUtils.h"

ULGUIPrefabHelperObject::ULGUIPrefabHelperObject()
{
	
}


#if WITH_EDITOR
void ULGUIPrefabHelperObject::BeginDestroy()
{
	ClearLoadedPrefab();
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
	if (!IsValid(PrefabAsset))return;
	if (!LoadedRootActor.IsValid())
	{
		LoadedRootActor = PrefabAsset->LoadPrefabForEdit(InWorld
			, InParent
			, MapGuidToObject, SubPrefabMap
			, PrefabAsset->OverrideParameterData, PrefabOverrideParameterObject
		);
		if (LoadedRootActor == nullptr)return;
		if (PrefabOverrideParameterObject == nullptr)//old version prefab will not create it, so make sure it is created
		{
			PrefabOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>(PrefabAsset);
		}
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapGuidToObject)
		{
			if (auto Actor = Cast<AActor>(KeyValue.Value.Get()))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}
		//Make subprefab's actor invisible
		//TArray<AActor*> AllChildrenActorArray;
		//LGUIUtils::CollectChildrenActors(LoadedRootActor.Get(), AllChildrenActorArray, true);
		//for (auto ActorItem : AllChildrenActorArray)
		//{
		//	if (!AllLoadedActorArray.Contains(ActorItem))
		//	{
		//		SetActorPropertyInOutliner(ActorItem, false);
		//	}
		//}
	}
}

void ULGUIPrefabHelperObject::ClearLoadedPrefab()
{
	for (auto& KeyValue : SubPrefabMap)
	{
		if (KeyValue.Value.OverrideParameterObject)
		{
			KeyValue.Value.OverrideParameterObject->ConditionalBeginDestroy();
		}
	}
	if (IsValid(PrefabOverrideParameterObject))
	{
		PrefabOverrideParameterObject->ConditionalBeginDestroy();
	}
	if (LoadedRootActor.IsValid())
	{
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor.Get());
	}
	MapGuidToObject.Empty();
	SubPrefabMap.Empty();
	AllLoadedActorArray.Empty();
}

bool ULGUIPrefabHelperObject::IsActorBelongsToSubPrefab(AActor* InActor)
{
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InActor == KeyValue.Key.Get())
		{
			return true;
		}
		if (InActor->IsAttachedTo(KeyValue.Key.Get()))
		{
			return true;
		}
	}
	return false;
}

bool ULGUIPrefabHelperObject::IsActorBelongsToThis(AActor* InActor, bool InCludeSubPrefab)
{
	if (this->AllLoadedActorArray.Contains(InActor))
	{
		if (this->LoadedRootActor.IsValid())
		{
			if (InActor->IsAttachedTo(LoadedRootActor.Get()) || InActor == LoadedRootActor)
			{
				return true;
			}
		}
	}
	if (InCludeSubPrefab)
	{
		return IsActorBelongsToSubPrefab(InActor);
	}
	return false;
}

void ULGUIPrefabHelperObject::SavePrefab()
{
	if (IsValid(PrefabAsset))
	{
		//fill OverrideParameterObject with default value
		if (PrefabOverrideParameterObject == nullptr)//this could be a newly created prefab, so the PrefabOverrideParameterObject could be null
		{
			PrefabOverrideParameterObject = NewObject<ULGUIPrefabOverrideParameterObject>(PrefabAsset);
		}
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
		PrefabAsset->RefreshAgentObjectsInPreviewWorld();
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
	//TArray<AActor*> ChildrenActors;
	//LGUIUtils::CollectChildrenActors(InSubPrefabActor, ChildrenActors, false);
	//for (auto Actor : ChildrenActors)
	//{
	//	SetActorPropertyInOutliner(Actor, true);
	//}
}

void ULGUIPrefabHelperObject::UnlinkPrefab(AActor* InPrefabActor)
{
	for (auto& KeyValue : SubPrefabMap)
	{
		if (IsValid(KeyValue.Value.OverrideParameterObject))
		{
			KeyValue.Value.OverrideParameterObject->ConditionalBeginDestroy();
		}
	}
	if (IsValid(PrefabOverrideParameterObject))
	{
		PrefabOverrideParameterObject->ConditionalBeginDestroy();
	}
	LoadedRootActor = nullptr;
	PrefabAsset = nullptr;
	MapGuidToObject.Empty();
	SubPrefabMap.Empty();
	AllLoadedActorArray.Empty();
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
