// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/Actor.h"
#include "PrefabSystem/ActorSerializer4.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabManager"
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
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
	ULGUIEditorManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]{
		if (Object.IsValid())
		{
			GEditor->OnLevelActorAttached().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorAttached);
			GEditor->OnLevelActorDetached().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorDetached);
			GEditor->OnLevelActorDeleted().AddUObject(Object.Get(), &ULGUIPrefabHelperObject::OnLevelActorDeleted);
			Object->bCanNotifyAttachment = true;
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

		ULGUIEditorManagerObject::RefreshAllUI();
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
	Actor->bLockLocation = !InListed;

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
	CleanupInvalidSubPrefab();
	if (!IsValid(InActor))return false;
	return SubPrefabMap.Contains(InActor);
}

#include "PrefabSystem/LGUIPrefabHelperActor.h"
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
			if (auto ManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(Level, false))
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
	}
}

void ULGUIPrefabHelperObject::AddMemberPropertyToSubPrefab(AActor* InSubPrefabActor, UObject* InObject, FName InPropertyName)
{
	CleanupInvalidSubPrefab();
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
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.RemoveMemberProperty(InObject, InPropertyName);
		}
	}
}

void ULGUIPrefabHelperObject::RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabRootActor)
{
	CleanupInvalidSubPrefab();
	if (!IsValid(InSubPrefabRootActor))return;
	for (auto& KeyValue : SubPrefabMap)
	{
		auto SubPrefabRootActor = KeyValue.Key;
		FLGUISubPrefabData& SubPrefabData = KeyValue.Value;
		SubPrefabData.CheckParameters();
		if (InSubPrefabRootActor == SubPrefabRootActor || InSubPrefabRootActor->IsAttachedTo(SubPrefabRootActor))
		{
			for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
			{
				auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
				TSet<FName> FilterNameSet;
				if (i == 0)//first object is always the root component of prefab's root actor
				{
					if (auto UIItem = Cast<UUIItem>(DataItem.Object))
					{
						FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
					}
				}

				TSet<FName> NamesToClear;
				for (auto& PropertyName : DataItem.MemberPropertyName)
				{
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
				}
				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyName.Remove(PropertyName);
				}
				if (DataItem.MemberPropertyName.Num() == 0)
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
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			KeyValue.Value.CheckParameters();
			return KeyValue.Value;
		}
	}
	return FLGUISubPrefabData();
}

