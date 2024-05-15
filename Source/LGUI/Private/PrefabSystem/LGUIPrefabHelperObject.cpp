﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/Actor.h"
#include "PrefabSystem/LGUIObjectReaderAndWriter.h"
#include LGUIPREFAB_SERIALIZER_NEWEST_INCLUDE

#define LOCTEXT_NAMESPACE "LGUIPrefabManager"
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

ULGUIPrefabHelperObject::ULGUIPrefabHelperObject()
{
	
}

#if WITH_EDITOR
void ULGUIPrefabHelperObject::BeginDestroy()
{
	ClearLoadedPrefab();
	Super::BeginDestroy();
#if WITH_EDITORONLY_DATA
	bCanNotifyAttachment = false;
	if (NewVersionPrefabNotificationArray.Num() > 0)
	{
		OnNewVersionDismissAllClicked();
	}
#endif
}

void ULGUIPrefabHelperObject::MarkAsManagerObject()
{
	if (bIsMarkedAsManagerObject)return;
	bIsMarkedAsManagerObject = true;
	ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]{
		if (Object.IsValid())
		{
			GEditor->OnLevelActorAttached().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorAttached);
			GEditor->OnLevelActorDetached().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorDetached);
			GEditor->OnLevelActorDeleted().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorDeleted);
			Object->bCanNotifyAttachment = true;
			ULGUIPrefabManagerObject::OnComponentCreateDelete().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnComponentCreateDelete);
		}
		}, 1);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &ULGUIPrefabHelperObject::OnObjectPropertyChanged);
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddUObject(this, &ULGUIPrefabHelperObject::OnPreObjectPropertyChanged);
}

void ULGUIPrefabHelperObject::LoadPrefab(UWorld* InWorld, USceneComponent* InParent)
{
	if (!IsValid(PrefabAsset))
	{
		UE_LOG(LGUI, Error, TEXT("LoadPrefab failed! PrefabAsset=%s, InParentComp=%s"), *GetNameSafe(PrefabAsset), *GetNameSafe(InParent));
		return;
	}
	if (!IsValid(LoadedRootActor))
	{
		LoadedRootActor = PrefabAsset->LoadPrefabWithExistingObjects(InWorld
			, InParent
			, MapGuidToObject, SubPrefabMap
		);

		if (LoadedRootActor == nullptr)return;

		ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	}
}
#endif

#if WITH_EDITOR

void ULGUIPrefabHelperObject::SetActorPropertyInOutliner(AActor* Actor, bool InListed)
{
	auto bEditable_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bEditable"));
	bEditable_Property->SetPropertyValue_InContainer(Actor, InListed);

	Actor->bHiddenEd = !InListed;
	Actor->bHiddenEdLayer = !InListed;
	Actor->bHiddenEdLevel = !InListed;
	Actor->SetLockLocation(!InListed);

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
}

bool ULGUIPrefabHelperObject::IsActorBelongsToSubPrefab(const AActor* InActor)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InActor))return false;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		auto& SubMapGuidToObject = SubPrefabKeyValue.Value.MapGuidToObject;
		for (auto& SubMapGuidToObjectKeyValue : SubMapGuidToObject)
		{
			if (SubMapGuidToObjectKeyValue.Value == InActor)
			{
				return true;
			}
		}
	}
	return false;
}
bool ULGUIPrefabHelperObject::IsActorBelongsToMissingSubPrefab(const AActor* InActor)
{
	if (!IsValid(InActor))return false;
#if WITH_EDITOR
	for (auto& Item : MissingPrefab)//@todo: should directly reference to every actor
	{
		if (InActor == Item || InActor->IsAttachedTo(Item))
		{
			return true;
		}
	}
#endif
	return false;
}

bool ULGUIPrefabHelperObject::ActorIsSubPrefabRootActor(const AActor* InActor)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InActor))return false;
	return SubPrefabMap.Contains(InActor);
}

#include "PrefabSystem/LGUIPrefabLevelManagerActor.h"
bool ULGUIPrefabHelperObject::IsActorBelongsToThis(const AActor* InActor)
{
	if (this->IsInsidePrefabEditor())
	{
		if (IsValid(this->LoadedRootActor))
		{
			if (InActor->IsAttachedTo(LoadedRootActor) || InActor == LoadedRootActor)
			{
				return true;
			}
		}
	}
	else
	{
		if (auto Level = InActor->GetLevel())
		{
			if (auto ManagerActor = ALGUIPrefabLevelManagerActor::GetInstance(Level, false))
			{
				return ManagerActor->PrefabHelperObject == this;//if this level have a PrefabManagerActor, then all actor is belongs to this PrefabHelperObject.
			}
		}
	}
	return false;
}

void ULGUIPrefabHelperObject::ClearInvalidObjectAndGuid()
{
	TSet<FGuid> GuidsToRemove;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (!IsValid(KeyValue.Value))
		{
			GuidsToRemove.Add(KeyValue.Key);
		}
	}
	for (auto& Item : GuidsToRemove)
	{
		MapGuidToObject.Remove(Item);

		for (auto& SubPrefabKeyValue : SubPrefabMap)
		{
			if (auto GuidInSubPrefabPtr = SubPrefabKeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab.Find(Item))
			{
				auto GuidInParentPrefab = Item;
				auto GuidInSubPrefab = *GuidInSubPrefabPtr;
				SubPrefabKeyValue.Value.MapGuidToObject.Remove(GuidInSubPrefab);
				SubPrefabKeyValue.Value.MapObjectGuidFromParentPrefabToSubPrefab.Remove(GuidInParentPrefab);
				break;
			}
		}
	}
}

void ULGUIPrefabHelperObject::AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabActor))return;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabActor == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyName);
			}
		}
	}
}

void ULGUIPrefabHelperObject::RemoveMemberPropertyFromSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabActor))return;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabActor == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.RemoveMemberProperty(InObject, InPropertyName);
				break;
			}
		}
	}
}

void ULGUIPrefabHelperObject::RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabRootActor, bool InIncludeRootTransform)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabRootActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		auto SubPrefabRootActor = KeyValue.Key;
		FLGUISubPrefabData& SubPrefabData = KeyValue.Value;
		SubPrefabData.CheckParameters();
		if (InSubPrefabRootActor == SubPrefabRootActor)
		{
			for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
			{
				auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
				TSet<FName> FilterNameSet;
				if (InSubPrefabRootActor->GetRootComponent() == DataItem.Object)//if is root component of prefab's root actor, then skip it's transform
				{
					if (!InIncludeRootTransform)
					{
						FilterNameSet.Add(USceneComponent::GetRelativeLocationPropertyName());
						FilterNameSet.Add(USceneComponent::GetRelativeRotationPropertyName());
						FilterNameSet.Add(USceneComponent::GetRelativeScale3DPropertyName());
					}
				}

				TSet<FName> NamesToClear;
				for (auto& PropertyName : DataItem.MemberPropertyNames)
				{
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
				}
				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyNames.RemoveSwap(PropertyName);
				}
				if (DataItem.MemberPropertyNames.Num() == 0)
				{
					SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
					i--;
				}
			}
			return;
		}
	}
}

FLGUISubPrefabData ULGUIPrefabHelperObject::GetSubPrefabData(AActor* InSubPrefabActor)
{
	CleanupInvalidSubPrefab();
	check(IsValid(InSubPrefabActor));
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabActor == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.CheckParameters();
				return SubPrefabKeyValue.Value;
			}
		}
	}
	return FLGUISubPrefabData();
}

AActor* ULGUIPrefabHelperObject::GetSubPrefabRootActor(AActor* InSubPrefabActor)
{
	CleanupInvalidSubPrefab();
	check(IsValid(InSubPrefabActor));
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabActor == KeyValue.Value)
			{
				return SubPrefabKeyValue.Key;
			}
		}
	}
	return nullptr;
}

void ULGUIPrefabHelperObject::SavePrefab()
{
	CleanupInvalidSubPrefab();
	if (IsValid(PrefabAsset))
	{
		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto& KeyValue : MapGuidToObject)
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
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}
		PrefabAsset->RefreshAgentObjectsInPreviewWorld();
		bAnythingDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

ULGUIPrefab* ULGUIPrefabHelperObject::GetSubPrefabAsset(AActor* InSubPrefabActor)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabActor))return nullptr;
	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (InSubPrefabActor == KeyValue.Value)
			{
				return SubPrefabKeyValue.Value.PrefabAsset;
			}
		}
	}
	return nullptr;
}

void ULGUIPrefabHelperObject::MarkOverrideParameterFromParentPrefab(UObject* InObject, const TArray<FName>& InPropertyNames)
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}

	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (Actor == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyNames);
			}
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}

	for (auto& SubPrefabKeyValue : SubPrefabMap)
	{
		for (auto& KeyValue : SubPrefabKeyValue.Value.MapGuidToObject)
		{
			if (Actor == KeyValue.Value)
			{
				SubPrefabKeyValue.Value.AddMemberProperty(InObject, InPropertyName);
				break;
			}
		}
	}
}




bool ULGUIPrefabHelperObject::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab, AActor* InSubPrefabRootActor)
{
	CleanupInvalidSubPrefab();

	//temporary disable these, so restore prefab data goes silently
	bCanNotifyAttachment = false;
	bCanCollectProperty = false;
	bCanNotifyComponentCreateDelete = false;

	bool AnythingChange = false;

	for (auto& SubPrefabKeyValue : this->SubPrefabMap)
	{
		auto SubPrefabRootActor = SubPrefabKeyValue.Key;
		auto& SubPrefabData = SubPrefabKeyValue.Value;
		SubPrefabData.CheckParameters();
		if (SubPrefabData.PrefabAsset == InSubPrefab
			&& (InSubPrefabRootActor != nullptr ? SubPrefabRootActor == InSubPrefabRootActor : true)
			)
		{
			//store override parameter to data
			LGUIPREFAB_SERIALIZER_NEWEST_NAMESPACE::ActorSerializer serializer;
			serializer.bOverrideVersions = false;
			auto OverrideData = serializer.SaveOverrideParameterToData(SubPrefabData.ObjectOverrideParameterArray);

			auto& SubPrefabMapGuidToObject = SubPrefabData.MapGuidToObject;

			TSet<FGuid> ExtraObjectsGuidsToRemove;
			TSet<UObject*> ExtraObjectsToDelete;
			//check objects to delete: compare guid in sub-prefab's assets and this parent stored guid
			SubPrefabData.PrefabAsset->ClearAgentObjectsInPreviewWorld();//force it create new agent object and use new data, because the prefab's sub-prefab or sub-sub-prefab could change
			auto& MapGuidToObjectInSubPrefab = SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject;
			for (auto& KeyValue : SubPrefabMapGuidToObject)
			{
				if (!MapGuidToObjectInSubPrefab.Contains(KeyValue.Key))
				{
					ExtraObjectsGuidsToRemove.Add(KeyValue.Key);
					ExtraObjectsToDelete.Add(KeyValue.Value);
					AnythingChange = true;
				}
			}
			for (auto& Item : ExtraObjectsGuidsToRemove)
			{
				SubPrefabMapGuidToObject.Remove(Item);

				FGuid FoundGuid;
				for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
				{
					if (KeyValue.Value == Item)
					{
						FoundGuid = KeyValue.Key;
						break;
					}
				}
				if (FoundGuid.IsValid())
				{
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Remove(FoundGuid);
				}
				AnythingChange = true;
			}

			//refresh sub-prefab's object
			TMap<TObjectPtr<AActor>, FLGUISubPrefabData> TempSubSubPrefabMap;
			auto AttachParentActor = SubPrefabRootActor->GetAttachParentActor();
			InSubPrefab->LoadPrefabWithExistingObjects(GetPrefabWorld()
				, AttachParentActor == nullptr ? nullptr : AttachParentActor->GetRootComponent()
				, SubPrefabMapGuidToObject, TempSubSubPrefabMap
			);

			//collect newly added object and guid
			auto ObjectExist = [&](UObject* InObject) {
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == InObject)
					{
						return true;
					}
				}
				return false;
			};
			for (auto& KeyValue : SubPrefabMapGuidToObject)
			{
				if (!ObjectExist(KeyValue.Value))
				{
					auto NewGuid = FGuid::NewGuid();
					this->MapGuidToObject.Add(NewGuid, KeyValue.Value);
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(NewGuid, KeyValue.Key);
					AnythingChange = true;
				}

				//this object could be the same one, but with different guid (because it's guid not stored in parent prefab, and generated every time)
				if (ExtraObjectsToDelete.Contains(KeyValue.Value))
				{
					ExtraObjectsToDelete.Remove(KeyValue.Value);
				}
			}

			//delete extra objects
			for (auto& Item : ExtraObjectsToDelete)
			{
				if (!IsValid(Item))continue;
				if (auto Comp = Cast<UActorComponent>(Item))
				{
					Comp->DestroyComponent();
				}
				else if (auto Actor = Cast<AActor>(Item))
				{
					LGUIUtils::DestroyActorWithHierarchy(Actor, false);
				}
				else
				{
					Item->ConditionalBeginDestroy();
				}
			}

			//no need to clear invalid objects, because when SavePrefab it will do the clear work. But if we are in level editor, then there is no SavePrefab, so clear invalid objects is required: ClearInvalidObjectAndGuid()

			//apply override parameter.
			{
				//clear invaid first
				for (int i = SubPrefabData.ObjectOverrideParameterArray.Num() - 1; i >= 0; i--)
				{
					auto& Item = SubPrefabData.ObjectOverrideParameterArray[i];
					if (!Item.Object.IsValid())
					{
						SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
						AnythingChange = true;
					}
				}

				for (auto& ObjectOverrideItem : SubPrefabData.ObjectOverrideParameterArray)
				{
					for (auto& PropName : ObjectOverrideItem.MemberPropertyNames)
					{
						LGUIUtils::NotifyPropertyPreChange(ObjectOverrideItem.Object.Get(), PropName);
					}
				}
				serializer.RestoreOverrideParameterFromData(OverrideData, SubPrefabData.ObjectOverrideParameterArray);
				for (auto& ObjectOverrideItem : SubPrefabData.ObjectOverrideParameterArray)
				{
					for (auto& PropName : ObjectOverrideItem.MemberPropertyNames)
					{
						LGUIUtils::NotifyPropertyChanged(ObjectOverrideItem.Object.Get(), PropName);
					}
				}
			}

			SubPrefabRootActor->GetRootComponent()->UpdateComponentToWorld();//root comp may stay prev position if not do this

			if (SubPrefabData.CheckParameters())
			{
				AnythingChange = true;
			}
		}
	}

	if (AnythingChange)
	{
		if (this->IsInsidePrefabEditor())
		{
			this->SavePrefab();
		}
		if (this->PrefabAsset != nullptr)//could be null in level editor
		{
#if WITH_EDITOR
			this->PrefabAsset->ThumbnailDirty = true;
#endif
			this->PrefabAsset->MarkPackageDirty();
		}
		ClearInvalidObjectAndGuid();//incase LevelPrefab reference invalid object, eg: delete object in sub-prefab's sub-prefab, and update the prefab in level
	}
	RefreshSubPrefabVersion(InSubPrefabRootActor);
	bCanNotifyAttachment = true;
	bCanCollectProperty = true;
	bCanNotifyComponentCreateDelete = true;
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	return AnythingChange;
}

