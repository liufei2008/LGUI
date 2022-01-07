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

void ULGUIPrefabHelperObject::RevertPrefab()
{
	if (IsValid(PrefabAsset))
	{
		AActor* OldParentActor = nullptr;
		bool haveRootTransform = true;
		FTransform OldTransform;
		FUIAnchorData OldAnchorData;
		//store root transform
		if (IsValid(LoadedRootActor))
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

		if (IsValid(LoadedRootActor))
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
#if WITH_EDITOR
		ULGUIEditorManagerObject::RefreshAllUI();
#endif
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
		AllLoadedActorArray.Empty();
		for (auto KeyValue : MapGuidToObject)
		{
			if (auto Actor = Cast<AActor>(KeyValue.Value))
			{
				AllLoadedActorArray.Add(Actor);
			}
		}

#if WITH_EDITOR
		TimePointWhenSavePrefab = PrefabAsset->CreateTime;
#endif
	}
}

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

bool ULGUIPrefabHelperObject::IsActorBelongsToSubPrefab(AActor* InActor)
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

bool ULGUIPrefabHelperObject::IsActorBelongsToThis(AActor* InActor, bool InCludeSubPrefab)
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
}

void ULGUIPrefabHelperObject::UnlinkPrefab(AActor* InPrefabActor)
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
#endif
