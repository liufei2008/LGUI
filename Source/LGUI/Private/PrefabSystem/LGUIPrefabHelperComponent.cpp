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
void ULGUIPrefabHelperComponent::LoadPrefab()
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorsGuidArrayInPrefab;
		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
			, IsValid(ParentActorForEditor) ? ParentActorForEditor->GetRootComponent()
			: nullptr, ExistingActors, ExistingActorsGuid, AllLoadedActorArray, AllLoadedActorsGuidArrayInPrefab);
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
void ULGUIPrefabHelperComponent::SavePrefab(bool InCreateOrApply)
{
	if (PrefabAsset)
	{
		AActor* Target = nullptr;
		if (GetOwner()->GetClass()->IsChildOf(ALGUIPrefabActor::StaticClass()) && this == GetOwner()->GetRootComponent())//if this component is created from LGUIPrefabActor
		{
			Target = LoadedRootActor;
		}
		else//if this component is attached to Actor, then consider that actor as prefab root
		{
			Target = GetOwner();
		}

		CleanupLoadedActors();
		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorsGuidArrayInPrefab;
		LGUIPrefabSystem::ActorSerializer::SavePrefab(Target, PrefabAsset
			, InCreateOrApply ? LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::CreateNew : LGUIPrefabSystem::ActorSerializer::EPrefabSerializeMode::Apply
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