AActor* ULGUIPrefabHelperObject::GetSubPrefabRootActor(AActor* InSubPrefabActor)
{
	CleanupInvalidSubPrefab();
	check(IsValid(InSubPrefabActor));
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			return KeyValue.Key;
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




bool ULGUIPrefabHelperObject::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab, AActor* InSubPrefabRootActor)
{
	CleanupInvalidSubPrefab();
	bCanNotifyAttachment = false;
	bool AnythingChange = false;

	auto OriginObjectContainsInSourcePrefabByGuid = [=](UObject* InObject, FLGUISubPrefabData& SubPrefabData) {
		FGuid ObjectGuid;
		for (auto& KeyValue : this->MapGuidToObject)
		{
			if (KeyValue.Value == InObject)
			{
				ObjectGuid = KeyValue.Key;
			}
		}
		FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
		return SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject.Contains(ObjectGuidInSubPrefab);
	};
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
			LGUIPrefabSystem4::ActorSerializer serializer;
			serializer.bOverrideVersions = false;
			auto OverrideData = serializer.SaveOverrideParameterToData(SubPrefabData.ObjectOverrideParameterArray);

			TArray<AActor*> ChildrenActors;
			LGUIUtils::CollectChildrenActors(SubPrefabRootActor, ChildrenActors);

			TMap<FGuid, UObject*>& SubPrefabMapGuidToObject = SubPrefabData.MapGuidToObject;
			for (auto& GuidToObject : this->MapGuidToObject)
			{
				if (auto ObjectGuidInSubPrefabPtr = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Find(GuidToObject.Key))
				{
					if (!SubPrefabMapGuidToObject.Contains(*ObjectGuidInSubPrefabPtr))
					{
						SubPrefabMapGuidToObject.Add(*ObjectGuidInSubPrefabPtr, GuidToObject.Value);
					}
				}
			}

			TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
			auto AttachParentActor = SubPrefabRootActor->GetAttachParentActor();
			InSubPrefab->LoadPrefabWithExistingObjects(GetPrefabWorld()
				, AttachParentActor == nullptr ? nullptr : AttachParentActor->GetRootComponent()
				, SubPrefabMapGuidToObject, SubSubPrefabMap
				, false
			);

			//delete extra actors
			for (auto& OldChild : ChildrenActors)
			{
				if (!OriginObjectContainsInSourcePrefabByGuid(OldChild, SubPrefabData))
				{
					LGUIUtils::DestroyActorWithHierarchy(OldChild, false);
					AnythingChange = true;
				}
			}
			//collect added object and guid
			auto FindOrAddSubPrefabObjectGuidInParentPrefab = [&](UObject* InObject) {
				for (auto& KeyValue : this->MapGuidToObject)
				{
					if (KeyValue.Value == InObject)
					{
						return KeyValue.Key;
					}
				}
				auto NewGuid = FGuid::NewGuid();
				this->MapGuidToObject.Add(NewGuid, InObject);
				AnythingChange = true;
				return NewGuid;
			};
			for (auto& SubPrefabGuidToObject : SubPrefabMapGuidToObject)
			{
				auto ObjectGuidInParentPrefab = FindOrAddSubPrefabObjectGuidInParentPrefab(SubPrefabGuidToObject.Value);
				if (!SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(ObjectGuidInParentPrefab))
				{
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(ObjectGuidInParentPrefab, SubPrefabGuidToObject.Key);
					AnythingChange = true;
				}
			}
			//no need to clear invalid objects, because when SavePrefab it will do the clear work
			//apply override parameter. 
			serializer.RestoreOverrideParameterFromData(OverrideData, SubPrefabData.ObjectOverrideParameterArray);

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
			this->PrefabAsset->MarkPackageDirty();
		}
	}
	ULGUIEditorManagerObject::RefreshAllUI();
	RefreshSubPrefabVersion(InSubPrefabRootActor);
	bCanNotifyAttachment = true;
	return AnythingChange;
}

void ULGUIPrefabHelperObject::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!IsValid(InObject))return;
	if (InPropertyChangedEvent.MemberProperty == nullptr || InPropertyChangedEvent.Property == nullptr)return;
	if (InPropertyChangedEvent.MemberProperty->HasAnyPropertyFlags(CPF_Transient))return;
	if (InPropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))return;

	TryCollectPropertyToOverride(InObject, InPropertyChangedEvent.MemberProperty);
}
void ULGUIPrefabHelperObject::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
{
	if (!IsValid(InObject))return;
	auto ActiveMemberNode = InEditPropertyChain.GetActiveMemberNode();
	if (ActiveMemberNode == nullptr)return;
	auto MemberProperty = ActiveMemberNode->GetValue();
	if (MemberProperty == nullptr)return;
	if (MemberProperty->HasAnyPropertyFlags(CPF_Transient))return;
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
		SetAnythingDirty();

		auto PropertyName = InMemberProperty->GetFName();
		AActor* PropertyActor = nullptr;
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
				PropertyActor = Actor;
			}
		}
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			if (auto Actor = Component->GetOwner())
			{
				if (IsActorBelongsToSubPrefab(Actor))
				{
					PropertyActor = Actor;
				}
			}
		}
		else if(PropertyActor != nullptr//if drag in level editor, then property change event will notify actor, so we need to collect property on actor's root component
			&& (PropertyName == USceneComponent::GetRelativeLocationPropertyName()
				|| PropertyName == USceneComponent::GetRelativeRotationPropertyName()
				|| PropertyName == USceneComponent::GetRelativeScale3DPropertyName()
				)
			)
		{
			InObject = PropertyActor->GetRootComponent();
		}
		if (PropertyActor)//only allow actor or component's member property
		{
			auto Property = FindFProperty<FProperty>(InObject->GetClass(), PropertyName);
			if (Property != nullptr)
			{
				AddMemberPropertyToSubPrefab(PropertyActor, InObject, PropertyName);
				if (auto UIItem = Cast<UUIItem>(InObject))
				{
					if (PropertyName == USceneComponent::GetRelativeLocationPropertyName())//if UI's relative location change, then record anchor data too
					{
						AddMemberPropertyToSubPrefab(PropertyActor, InObject, UUIItem::GetAnchorDataPropertyName());
					}
					else if (PropertyName == UUIItem::GetAnchorDataPropertyName())//if UI's anchor data change, then record relative location too
					{
						AddMemberPropertyToSubPrefab(PropertyActor, InObject, USceneComponent::GetRelativeLocationPropertyName());
					}
				}
				//refresh override parameter
			}
		}
	}
}

