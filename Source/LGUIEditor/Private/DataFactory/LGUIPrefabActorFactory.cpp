// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DataFactory/LGUIPrefabActorFactory.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabLevelManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "LGUIEditorTools.h"
#include "AssetRegistry/AssetData.h"
#include "Utils/LGUIUtils.h"
#include "Editor.h"
#include "EditorActorFolders.h"


#define LOCTEXT_NAMESPACE "LGUIPrefabActorFactory"


ULGUIPrefabActorFactory::ULGUIPrefabActorFactory()
{
	DisplayName = LOCTEXT("PrefabDisplayName", "Prefab");
	NewActorClass = ALGUIPrefabLoadHelperActor::StaticClass();
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

	auto PrefabActor = CastChecked<ALGUIPrefabLoadHelperActor>(InNewActor);

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
	ULGUIPrefabManagerObject::AddOneShotTickFunction([=]() {
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
		auto PrefabActor = CastChecked<ALGUIPrefabLoadHelperActor>(CDO);

		PrefabActor->PrefabAsset = Prefab;
	}
}

UObject* ULGUIPrefabActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	check(ActorInstance->IsA(NewActorClass));
	auto PrefabActor = CastChecked<ALGUIPrefabLoadHelperActor>(ActorInstance);
	return PrefabActor->PrefabAsset;
}

#undef LOCTEXT_NAMESPACE



// Sets default values
ALGUIPrefabLoadHelperActor::ALGUIPrefabLoadHelperActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
}

void ALGUIPrefabLoadHelperActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALGUIPrefabLoadHelperActor::Destroyed()
{
	Super::Destroyed();
	if (bAutoDestroyLoadedActors)
	{
		if (IsValid(LoadedRootActor))
		{
			if (auto PrefabManagerActor = ALGUIPrefabLevelManagerActor::GetInstance(this->GetLevel()))
			{
				PrefabManagerActor->PrefabHelperObject->RemoveSubPrefabByAnyActorOfSubPrefab(LoadedRootActor);
				LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
			}
		}
	}
}
void ALGUIPrefabLoadHelperActor::BeginDestroy()
{
	Super::BeginDestroy();
}

void ALGUIPrefabLoadHelperActor::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), FFolder(FFolder::FRootObject(this), ALGUIPrefabLevelManagerActor::PrefabFolderName));
	this->SetFolderPath(ALGUIPrefabLevelManagerActor::PrefabFolderName);
}

void ALGUIPrefabLoadHelperActor::LoadPrefab(USceneComponent* InParent)
{
	if (this->GetWorld() != nullptr && this->GetWorld()->IsGameWorld())return;
	if (IsValid(LoadedRootActor))return;
	auto PrefabHelperObject = ALGUIPrefabLevelManagerActor::GetInstance(this->GetLevel())->PrefabHelperObject;
	PrefabHelperObject->SetCanNotifyAttachment(false);
	TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
	TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubSubPrefabMap;
	LoadedRootActor = PrefabAsset->LoadPrefabWithExistingObjects(this->GetWorld()
		, InParent
		, SubPrefabMapGuidToObject, SubSubPrefabMap
	);
	PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedRootActor, SubPrefabMapGuidToObject, {});
	PrefabHelperObject->SetCanNotifyAttachment(true);
}

