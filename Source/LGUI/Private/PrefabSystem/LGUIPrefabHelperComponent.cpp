// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"
#include "PrefabSystem/ActorSerializer.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabActor.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif

#if WITH_EDITORONLY_DATA
FName ULGUIPrefabHelperComponent::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
#endif

ULGUIPrefabHelperComponent::ULGUIPrefabHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
#if WITH_EDITORONLY_DATA
	IdentityColor = FColor::MakeRandomColor();
#endif
}

void ULGUIPrefabHelperComponent::OnRegister()
{
	Super::OnRegister();
}
void ULGUIPrefabHelperComponent::OnUnregister()
{
	Super::OnUnregister();
}

#if WITH_EDITOR
void ULGUIPrefabHelperComponent::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
	this->GetOwner()->SetFolderPath(PrefabFolderName);
}
bool ULGUIPrefabHelperComponent::IsRootPrefab()const
{
	return !IsValid(ParentPrefab);
}

FGuid ULGUIPrefabHelperComponent::GetGuidByActor(AActor* InActor)
{
	auto foundIndex = AllLoadedActorArray.Find(InActor);
	if (foundIndex != INDEX_NONE)
	{
		return AllLoadedActorGuidArrayInPrefab[foundIndex];
	}
	else
	{
		return FGuid();
	}
}

bool ULGUIPrefabHelperComponent::IsActorBelongsToPrefab(AActor* InActor)
{
	return AllLoadedActorArray.Contains(InActor);
}
bool ULGUIPrefabHelperComponent::IsActorBelongsToSubPrefab(AActor* InActor)
{
	for (auto SubPrefabKeyValue : this->SubPrefabs)
	{
		auto SubPrefabComp = SubPrefabKeyValue.Value->GetPrefabComponent();
		bool contains = SubPrefabComp->IsActorBelongsToPrefab(InActor);
		if (contains)
		{
			return true;
		}
		else
		{
			return SubPrefabComp->IsActorBelongsToSubPrefab(InActor);
		}
	}
	return false;
}

ULGUIPrefabHelperComponent* ULGUIPrefabHelperComponent::GetSubPrefabWhichManageTheActor(AActor* InActor)
{
	for (auto SubPrefabKeyValue: SubPrefabs)
	{
		if (InActor->IsAttachedTo(SubPrefabKeyValue.Key))
		{
			return SubPrefabKeyValue.Value->GetPrefabComponent();
		}
	}
	check(0);//Always check IsActorBelongsToSubPrefab before this function.
	return nullptr;
}

#include "EngineUtils.h"
bool ULGUIPrefabHelperComponent::IsSubPrefabRootActor(AActor* InActor)
{
	for (TActorIterator<ALGUIPrefabActor> ActorItr(InActor->GetWorld()); ActorItr; ++ActorItr)
	{
		auto prefabActor = *ActorItr;
		if (IsValid(prefabActor))
		{
			if (InActor == prefabActor->GetPrefabComponent()->GetLoadedRootActor())
			{
				return true;
			}
		}
	}
	return false;
}

void ULGUIPrefabHelperComponent::LoadPrefab(USceneComponent* InParent)
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorGuidArrayInPrefab;
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
			, InParent ? InParent : (IsValid(ParentActorForEditor) ? ParentActorForEditor->GetRootComponent() : nullptr)
			//, ExistingActors, ExistingActorsGuid
			, [=](FGuid guid) 
			{
				int FoundActorIndex = AllLoadedActorGuidArrayInPrefab.Find(guid);
				if (FoundActorIndex != INDEX_NONE)
				{
					return AllLoadedActorArray[FoundActorIndex];
				}
				return (AActor*)nullptr;
			}
			, AllLoadedActorArray, AllLoadedActorGuidArrayInPrefab);

		RestoreSubPrefabs();
		ParentActorForEditor = nullptr;

		FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
		this->GetOwner()->SetFolderPath(PrefabFolderName);
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(LoadedRootActor, true, false);

		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;

		if (ULGUIEditorManagerObject::Instance != nullptr)
		{
			EditorTickDelegateHandle = ULGUIEditorManagerObject::Instance->EditorTick.AddUObject(this, &ULGUIPrefabHelperComponent::EditorTick);
		}
	}
}

void ULGUIPrefabHelperComponent::RestoreSubPrefabs()
{
	for (auto SubPrefabKeyValue : PrefabAsset->SubPrefabs)
	{
		auto SubPrefabRootActorGuid = SubPrefabKeyValue.Key;
		int SubPrefabRootActorIndex = AllLoadedActorGuidArrayInPrefab.Find(SubPrefabRootActorGuid);
		check(SubPrefabRootActorIndex != INDEX_NONE);
		auto SubPrefabRootActor = AllLoadedActorArray[SubPrefabRootActorIndex];
		ALGUIPrefabActor* SubPrefabActor = nullptr;
		if (SubPrefabs.Contains(SubPrefabRootActor))
		{
			SubPrefabActor = SubPrefabs[SubPrefabRootActor];
		}
		else
		{
			SubPrefabActor = GetWorld()->SpawnActor<ALGUIPrefabActor>(ALGUIPrefabActor::StaticClass(), FTransform::Identity);
		}
		SubPrefabActor->SetActorLabel(SubPrefabRootActor->GetActorLabel() + TEXT("_Prefab"));
		auto SubPrefabComp = SubPrefabActor->GetPrefabComponent();
		SubPrefabComp->PrefabAsset = SubPrefabKeyValue.Value.Prefab;
		SubPrefabComp->ParentPrefab = Cast<ALGUIPrefabActor>(this->GetOwner());
		SubPrefabComp->LoadSubPrefab(SubPrefabRootActor->GetRootComponent()->GetAttachParent(), SubPrefabKeyValue.Value.GuidFromPrefabToInstance);

		SubPrefabComp->MoveActorToPrefabFolder();
		SubPrefabComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

		this->SubPrefabs.Add(SubPrefabComp->LoadedRootActor, SubPrefabActor);
	}
}