void ULGUIPrefabHelperObject::OnLevelActorAttached(AActor* Actor, const AActor* AttachTo)
{
	if (!bCanNotifyAttachment)return;
	if (Actor->GetWorld() != this->GetPrefabWorld())return;
	if (ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(Actor))return;

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
			ULGUIEditorManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
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
	if (!bCanNotifyAttachment)return;
	if (Actor->GetWorld() != this->GetPrefabWorld())return;
	if (ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(Actor))return;

	AttachmentActor.Actor = Actor;
	AttachmentActor.AttachTo = nullptr;
	AttachmentActor.DetachFrom = (AActor*)DetachFrom;

	ULGUIEditorManagerObject::AddOneShotTickFunction([Object = MakeWeakObjectPtr(this)]() {
		if (Object.IsValid())
		{
			Object->CheckAttachment();
		}
		}, 1);
}

void ULGUIPrefabHelperObject::OnLevelActorDeleted(AActor* Actor)
{
	if (!bCanNotifyAttachment)return;
	if (this->IsInsidePrefabEditor())return;

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
		ULGUIEditorManagerObject::AddOneShotTickFunction([]() {
			auto InfoText = LOCTEXT("CannotRestructurePrefaInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
			FMessageDialog::Open(EAppMsgType::Ok, InfoText);
			GEditor->UndoTransaction(false);
			}, 1);
	}

	ULGUIEditorManagerObject::AddOneShotTickFunction([this]() {
		if (CleanupInvalidLinkToSubPrefabObject())
		{
			this->Modify();
		}
		}, 1);
}

void ULGUIPrefabHelperObject::CheckAttachment()
{
	if (!bCanNotifyAttachment)return;
	if (!AttachmentActor.Actor.IsValid())return;
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
	if (
		(IsActorBelongsToSubPrefab(AttachmentActor.DetachFrom.Get())//why use DetachFrom(not Actor)? because Actor is already dettached
			&& !SubPrefabMap.Contains(AttachmentActor.Actor.Get()))//is sub prefab's children actor
		|| IsActorBelongsToSubPrefab(AttachmentActor.AttachTo.Get())//attach to sub prefab
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
		break;
	case EAttachementError::ActorMustBelongToRoot:
	{
		auto InfoText = LOCTEXT("ActorMustBelongToRoot", "All actor must attach to root actor.");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
		GEditor->UndoTransaction(false);
	}
	break;
	case EAttachementError::CannotRestructurePrefabInstance:
	{
		auto InfoText = LOCTEXT("CannotRestructurePrefaInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
		GEditor->UndoTransaction(false);
	}
	break;
	}
	AttachmentActor = FAttachmentActorStruct();
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
void ULGUIPrefabHelperObject::CopyRootObjectParentAnchorData(UObject* InObject, UObject* OriginObject)
{
	if (auto SceneComp = Cast<USceneComponent>(InObject))
	{
		if (SubPrefabMap.Contains(SceneComp->GetOwner()))//if is sub prefab's root component
		{
			auto InObjectUIItem = Cast<UUIItem>(InObject);
			auto OriginObjectUIItem = Cast<UUIItem>(OriginObject);
			if (InObjectUIItem != nullptr && OriginObjectUIItem != nullptr)//if is UI item, we need to copy parent's property to origin object's parent property, to make anchor & location calculation right
			{
				auto InObjectParent = InObjectUIItem->GetParentUIItem();
				auto OriginObjectParent = OriginObjectUIItem->GetParentUIItem();
				if (InObjectParent != nullptr && OriginObjectParent != nullptr)
				{
					//copy relative location
					auto RelativeLocationProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), USceneComponent::GetRelativeLocationPropertyName());
					RelativeLocationProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					LGUIUtils::NotifyPropertyChanged(OriginObjectParent, RelativeLocationProperty);
					//copy anchor data
					auto AnchorDataProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), UUIItem::GetAnchorDataPropertyName());
					AnchorDataProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					LGUIUtils::NotifyPropertyChanged(OriginObjectParent, AnchorDataProperty);
				}
			}
		}
	}
}

void ULGUIPrefabHelperObject::RevertPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
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
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	CopyRootObjectParentAnchorData(InObject, OriginObject);

	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(InObject, OriginObject);
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
	ULGUIEditorManagerObject::RefreshAllUI();
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
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	CopyRootObjectParentAnchorData(InObject, OriginObject);

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperty", "Revert Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		this->Modify();

		if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), InPropertyName))
		{
			//set to default value
			Property->CopyCompleteValue_InContainer(InObject, OriginObject);
			//delete item
			RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(InObject, Property);
			SetAnythingDirty();
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
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
		auto SubPrefabData = GetSubPrefabData(Actor);

		GEditor->BeginTransaction(LOCTEXT("RevertPrefabOnAll", "Revert Prefab Override"));
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
			if (i == 0)//first object is always the root component of prefab's root actor
			{
				if (auto UIItem = Cast<UUIItem>(SourceObject))
				{
					FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
				}
			}
			auto OriginObject = FindOriginObjectInSourcePrefab(SourceObject);
			CopyRootObjectParentAnchorData(SourceObject, OriginObject);

			TSet<FName> NamesToClear;
			for (auto& PropertyName : DataItem.MemberPropertyName)
			{
				if (FilterNameSet.Contains(PropertyName))continue;
				NamesToClear.Add(PropertyName);
				if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
				{
					//set to default value
					Property->CopyCompleteValue_InContainer(SourceObject, OriginObject);
					//notify
					LGUIUtils::NotifyPropertyChanged(SourceObject, Property);
				}
			}
			for (auto& PropertyName : NamesToClear)
			{
				DataItem.MemberPropertyName.Remove(PropertyName);
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(Actor);

		SetAnythingDirty();
		GEditor->EndTransaction();
		//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
		RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}

void ULGUIPrefabHelperObject::ApplyPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
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
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(OriginObject, InObject);
				//delete item
				RemoveMemberPropertyFromSubPrefab(Actor, InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(OriginObject, Property);

				SetAnythingDirty();
			}
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			//mark on sub prefab, because the object could belongs to subprefab's subprefab.
			SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(OriginObject, InPropertyNameSet);

			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
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
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperty", "Apply Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		this->Modify();

		if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), InPropertyName))
		{
			//set to default value
			Property->CopyCompleteValue_InContainer(OriginObject, InObject);
			//delete item
			RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(OriginObject, Property);
			SetAnythingDirty();
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			//mark on sub prefab, because the object could belongs to subprefab's subprefab.
			SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(OriginObject, InPropertyName);

			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
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
	auto SubPrefabData = GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll", "Apply Prefab Override"));
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
				}
			}
			if (ObjectGuid.IsValid())
			{
				FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
				return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
			}
			else
			{
				return (UObject*)nullptr;
			}
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			if (i == 0)//first object is always the root component of prefab's root actor
			{
				if (auto UIItem = Cast<UUIItem>(SourceObject))
				{
					FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
				}
			}
			if (auto OriginObject = FindOriginObjectInSourcePrefab(SourceObject))
			{
				TSet<FName> NamesToClear;
				for (auto& PropertyName : DataItem.MemberPropertyName)
				{
					if (FilterNameSet.Contains(PropertyName))continue;
					NamesToClear.Add(PropertyName);
					if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
					{
						//set to default value
						Property->CopyCompleteValue_InContainer(OriginObject, SourceObject);
						//notify
						LGUIUtils::NotifyPropertyChanged(OriginObject, Property);
					}
				}
				//mark on sub prefab, because the object could belongs to subprefab's subprefab.
				SubPrefabAsset->GetPrefabHelperObject()->MarkOverrideParameterFromParentPrefab(OriginObject, DataItem.MemberPropertyName);

				for (auto& PropertyName : NamesToClear)
				{
					DataItem.MemberPropertyName.Remove(PropertyName);
				}
			}
			else//if not find OriginObject, means the SourceObject is newly created (added new component) @todo: automatic add component to origin prefab
			{
				if (SourceObject->IsA(UActorComponent::StaticClass()))
				{
					auto InfoText = FText::Format(LOCTEXT("NewComponentInPrefaInstance", "Detect none tracked component: '{0}' in PrefabInstance. Note children of a Prefab instance cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to add component to the prefab asset itself, or unpack the prefab instance to remove its prefab connection."), FText::FromString(SourceObject->GetName()));
					FMessageDialog::Open(EAppMsgType::Ok, InfoText);
				}
			}
		}
		RemoveAllMemberPropertyFromSubPrefab(Actor);
		//save origin prefab
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		SetAnythingDirty();
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
	//when apply or revert parameters in level editor, means we accept sub-prefab's current version, so we mark the version to newest, and we won't get 'update warning'.
	RefreshSubPrefabVersion(GetSubPrefabRootActor(Actor));
}
#pragma endregion RevertAndApply

void ULGUIPrefabHelperObject::RefreshSubPrefabVersion(AActor* InSubPrefabRootActor)
{
	if (IsInsidePrefabEditor())return;
	auto& SubPrefabData = SubPrefabMap[InSubPrefabRootActor];
	SubPrefabData.TimePointWhenSavePrefab = SubPrefabData.PrefabAsset->CreateTime;
}

void ULGUIPrefabHelperObject::MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, const TMap<FGuid, UObject*>& InSubMapGuidToObject, const TArray<FLGUIPrefabOverrideParameterData>& InObjectOverrideParameterArray)
{
	FLGUISubPrefabData SubPrefabData;
	SubPrefabData.PrefabAsset = InPrefab;
	SubPrefabData.TimePointWhenSavePrefab = InPrefab->CreateTime;
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
	//mark HierarchyIndex as default override parameter
	auto RootComp = InActor->GetRootComponent();
	auto RootUIComp = Cast<UUIItem>(RootComp);
	if (RootUIComp)
	{
		AddMemberPropertyToSubPrefab(InActor, RootUIComp, UUIItem::GetHierarchyIndexPropertyName());
	}

	SetAnythingDirty();
}

void ULGUIPrefabHelperObject::RemoveSubPrefabByRootActor(AActor* InPrefabRootActor)
{
	auto SubPrefabData = SubPrefabMap[InPrefabRootActor];
	for (auto& KeyValue : SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab)
	{
		MapGuidToObject.Remove(KeyValue.Key);
	}
	SubPrefabMap.Remove(InPrefabRootActor);
	ClearInvalidObjectAndGuid();
}

