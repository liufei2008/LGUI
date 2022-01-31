// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
				PrefabManagerActor->PrefabHelperObject->RemoveSubPrefab(LoadedRootActor);
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
	FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
	this->SetFolderPath(PrefabFolderName);
}

void ALGUIPrefabHelperActor::LoadPrefab(USceneComponent* InParent)
{
	if (this->GetWorld() != nullptr && this->GetWorld()->IsGameWorld())return;
	TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
	TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
	LoadedRootActor = PrefabAsset->LoadPrefabWithExistingObjects(this->GetWorld()
		, InParent
		, SubPrefabMapGuidToObject, SubSubPrefabMap
	);
	ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel())->PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedRootActor, SubPrefabMapGuidToObject);
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

TMap<ULevel*, ALGUIPrefabManagerActor*> ALGUIPrefabManagerActor::MapLevelToManagerActor;

ALGUIPrefabManagerActor* ALGUIPrefabManagerActor::GetPrefabManagerActor(ULevel* InLevel)
{
	if (!InLevel->GetWorld()->IsGameWorld())
	{
		if (!MapLevelToManagerActor.Contains(InLevel))
		{
			auto PrefabManagerActor = InLevel->GetWorld()->SpawnActor<ALGUIPrefabManagerActor>();
			MapLevelToManagerActor.Add(InLevel, PrefabManagerActor);
			InLevel->MarkPackageDirty();
			FActorFolders::Get().CreateFolder(*PrefabManagerActor->GetWorld(), ALGUIPrefabHelperActor::PrefabFolderName);
			PrefabManagerActor->SetFolderPath(ALGUIPrefabHelperActor::PrefabFolderName);
		}
		auto Result = MapLevelToManagerActor[InLevel];
		Result->PrefabHelperObject->MarkAsManagerObject();
		return Result;
	}
	else
	{
		checkf(0, TEXT("This should only be called in editor world!"));
		return nullptr;
	}
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
		if (auto Level = this->GetLevel())
		{
			if (!MapLevelToManagerActor.Contains(Level))
			{
				MapLevelToManagerActor.Add(Level, this);
			}
			PrefabHelperObject->OnSubPrefabNewVersionUpdated.AddLambda([Actor = MakeWeakObjectPtr(this)]() {
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

		FEditorDelegates::BeginPIE.AddLambda([Actor = MakeWeakObjectPtr(this)](const bool isSimulating) {
			if (Actor.IsValid())
			{
				Actor->PrefabHelperObject->DismissAllVersionNotifications();
			}
		});
	}
}

void ALGUIPrefabManagerActor::PostActorCreated()
{
	Super::PostActorCreated();
}

void ALGUIPrefabManagerActor::BeginDestroy()
{
	Super::BeginDestroy();
	if (this != GetDefault<ALGUIPrefabManagerActor>())
	{
		if (this->GetWorld() != nullptr && !this->GetWorld()->IsGameWorld())
		{
			auto Level = this->GetLevel();
			if (MapLevelToManagerActor.Contains(Level))
			{
				MapLevelToManagerActor.Remove(Level);
			}
			else
			{
				check(0);
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE