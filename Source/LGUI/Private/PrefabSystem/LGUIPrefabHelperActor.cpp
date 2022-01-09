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

		FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &ALGUIPrefabHelperActor::OnObjectPropertyChanged);
		FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddUObject(this, &ALGUIPrefabHelperActor::OnPreObjectPropertyChanged);
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

	if (AutoDestroyLoadedActors)
	{
		PrefabHelperObject->ClearLoadedPrefab();
		PrefabHelperObject->ConditionalBeginDestroy();
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
	{
		if (!IsRandomColor)
		{
			if (!AllColors.Contains(IdentityColor))
			{
				AllColors.Add(IdentityColor);
			}
		}
	}

	if (NewVersionPrefabNotification.IsValid())
	{
		OnNewVersionDismissClicked();
	}
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
void ALGUIPrefabHelperActor::AddMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index == INDEX_NONE)
	{
		FLGUIPrefabOverrideParameterData DataItem;
		DataItem.Object = InObject;
		DataItem.MemberPropertyName.Add(InPropertyName);
		ObjectOverrideParameterArray.Add(DataItem);
	}
	else
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (!DataItem.MemberPropertyName.Contains(InPropertyName))
		{
			DataItem.MemberPropertyName.Add(InPropertyName);
		}
	}
}
void ALGUIPrefabHelperActor::RemoveMemberProperty(UObject* InObject, FName InPropertyName)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		auto& DataItem = ObjectOverrideParameterArray[Index];
		if (DataItem.MemberPropertyName.Contains(InPropertyName))
		{
			DataItem.MemberPropertyName.Remove(InPropertyName);
		}
		if (DataItem.MemberPropertyName.Num() <= 0)
		{
			ObjectOverrideParameterArray.RemoveAt(Index);
		}
	}
}

void ALGUIPrefabHelperActor::RemoveMemberProperty(UObject* InObject)
{
	auto Index = ObjectOverrideParameterArray.IndexOfByPredicate([=](const FLGUIPrefabOverrideParameterData& Item) {
		return Item.Object == InObject;
		});
	if (Index != INDEX_NONE)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
	}
}

bool ALGUIPrefabHelperActor::CheckParameters()
{
	bool AnythingChanged = false;
	TSet<int> ObjectsNeedToRemove;
	for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
	{
		auto DataItem = ObjectOverrideParameterArray[i];
		if (!DataItem.Object.IsValid())
		{
			ObjectsNeedToRemove.Add(i);
		}
		else
		{
			TSet<FName> PropertyNamesToRemove;
			auto Object = DataItem.Object;
			for (auto PropertyName : DataItem.MemberPropertyName)
			{
				auto Property = FindFProperty<FProperty>(Object->GetClass(), PropertyName);
				if (Property == nullptr)
				{
					PropertyNamesToRemove.Add(PropertyName);
				}
			}
			for (auto PropertyName : PropertyNamesToRemove)
			{
				DataItem.MemberPropertyName.Remove(PropertyName);
				AnythingChanged = true;
			}
		}
	}
	for (auto Index : ObjectsNeedToRemove)
	{
		ObjectOverrideParameterArray.RemoveAt(Index);
		AnythingChanged = true;
	}
	return AnythingChanged;
}

void ALGUIPrefabHelperActor::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!IsValid(InObject))return;
	if (InPropertyChangedEvent.MemberProperty == nullptr || InPropertyChangedEvent.Property == nullptr)return;
	if (InPropertyChangedEvent.MemberProperty->HasAnyPropertyFlags(CPF_Transient))return;
	if (InPropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))return;

	TryCollectPropertyToOverride(InObject, InPropertyChangedEvent.MemberProperty);
}
void ALGUIPrefabHelperActor::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
{
	if (!IsValid(InObject))return;
	auto ActiveMemberNode = InEditPropertyChain.GetActiveMemberNode();
	if (ActiveMemberNode == nullptr)return;
	auto MemberProperty = ActiveMemberNode->GetValue();
	if (MemberProperty == nullptr)return;
	if (MemberProperty->HasAnyPropertyFlags(CPF_Transient))return;
	auto ActiveNode = InEditPropertyChain.GetActiveNode();
	if (ActiveNode != ActiveMemberNode)
	{
		auto Property = ActiveNode->GetValue();
		if (Property == nullptr)return;
		if (Property->HasAnyPropertyFlags(CPF_Transient))return;
	}

	TryCollectPropertyToOverride(InObject, MemberProperty);
}

void ALGUIPrefabHelperActor::TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty)
{
	if (!bCanCollectProperty)return;
	if (InObject->GetWorld() == this->GetWorld())
	{
		bAnythingDirty = true;

		AActor* PropertyActor = nullptr;
		if (auto Actor = Cast<AActor>(InObject))
		{
			if (auto ObjectProperty = CastField<FObjectPropertyBase>(InMemberProperty))
			{
				if (ObjectProperty->PropertyClass->IsChildOf(UActorComponent::StaticClass()))
				{
					return;//property change is propergated from ActorComponent to Actor, ignore it
				}
			}
			if (PrefabHelperObject->AllLoadedActorArray.Contains(Actor))
			{
				PropertyActor = Actor;
			}
		}
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			if (auto Actor = Component->GetOwner())
			{
				if (PrefabHelperObject->AllLoadedActorArray.Contains(Actor))
				{
					PropertyActor = Actor;
				}
			}
		}
		if (PropertyActor)//only allow actor or component's member property
		{
			auto PropertyName = InMemberProperty->GetFName();
			auto Property = FindFProperty<FProperty>(InObject->GetClass(), PropertyName);
			if (Property != nullptr)
			{
				AddMemberProperty(InObject, PropertyName);
				if (auto UIItem = Cast<UUIItem>(InObject))
				{
					if (PropertyName == FName(TEXT("RelativeLocation")))//if UI's relative location change, then record anchor data too
					{
						AddMemberProperty(InObject, FName(TEXT("AnchorData")));
					}
					else if (PropertyName == FName(TEXT("AnchorData")))//if UI's anchor data change, then record relative location too
					{
						AddMemberProperty(InObject, FName(TEXT("RelativeLocation")));
					}
				}
			}
		}
	}
}


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