void ULGUIPrefabHelperObject::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!IsValid(InObject))return;
	if (InPropertyChangedEvent.MemberProperty == nullptr || InPropertyChangedEvent.Property == nullptr)return;
	if (LGUIPrefabSystem::LGUIPrefab_ShouldSkipProperty(InPropertyChangedEvent.MemberProperty))return;
	if (LGUIPrefabSystem::LGUIPrefab_ShouldSkipProperty(InPropertyChangedEvent.Property))return;

	TryCollectPropertyToOverride(InObject, InPropertyChangedEvent.MemberProperty);
}
void ULGUIPrefabHelperObject::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
{
	if (!IsValid(InObject))return;
	auto ActiveMemberNode = InEditPropertyChain.GetActiveMemberNode();
	if (ActiveMemberNode == nullptr)return;
	auto MemberProperty = ActiveMemberNode->GetValue();
	if (MemberProperty == nullptr)return;
	if (LGUIPrefabSystem::LGUIPrefab_ShouldSkipProperty(MemberProperty))return;
	auto ActiveNode = InEditPropertyChain.GetActiveNode();
	if (ActiveNode != ActiveMemberNode)
	{
		auto Property = ActiveNode->GetValue();
		if (Property == nullptr)return;
		if (Property->HasAnyPropertyFlags(CPF_Transient))return;
	}

	TryCollectPropertyToOverride(InObject, MemberProperty);
}

void ULGUIPrefabHelperObject::TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty)
{
	if (!bCanCollectProperty)return;
	if (InObject->GetWorld() == this->GetPrefabWorld())
	{
		auto PropertyName = InMemberProperty->GetFName();
		AActor* PropertyActorInSubPrefab = nullptr;
		if (auto Actor = Cast<AActor>(InObject))
		{
			if (auto ObjectProperty = CastField<FObjectPropertyBase>(InMemberProperty))
			{
				if (ObjectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
				{
					return;//property change is propergated from ActorComponent to Actor, ignore it
				}
			}
			if (IsActorBelongsToSubPrefab(Actor))
			{
				PropertyActorInSubPrefab = Actor;
			}

			if (PropertyActorInSubPrefab != nullptr//if drag in level editor, then property change event will notify actor, so we need to collect property on actor's root component
				&& (PropertyName == USceneComponent::GetRelativeLocationPropertyName()
					|| PropertyName == USceneComponent::GetRelativeRotationPropertyName()
					|| PropertyName == USceneComponent::GetRelativeScale3DPropertyName()
					)
				)
			{
				InObject = PropertyActorInSubPrefab->GetRootComponent();
			}
		}
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			if (auto Actor = Component->GetOwner())
			{
				if (IsActorBelongsToSubPrefab(Actor))
				{
					bool bFindObjectInGuidMap = false;
					for (auto& KeyValue : this->MapGuidToObject)
					{
						if (KeyValue.Value == InObject)
						{
							bFindObjectInGuidMap = true;
							break;
						}
					}
					if (bFindObjectInGuidMap)
					{
						PropertyActorInSubPrefab = Actor;
					}
				}
			}
		}

		if (auto OuterActor = InObject->GetTypedOuter<AActor>())
		{
			if (IsActorBelongsToSubPrefab(OuterActor))
			{
				PropertyActorInSubPrefab = OuterActor;
			}
		}

		if (PropertyActorInSubPrefab)//object's member property
		{
			auto Property = FindFProperty<FProperty>(InObject->GetClass(), PropertyName);
			if (Property != nullptr)
			{
				SetAnythingDirty();
				AddMemberPropertyToSubPrefab(PropertyActorInSubPrefab, InObject, PropertyName);
				ULGUIPrefabManagerObject::OnPrefabEditor_AfterCollectPropertyToOverride.ExecuteIfBound(this, InObject, PropertyName);
				//refresh override parameter
			}
		}
		else
		{
			SetAnythingDirty();
		}
	}
}

void ULGUIPrefabHelperObject::OnLevelActorAttached(AActor* Actor, const AActor* AttachTo)
{
	if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
	if (!bCanNotifyAttachment)return;
	if (Actor->GetWorld() != this->GetPrefabWorld())return;
	if (auto PrefabManager = ULGUIPrefabWorldSubsystem::GetInstance(Actor->GetWorld()))
	{
		if (PrefabManager->IsPrefabSystemProcessingActor(Actor))return;
	}

	if (AttachmentActor.Actor == Actor)
	{
		AttachmentActor.AttachTo = (AActor*)AttachTo;
	}
	else
	{
		if (!AttachmentActor.Actor.IsValid())
		{
			AttachmentActor.Actor = Actor;
			AttachmentActor.AttachTo = (AActor*)AttachTo;
			AttachmentActor.DetachFrom = nullptr;
			this->bAlreadyShowMessageAtThisFrame = false;
			ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
				if (Object.IsValid())
				{
					Object->CheckAttachment();
				}
				}, 1);
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabHelperObject::OnLevelActorAttached] Should never reach this point!"));
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
	}
}
void ULGUIPrefabHelperObject::OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom)
{
	if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
	if (!bCanNotifyAttachment)return;
	if (Actor->GetWorld() != this->GetPrefabWorld())return;
	if (auto PrefabManager = ULGUIPrefabWorldSubsystem::GetInstance(Actor->GetWorld()))
	{
		if (PrefabManager->IsPrefabSystemProcessingActor(Actor))return;
	}

	AttachmentActor.Actor = Actor;
	AttachmentActor.AttachTo = nullptr;
	AttachmentActor.DetachFrom = (AActor*)DetachFrom;
	this->bAlreadyShowMessageAtThisFrame = false;
	ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
		if (Object.IsValid())
		{
			Object->CheckAttachment();
		}
		}, 1);
}

void ULGUIPrefabHelperObject::OnLevelActorDeleted(AActor* Actor)
{
	if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
	if (!bCanNotifyAttachment)return;
	if (this->IsInsidePrefabEditor())return;

	if (this->SubPrefabMap.Contains(Actor))
	{
		if (!ULGUIPrefabManagerObject::GetIsProcessingDelete())//delete by Unreal, should not allowed, but I can't prevent this operation, so I remove the sub prefab of it
		{
			this->Modify();
			this->RemoveSubPrefabByAnyActorOfSubPrefab(Actor);
			LGUIUtils::DestroyActorWithHierarchy(Actor);
			return;
		}
	}

	auto ActorBelongsToPrefab = false;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == Actor)
		{
			ActorBelongsToPrefab = true;
			break;
		}
	}

	if (ActorBelongsToPrefab//Cannot delete sub prefab's actor. Why cannot use IsActorBelongsToSubPrefab()? Because already dettached when deleted
		&& !this->SubPrefabMap.Contains(Actor)//But sub prefab's root actor is good to delete
		)
	{
		this->Modify();
		this->bAlreadyShowMessageAtThisFrame = false;
		ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
			if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
			if (!Object->bAlreadyShowMessageAtThisFrame)
			{
				Object->bAlreadyShowMessageAtThisFrame = true;
				auto InfoText = LOCTEXT("CannotRestructurePrefabInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
				FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				GEditor->UndoTransaction(false);
			}
			}, 1);
	}

	ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
		if (Object.IsValid())
		{
			if (Object->CleanupInvalidLinkToSubPrefabObject())
			{
				Object->Modify();
			}
		}
		}, 1);
}

