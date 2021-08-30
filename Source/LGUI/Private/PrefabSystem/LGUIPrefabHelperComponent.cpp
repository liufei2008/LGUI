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

void ULGUIPrefabHelperComponent::RestoreSubPrefabs()
{
	for (auto actorGuidAndPrefab : PrefabAsset->SubPrefabs)
	{
		auto subPrefabActor = GetWorld()->SpawnActorDeferred<ALGUIPrefabActor>(ALGUIPrefabActor::StaticClass(), FTransform::Identity);
		if (IsValid(subPrefabActor))
		{
			int foundActorGuidIndex = AllLoadedActorsGuidArrayInPrefab.Find(actorGuidAndPrefab.Key);
			if (foundActorGuidIndex != INDEX_NONE)
			{
				auto subPrefabRootActor = AllLoadedActorArray[foundActorGuidIndex];
				subPrefabActor->SetActorLabel(subPrefabRootActor->GetActorLabel() + TEXT("_Prefab"));
				auto subPrefabComp = subPrefabActor->GetPrefabComponent();
				subPrefabComp->PrefabAsset = actorGuidAndPrefab.Value;
				subPrefabComp->ParentPrefab = Cast<ALGUIPrefabActor>(this->GetOwner());
				subPrefabComp->LoadPrefab();
				subPrefabActor->FinishSpawning(FTransform::Identity, true);

				subPrefabComp->MoveActorToPrefabFolder();
				subPrefabComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

				this->SubPrefabs.Add(subPrefabActor);
			}
		}
	}
}
FGuid ULGUIPrefabHelperComponent::GetGuidByActor(AActor* InActor, bool InIncludeSubPrefabs)
{
	auto foundIndex = AllLoadedActorArray.Find(InActor);
	if (foundIndex != INDEX_NONE)
	{
		return AllLoadedActorsGuidArrayInPrefab[foundIndex];
	}
	else
	{
		if (InIncludeSubPrefabs)
		{
			for (auto subPrefabItem : SubPrefabs)
			{
				auto guid = subPrefabItem->GetPrefabComponent()->GetGuidByActor(InActor, InIncludeSubPrefabs);
				if (guid.IsValid())
				{
					return guid;
				}
			}
		}
		return FGuid();
	}
}
int32 ULGUIPrefabHelperComponent::GetActorIndexFromCreatedActors(AActor* InActor, bool InIncludeSubPrefabs)
{
	int foundActorIndex = AllLoadedActorArray.Find(InActor);
	if (foundActorIndex != INDEX_NONE)
	{
		return foundActorIndex;
	}
	else
	{
		if (InIncludeSubPrefabs)
		{
			for (auto subPrefabItem : SubPrefabs)
			{
				foundActorIndex = subPrefabItem->GetPrefabComponent()->GetActorIndexFromCreatedActors(InActor, InIncludeSubPrefabs);
				if (foundActorIndex != INDEX_NONE)
				{
					return foundActorIndex;
				}
			}
		}
		return foundActorIndex;
	}
}
bool ULGUIPrefabHelperComponent::GetActorAndGuidsFromCreatedActors(AActor* InActor, bool InIncludeSubPrefabs, bool InRemoveFromList, FGuid& OutRemovedActorGuid)
{
	int foundActorIndex = AllLoadedActorArray.Find(InActor);
	if (foundActorIndex != INDEX_NONE)
	{
		OutRemovedActorGuid = AllLoadedActorsGuidArrayInPrefab[foundActorIndex];
		if (InRemoveFromList)
		{
			AllLoadedActorArray.RemoveAtSwap(foundActorIndex);
			AllLoadedActorsGuidArrayInPrefab.RemoveAtSwap(foundActorIndex);
		}
		return true;
	}
	else
	{
		if (InIncludeSubPrefabs)
		{
			for (auto subPrefabItem : SubPrefabs)
			{
				auto result = subPrefabItem->GetPrefabComponent()->GetActorAndGuidsFromCreatedActors(InActor, InIncludeSubPrefabs, InRemoveFromList, OutRemovedActorGuid);
				if (result)
				{
					return true;
				}
			}
		}
		return false;
	}
}
bool ULGUIPrefabHelperComponent::IsInsideSubPrefab(AActor* InActor)
{
	return AllLoadedActorArray.Find(InActor) == INDEX_NONE;
}

void ULGUIPrefabHelperComponent::LoadPrefab()
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		if (IsValid(ParentPrefab))
		{
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
				, ParentPrefab->GetPrefabComponent()->LoadedRootActor->GetRootComponent()
				, ParentPrefab->GetPrefabComponent()->AllLoadedActorArray, ParentPrefab->GetPrefabComponent()->AllLoadedActorsGuidArrayInPrefab
				, AllLoadedActorArray, AllLoadedActorsGuidArrayInPrefab);
		}
		else
		{
			auto ExistingActors = AllLoadedActorArray;
			auto ExistingActorsGuid = AllLoadedActorsGuidArrayInPrefab;
			LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
				, IsValid(ParentActorForEditor) ? ParentActorForEditor->GetRootComponent() : nullptr
				, ExistingActors, ExistingActorsGuid
				, AllLoadedActorArray, AllLoadedActorsGuidArrayInPrefab);
		}
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
void ULGUIPrefabHelperComponent::SavePrefab(bool InCreateOrApply, bool InIncludeOtherPrefabAsSubPrefab)
{
	if (PrefabAsset)
	{
		CleanupLoadedActors();
		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorsGuidArrayInPrefab;
		LGUIPrefabSystem::ActorSerializer::SavePrefab(LoadedRootActor, PrefabAsset
			, InCreateOrApply ? LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::CreateNew : LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::Apply
			, InIncludeOtherPrefabAsSubPrefab
			, this
			, ExistingActors, ExistingActorsGuid, AllLoadedActorArray, AllLoadedActorsGuidArrayInPrefab);
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
			CleanupLoadedActors();
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
#if WITH_EDITOR
		GEditor->RedrawAllViewports();
#endif
		for (auto subPrefabItem : SubPrefabs)
		{
			subPrefabItem->GetPrefabComponent()->DeleteThisInstance();
		}
		LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
		LGUIUtils::DestroyActorWithHierarchy(GetOwner(), false);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("This actor is not a prefab, ignore this action"));
	}
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
void ULGUIPrefabHelperComponent::CleanupLoadedActors()
{
	//Clear AllLoadedActorArray, remove it if not under root actor
	if (AllLoadedActorArray.Num() == AllLoadedActorsGuidArrayInPrefab.Num())
	{
		for (int i = AllLoadedActorArray.Num() - 1; i >= 0; i--)
		{
			if (!IsValid(AllLoadedActorArray[i])
				|| (!AllLoadedActorArray[i]->IsAttachedTo(LoadedRootActor) && AllLoadedActorArray[i] != LoadedRootActor)
				)
			{
				AllLoadedActorArray.RemoveAt(i);
				AllLoadedActorsGuidArrayInPrefab.RemoveAt(i);
			}
		}
	}
	else
	{
		AllLoadedActorArray.Empty();
		AllLoadedActorsGuidArrayInPrefab.Empty();
	}
}
#endif