void ULGUIPrefabHelperComponent::LoadSubPrefab(USceneComponent* InParent, TMap<FGuid, FGuid> InGuidFromPrefabToInstance)
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		check(ParentPrefab);
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
			, InParent
			//, ParentPrefab->GetPrefabComponent()->AllLoadedActorArray, ParentPrefab->GetPrefabComponent()->AllLoadedActorsGuidArrayInPrefab
			, [=](FGuid guid)
			{
				if (auto InstanceGuidPtr = InGuidFromPrefabToInstance.Find(guid))
				{
					int FoundActorIndex = ParentPrefab->GetPrefabComponent()->AllLoadedActorGuidArrayInPrefab.Find(*InstanceGuidPtr);
					if (FoundActorIndex != INDEX_NONE)
					{
						return ParentPrefab->GetPrefabComponent()->AllLoadedActorArray[FoundActorIndex];
					}
				}
				check(0);
				return (AActor*)nullptr;
			}
		, AllLoadedActorArray, AllLoadedActorGuidArrayInPrefab);

		FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
		this->GetOwner()->SetFolderPath(PrefabFolderName);
		GEditor->SelectNone(false, true);
		GEditor->SelectActor(LoadedRootActor, true, false);

		ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;

		if (ULGUIEditorManagerObject::Instance != nullptr)
		{
			EditorTickDelegateHandle = ULGUIEditorManagerObject::Instance->EditorTick.AddUObject(this, &ULGUIPrefabHelperComponent::EditorTick);
		}
	}
}
void ULGUIPrefabHelperComponent::SavePrefab(bool InCreateOrApply, bool InIncludeOtherPrefabAsSubPrefab)
{
	if (PrefabAsset)
	{
		CleanupPrefabAndActor();
		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorGuidArrayInPrefab;
		LGUIPrefabSystem::ActorSerializer::SavePrefab(LoadedRootActor, PrefabAsset
			, InCreateOrApply ? LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::CreateNew : LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::Apply
			, InIncludeOtherPrefabAsSubPrefab
			, this
			, ExistingActors, ExistingActorsGuid, AllLoadedActorArray, AllLoadedActorGuidArrayInPrefab);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ULGUIPrefabHelperComponent::RevertPrefab()
{
	if (IsValid(PrefabAsset))
	{
		AActor* OldParentActor = nullptr;
		bool haveRootTransform = true;
		FTransform OldTransform;
		FUIWidget OldWidget;
		//store root transform
		if (IsValid(LoadedRootActor))
		{
			OldParentActor = LoadedRootActor->GetAttachParentActor();
			if(auto RootComp = LoadedRootActor->GetRootComponent())
			{
				haveRootTransform = true;
				OldTransform = LoadedRootActor->GetRootComponent()->GetRelativeTransform();
				if (auto RootUIComp = Cast<UUIItem>(RootComp))
				{
					OldWidget = RootUIComp->GetWidget();
				}
			}
		}
		//create new actor
		{
			CleanupPrefabAndActor();
			LoadedRootActor = nullptr;
			LoadPrefab();
		}

		if (IsValid(LoadedRootActor))
		{
			LoadedRootActor->AttachToActor(OldParentActor, FAttachmentTransformRules::KeepRelativeTransform);
			if (haveRootTransform)
			{
				if (auto RootComp = LoadedRootActor->GetRootComponent())
				{
					RootComp->SetRelativeTransform(OldTransform);
					if (auto RootUIItem = Cast<UUIItem>(RootComp))
					{
						auto newWidget = RootUIItem->GetWidget();
						OldWidget.color = newWidget.color;
						OldWidget.depth = newWidget.depth;
						RootUIItem->SetWidget(OldWidget);
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

void ULGUIPrefabHelperComponent::DeleteThisInstance()
{
	if (LoadedRootActor)
	{
		for (auto SubPrefabKeyValue : SubPrefabs)
		{
			if (IsValid(SubPrefabKeyValue.Value))
			{
				SubPrefabKeyValue.Value->GetPrefabComponent()->DeleteThisInstance();
			}
		}
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("This actor is not a prefab, ignore this action"));
	}
	LGUIUtils::DestroyActorWithHierarchy(GetOwner(), false);
}

void ULGUIPrefabHelperComponent::RemoveEditorTickDelegate()
{
	if (ULGUIEditorManagerObject::Instance != nullptr)
	{
		if (EditorTickDelegateHandle.IsValid())
		{
			ULGUIEditorManagerObject::Instance->EditorTick.Remove(EditorTickDelegateHandle);
		}
	}
}
void ULGUIPrefabHelperComponent::EditorTick(float DeltaTime)
{
	GEditor->SelectActor(this->GetOwner(), false, true);
	RemoveEditorTickDelegate();
}
void ULGUIPrefabHelperComponent::CleanupPrefabAndActor()
{
	//Clear sub prefabs
	TArray<AActor*> NeedToDelete;
	for (auto KeyValue : SubPrefabs)
	{
		if (!IsValid(KeyValue.Key) || !IsValid(KeyValue.Value))
		{
			NeedToDelete.Add(KeyValue.Key);
		}
	}
	for (auto Item : NeedToDelete)
	{
		SubPrefabs.Remove(Item);
	}
	//Cleanup AllLoadedActorArray, remove it if not under root actor, remove it if not belong to prefab or sub prefab
	if (AllLoadedActorArray.Num() == AllLoadedActorGuidArrayInPrefab.Num())
	{
		for (int i = AllLoadedActorArray.Num() - 1; i >= 0; i--)
		{
			auto ActorItem = AllLoadedActorArray[i];
			if (!IsValid(ActorItem)//not valid
				|| (!ActorItem->IsAttachedTo(LoadedRootActor) && ActorItem != LoadedRootActor)//not attached to this prefab
				|| (!IsActorBelongsToPrefab(ActorItem) && !IsActorBelongsToSubPrefab(ActorItem))//not belong to this prefab or sub prefab
				)
			{
				AllLoadedActorArray.RemoveAt(i);
				AllLoadedActorGuidArrayInPrefab.RemoveAt(i);
			}
		}
	}
	else//size not equal, maybe old prefab
	{
		AllLoadedActorArray.Empty();
		AllLoadedActorGuidArrayInPrefab.Empty();
	}
}
#endif