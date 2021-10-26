// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"
#include "PrefabSystem/ActorSerializer.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/Actor/LGUIManagerActor.h"
#endif

#if WITH_EDITORONLY_DATA
FName ULGUIPrefabHelperComponent::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
TArray<FColor> ULGUIPrefabHelperComponent::AllColors;
#endif

ULGUIPrefabHelperComponent::ULGUIPrefabHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bIsEditorOnly = true;
}


void ULGUIPrefabHelperComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITORONLY_DATA
	if (AllColors.Num() == 0)
	{
		int ColorCount = 10;
		float Interval = 1.0f / ColorCount;
		float StartHue01 = 0;
		for (int i = 0; i < ColorCount; i++)
		{
			auto Hue = (uint8)(StartHue01 * 255);
			auto Color1 = FLinearColor::MakeFromHSV8(Hue, 255, 255).ToFColor(false);
			AllColors.Add(Color1);
			auto Color2 = FLinearColor::MakeFromHSV8(Hue, 255, 128).ToFColor(false);
			AllColors.Add(Color2);
			StartHue01 += Interval;
		}
	}

	{
		if (AllColors.Num() == 0)
		{
			IdentityColor = FColor::MakeRandomColor();
			IsRandomColor = true;
		}
		else
		{
			int RandomIndex = FMath::RandRange(0, AllColors.Num() - 1);
			IdentityColor = AllColors[RandomIndex];
			AllColors.RemoveAt(RandomIndex);
			IsRandomColor = false;
		}
	}
#endif
}
void ULGUIPrefabHelperComponent::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITORONLY_DATA
	{
		if (!IsRandomColor)
		{
			if (!AllColors.Contains(IdentityColor))
			{
				AllColors.Add(IdentityColor);
			}
		}
	}
#endif
}

#if WITH_EDITOR
void ULGUIPrefabHelperComponent::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
	this->GetOwner()->SetFolderPath(PrefabFolderName);
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

void ULGUIPrefabHelperComponent::LoadPrefab(USceneComponent* InParent)
{
	if (!IsValid(LoadedRootActor))
	{
		ULGUIEditorManagerObject::CanExecuteSelectionConvert = false;

		LoadedRootActor = LGUIPrefabSystem::ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
			, InParent ? InParent : (IsValid(ParentActorForEditor) ? ParentActorForEditor->GetRootComponent() : nullptr)
			, [=](FGuid guid) 
			{
				int FoundActorIndex = AllLoadedActorGuidArrayInPrefab.Find(guid);
				if (FoundActorIndex != INDEX_NONE)
				{
					return AllLoadedActorArray[FoundActorIndex];
				}
				return (AActor*)nullptr;
			}
			, nullptr
			, AllLoadedActorArray, AllLoadedActorGuidArrayInPrefab);

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

void ULGUIPrefabHelperComponent::SavePrefab(bool InIncludeOtherPrefabAsSubPrefab)
{
	if (PrefabAsset)
	{
		CleanupPrefabAndActor();
		auto ExistingActors = AllLoadedActorArray;
		auto ExistingActorsGuid = AllLoadedActorGuidArrayInPrefab;
		LGUIPrefabSystem::ActorSerializer::SavePrefab(LoadedRootActor, PrefabAsset
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
	//Cleanup AllLoadedActorArray, remove it if not under root actor, remove it if not belong to prefab or sub prefab
	if (AllLoadedActorArray.Num() == AllLoadedActorGuidArrayInPrefab.Num())
	{
		for (int i = AllLoadedActorArray.Num() - 1; i >= 0; i--)
		{
			auto ActorItem = AllLoadedActorArray[i];
			if (!IsValid(ActorItem)//not valid
				|| (!ActorItem->IsAttachedTo(LoadedRootActor) && ActorItem != LoadedRootActor)//not attached to this prefab
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