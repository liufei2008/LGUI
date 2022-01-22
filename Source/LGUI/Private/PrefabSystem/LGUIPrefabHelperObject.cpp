// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUI.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Utils/LGUIUtils.h"
#include "GameFramework/Actor.h"
#include "PrefabSystem/ActorSerializer3.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabManager"
PRAGMA_DISABLE_OPTIMIZATION

ULGUIPrefabHelperObject::ULGUIPrefabHelperObject()
{
	
}

#if WITH_EDITOR
void ULGUIPrefabHelperObject::BeginDestroy()
{
	ClearLoadedPrefab();
	Super::BeginDestroy();
#if WITH_EDITORONLY_DATA
	bCanNotifyDetachment = false;
#endif
}

void ULGUIPrefabHelperObject::MarkAsManagerObject()
{
	if (bIsMarkedAsManagerObject)return;
	bIsMarkedAsManagerObject = true;
	ULGUIEditorManagerObject::AddOneShotTickFunction([Actor = MakeWeakObjectPtr(this)]{
		if (Actor.IsValid())
		{
			GEditor->OnLevelActorAttached().AddUObject(Actor.Get(), &ULGUIPrefabHelperObject::OnLevelActorAttached);
			GEditor->OnLevelActorDetached().AddUObject(Actor.Get(), &ULGUIPrefabHelperObject::OnLevelActorDetached);
			GEditor->OnLevelActorDeleted().AddUObject(Actor.Get(), &ULGUIPrefabHelperObject::OnLevelActorDeleted);
			Actor->bCanNotifyDetachment = true;
		}
		}, 1);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &ULGUIPrefabHelperObject::OnObjectPropertyChanged);
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddUObject(this, &ULGUIPrefabHelperObject::OnPreObjectPropertyChanged);
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
		for (auto& ObjectGuid : ObjectsToIgnore)
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

void ULGUIPrefabHelperObject::RemoveAllMemberPropertyFromSubPrefab(AActor* InSubPrefabActor)
{
	if (!IsValid(InSubPrefabActor))return;
	AActor* SubPrefabRootActor = nullptr;
	for (auto& KeyValue : SubPrefabMap)
	{
		if (InSubPrefabActor == KeyValue.Key || InSubPrefabActor->IsAttachedTo(KeyValue.Key))
		{
			SubPrefabRootActor = KeyValue.Key;
		}
	}
	if (SubPrefabRootActor != nullptr)
	{
		SubPrefabMap.Remove(SubPrefabRootActor);
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

AActor* ULGUIPrefabHelperObject::GetSubPrefabRootActor(AActor* InSubPrefabActor)
{
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
		bAnythingDirty = false;
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




bool ULGUIPrefabHelperObject::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab, AActor* InSubPrefabRootActor)
{
	bCanNotifyDetachment = false;
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
		if (SubPrefabData.PrefabAsset == InSubPrefab
			&& (InSubPrefabRootActor != nullptr ? SubPrefabRootActor == InSubPrefabRootActor : true)
			)
		{
			//store override parameter to data
			LGUIPrefabSystem3::ActorSerializer3 serailizer;
			auto OverrideData = serailizer.SaveOverrideParameterToData(SubPrefabData.ObjectOverrideParameterArray);

			TArray<AActor*> ChildrenActors;
			LGUIUtils::CollectChildrenActors(SubPrefabRootActor, ChildrenActors);

			TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
			for (auto& GuidToObject : this->MapGuidToObject)
			{
				if (auto ObjectGuidInSubPrefabPtr = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Find(GuidToObject.Key))
				{
					SubPrefabMapGuidToObject.Add(*ObjectGuidInSubPrefabPtr, GuidToObject.Value);
				}
			}

			//Because prefab serailize with delta, so if use prefab's default serialized data then default value will not be set in parent prefab. So we need to make a agent prefab with fully serialized properties include default value.
			auto AgentSubPrefab = InSubPrefab->MakeAgentFullSerializedPrefab();
			TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
			AgentSubPrefab->LoadPrefabWithExistingObjects(GetPrefabWorld()
				, SubPrefabRootActor->GetAttachParentActor()->GetRootComponent()
				, SubPrefabMapGuidToObject, SubSubPrefabMap
				, false
			);
			AgentSubPrefab->ConditionalBeginDestroy();

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
			serailizer.RestoreOverrideParameterFromData(OverrideData, SubPrefabData.ObjectOverrideParameterArray);

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
		this->PrefabAsset->MarkPackageDirty();
	}
	ULGUIEditorManagerObject::RefreshAllUI();
	bCanNotifyDetachment = true;
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
		bAnythingDirty = true;

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
		if (PropertyActor)//only allow actor or component's member property
		{
			auto PropertyName = InMemberProperty->GetFName();
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
	if (!bCanNotifyDetachment)return;
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
			check(0);//why this happed?
		}
	}
}
void ULGUIPrefabHelperObject::OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom)
{
	if (!bCanNotifyDetachment)return;
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
	auto ActorBelongsToPrefab = false;
	for (auto& KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value == Actor)
		{
			ActorBelongsToPrefab = true;
		}
	}
	if (ActorBelongsToPrefab//Cannot delete sub prefab's actor
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
}

void ULGUIPrefabHelperObject::CheckAttachment()
{
	if (!bCanNotifyDetachment)return;
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
		AttachmentActor = FAttachmentActorStruct();
	}
	break;
	case EAttachementError::CannotRestructurePrefabInstance:
	{
		auto InfoText = LOCTEXT("CannotRestructurePrefaInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
		GEditor->UndoTransaction(false);
		AttachmentActor = FAttachmentActorStruct();
	}
	break;
	}
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

				bAnythingDirty = true;
			}
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
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
			bAnythingDirty = true;
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
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
			if (i == 0)
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

		bAnythingDirty = true;
		GEditor->EndTransaction();
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

				bAnythingDirty = true;
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
			bAnythingDirty = true;
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
}
void ULGUIPrefabHelperObject::ApplyAllOverrideToPrefab(UObject* InObject)
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

		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll", "Apply Prefab Override"));
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
			if (i == 0)
			{
				if (auto UIItem = Cast<UUIItem>(SourceObject))
				{
					FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
				}
			}
			auto OriginObject = FindOriginObjectInSourcePrefab(SourceObject);
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
		RemoveAllMemberPropertyFromSubPrefab(Actor);
		//save origin prefab
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
		}

		bAnythingDirty = true;
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
#pragma endregion RevertAndApply

void ULGUIPrefabHelperObject::MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, TMap<FGuid, UObject*> InSubMapGuidToObject)
{
	FLGUISubPrefabData SubPrefabData;
	SubPrefabData.PrefabAsset = InPrefab;

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
	//mark AnchorData, RelativeLocation to default override parameter
	auto RootComp = InActor->GetRootComponent();
	auto RootUIComp = Cast<UUIItem>(RootComp);
	AddMemberPropertyToSubPrefab(InActor, RootComp, USceneComponent::GetRelativeLocationPropertyName());
	if (RootUIComp)
	{
		AddMemberPropertyToSubPrefab(InActor, RootUIComp, UUIItem::GetHierarchyIndexPropertyName());
		AddMemberPropertyToSubPrefab(InActor, RootUIComp, UUIItem::GetAnchorDataPropertyName());
	}

	bAnythingDirty = true;
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

#endif
PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE