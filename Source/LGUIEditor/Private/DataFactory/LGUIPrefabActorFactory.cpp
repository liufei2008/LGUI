// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabActorFactory.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperComponent.h"
#include "LGUIEditorTools.h"
#include "AssetData.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabActorFactory"


ULGUIPrefabActorFactory::ULGUIPrefabActorFactory()
{
	DisplayName = LOCTEXT("PrefabDisplayName", "Prefab");
	NewActorClass = ALGUIPrefabHelperActor::StaticClass();
	bShowInEditorQuickMenu = false;
	bUseSurfaceOrientation = false;
}

bool ULGUIPrefabActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(ULGUIPrefab::StaticClass()))
	{
		return true;
	}

	return false;
}

bool ULGUIPrefabActorFactory::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	ULGUIPrefab* Prefab = CastChecked<ULGUIPrefab>(Asset);

	if (Prefab == NULL)
	{
		return false;
	}
	return true;
}

void ULGUIPrefabActorFactory::PostSpawnActor(UObject* Asset, AActor* InNewActor)
{
	Super::PostSpawnActor(Asset, InNewActor);

	ULGUIPrefab* Prefab = CastChecked<ULGUIPrefab>(Asset);

	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(InNewActor);
	auto PrefabComponent = PrefabActor->GetPrefabComponent();
	check(PrefabComponent);

	PrefabComponent->SetPrefabAsset(Prefab);
	//PrefabComponent->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 90)));
	if (auto selectedActor = LGUIEditorTools::GetFirstSelectedActor())
	{
		PrefabComponent->ParentActorForEditor = selectedActor;
	}
}

void ULGUIPrefabActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		auto Prefab = CastChecked<ULGUIPrefab>(Asset);
		auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(CDO);
		auto PrefabComponent = PrefabActor->GetPrefabComponent();

		PrefabComponent->SetPrefabAsset(Prefab);
	}
}

UObject* ULGUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(ActorInstance);
	check(PrefabActor->GetPrefabComponent());
	return PrefabActor->GetPrefabComponent()->GetPrefabAsset();
}

#undef LOCTEXT_NAMESPACE
