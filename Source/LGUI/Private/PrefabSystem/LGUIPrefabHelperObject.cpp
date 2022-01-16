// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/Actor.h"

ULGUIPrefabHelperObject::ULGUIPrefabHelperObject()
{
	
}

void ULGUIPrefabHelperObject::PostInitProperties()
{
	Super::PostInitProperties();
}

#if WITH_EDITOR
void ULGUIPrefabHelperObject::RevertPrefab()
{
	if (IsValid(PrefabAsset))
	{
		USceneComponent* OldParent = nullptr;
		//store root transform
		if (IsValid(LoadedRootActor))
		{
			OldParent = LoadedRootActor->GetAttachParentActor() != nullptr ? LoadedRootActor->GetAttachParentActor()->GetRootComponent() : nullptr;
		}
		//collect current children
		TArray<AActor*> ChildrenActors;
		LGUIUtils::CollectChildrenActors(LoadedRootActor, ChildrenActors);
		//Revert exsiting objects with parameters inside prefab
		{
			LoadedRootActor = nullptr;
			LoadPrefab(this->GetWorld(), OldParent);
		}
		//delete extra actors
		for (auto& OldChild : ChildrenActors)
		{
			if (!AllLoadedActorArray.Contains(OldChild))
			{
				LGUIUtils::DestroyActorWithHierarchy(OldChild, false);
			}
		}

		ULGUIEditorManagerObject::RefreshAllUI();
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ULGUIPrefabHelperObject::LoadPrefab(UWorld* InWorld, USceneComponent* InParent)
{
	if (!IsValid(PrefabAsset))return;
	if (!IsValid(LoadedRootActor))
	{
		LoadedRootActor = PrefabAsset->LoadPrefabWithExistingObjects(InWorld
			, InParent
			, MapGuidToObject, SubPrefabMap
		);
		if (LoadedRootActor == nullptr)return;
		//remove extra objects
		TSet<FGuid> ObjectsToIgnore;
		for (auto KeyValue : MapGuidToObject)
		{
			if (!PrefabAsset->GetPrefabHelperObject()->MapGuidToObject.Contains(KeyValue.Key))//Prefab's agent object is clean, so compare with it
			{
				ObjectsToIgnore.Add(KeyValue.Key);
			}
		}
		for (auto ObjectGuid : ObjectsToIgnore)
		{
			MapGuidToObject.Remove(ObjectGuid);
		}
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapGuidToObject)
		{
			if (auto Actor = Cast<AActor>(KeyValue.Value))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}

		TimePointWhenSavePrefab = PrefabAsset->CreateTime;
		ULGUIEditorManagerObject::RefreshAllUI();
	}
}
#endif

#if WITH_EDITOR
void ULGUIPrefabHelperObject::BeginDestroy()
{
	ClearLoadedPrefab();
	Super::BeginDestroy();
}

void ULGUIPrefabHelperObject::PostEditUndo()
{
	Super::PostEditUndo();
}

void ULGUIPrefabHelperObject::SetActorPropertyInOutliner(AActor* Actor, bool InListed)
{
	auto bEditable_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bEditable"));
	bEditable_Property->SetPropertyValue_InContainer(Actor, InListed);

	Actor->bHiddenEd = !InListed;
	Actor->bHiddenEdLayer = !InListed;
	Actor->bHiddenEdLevel = !InListed;
#if ENGINE_MAJOR_VERSION >= 5
	Actor->SetLockLocation(!InListed);
#else
	Actor->bLockLocation = !InListed;
#endif

	auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
	bListedInSceneOutliner_Property->SetPropertyValue_InContainer(Actor, InListed);
}

void ULGUIPrefabHelperObject::ClearLoadedPrefab()
{
	if (IsValid(LoadedRootActor))
	{
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor);
	}
	MapGuidToObject.Empty();
	SubPrefabMap.Empty();
	AllLoadedActorArray.Empty();
}

