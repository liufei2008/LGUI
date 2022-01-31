// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	bool bCanLoadPrefab = true;
	auto SelectedActor = LGUIEditorTools::GetFirstSelectedActor();
	if (SelectedActor != nullptr)
	{
		if (auto PrefabHelperObject = LGUIEditorTools::GetPrefabHelperObject_WhichManageThisActor(SelectedActor))
		{
			if (PrefabHelperObject->IsActorBelongsToSubPrefab(SelectedActor))
			{
				bCanLoadPrefab = false;
			}
		}
	}
	if (!bCanLoadPrefab)
	{
		return false;
	}

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
