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

AActor* ULGUIPrefabActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	return Super::SpawnActor(Asset, InLevel, Transform, InObjectFlags, Name);
}

void ULGUIPrefabActorFactory::PostSpawnActor(UObject* Asset, AActor* InNewActor)
{
	Super::PostSpawnActor(Asset, InNewActor);

	ULGUIPrefab* Prefab = CastChecked<ULGUIPrefab>(Asset);

	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(InNewActor);

	PrefabActor->SetPrefabAsset(Prefab);
	//PrefabComponent->SetRelativeRotation(FQuat::MakeFromEuler(FVector(-90, 0, 90)));
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	USceneComponent* ParentComp = nullptr;

	if (IsValid(SelectedActor))
	{
		LGUIEditorTools::MakeCurrentLevel(SelectedActor);
		ParentComp = SelectedActor->GetRootComponent();
	}
	PrefabActor->LoadPrefab(ParentComp);
	PrefabActor->LoadedRootActor = PrefabActor->LoadedRootActor;
}

void ULGUIPrefabActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		auto Prefab = CastChecked<ULGUIPrefab>(Asset);
		auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(CDO);

		PrefabActor->SetPrefabAsset(Prefab);
	}
}

UObject* ULGUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(ActorInstance);
	return PrefabActor->GetPrefabAsset();
}

#undef LOCTEXT_NAMESPACE