bool ULGUIPrefabHelperObject::bFirstTimeShow_RestructActorBlueprint = true;
void ULGUIPrefabHelperObject::CheckAttachment()
{
	if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
	if (!bCanNotifyAttachment)return;
	if (!AttachmentActor.Actor.IsValid())return;

	auto CheckActorBlueprintInLevelActorRestruction = [](TWeakObjectPtr<AActor> Actor){
		if (!Actor.IsValid())return true;
		if (Actor->GetClass() && Actor->GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
		{
			auto InfoText = LOCTEXT("DetectRestructureActorBlueprintInPrefabInstance", "Looks like you are trying to modify actor-blueprint in a child Prefab.\
You should know that using actor-blueprint inside Prefab is not a good idea, so use it at your own risk.");
			UE_LOG(LGUI, Warning, TEXT("%s"), *(InfoText.ToString()));
			if (bFirstTimeShow_RestructActorBlueprint)
			{
				bFirstTimeShow_RestructActorBlueprint = false;
				LGUIUtils::EditorNotification(InfoText, 10);
			}
			return false;
		}
		return true;
	};
	if (!CheckActorBlueprintInLevelActorRestruction(AttachmentActor.Actor))return;
	if (!CheckActorBlueprintInLevelActorRestruction(AttachmentActor.AttachTo))return;
	if (!CheckActorBlueprintInLevelActorRestruction(AttachmentActor.DetachFrom))return;
	enum class EAttachementError
	{
		None,
		ActorMustBelongToRoot,
		CannotRestructurePrefabInstance,
	};
	EAttachementError AttachementError = EAttachementError::None;
	if (SubPrefabMap.Contains(AttachmentActor.Actor.Get()))//is sub prefab root actor
	{
		if (IsInsidePrefabEditor())
		{
			if (!AttachmentActor.AttachTo.IsValid() || AttachmentActor.AttachTo == RootAgentActorForPrefabEditor.Get())//sub prefab root actor cannot attach to world or root agent
			{
				AttachementError = EAttachementError::CannotRestructurePrefabInstance;
			}
		}
	}
	else if (
		(AttachmentActor.DetachFrom.IsValid() && (IsActorBelongsToSubPrefab(AttachmentActor.Actor.Get()) && IsActorBelongsToSubPrefab(AttachmentActor.DetachFrom.Get())))//detatch, restruct sbuprefab
		|| 
		(AttachmentActor.AttachTo.IsValid() && (IsActorBelongsToSubPrefab(AttachmentActor.Actor.Get()) && IsActorBelongsToSubPrefab(AttachmentActor.AttachTo.Get())))//attach, restruct sbuprefab
		)
	{
		AttachementError = EAttachementError::CannotRestructurePrefabInstance;
	}
	if (IsInsidePrefabEditor())
	{
		if (AttachmentActor.AttachTo == nullptr)//cannot attach to world
		{
			AttachementError = EAttachementError::ActorMustBelongToRoot;
		}
		if (AttachmentActor.AttachTo == RootAgentActorForPrefabEditor.Get())//cannot attach actor to root agent
		{
			AttachementError = EAttachementError::ActorMustBelongToRoot;
		}
	}
	switch (AttachementError)
	{
	default:
	case EAttachementError::None:
		this->SetAnythingDirty();
		break;
	case EAttachementError::ActorMustBelongToRoot:
	{
		if (!bAlreadyShowMessageAtThisFrame)
		{
			bAlreadyShowMessageAtThisFrame = true;
			auto InfoText = LOCTEXT("ActorMustBelongToRoot", "All actor must attach to root actor.");
			FMessageDialog::Open(EAppMsgType::Ok, InfoText);
			GEditor->UndoTransaction(false);
		}
	}
	break;
	case EAttachementError::CannotRestructurePrefabInstance:
	{
		if (!bAlreadyShowMessageAtThisFrame)
		{
			bAlreadyShowMessageAtThisFrame = true;
			auto InfoText = LOCTEXT("CannotRestructurePrefabInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
			FMessageDialog::Open(EAppMsgType::Ok, InfoText);
			GEditor->UndoTransaction(false);
		}
	}
	break;
	}
	AttachmentActor = FAttachmentActorStruct();
}

void ULGUIPrefabHelperObject::OnComponentCreateDelete(bool InCreateOrDelete, UActorComponent* InComponent, AActor* InActor)
{
#if 0//not work as I want, toooooooo many unexpected operations can trigger this
	if (!bCanNotifyComponentCreateDelete)return;
	if (InComponent->IsDefaultSubobject())return;
	if (this->IsActorBelongsToSubPrefab(InActor))
	{
		bAlreadyShowMessageAtThisFrame = false;
		ULGUIPrefabManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this), InCreateOrDelete]() {
			if (ULGUIPrefabManagerObject::GetIsBlueprintCompiling())return;
			if (!Object.IsValid())return;
			if (!Object->bAlreadyShowMessageAtThisFrame)
			{
				Object->bAlreadyShowMessageAtThisFrame = true;
				if (InCreateOrDelete)
				{
					auto InfoText = LOCTEXT("CannotAddComponentToPrefabInstance", "Children of a Prefab instance cannot add or remove component, the added component will not saved to prefab.\
\n\nYou can open the prefab in prefab editor to add or remove component, or unpack the prefab instance to remove its prefab connection.");
					FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				}
				else
				{
					auto InfoText = LOCTEXT("CannotAddComponentToPrefabInstance", "Children of a Prefab instance cannot add or remove component, the removed component will still exist inside prefab.\
\n\nYou can open the prefab in prefab editor to add or remove component, or unpack the prefab instance to remove its prefab connection.");
					FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				}
			}
			}, 1);
	}
#endif
}

UWorld* ULGUIPrefabHelperObject::GetPrefabWorld() const
{
	if (RootAgentActorForPrefabEditor.IsValid())
	{
		return RootAgentActorForPrefabEditor->GetWorld();
	}
	else
	{
		return Super::GetWorld();
	}
}

bool ULGUIPrefabHelperObject::CleanupInvalidLinkToSubPrefabObject()
{
	auto IsValidParentLinkedGuid = [&](const FGuid& InCheckGuid) {
		for (auto& SubPrefabKeyValue : SubPrefabMap)
		{
			auto& SubPrefabData = SubPrefabKeyValue.Value;
			if (SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(InCheckGuid))
			{
				return true;
			}
		}
		return false;
	};

	TSet<FGuid> GuidsToRemove;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (!IsValidParentLinkedGuid(KeyValue.Key))
		{
			GuidsToRemove.Add(KeyValue.Key);
		}
	}
	for (auto& Item : GuidsToRemove)
	{
		MapGuidToObject.Remove(Item);
	}
	return GuidsToRemove.Num() > 0;
}

#pragma region RevertAndApply
/**
 * When revert, if the parameter is RelativeLocation, then UIItem's AnchorData will also be reverted. Revert parameter is just copy data from origin to dest, origin means the temporary created objects in prefab's preview world.
 * But since AnchorData is relative to parent, and parent may not have the same AnchorData (because parent is temporary created inside preview world), so we need to set parent's AnchorData to now object's parent's AnchorData.
 */
void ULGUIPrefabHelperObject::CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject)
{
	if (auto SceneComp = Cast<USceneComponent>(InObject))
	{
		if (SubPrefabMap.Contains(SceneComp->GetOwner()))//if is sub prefab's root component
		{
			ULGUIPrefabManagerObject::OnPrefabEditor_CopyRootObjectParentAnchorData.ExecuteIfBound(this, InObject, OriginObject);
		}
	}
}

