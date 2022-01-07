// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
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
	bIsEditorOnlyActor = false;

#if WITH_EDITORONLY_DATA
	PrefabHelperObject = CreateDefaultSubobject<ULGUIPrefabHelperObject>(TEXT("PrefabHelper"));
	PrefabHelperObject->bIsInsidePrefabEditor = false;
#endif
}

void ALGUIPrefabHelperActor::BeginPlay()
{
	PrefabHelperObject->RevertPrefab();
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
				Actor->CheckPrefabVersion();
			}
			});
	}
#endif
}
void ALGUIPrefabHelperActor::OnConstruction(const FTransform& Transform)
{
#if WITH_EDITORONLY_DATA
	if (AllColors.Num() == 0)
	{
		int ColorCount = 10;
		float Interval = 1.0f / ColorCount;
		float StartHue01 = 0;
		for (int i = 0; i < ColorCount; i++)
		{
			auto Hue = (uint8)(StartHue01 * 255);
			auto Color1 = FLinearColor::MakeFromHSV8(Hue, 255, 255).ToFColor(false);
			AllColors.Add(Color1);
			auto Color2 = FLinearColor::MakeFromHSV8(Hue, 255, 128).ToFColor(false);
			AllColors.Add(Color2);
			StartHue01 += Interval;
		}
	}

	{
		if (AllColors.Num() == 0)
		{
			IdentityColor = FColor::MakeRandomColor();
			IsRandomColor = true;
		}
		else
		{
			int RandomIndex = FMath::RandRange(0, AllColors.Num() - 1);
			IdentityColor = AllColors[RandomIndex];
			AllColors.RemoveAt(RandomIndex);
			IsRandomColor = false;
		}
	}
#endif
}

void ALGUIPrefabHelperActor::Destroyed()
{
	Super::Destroyed();
#if WITH_EDITORONLY_DATA
	{
		if (!IsRandomColor)
		{
			if (!AllColors.Contains(IdentityColor))
			{
				AllColors.Add(IdentityColor);
			}
		}
	}

	PrefabHelperObject->ClearLoadedPrefab();
	PrefabHelperObject->ConditionalBeginDestroy();
#endif
}

#if WITH_EDITORONLY_DATA
FName ALGUIPrefabHelperActor::PrefabFolderName(TEXT("--LGUIPrefabActor--"));
TArray<FColor> ALGUIPrefabHelperActor::AllColors;
#endif

void ALGUIPrefabHelperActor::RevertPrefab()
{
	PrefabHelperObject->RevertPrefab();
}

#if WITH_EDITOR
void ALGUIPrefabHelperActor::MoveActorToPrefabFolder()
{
	FActorFolders::Get().CreateFolder(*this->GetWorld(), PrefabFolderName);
	this->SetFolderPath(PrefabFolderName);
}


void ALGUIPrefabHelperActor::LoadPrefab(USceneComponent* InParent)
{
	PrefabHelperObject->LoadPrefab(this->GetWorld(), InParent);
}

void ALGUIPrefabHelperActor::SavePrefab()
{
	PrefabHelperObject->SavePrefab();
}

void ALGUIPrefabHelperActor::DeleteThisInstance()
{
	PrefabHelperObject->ClearLoadedPrefab();
	LGUIUtils::DestroyActorWithHierarchy(this, false);
}

void ALGUIPrefabHelperActor::CheckPrefabVersion()
{
	if (IsValid(PrefabHelperObject) && PrefabHelperObject->PrefabAsset != nullptr)
	{
		if (PrefabHelperObject->TimePointWhenSavePrefab != PrefabHelperObject->PrefabAsset->CreateTime)
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
	if (PrefabHelperObject->TimePointWhenSavePrefab == PrefabHelperObject->PrefabAsset->CreateTime)
	{
		NewVersionPrefabNotification.Pin()->SetText(LOCTEXT("AlreadyUpdated", "Already updated."));
	}
	else
	{
		PrefabHelperObject->RevertPrefab();
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
#endif

#undef LOCTEXT_NAMESPACE