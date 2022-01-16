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

				GEditor->OnLevelActorAttached().AddUObject(Actor.Get(), &ALGUIPrefabHelperActor::OnLevelActorAttached);
				GEditor->OnLevelActorDetached().AddUObject(Actor.Get(), &ALGUIPrefabHelperActor::OnLevelActorDetached);
				Actor->bCanNotifyDetachment = true;
			}
			}, 1);

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

	bCanNotifyDetachment = false;
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

	bCanNotifyDetachment = false;
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

#if WITH_EDITOR
void ALGUIPrefabHelperActor::RevertPrefab()
{
	bCanNotifyDetachment = false;
	PrefabHelperObject->RevertPrefab();//@todo: revert in level editor could have a issue: property with default value could be ignored
	bCanNotifyDetachment = true;
}

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
	this->MarkPackageDirty();
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
	this->MarkPackageDirty();
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
	this->MarkPackageDirty();
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
		this->MarkPackageDirty();
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
					if (PropertyName == USceneComponent::GetRelativeLocationPropertyName())//if UI's relative location change, then record anchor data too
					{
						AddMemberProperty(InObject, UUIItem::GetAnchorDataPropertyName());
					}
					else if (PropertyName == UUIItem::GetAnchorDataPropertyName())//if UI's anchor data change, then record relative location too
					{
						AddMemberProperty(InObject, USceneComponent::GetRelativeLocationPropertyName());
					}
				}
			}
		}
	}
}

void ALGUIPrefabHelperActor::OnLevelActorAttached(AActor* Actor, const AActor* AttachTo)
{
	if (!bCanNotifyDetachment)return;
	if (Actor->GetWorld() != GetWorld())return;

	if (AttachmentActor.Actor == Actor)
	{
		AttachmentActor.AttachTo = (AActor*)AttachTo;
	}
	else
	{
		if (AttachmentActor.Actor == nullptr)
		{
			AttachmentActor.Actor = Actor;
			AttachmentActor.AttachTo = (AActor*)AttachTo;
			AttachmentActor.DetachFrom = nullptr;
			ULGUIEditorManagerObject::AddOneShotTickFunction([=]() {
				CheckAttachment();
				}, 1);
		}
		else
		{
			check(0);//why this happed?
		}
	}
}
void ALGUIPrefabHelperActor::OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom)
{
	if (!bCanNotifyDetachment)return;
	if (Actor->GetWorld() != GetWorld())return;

	AttachmentActor.Actor = Actor;
	AttachmentActor.AttachTo = nullptr;
	AttachmentActor.DetachFrom = (AActor*)DetachFrom;

	ULGUIEditorManagerObject::AddOneShotTickFunction([=]() {
		CheckAttachment();
		}, 1);
}

void ALGUIPrefabHelperActor::CheckAttachment()
{
	if (AttachmentActor.Actor == nullptr)return;
	bool bAttachementError = false;
	if (PrefabHelperObject->IsActorBelongsToThis(AttachmentActor.Actor, true) && PrefabHelperObject->LoadedRootActor != AttachmentActor.Actor)
	{
		bAttachementError = true;
	}
	if (PrefabHelperObject->IsActorBelongsToThis(AttachmentActor.AttachTo, true))
	{
		bAttachementError = true;
	}
	if (bAttachementError)
	{
		auto InfoText = LOCTEXT("CannotRestructurePrefaInstance", "Children of a Prefab instance cannot be deleted or moved, and cannot add or remove component.\
\n\nYou can open the prefab in prefab editor to restructure the prefab asset itself, or unpack the prefab instance to remove its prefab connection.");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
		GEditor->UndoTransaction(false);
		AttachmentActor = FAttachmentActorStruct();
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
		//store override parameter to data
		LGUIPrefabSystem3::ActorSerializer3 serailizer;
		auto OverrideData = serailizer.SaveOverrideParameterToData(ObjectOverrideParameterArray);

		PrefabHelperObject->RevertPrefab();

		//apply override parameter. 
		serailizer.RestoreOverrideParameterFromData(OverrideData, ObjectOverrideParameterArray);
		//mark package dirty
		this->MarkPackageDirty();
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

void ALGUIPrefabHelperActor::RevertPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperties", "Revert Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	this->Modify();

	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	auto OriginObject = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];

	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(InObject, OriginObject);
				//delete item
				this->RemoveMemberProperty(InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(InObject, Property);

				bAnythingDirty = true;
			}
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
}
void ALGUIPrefabHelperActor::RevertPrefabOverride(UObject* InObject, FName InPropertyName)
{
	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	auto OriginObject = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperty", "Revert Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		this->Modify();

		if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), InPropertyName))
		{
			//set to default value
			Property->CopyCompleteValue_InContainer(InObject, OriginObject);
			//delete item
			this->RemoveMemberProperty(InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(InObject, Property);
			bAnythingDirty = true;
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
void ALGUIPrefabHelperActor::RevertAllPrefabOverride()
{
	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(LOCTEXT("RevertPrefabOnAll", "Revert Prefab Override"));
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		this->Modify();

		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
				}
			}
			return PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];
		};
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			if (i == 0)
			{
				if (auto UIItem = Cast<UUIItem>(SourceObject))
				{
					FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
				}
			}
			auto OriginObject = FindOriginObjectInSourcePrefab(SourceObject);
			TSet<FName> NamesToClear;
			for (auto& PropertyName : DataItem.MemberPropertyName)
			{
				if (FilterNameSet.Contains(PropertyName))continue;
				NamesToClear.Add(PropertyName);
				if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
				{
					//set to default value
					Property->CopyCompleteValue_InContainer(SourceObject, OriginObject);
					//notify
					LGUIUtils::NotifyPropertyChanged(SourceObject, Property);
				}
			}
			for (auto& PropertyName : NamesToClear)
			{
				DataItem.MemberPropertyName.Remove(PropertyName);
			}
		}
		for (int i = ObjectOverrideParameterArray.Num() - 1; i > 0; i--)//Remove all but remain root component
		{
			ObjectOverrideParameterArray.RemoveAt(i);
		}

		bAnythingDirty = true;
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}

void ALGUIPrefabHelperActor::ApplyPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperties", "Apply Prefab Override: {0}"), FText::FromString(InObject->GetName())));
	InObject->Modify();
	PrefabHelperObject->Modify();

	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	auto OriginObject = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];

	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(OriginObject, InObject);
				//delete item
				RemoveMemberProperty(InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(OriginObject, Property);

				bAnythingDirty = true;
			}
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			PrefabHelperObject->PrefabAsset->Modify();
			PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->SavePrefab();
			PrefabHelperObject->TimePointWhenSavePrefab = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->TimePointWhenSavePrefab;
		}
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
}
void ALGUIPrefabHelperActor::ApplyPrefabOverride(UObject* InObject, FName InPropertyName)
{
	AActor* Actor = Cast<AActor>(InObject);
	UActorComponent* Component = Cast<UActorComponent>(InObject);
	if (Actor)
	{
	}
	else if (Component)
	{
		Actor = Component->GetOwner();
	}
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	auto OriginObject = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("ApplyPrefabOnObjectProperty", "Apply Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		PrefabHelperObject->Modify();

		if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), InPropertyName))
		{
			//set to default value
			Property->CopyCompleteValue_InContainer(OriginObject, InObject);
			//delete item
			RemoveMemberProperty(InObject, InPropertyName);
			//notify
			LGUIUtils::NotifyPropertyChanged(OriginObject, Property);
			bAnythingDirty = true;
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			PrefabHelperObject->PrefabAsset->Modify();
			PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->SavePrefab();
			PrefabHelperObject->TimePointWhenSavePrefab = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->TimePointWhenSavePrefab;
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
void ALGUIPrefabHelperActor::ApplyAllOverrideToPrefab()
{
	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll", "Apply Prefab Override"));
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		PrefabHelperObject->Modify();

		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
				}
			}
			return PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->MapGuidToObject[ObjectGuid];
		};
		for (int i = 0; i < ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = ObjectOverrideParameterArray[i];
			auto SourceObject = DataItem.Object.Get();
			TSet<FName> FilterNameSet;
			if (i == 0)
			{
				if (auto UIItem = Cast<UUIItem>(SourceObject))
				{
					FilterNameSet = UUIItem::PersistentOverridePropertyNameSet;
				}
			}
			auto OriginObject = FindOriginObjectInSourcePrefab(SourceObject);
			TSet<FName> NamesToClear;
			for (auto& PropertyName : DataItem.MemberPropertyName)
			{
				if (FilterNameSet.Contains(PropertyName))continue;
				NamesToClear.Add(PropertyName);
				if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
				{
					//set to default value
					Property->CopyCompleteValue_InContainer(OriginObject, SourceObject);
					//notify
					LGUIUtils::NotifyPropertyChanged(OriginObject, Property);
				}
			}
			for (auto& PropertyName : NamesToClear)
			{
				DataItem.MemberPropertyName.Remove(PropertyName);
			}
		}
		for (int i = ObjectOverrideParameterArray.Num() - 1; i > 0; i--)//Remove all but remain root component
		{
			ObjectOverrideParameterArray.RemoveAt(i);
		}
		//save origin prefab
		{
			PrefabHelperObject->PrefabAsset->Modify();
			PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->SavePrefab();
			PrefabHelperObject->TimePointWhenSavePrefab = PrefabHelperObject->PrefabAsset->GetPrefabHelperObject()->TimePointWhenSavePrefab;
		}

		bAnythingDirty = true;
		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
#endif

#undef LOCTEXT_NAMESPACE