// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/ActorSerializer3.h"
#if WITH_EDITOR
#include "Editor.h"
#include "EditorActorFolders.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EngineUtils.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIPrefabHelperActor"

// Sets default values
ALGUIPrefabHelperActor::ALGUIPrefabHelperActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;
}

void ALGUIPrefabHelperActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALGUIPrefabHelperActor::Destroyed()
{
	Super::Destroyed();
#if WITH_EDITORONLY_DATA
	if (bAutoDestroyLoadedActors)
	{
		if (IsValid(LoadedRootActor))
		{
			if (auto PrefabManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel()))
			{
				PrefabManagerActor->PrefabHelperObject->RemoveSubPrefabByAnyActorOfSubPrefab(LoadedRootActor);
				LGUIUtils::DestroyActorWithHierarchy(LoadedRootActor, true);
			}
		}
	}
#endif
}
void ALGUIPrefabHelperActor::BeginDestroy()
{
	Super::BeginDestroy();
}

#if WITH_EDITORONLY_DATA
FName ALGUIPrefabHelperActor::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
#endif

#if WITH_EDITOR
void ALGUIPrefabHelperActor::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), FFolder(FFolder::FRootObject(this), PrefabFolderName));
	this->SetFolderPath(PrefabFolderName);
}

void ALGUIPrefabHelperActor::LoadPrefab(USceneComponent* InParent)
{
	if (this->GetWorld() != nullptr && this->GetWorld()->IsGameWorld())return;
	if (IsValid(LoadedRootActor))return;
	auto PrefabHelperObject = ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel())->PrefabHelperObject;
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
#endif


ALGUIPrefabManagerActor::ALGUIPrefabManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	PrefabHelperObject = CreateDefaultSubobject<ULGUIPrefabHelperObject>(TEXT("PrefabHelper"));
#endif
}

#if WITH_EDITOR

TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALGUIPrefabManagerActor>> ALGUIPrefabManagerActor::MapLevelToManagerActor;

ALGUIPrefabManagerActor* ALGUIPrefabManagerActor::GetPrefabManagerActor(ULevel* InLevel, bool CreateIfNotExist)
{
	if (!MapLevelToManagerActor.Contains(InLevel) && CreateIfNotExist)
	{
		auto PrefabManagerActor = InLevel->GetWorld()->SpawnActor<ALGUIPrefabManagerActor>();
		MapLevelToManagerActor.Add(InLevel, PrefabManagerActor);
		InLevel->MarkPackageDirty();
		FActorFolders::Get().CreateFolder(*PrefabManagerActor->GetWorld(), FFolder(FFolder::FRootObject(PrefabManagerActor), ALGUIPrefabHelperActor::PrefabFolderName));
		PrefabManagerActor->SetFolderPath(ALGUIPrefabHelperActor::PrefabFolderName);
	}
	if (auto ResultPtr = MapLevelToManagerActor.Find(InLevel))
	{
		if (ResultPtr->IsValid())
		{
			if (auto World = ResultPtr->Get()->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					(*ResultPtr)->PrefabHelperObject->MarkAsManagerObject();//can only manage prefab in edit mode
				}
			}
			return ResultPtr->Get();
		}
		else
		{
			MapLevelToManagerActor.Remove(InLevel);
			return nullptr;
		}
	}
	else
	{
		return nullptr;
	}
}

ALGUIPrefabManagerActor* ALGUIPrefabManagerActor::GetPrefabManagerActorByPrefabHelperObject(ULGUIPrefabHelperObject* InHelperObject)
{
	for (auto& KeyValue : MapLevelToManagerActor)
	{
		if (KeyValue.Value->PrefabHelperObject == InHelperObject)
		{
			return KeyValue.Value.Get();
		}
	}
	return nullptr;
}

void ALGUIPrefabManagerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALGUIPrefabManagerActor::PostInitProperties()
{
	Super::PostInitProperties();
	if (this != GetDefault<ALGUIPrefabManagerActor>())
	{
		CollectWhenCreate();
	}
}