void ULGUIPrefabHelperObject::RevertPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInPrefab, const FLGUISubPrefabData& SubPrefabData, int RawArrayIndex, bool IsInsideRawArray)
{
	if (Property->ArrayDim > 1 && !IsInsideRawArray)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			RevertPrefabPropertyValue(ContextObject, Property, ContainerPointerInSrc, ContainerPointerInPrefab, SubPrefabData, i, true);
		}
		return;
	}
	bool bPropertySupportDirectCopyValue = false;
	if (CastField<FClassProperty>(Property) != nullptr)
	{
		bPropertySupportDirectCopyValue = true;
	}
	else if (auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		if (auto ObjectInPrefab = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPointerInPrefab))
		{
			auto ObjectClass = ObjectInPrefab->GetClass();
			if (ObjectInPrefab->IsAsset() && !ObjectClass->IsChildOf(AActor::StaticClass()))
			{
				bPropertySupportDirectCopyValue = true;
			}
			else
			{
				//search object in guid
				FGuid ObjectGuidInPrefab;
				for (auto& KeyValue : SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject)
				{
					if (KeyValue.Value == ObjectInPrefab)
					{
						ObjectGuidInPrefab = KeyValue.Key;
						break;
					}
				}
				FGuid ObjectGuidInParent;
				//find valid guid, get the guid in prarent, with the guid then get object, so that object is the real value
				if (ObjectGuidInPrefab.IsValid())
				{
					for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
					{
						if (KeyValue.Value == ObjectGuidInPrefab)
						{
							ObjectGuidInParent = KeyValue.Key;
							break;
						}
					}
				}
				if (ObjectGuidInParent.IsValid())
				{
					auto ObjectInParent = this->MapGuidToObject[ObjectGuidInParent];
					if (ObjectClass->IsChildOf(AActor::StaticClass()) || ObjectClass->IsChildOf(UActorComponent::StaticClass()))
					{
						ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInSrc, ObjectInParent, RawArrayIndex);
					}
					else
					{
						if (ObjectClass->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
							//&& ObjectProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference)//is this necessary?
							)
						{
							if (!IsValid(ObjectInParent))
							{
								//EditInlineNew object can create new one
								ObjectInParent = NewObject<UObject>(ContextObject, ObjectClass, NAME_None, RF_NoFlags, ObjectInPrefab);
								ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInSrc, ObjectInParent, RawArrayIndex);
								this->MapGuidToObject[ObjectGuidInParent] = ObjectInParent;
							}
							for (const auto PropertyItem : TFieldRange<FProperty>(ObjectClass))//check property inside object
							{
								RevertPrefabPropertyValue(ObjectInParent, PropertyItem, ObjectInParent, ObjectInPrefab, SubPrefabData);
							}
						}
						else
						{
							auto InfoText = FText::Format(LOCTEXT("RevertPrefabPropertyValue_MissingConditionWarning", "LGUI have not handle this condition:\nobject: '{0}'\nobjectClass: '{1}'")
								, FText::FromString(ObjectInPrefab->GetPathName()), FText::FromString(ObjectClass->GetPathName()));
							UE_LOG(LGUI, Log, TEXT("%s"), *InfoText.ToString());
							LGUIUtils::EditorNotification(InfoText);
						}
					}
				}
			}
		}
		else
		{
			bPropertySupportDirectCopyValue = true;
		}
	}
	else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptArrayHelper ArrayHelperForPrefab(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), ArrayHelperForPrefab.GetRawPtr(i), SubPrefabData);
		}
	}
	else if (auto MapProperty = CastField<FMapProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptMapHelper MapHelperForPrefab(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < MapHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, MapProperty->KeyProp, MapHelper.GetKeyPtr(i), MapHelperForPrefab.GetKeyPtr(i), SubPrefabData);
			RevertPrefabPropertyValue(ContextObject, MapProperty->ValueProp, MapHelper.GetPairPtr(i), MapHelperForPrefab.GetPairPtr(i), SubPrefabData);
		}
		MapHelperForPrefab.Rehash();
	}
	else if (auto SetProperty = CastField<FSetProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);//just copy so we don't need to resize it
		FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptSetHelper SetHelperForPrefab(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < SetHelper.Num(); i++)
		{
			RevertPrefabPropertyValue(ContextObject, SetProperty->ElementProp, SetHelper.GetElementPtr(i), SetHelperForPrefab.GetElementPtr(i), SubPrefabData);
		}
		SetHelperForPrefab.Rehash();
	}
	else if (auto StructProperty = CastField<FStructProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);
		auto StructPtr = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInSrc, RawArrayIndex);
		auto StructPtrForPrefab = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInPrefab, RawArrayIndex);
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			RevertPrefabPropertyValue(ContextObject, *It, StructPtr, StructPtrForPrefab, SubPrefabData);
		}
	}
	else
	{
		bPropertySupportDirectCopyValue = true;
	}
	if (bPropertySupportDirectCopyValue)
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInSrc, ContainerPointerInPrefab);
	}
}
void ULGUIPrefabHelperObject::RevertPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperties", "Revert Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	this->Modify();

	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	CopyRootObjectParentAnchorData(InObject, ObjectInPrefab);

	bCanCollectProperty = false;
	{
		for (auto PropertyName : InPropertyNames)
		{
			PropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, PropertyName);
			if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
			{
				//notify
				LGUIUtils::NotifyPropertyPreChange(InObject, Property);//need to do PreChange here, so that actor's PostContructionScript can work
				//set to default value
				RevertPrefabPropertyValue(InObject, Property, InObject, ObjectInPrefab, SubPrefabData);
				AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
				//delete item
				RemoveMemberPropertyFromSubPrefab(Actor, InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(InObject, Property);

				SetAnythingDirty();
			}
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}
void ULGUIPrefabHelperObject::RevertPrefabOverride(UObject* InObject, FName InPropertyName)
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	CopyRootObjectParentAnchorData(InObject, ObjectInPrefab);

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperty", "Revert Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		this->Modify();

		InPropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, InPropertyName);
		if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), InPropertyName))
		{
			//notify
			LGUIUtils::NotifyPropertyPreChange(InObject, Property);//need to do PreChange here, so that actor's PostContructionScript can work
			//set to default value
			RevertPrefabPropertyValue(InObject, Property, InObject, ObjectInPrefab, SubPrefabData);
			AfterObjectPropertyApplyOrRevert(InObject, InPropertyName);
			//delete item
			RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(InObject, Property);
			SetAnythingDirty();
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}

void ULGUIPrefabHelperObject::RevertAllPrefabOverride(UObject* InObject)
{
	bCanCollectProperty = false;
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
		else
		{
			Actor = InObject->GetTypedOuter<AActor>();
		}
		auto SubPrefabData = GetSubPrefabData(Actor);
		auto SubPrefabRootActor = GetSubPrefabRootActor(Actor);

		GEditor->BeginTransaction(LOCTEXT("RevertPrefabOnAll_Transaction", "Revert Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		this->Modify();

		auto SubPrefabAsset = SubPrefabData.PrefabAsset;
		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
					break;
				}
			}
			FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
			return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			auto ObjectInPrefab = FindOriginObjectInSourcePrefab(SourceObject);
			CopyRootObjectParentAnchorData(SourceObject, ObjectInPrefab);

			TSet<FName> NamesToClear;
			for (auto PropertyName : DataItem.MemberPropertyNames)
			{
				PropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, PropertyName);
				if (FilterNameSet.Contains(PropertyName))continue;
				NamesToClear.Add(PropertyName);
				if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
				{
					//notify
					LGUIUtils::NotifyPropertyPreChange(SourceObject, Property);//need to do PreChange here, so that actor's PostContructionScript can work
					//set to default value
					RevertPrefabPropertyValue(InObject, Property, SourceObject, ObjectInPrefab, SubPrefabData);
					AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
					//notify
					LGUIUtils::NotifyPropertyChanged(SourceObject, Property);
				}
			}
			for (auto& PropertyName : NamesToClear)
			{
				DataItem.MemberPropertyNames.Remove(PropertyName);
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(SubPrefabRootActor, true);

		SetAnythingDirty();
		GEditor->EndTransaction();
		//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
		RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
	}
	bCanCollectProperty = true;
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
}

FName ULGUIPrefabHelperObject::ReplaceObjectPropertyForApplyOrRevert(UObject* InObject, FName InPropertyName)
{
	ULGUIPrefabManagerObject::OnPrefabEditor_ReplaceObjectPropertyForApplyOrRevert.ExecuteIfBound(this, InObject, InPropertyName);
	return InPropertyName;
}
void ULGUIPrefabHelperObject::AfterObjectPropertyApplyOrRevert(UObject* InObject, FName InPropertyName)
{
	ULGUIPrefabManagerObject::OnPrefabEditor_AfterObjectPropertyApplyOrRevert.ExecuteIfBound(this, InObject, InPropertyName);
}

void ULGUIPrefabHelperObject::ApplyPrefabPropertyValue(UObject* ContextObject, FProperty* Property, void* ContainerPointerInSrc, void* ContainerPointerInPrefab, const FLGUISubPrefabData& SubPrefabData, int RawArrayIndex, bool IsInsideRawArray)
{
	if (Property->ArrayDim > 1 && !IsInsideRawArray)
	{
		for (int i = 0; i < Property->ArrayDim; i++)
		{
			ApplyPrefabPropertyValue(ContextObject, Property, ContainerPointerInSrc, ContainerPointerInPrefab, SubPrefabData, i, true);
		}
		return;
	}
	bool bPropertySupportDirectCopyValue = false;
	if (CastField<FClassProperty>(Property) != nullptr)
	{
		bPropertySupportDirectCopyValue = true;
	}
	else if (auto ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		if (auto ObjectInParent = ObjectProperty->GetObjectPropertyValue_InContainer(ContainerPointerInSrc))
		{
			auto ObjectClass = ObjectInParent->GetClass();
			if (ObjectInParent->IsAsset() && !ObjectClass->IsChildOf(AActor::StaticClass()))
			{
				bPropertySupportDirectCopyValue = true;
			}
			else
			{
				//search object in guid
				FGuid ObjectGuidInParent;
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == ObjectInParent)
					{
						ObjectGuidInParent = KeyValue.Key;
						break;
					}
				}
				FGuid ObjectGuidInPrefab;
				//find valid guid, get the guid in prefab, with the guid then get object, so that object is the real value
				if (ObjectGuidInParent.IsValid()
					&& SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(ObjectGuidInParent)//check if the guid exist in this sub-prefab, because there could be multiple sub-prefabs
					)
				{
					ObjectGuidInPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuidInParent];
				}
				if (ObjectGuidInPrefab.IsValid())
				{
					auto ObjectInPrefab = SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuidInPrefab];
					if (ObjectClass->IsChildOf(AActor::StaticClass()) || ObjectClass->IsChildOf(UActorComponent::StaticClass()))
					{
						ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInPrefab, ObjectInPrefab, RawArrayIndex);
					}
					else
					{
						if (ObjectClass->HasAnyClassFlags(EClassFlags::CLASS_EditInlineNew)
							//&& ObjectProperty->HasAnyPropertyFlags(EPropertyFlags::CPF_InstancedReference)//is this necessary?
							)
						{
							if (!IsValid(ObjectInPrefab))
							{
								//EditInlineNew object can create new one
								ObjectInPrefab = NewObject<UObject>(ContextObject, ObjectClass, NAME_None, RF_NoFlags, ObjectInParent);
								ObjectProperty->SetObjectPropertyValue_InContainer(ContainerPointerInPrefab, ObjectInPrefab, RawArrayIndex);
								SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuidInPrefab] = ObjectInPrefab;
							}
							for (const auto PropertyItem : TFieldRange<FProperty>(ObjectClass))//check property inside object
							{
								ApplyPrefabPropertyValue(ObjectInPrefab, PropertyItem, ObjectInParent, ObjectInPrefab, SubPrefabData);
							}
						}
						else
						{
							auto InfoText = FText::Format(LOCTEXT("ApplyPrefabPropertyValue_MissingConditionWarning", "LGUI have not handle this condition:\nobject: '{0}'\nobjectClass: '{1}'")
								, FText::FromString(ObjectInParent->GetPathName()), FText::FromString(ObjectClass->GetPathName()));
							UE_LOG(LGUI, Warning, TEXT("%s"), *InfoText.ToString());
							LGUIUtils::EditorNotification(InfoText);
						}
					}
				}
				else
				{
					auto InfoText = FText::Format(LOCTEXT("ApplyPrefabPropertyValue_ReferencingOuterObject", "This property '{0}' is referencing object which is not belongs to this prefab, will ignore it.")
						, FText::FromString(ObjectProperty->GetPathName()));
					UE_LOG(LGUI, Log, TEXT("%s"), *InfoText.ToString());
					LGUIUtils::EditorNotification(InfoText);
				}
			}
		}
		else
		{
			bPropertySupportDirectCopyValue = true;
		}
	}
	else if (auto ArrayProperty = CastField<FArrayProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptArrayHelper ArrayHelperForDst(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < ArrayHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, ArrayProperty->Inner, ArrayHelper.GetRawPtr(i), ArrayHelperForDst.GetRawPtr(i), SubPrefabData);
		}
	}
	else if (auto MapProperty = CastField<FMapProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptMapHelper MapHelperForDst(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < MapHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, MapProperty->KeyProp, MapHelper.GetKeyPtr(i), MapHelperForDst.GetKeyPtr(i), SubPrefabData);
			ApplyPrefabPropertyValue(ContextObject, MapProperty->ValueProp, MapHelper.GetPairPtr(i), MapHelperForDst.GetPairPtr(i), SubPrefabData);
		}
		MapHelperForDst.Rehash();
	}
	else if (auto SetProperty = CastField<FSetProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);//just copy so we don't need to resize it
		FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInSrc, RawArrayIndex));
		FScriptSetHelper SetHelperForDst(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ContainerPointerInPrefab, RawArrayIndex));
		for (int i = 0; i < SetHelper.Num(); i++)
		{
			ApplyPrefabPropertyValue(ContextObject, SetProperty->ElementProp, SetHelper.GetElementPtr(i), SetHelperForDst.GetElementPtr(i), SubPrefabData);
		}
		SetHelperForDst.Rehash();
	}
	else if (auto StructProperty = CastField<FStructProperty>(Property))
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);
		auto StructPtr = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInSrc, RawArrayIndex);
		auto StructPtrForDst = Property->ContainerPtrToValuePtr<uint8>(ContainerPointerInPrefab, RawArrayIndex);
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			ApplyPrefabPropertyValue(ContextObject, *It, StructPtr, StructPtrForDst, SubPrefabData);
		}
	}
	else
	{
		bPropertySupportDirectCopyValue = true;
	}
	if (bPropertySupportDirectCopyValue)
	{
		Property->CopyCompleteValue_InContainer(ContainerPointerInPrefab, ContainerPointerInSrc);
	}
}
void ULGUIPrefabHelperObject::ApplyPrefabOverride(UObject* InObject, const TArray<FName>& InPropertyNames)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperties", "Apply Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	this->Modify();

	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	//object not exist
	if (!ObjectGuid.IsValid())
	{
		return;
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		for (auto PropertyName : InPropertyNames)
		{
			PropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, PropertyName);
			if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
			{
				//set to default value
				ApplyPrefabPropertyValue(ObjectInPrefab, Property, InObject, ObjectInPrefab, SubPrefabData);
				AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
				//delete item
				RemoveMemberPropertyFromSubPrefab(Actor, InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(ObjectInPrefab, Property);

				SetAnythingDirty();
			}
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			//mark on sub prefab, because the object could belongs to subprefab's subprefab.
			SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(ObjectInPrefab, InPropertyNames);

			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}
void ULGUIPrefabHelperObject::ApplyPrefabOverride(UObject* InObject, FName InPropertyName)
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
			break;
		}
	}
	//object not exist
	if (!ObjectGuid.IsValid())
	{
		return;
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto ObjectInPrefab = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperty", "Apply Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		this->Modify();

		InPropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, InPropertyName);
		if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), InPropertyName))
		{
			//set to default value
			ApplyPrefabPropertyValue(ObjectInPrefab, Property, InObject, ObjectInPrefab, SubPrefabData);
			AfterObjectPropertyApplyOrRevert(InObject, InPropertyName);
			//delete item
			RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(ObjectInPrefab, Property);
			SetAnythingDirty();
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			//mark on sub prefab, because the object could belongs to subprefab's subprefab.
			SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(ObjectInPrefab, InPropertyName);

			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}
