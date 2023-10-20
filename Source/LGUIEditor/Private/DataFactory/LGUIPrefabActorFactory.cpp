// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabActorFactory.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "LGUIEditorTools.h"
#include "AssetRegistry/AssetData.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Utils/LGUIUtils.h"

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

	PrefabActor->PrefabAsset = Prefab;
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor != nullptr && PrefabActor->GetWorld() == SelectedActor->GetWorld())
	{
		LGUIEditorTools::MakeCurrentLevel(SelectedActor);
		auto ParentComp = SelectedActor->GetRootComponent();
		PrefabActor->LoadPrefab(ParentComp);
	}
	else
	{
		PrefabActor->LoadPrefab(nullptr);
	}
	PrefabActor->MoveActorToPrefabFolder();
	PrefabActor->SetFlags(EObjectFlags::RF_Transient);
	ULGUIEditorManagerObject::AddOneShotTickFunction([=]() {
		auto LoadedRootActor = PrefabActor->LoadedRootActor;
		PrefabActor->LoadedRootActor = nullptr;
		LGUIUtils::DestroyActorWithHierarchy(PrefabActor);
		//GEditor->SelectNone(true, true);
		GEditor->SelectActor(LoadedRootActor, true, true, false, true);
		});
}

void ULGUIPrefabActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if (Asset != NULL && CDO != NULL)
	{
		auto Prefab = CastChecked<ULGUIPrefab>(Asset);
		auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(CDO);

		PrefabActor->PrefabAsset = Prefab;
	}
}

UObject* ULGUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALGUIPrefabHelperActor>(ActorInstance);
	return PrefabActor->PrefabAsset;
}

#undef LOCTEXT_NAMESPACE