bool ULGUIPrefabHelperObject::IsActorBelongsToSubPrefab(const AActor* InActor)
{
	if (!IsValid(InActor))return false;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InActor == KeyValue.Key)
		{
			return true;
		}
		if (InActor->IsAttachedTo(KeyValue.Key))
		{
			return true;
		}
	}
	return false;
}

bool ULGUIPrefabHelperObject::ActorIsSubPrefabRootActor(const AActor* InActor)
{
	if (!IsValid(InActor))return false;
	return SubPrefabMap.Contains(InActor);
}

bool ULGUIPrefabHelperObject::IsActorBelongsToThis(const AActor* InActor, bool InCludeSubPrefab)
{
	if (this->AllLoadedActorArray.Contains(InActor))
	{
		if (IsValid(this->LoadedRootActor))
		{
			if (InActor->IsAttachedTo(LoadedRootActor) || InActor == LoadedRootActor)
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

void ULGUIPrefabHelperObject::AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName)
{
	if (!IsValid(InSubPrefabActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.AddMemberProperty(InObject, InPropertyName);
		}
	}
}

void ULGUIPrefabHelperObject::RemoveMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName)
{
	if (!IsValid(InSubPrefabActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.RemoveMemberProperty(InObject, InPropertyName);
		}
	}
}

void ULGUIPrefabHelperObject::RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject)
{
	if (!IsValid(InSubPrefabActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.RemoveMemberProperty(InObject);
		}
	}
}

FLGUISubPrefabData ULGUIPrefabHelperObject::GetSubPrefabData(AActor* InSubPrefabActor)
{
	check(IsValid(InSubPrefabActor));
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			return KeyValue.Value;
		}
	}
	return FLGUISubPrefabData();
}

void ULGUIPrefabHelperObject::SavePrefab()
{
	if (IsValid(PrefabAsset))
	{
		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto KeyValue : MapGuidToObject)
		{
			if (IsValid(KeyValue.Value))
			{
				MapObjectToGuid.Add(KeyValue.Value, KeyValue.Key);
			}
		}
		PrefabAsset->SavePrefab(LoadedRootActor
			, MapObjectToGuid, SubPrefabMap
		);
		MapGuidToObject.Empty();
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
			if (auto Actor = Cast<AActor>(KeyValue.Key))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}
		PrefabAsset->RefreshAgentObjectsInPreviewWorld();

		TimePointWhenSavePrefab = PrefabAsset->CreateTime;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ULGUIPrefabHelperObject::UnpackSubPrefab(AActor* InSubPrefabActor)
{
	check(SubPrefabMap.Contains(InSubPrefabActor));
	SubPrefabMap.Remove(InSubPrefabActor);
}

void ULGUIPrefabHelperObject::UnpackPrefab(AActor* InPrefabActor)
{
	LoadedRootActor = nullptr;
	PrefabAsset = nullptr;
	MapGuidToObject.Empty();
	SubPrefabMap.Empty();
	AllLoadedActorArray.Empty();
}

ULGUIPrefab* ULGUIPrefabHelperObject::GetSubPrefabAsset(AActor* InSubPrefabActor)
{
	if (!IsValid(InSubPrefabActor))return false;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			return KeyValue.Value.PrefabAsset;
		}
	}
	return nullptr;
}

void ULGUIPrefabHelperObject::MarkOverrideParameterFromParentPrefab(UObject* InObject, const TSet<FName>& InPropertyNameSet)
{
	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}

	for (auto& KeyValue : SubPrefabMap)
	{
		if (Actor == KeyValue.Key || Actor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.AddMemberProperty(InObject, InPropertyNameSet);
		}
	}
}
void ULGUIPrefabHelperObject::MarkOverrideParameterFromParentPrefab(UObject* InObject, FName InPropertyName)
{
	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}

	for (auto& KeyValue : SubPrefabMap)
	{
		if (Actor == KeyValue.Key || Actor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.AddMemberProperty(InObject, InPropertyName);
			break;
		}
	}
}
#endif