void ULGUIPrefabHelperObject::ApplyAllOverrideToPrefab(UObject* InObject)
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabRootActor = GetSubPrefabRootActor(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll_Transaction", "Apply Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		this->Modify();

		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
					break;
				}
			}
			if (ObjectGuid.IsValid())
			{
				FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
				return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
			}
			else
			{
				return (TObjectPtr<UObject>)nullptr;
			}
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			if (SourceObject == SubPrefabRootActor->GetRootComponent())//if is root component of prefab's root actor, then skip it's transform
			{
				FilterNameSet.Add(USceneComponent::GetRelativeLocationPropertyName());
				FilterNameSet.Add(USceneComponent::GetRelativeRotationPropertyName());
				FilterNameSet.Add(USceneComponent::GetRelativeScale3DPropertyName());
			}
			if (auto ObjectInPrefab = FindOriginObjectInSourcePrefab(SourceObject))
			{
				TSet<FName> NamesToClear;
				for (auto PropertyName : DataItem.MemberPropertyNames)
				{
					PropertyName = ReplaceObjectPropertyForApplyOrRevert(InObject, PropertyName);
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
					if (auto Property = FindFProperty<FProperty>(ObjectInPrefab->GetClass(), PropertyName))
					{
						//set to default value
						ApplyPrefabPropertyValue(ObjectInPrefab, Property, SourceObject, ObjectInPrefab, SubPrefabData);
						AfterObjectPropertyApplyOrRevert(InObject, PropertyName);
						//notify
						LGUIUtils::NotifyPropertyChanged(ObjectInPrefab, Property);
					}
				}
				//mark on sub prefab, because the object could belongs to subprefab's subprefab.
				SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(ObjectInPrefab, DataItem.MemberPropertyNames);

				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyNames.Remove(PropertyName);
				}
			}
			else//if not find OriginObject, means the SourceObject is newly created (added new component) @todo: automatic add component to origin prefab
			{
				if (SourceObject->IsA(UActorComponent::StaticClass()))
				{
					auto InfoText = FText::Format(LOCTEXT("NewComponentInPrefabInstance", "Detect none tracked component: '{0}' in PrefabInstance. Note children of a Prefab instance cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to add component to the prefab asset itself, or unpack the prefab instance to remove its prefab connection."), FText::FromString(SourceObject->GetName()));
					FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				}
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(SubPrefabRootActor, false);
		//save origin prefab
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		SetAnythingDirty();
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.ExecuteIfBound();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}
#pragma endregion RevertAndApply

void ULGUIPrefabHelperObject::RefreshSubPrefabVersion(AActor* InSubPrefabRootActor)
{
	if (IsInsidePrefabEditor())return;
	if (InSubPrefabRootActor != nullptr)
	{
		auto& SubPrefabData = SubPrefabMap[InSubPrefabRootActor];
		SubPrefabData.OverallVersionMD5 = SubPrefabData.PrefabAsset->GenerateOverallVersionMD5();
	}
	else
	{
		for (auto& KeyValue : SubPrefabMap)
		{
			KeyValue.Value.OverallVersionMD5 = KeyValue.Value.PrefabAsset->GenerateOverallVersionMD5();
		}
	}
}

void ULGUIPrefabHelperObject::MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, const TMap<FGuid, TObjectPtr<UObject>>& InSubMapGuidToObject, const TArray<FLGUIPrefabOverrideParameterData>& InObjectOverrideParameterArray)
{
	FLGUISubPrefabData SubPrefabData;
	SubPrefabData.PrefabAsset = InPrefab;
	SubPrefabData.OverallVersionMD5 = InPrefab->GenerateOverallVersionMD5();
	SubPrefabData.MapGuidToObject = InSubMapGuidToObject;
	SubPrefabData.ObjectOverrideParameterArray = InObjectOverrideParameterArray;

	TArray<AActor*> ChildrenActors;
	LGUIUtils::CollectChildrenActors(InActor, ChildrenActors, false);

	auto FindOrAddSubPrefabObjectGuidInParentPrefab = [&](UObject* InObject) {
		for (auto& KeyValue : MapGuidToObject)
		{
			if (KeyValue.Value == InObject)
			{
				return KeyValue.Key;
			}
		}
		auto NewGuid = FGuid::NewGuid();
		MapGuidToObject.Add(NewGuid, InObject);
		return NewGuid;
	};
	for (auto& KeyValue : InSubMapGuidToObject)
	{
		auto GuidInParentPrefab = FindOrAddSubPrefabObjectGuidInParentPrefab(KeyValue.Value);
		if (!SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(GuidInParentPrefab))
		{
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParentPrefab, KeyValue.Key);
		}
	}
	SubPrefabMap.Add(InActor, SubPrefabData);
	ULGUIPrefabManagerObject::OnPrefabEditor_AfterMakePrefabAsSubPrefab.ExecuteIfBound(this, InActor);

	SetAnythingDirty();
}

void ULGUIPrefabHelperObject::RemoveSubPrefabByRootActor(AActor* InPrefabRootActor)
{
	if (SubPrefabMap.Contains(InPrefabRootActor))
	{
		auto SubPrefabData = SubPrefabMap[InPrefabRootActor];
		for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
		{
			MapGuidToObject.Remove(KeyValue.Key);
		}
		SubPrefabMap.Remove(InPrefabRootActor);
	}
#if WITH_EDITOR
	else if (MissingPrefab.Contains(InPrefabRootActor))
	{
		MissingPrefab.Remove(InPrefabRootActor);
	}
#endif
	ClearInvalidObjectAndGuid();
}

void ULGUIPrefabHelperObject::RemoveSubPrefabByAnyActorOfSubPrefab(AActor* InPrefabActor)
{
	auto RootActor = GetSubPrefabRootActor(InPrefabActor);
	if (RootActor)
	{
		RemoveSubPrefabByRootActor(RootActor);
	}
}

ULGUIPrefab* ULGUIPrefabHelperObject::GetPrefabAssetBySubPrefabObject(UObject* InObject)
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
	else
	{
		Actor = InObject->GetTypedOuter<AActor>();
	}
	return GetSubPrefabData(Actor).PrefabAsset;
}

bool ULGUIPrefabHelperObject::CleanupInvalidSubPrefab()
{
	bool bAnythingChanged = false;

	{
		SubPrefabMap.Remove(nullptr);
		//invalid sub prefab
		TSet<AActor*> SubPrefabKeysToRemove;
		for (auto& KeyValue : SubPrefabMap)
		{
			if (IsValid(KeyValue.Key) && !IsValid(KeyValue.Value.PrefabAsset))
			{
				SubPrefabKeysToRemove.Add(KeyValue.Key);
#if WITH_EDITOR
				MissingPrefab.Add(KeyValue.Key);
#endif
			}
		}
		//invalid guid mapped object
		TSet<FGuid> GuidKeysToRemove;
		for (auto& Item : SubPrefabKeysToRemove)
		{
			SubPrefabMap.Remove(Item);
			if (IsValid(Item))
			{
				//cleanup MapGuidToObject, because these object could belongs to sub prefab that is about to remove
				for (auto& GuidToObjectKeyValue : MapGuidToObject)
				{
					if (IsValid(GuidToObjectKeyValue.Value))
					{
						if (GuidToObjectKeyValue.Value->IsInOuter(Item) || GuidToObjectKeyValue.Value == Item)
						{
							if (!GuidKeysToRemove.Contains(GuidToObjectKeyValue.Key))
							{
								GuidKeysToRemove.Add(GuidToObjectKeyValue.Key);
							}
						}
					}
					else
					{
						if (!GuidKeysToRemove.Contains(GuidToObjectKeyValue.Key))
						{
							GuidKeysToRemove.Add(GuidToObjectKeyValue.Key);
						}
					}
				}
			}
		}
		if (SubPrefabKeysToRemove.Num() > 0)
		{
			if (OnSubPrefabNewVersionUpdated.IsBound())
			{
				OnSubPrefabNewVersionUpdated.Broadcast();
			}
		}
		for (auto& Item : GuidKeysToRemove)
		{
			MapGuidToObject.Remove(Item);
		}
		bAnythingChanged = SubPrefabKeysToRemove.Num() > 0 || GuidKeysToRemove.Num() > 0;
		if (bAnythingChanged)
		{
			SetAnythingDirty();
		}
#if WITH_EDITOR
		MissingPrefab.Remove(nullptr);
#endif
	}
	return bAnythingChanged;
}
bool ULGUIPrefabHelperObject::GetAnythingDirty()const
{
	return bAnythingDirty; 
}
void ULGUIPrefabHelperObject::SetNothingDirty()
{ 
	bAnythingDirty = false; 
}
void ULGUIPrefabHelperObject::SetAnythingDirty() 
{
	bAnythingDirty = true; 
}