void ALGUIPrefabManagerActor::CollectWhenCreate()
{
	if (auto Level = this->GetLevel())
	{
		if (!MapLevelToManagerActor.Contains(Level))
		{
			MapLevelToManagerActor.Add(Level, this);
		}
		OnSubPrefabNewVersionUpdatedDelegateHandle = PrefabHelperObject->OnSubPrefabNewVersionUpdated.AddLambda([Actor = MakeWeakObjectPtr(this)]() {
			if (Actor.IsValid())
			{
				Actor->MarkPackageDirty();
			}
		});
	}
	ULGUIEditorManagerObject::AddOneShotTickFunction([Actor = MakeWeakObjectPtr(this)]{
		if (Actor.IsValid())
		{
			if (auto World = Actor->GetWorld())
			{
				if (!World->IsGameWorld())
				{
					Actor->PrefabHelperObject->CheckPrefabVersion();
				}
			}
		}
		}, 1);

	BeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddLambda([Actor = MakeWeakObjectPtr(this)](const bool isSimulating) {
		if (Actor.IsValid())
		{
			Actor->PrefabHelperObject->DismissAllVersionNotifications();
		}
	});
}

void ALGUIPrefabManagerActor::PostActorCreated()
{
	Super::PostActorCreated();
}

void ALGUIPrefabManagerActor::BeginDestroy()
{
	Super::BeginDestroy();
	CleanupWhenDestroy();
}
void ALGUIPrefabManagerActor::Destroyed()
{
	Super::Destroyed();
	CleanupWhenDestroy();
	if (!this->GetWorld()->IsGameWorld())
	{
		ULGUIEditorManagerObject::AddOneShotTickFunction([Actor = this, World = this->GetWorld()]() {
			auto InfoText = LOCTEXT("DeleteLGUIPrefabManagerActor", "\
LGUIPrefabManagerActor is being destroyed!\
\nThis actor is responsible for managing LGUI-Prefabs in current level, if you delete it then all LGUI-Prefabs linked in the level will be lost!\
\nCilck OK to confirm delete, or Cancel to undo it.");
			auto Return = FMessageDialog::Open(EAppMsgType::OkCancel, InfoText);
			if (Return == EAppReturnType::Cancel)
			{
				GEditor->UndoTransaction(false);
				Actor->CollectWhenCreate();
			}
			else if (Return == EAppReturnType::Ok)
			{
				//cleanup LGUIPrefabHelperActor
				for (TActorIterator<ALGUIPrefabHelperActor> ActorItr(World); ActorItr; ++ActorItr)
				{
					auto PrefabActor = *ActorItr;
					if (IsValid(PrefabActor))
					{
						PrefabActor->bAutoDestroyLoadedActors = false;
						LGUIUtils::DestroyActorWithHierarchy(PrefabActor, false);
					}
				}
				for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
				{
					Itr->CleanupInvalidSubPrefab();
				}
			}
			}, 1);
	}
}
void ALGUIPrefabManagerActor::CleanupWhenDestroy()
{
	if (OnSubPrefabNewVersionUpdatedDelegateHandle.IsValid())
	{
		PrefabHelperObject->OnSubPrefabNewVersionUpdated.Remove(OnSubPrefabNewVersionUpdatedDelegateHandle);
	}
	if (BeginPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(BeginPIEDelegateHandle);
	}
	if (this != GetDefault<ALGUIPrefabManagerActor>())
	{
		if (this->GetWorld() != nullptr && !this->GetWorld()->IsGameWorld())
		{
			auto Level = this->GetLevel();
			if (MapLevelToManagerActor.Contains(Level))
			{
				MapLevelToManagerActor.Remove(Level);
			}
		}
	}
	//cleanup
	{
		MapLevelToManagerActor.Remove(nullptr);
		TSet<TWeakObjectPtr<ULevel>> ToClear;
		for (auto& KeyValue : MapLevelToManagerActor)
		{
			if (!KeyValue.Key.IsValid() && !KeyValue.Value.IsValid())
			{
				ToClear.Add(KeyValue.Key.Get());
			}
		}
		for (auto& Item : ToClear)
		{
			MapLevelToManagerActor.Remove(Item);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE