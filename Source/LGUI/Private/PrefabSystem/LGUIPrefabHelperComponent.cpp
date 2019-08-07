// Copyright 2019 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUI.h"
#include "PrefabSystem/ActorSerializer.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabActor.h"
#include "Core/ActorComponent/UIItem.h"
#if WITH_EDITOR
#include "Editor.h"
#endif


ULGUIPrefabHelperComponent::ULGUIPrefabHelperComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void ULGUIPrefabHelperComponent::BeginPlay()
{
	Super::BeginPlay();
}
void ULGUIPrefabHelperComponent::OnRegister()
{
	Super::OnRegister();
}

#if WITH_EDITOR
void ULGUIPrefabHelperComponent::LoadPrefab()
{
	if (!LoadedRootActor)
	{
		LoadedRootActor = ActorSerializer::LoadPrefabForEdit(this->GetWorld(), PrefabAsset
			, IsValid(ParentActorForEditor) ? ParentActorForEditor->GetRootComponent() 
			: this->GetOwner()->GetRootComponent(), AllLoadedActorArray);
		GEditor->SelectActor(LoadedRootActor, true, false);
		ParentActorForEditor = nullptr;
	}
}

void ULGUIPrefabHelperComponent::SavePrefab()
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
		ActorSerializer serializer(this->GetWorld());
		serializer.SerializeActor(Target, PrefabAsset);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("PrefabAsset is null, please create a LGUIPrefab asset and assign to PrefabAsset"));
	}
}

void ULGUIPrefabHelperComponent::RevertPrefab()
{
	if (PrefabAsset)
	{
		auto OldParentActor = LoadedRootActor->GetAttachParentActor();
		bool haveRootTransform = LoadedRootActor->GetRootComponent() != nullptr;
		FTransform OldTransform;
		if (haveRootTransform)
		{
			OldTransform = LoadedRootActor->GetRootComponent()->GetRelativeTransform();
		}
		LGUIUtils::DeleteActor(LoadedRootActor, true);
		LoadedRootActor = nullptr;
		LoadPrefab();
		if (LoadedRootActor)
		{
			LoadedRootActor->AttachToActor(OldParentActor, FAttachmentTransformRules::KeepRelativeTransform);
			if (haveRootTransform)
			{
				if (auto rootComp = LoadedRootActor->GetRootComponent())
				{
					rootComp->SetRelativeTransform(OldTransform);
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
		LGUIUtils::DeleteActor(LoadedRootActor, true);
		LGUIUtils::DeleteActor(GetOwner(), false);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("This actor is not a prefab, ignore this action"));
	}
}
#endif