#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/LGUIManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif
void ULGUIPrefabHelperObject::CheckPrefabVersion()
{
	CleanupInvalidSubPrefab();
	bool bSomeAutoUpdate = false;
	for (auto& KeyValue : SubPrefabMap)
	{
		auto& SubPrefabData = KeyValue.Value;
		if (SubPrefabData.OverallVersionMD5 != SubPrefabData.PrefabAsset->GenerateOverallVersionMD5())
		{
			if (SubPrefabData.bAutoUpdate)
			{
				bSomeAutoUpdate = true;
			}
		}
	}
	if (bSomeAutoUpdate)
	{
		GEditor->BeginTransaction(LOCTEXT("LGUIAutoUpdatePrefab_Transaction", "LGUI Update Prefabs"));
		this->Modify();
	}
	for (auto& KeyValue : SubPrefabMap)
	{
		auto& SubPrefabData = KeyValue.Value;
		if (SubPrefabData.OverallVersionMD5 != SubPrefabData.PrefabAsset->GenerateOverallVersionMD5())
		{
			if (SubPrefabData.bAutoUpdate)
			{
				KeyValue.Key->GetLevel()->Modify();
				this->RefreshOnSubPrefabDirty(SubPrefabData.PrefabAsset, KeyValue.Key);
				auto InfoText = FText::Format(LOCTEXT("AutoUpdatePrefabInfo", "Auto update old version prefab to latest version:\nActor:'{0}' Prefab:'{1}'."), FText::FromString(KeyValue.Key->GetActorLabel()), FText::FromString(SubPrefabData.PrefabAsset->GetName()));
				UE_LOG(LGUI, Log, TEXT("%s"), *InfoText.ToString());
				LGUIUtils::EditorNotification(InfoText);
			}
			else
			{
				auto FoundIndex = NewVersionPrefabNotificationArray.IndexOfByPredicate([SubPrefabRootActor = KeyValue.Key](const FNotificationContainer& Item) {
					return Item.SubPrefabRootActor == SubPrefabRootActor;
					});
				if (FoundIndex != INDEX_NONE)
				{
					return;
				}
				auto InfoText = FText::Format(LOCTEXT("OldPrefabVersion", "Detect old prefab: Actor:'{0}' Prefab:'{1}', Would you want to update it?"), FText::FromString(KeyValue.Key->GetActorLabel()), FText::FromString(SubPrefabData.PrefabAsset->GetName()));
				FNotificationInfo Info(InfoText);
				Info.bFireAndForget = false;
				Info.bUseLargeFont = true;
				Info.bUseThrobber = false;
				Info.FadeOutDuration = 0.0f;
				Info.ExpireDuration = 0.0f;
				Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("UpdateToNewPrefabButton", "Update"), LOCTEXT("UpdateToNewPrefabButton_Tooltip", "Update the prefab to new.")
					, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionUpdateClicked, KeyValue.Key.Get())));
				Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("UpdateAllToNewPrefabButton", "Update All"), LOCTEXT("UpdateToAllNewPrefabButton_Tooltip", "Update all prefabs to new.")
					, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionUpdateAllClicked)));
				Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DismissButton", "Dismiss"), LOCTEXT("DismissButton_Tooltip", "Dismiss this notification")
					, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionDismissClicked, KeyValue.Key.Get())));
				Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DismissAllButton", "Dismiss All"), LOCTEXT("DismissAllButton_Tooltip", "Dismiss all notifications")
					, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionDismissAllClicked)));

				auto Notification = FSlateNotificationManager::Get().AddNotification(Info);
				Notification->SetCompletionState(SNotificationItem::CS_Pending);
				FNotificationContainer Item;
				Item.SubPrefabRootActor = KeyValue.Key;
				Item.Notification = Notification;
				NewVersionPrefabNotificationArray.Add(Item);
			}
		}
	}

	if (bSomeAutoUpdate)
	{
		this->ClearInvalidObjectAndGuid();
		GEditor->EndTransaction();

		ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();//make outliner refresh
	}
}

void ULGUIPrefabHelperObject::OnNewVersionUpdateClicked(AActor* InPrefabRootActor)
{
	auto FoundIndex = NewVersionPrefabNotificationArray.IndexOfByPredicate([SubPrefabRootActor = InPrefabRootActor](const FNotificationContainer& Item) {
		return Item.SubPrefabRootActor == SubPrefabRootActor;
	});
	if (FoundIndex != INDEX_NONE)
	{
		auto Item = NewVersionPrefabNotificationArray[FoundIndex];
		if (Item.Notification.IsValid())
		{
			auto SubPrefabDataPtr = SubPrefabMap.Find(InPrefabRootActor);
			if (SubPrefabDataPtr != nullptr)
			{
				GEditor->BeginTransaction(LOCTEXT("LGUIUpdatePrefab_Transaction", "LGUI Update Prefabs"));
				InPrefabRootActor->GetLevel()->Modify();
				this->Modify();
				if (SubPrefabDataPtr->OverallVersionMD5 == SubPrefabDataPtr->PrefabAsset->GenerateOverallVersionMD5())
				{
					Item.Notification.Pin()->SetText(LOCTEXT("AlreadyUpdated", "Already updated."));
				}
				else
				{
					this->RefreshOnSubPrefabDirty(SubPrefabDataPtr->PrefabAsset, InPrefabRootActor);
				}
				this->ClearInvalidObjectAndGuid();
				GEditor->EndTransaction();
			}
			Item.Notification.Pin()->SetCompletionState(SNotificationItem::CS_None);
			Item.Notification.Pin()->ExpireAndFadeout();
			ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();//make outliner refresh
		}
		NewVersionPrefabNotificationArray.RemoveAt(FoundIndex);
	}
}
void ULGUIPrefabHelperObject::OnNewVersionDismissClicked(AActor* InPrefabRootActor)
{
	auto FoundIndex = NewVersionPrefabNotificationArray.IndexOfByPredicate([SubPrefabRootActor = InPrefabRootActor](const FNotificationContainer& Item) {
		return Item.SubPrefabRootActor == SubPrefabRootActor;
		});
	if (FoundIndex != INDEX_NONE)
	{
		auto Item = NewVersionPrefabNotificationArray[FoundIndex];
		if (Item.Notification.IsValid())
		{
			Item.Notification.Pin()->SetCompletionState(SNotificationItem::CS_None);
			Item.Notification.Pin()->ExpireAndFadeout();
		}
		NewVersionPrefabNotificationArray.RemoveAt(FoundIndex);
	}
}

void ULGUIPrefabHelperObject::OnNewVersionUpdateAllClicked()
{
	GEditor->BeginTransaction(LOCTEXT("LGUIUpdateAllPrefab_Transaction", "LGUI Update Prefabs"));
	this->Modify();

	bool bUpdated = false;
	for (auto& Item : NewVersionPrefabNotificationArray)
	{
		if (Item.Notification.IsValid())
		{
			if (Item.SubPrefabRootActor.IsValid())
			{
				auto SubPrefabDataPtr = SubPrefabMap.Find(Item.SubPrefabRootActor.Get());
				if (SubPrefabDataPtr != nullptr)
				{
					if (SubPrefabDataPtr->OverallVersionMD5 == SubPrefabDataPtr->PrefabAsset->GenerateOverallVersionMD5())
					{
						Item.Notification.Pin()->SetText(LOCTEXT("AlreadyUpdated", "Already updated."));
					}
					else
					{
						Item.SubPrefabRootActor->GetLevel()->Modify();
						this->RefreshOnSubPrefabDirty(SubPrefabDataPtr->PrefabAsset, Item.SubPrefabRootActor.Get());
						bUpdated = true;
					}
				}
			}
			Item.Notification.Pin()->SetCompletionState(SNotificationItem::CS_None);
			Item.Notification.Pin()->ExpireAndFadeout();
		}
	}
	NewVersionPrefabNotificationArray.Empty();
	this->ClearInvalidObjectAndGuid();
	GEditor->EndTransaction();

	if (bUpdated)
	{
		ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();//make outliner refresh
	}
}
void ULGUIPrefabHelperObject::OnNewVersionDismissAllClicked()
{
	for (auto& Item : NewVersionPrefabNotificationArray)
	{
		if (Item.Notification.IsValid())
		{
			Item.Notification.Pin()->SetCompletionState(SNotificationItem::CS_None);
			Item.Notification.Pin()->ExpireAndFadeout();
		}
	}
	NewVersionPrefabNotificationArray.Empty();
}

#endif
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif

#undef LOCTEXT_NAMESPACE