void ULGUIPrefabHelperObject::RemoveSubPrefab(AActor* InPrefabActor)
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
	return GetSubPrefabData(Actor).PrefabAsset;
}

bool ULGUIPrefabHelperObject::CleanupInvalidSubPrefab()
{
	bool bAnythingChanged = false;

	//invalid object
	{
		TSet<AActor*> KeysToRemove;
		for (auto& KeyValue : SubPrefabMap)
		{
			if (!IsValid(KeyValue.Key) || !IsValid(KeyValue.Value.PrefabAsset))
			{
				KeysToRemove.Add(KeyValue.Key);
			}
		}
		for (auto& Item : KeysToRemove)
		{
			SubPrefabMap.Remove(Item);
		}
		if (KeysToRemove.Num() > 0)
		{
			SetAnythingDirty();
			if (OnSubPrefabNewVersionUpdated.IsBound())
			{
				OnSubPrefabNewVersionUpdated.Broadcast();
			}
		}
		bAnythingChanged = bAnythingChanged || KeysToRemove.Num() > 0;
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
#include "Core/Actor/LGUIManagerActor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#endif
void ULGUIPrefabHelperObject::CheckPrefabVersion()
{
	CleanupInvalidSubPrefab();
	for (auto& KeyValue : SubPrefabMap)
	{
		auto& SubPrefabData = KeyValue.Value;
		if (SubPrefabData.TimePointWhenSavePrefab != SubPrefabData.PrefabAsset->CreateTime)
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
				, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionUpdateClicked, KeyValue.Key)));
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("UpdateAllToNewPrefabButton", "Update All"), LOCTEXT("UpdateToAllNewPrefabButton_Tooltip", "Update all prefabs to new.")
				, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionUpdateAllClicked)));
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DismissButton", "Dismiss"), LOCTEXT("DismissButton_Tooltip", "Dismiss this notification")
				, FSimpleDelegate::CreateUObject(this, &ULGUIPrefabHelperObject::OnNewVersionDismissClicked, KeyValue.Key)));
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
				GEditor->BeginTransaction(LOCTEXT("LGUIUpdatePrefab", "LGUI Update Prefabs"));
				InPrefabRootActor->GetLevel()->Modify();
				this->Modify();
				if (SubPrefabDataPtr->TimePointWhenSavePrefab == SubPrefabDataPtr->PrefabAsset->CreateTime)
				{
					Item.Notification.Pin()->SetText(LOCTEXT("AlreadyUpdated", "Already updated."));
				}
				else
				{
					this->RefreshOnSubPrefabDirty(SubPrefabDataPtr->PrefabAsset, InPrefabRootActor);
					if (OnSubPrefabNewVersionUpdated.IsBound())
					{
						OnSubPrefabNewVersionUpdated.Broadcast();
					}
				}
				GEditor->EndTransaction();
			}
			Item.Notification.Pin()->SetCompletionState(SNotificationItem::CS_None);
			Item.Notification.Pin()->ExpireAndFadeout();
			ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//make outliner refresh
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
	GEditor->BeginTransaction(LOCTEXT("LGUIUpdatePrefab", "LGUI Update Prefabs"));
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
					if (SubPrefabDataPtr->TimePointWhenSavePrefab == SubPrefabDataPtr->PrefabAsset->CreateTime)
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
	GEditor->EndTransaction();

	if (bUpdated)
	{
		if (OnSubPrefabNewVersionUpdated.IsBound())
		{
			OnSubPrefabNewVersionUpdated.Broadcast();
		}
		ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged();//make outliner refresh
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
PRAGMA_ENABLE_OPTIMIZATION
#endif

#undef LOCTEXT_NAMESPACE