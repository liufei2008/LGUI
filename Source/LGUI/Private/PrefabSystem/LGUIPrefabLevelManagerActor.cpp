// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabLevelManagerActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#if WITH_EDITOR
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "EditorActorFolders.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIPrefabHelperActor"


ALGUIPrefabLevelManagerActor::ALGUIPrefabLevelManagerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bIsEditorOnlyActor = true;

#if WITH_EDITORONLY_DATA
	PrefabHelperObject = CreateDefaultSubobject<ULGUIPrefabHelperObject>(TEXT("PrefabHelper"));
#endif
}

#if WITH_EDITOR

FName ALGUIPrefabLevelManagerActor::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
TMap<TWeakObjectPtr<ULevel>, TWeakObjectPtr<ALGUIPrefabLevelManagerActor>> ALGUIPrefabLevelManagerActor::MapLevelToManagerActor;

ALGUIPrefabLevelManagerActor* ALGUIPrefabLevelManagerActor::GetPrefabManagerActor(ULevel* InLevel, bool CreateIfNotExist)
{
	if (!MapLevelToManagerActor.Contains(InLevel) && CreateIfNotExist)
	{
		auto PrefabManagerActor = InLevel->GetWorld()->SpawnActor<ALGUIPrefabLevelManagerActor>();
		MapLevelToManagerActor.Add(InLevel, PrefabManagerActor);
		InLevel->MarkPackageDirty();
		FActorFolders::Get().CreateFolder(*PrefabManagerActor->GetWorld(), FFolder(FFolder::FRootObject(PrefabManagerActor), ALGUIPrefabLevelManagerActor::PrefabFolderName));
		PrefabManagerActor->SetFolderPath(ALGUIPrefabLevelManagerActor::PrefabFolderName);
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

ALGUIPrefabLevelManagerActor* ALGUIPrefabLevelManagerActor::GetPrefabManagerActorByPrefabHelperObject(ULGUIPrefabHelperObject* InHelperObject)
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

void ALGUIPrefabLevelManagerActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALGUIPrefabLevelManagerActor::PostInitProperties()
{
	Super::PostInitProperties();
	if (this != GetDefault<ALGUIPrefabLevelManagerActor>())
	{
		CollectWhenCreate();
	}
}

void ALGUIPrefabLevelManagerActor::CollectWhenCreate()
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
	ULGUIPrefabManagerObject::AddOneShotTickFunction([Actor = MakeWeakObjectPtr(this)]{
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

void ALGUIPrefabLevelManagerActor::PostActorCreated()
{
	Super::PostActorCreated();
}

void ALGUIPrefabLevelManagerActor::BeginDestroy()
{
	Super::BeginDestroy();
	CleanupWhenDestroy();
}
void ALGUIPrefabLevelManagerActor::Destroyed()
{
	Super::Destroyed();
	CleanupWhenDestroy();
	if (!this->GetWorld()->IsGameWorld())
	{
		ULGUIPrefabManagerObject::AddOneShotTickFunction([Actor = this, World = this->GetWorld()]() {
			auto InfoText = LOCTEXT("DeleteLGUIPrefabLevelManagerActor", "\
LGUIPrefabLevelManagerActor is being destroyed!\
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
				for (TObjectIterator<ULGUIPrefabHelperObject> Itr; Itr; ++Itr)
				{
					Itr->CleanupInvalidSubPrefab();
				}
			}
			}, 1);
	}
}
void ALGUIPrefabLevelManagerActor::CleanupWhenDestroy()
{
	if (OnSubPrefabNewVersionUpdatedDelegateHandle.IsValid())
	{
		PrefabHelperObject->OnSubPrefabNewVersionUpdated.Remove(OnSubPrefabNewVersionUpdatedDelegateHandle);
	}
	if (BeginPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::BeginPIE.Remove(BeginPIEDelegateHandle);
	}
	if (this != GetDefault<ALGUIPrefabLevelManagerActor>())
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