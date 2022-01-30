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
void ALGUIPrefabHelperActor::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	if (this != GetDefault<ALGUIPrefabHelperActor>())
	{
		ULGUIEditorManagerObject::AddOneShotTickFunction([Actor = MakeWeakObjectPtr(this)]{
			if (Actor.IsValid())
			{
				if (auto World = Actor->GetWorld())
				{
					if (!World->IsGameWorld())
					{
						ALGUIPrefabManagerActor::GetPrefabManagerActor(Actor->GetLevel());
						Actor->CheckPrefabVersion();
					}
				}
			}
			}, 1);
	}
#endif
}

void ALGUIPrefabHelperActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
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

	if (NewVersionPrefabNotification.IsValid())
	{
		OnNewVersionDismissClicked();
	}
#endif
}
void ALGUIPrefabHelperActor::BeginDestroy()
{
	Super::BeginDestroy();
#if WITH_EDITORONLY_DATA
	if (NewVersionPrefabNotification.IsValid())
	{
		OnNewVersionDismissClicked();
	}
#endif
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

void ALGUIPrefabHelperActor::MarkPrefabVersionAsLatest()
{
	if (TimePointWhenSavePrefab != PrefabAsset->CreateTime)
	{
		TimePointWhenSavePrefab = PrefabAsset->CreateTime;
		this->MarkPackageDirty();
	}
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

	TimePointWhenSavePrefab = PrefabAsset->CreateTime;
}

bool ALGUIPrefabHelperActor::IsValidPrefabHelperActor()
{
	if (this->GetWorld() != nullptr && this->GetWorld()->IsGameWorld())return false;
	if (auto PrefabManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel()))
	{
		return PrefabManagerActor->PrefabHelperObject->LoadedRootActor != nullptr;
	}
	return false;
}

void ALGUIPrefabHelperActor::CheckPrefabVersion()
{
	if (PrefabAsset != nullptr)
	{
		if (TimePointWhenSavePrefab != PrefabAsset->CreateTime)
		{
			if (NewVersionPrefabNotification.IsValid())
			{
				return;
			}
			auto InfoText = FText::Format(LOCTEXT("OldPrefabVersion", "Detect old prefab: '{0}', Would you want to update it?"), FText::FromString(this->GetActorLabel()));
			FNotificationInfo Info(InfoText);
			Info.bFireAndForget = false;
			Info.bUseLargeFont = true;
			Info.bUseThrobber = false;
			Info.FadeOutDuration = 0.0f;
			Info.ExpireDuration = 0.0f;
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("RevertToNewPrefabButton", "Update"), LOCTEXT("RevertToNewPrefabButton_Tooltip", "Revert the prefab to new.")
				, FSimpleDelegate::CreateUObject(this, &ALGUIPrefabHelperActor::OnNewVersionRevertPrefabClicked)));
			Info.ButtonDetails.Add(FNotificationButtonInfo(LOCTEXT("DismissButton", "Dismiss"), LOCTEXT("DismissButton_Tooltip", "Dismiss this notification")
				, FSimpleDelegate::CreateUObject(this, &ALGUIPrefabHelperActor::OnNewVersionDismissClicked)));

			NewVersionPrefabNotification = FSlateNotificationManager::Get().AddNotification(Info);
			NewVersionPrefabNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
		}
	}
}

void ALGUIPrefabHelperActor::OnNewVersionRevertPrefabClicked()
{
	if (!this->GetWorld()->IsGameWorld())
	{
		if (TimePointWhenSavePrefab == PrefabAsset->CreateTime)
		{
			NewVersionPrefabNotification.Pin()->SetText(LOCTEXT("AlreadyUpdated", "Already updated."));
		}
		else
		{
			if (auto PrefabManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel()))
			{
				auto PrefabHelperObject = PrefabManagerActor->PrefabHelperObject;
				PrefabHelperObject->RefreshOnSubPrefabDirty(PrefabAsset, LoadedRootActor);
			}
		}
	}
	NewVersionPrefabNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	NewVersionPrefabNotification.Pin()->ExpireAndFadeout();
	NewVersionPrefabNotification = nullptr;
}
void ALGUIPrefabHelperActor::OnNewVersionDismissClicked()
{
	NewVersionPrefabNotification.Pin()->SetCompletionState(SNotificationItem::CS_None);
	NewVersionPrefabNotification.Pin()->ExpireAndFadeout();
	NewVersionPrefabNotification = nullptr;
}
void ALGUIPrefabHelperActor::RevertPrefab()
{
	if (auto PrefabManagerActor = ALGUIPrefabManagerActor::GetPrefabManagerActor(this->GetLevel()))
	{
		auto PrefabHelperObject = PrefabManagerActor->PrefabHelperObject;
		PrefabHelperObject->RefreshOnSubPrefabDirty(PrefabAsset, LoadedRootActor);
	}
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

void ALGUIPrefabManagerActor::PostInitProperties()
{
	Super::PostInitProperties();
	if (this != GetDefault<ALGUIPrefabManagerActor>())
	{
		if (this->GetLevel() != nullptr)
		{
			if (!MapLevelToManagerActor.Contains(this->GetLevel()))
			{
				MapLevelToManagerActor.Add(this->GetLevel(), this);
			}
		}
	}
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