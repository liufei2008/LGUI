// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabActorFactory.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
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

	PrefabActor->PrefabHelperObject->PrefabAsset = Prefab;
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (PrefabActor->GetWorld() == SelectedActor->GetWorld())
	{
		USceneComponent* ParentComp = nullptr;

		if (IsValid(SelectedActor))
		{
			LGUIEditorTools::MakeCurrentLevel(SelectedActor);
			ParentComp = SelectedActor->GetRootComponent();
		}
		PrefabActor->LoadPrefab(ParentComp);
	}
	else
	{
		PrefabActor->LoadPrefab(nullptr);
	}
	PrefabActor->MoveActorToPrefabFolder();
}

void ULGUIPrefabActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		auto Prefab = CastChecked<ULGUIPrefab>(Asset);
		auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(CDO);

		PrefabActor->PrefabHelperObject->PrefabAsset = Prefab;
	}
}

UObject* ULGUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(ActorInstance);
	return PrefabActor->PrefabHelperObject->PrefabAsset;
}

#undef LOCTEXT_NAMESPACE
