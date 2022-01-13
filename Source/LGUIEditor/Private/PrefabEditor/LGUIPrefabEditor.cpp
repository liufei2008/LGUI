// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIEditorModule.h"
#include "LGUIPrefabEditorViewport.h"
#include "LGUIPrefabPreviewScene.h"
#include "LGUIPrefabEditorDetails.h"
#include "LGUIPrefabEditorOutliner.h"
#include "LGUIPrefabRawDataViewer.h"
#include "UnrealEdGlobals.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"
#include "LGUIPrefabEditorCommand.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LGUIEditorTools.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Engine/Selection.h"
#include "ToolMenus.h"
#include "LGUIEditorUtils.h"
#include "PrefabSystem/ActorSerializer3.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditor"

const FName PrefabEditorAppName = FName(TEXT("LGUIPrefabEditorApp"));

TArray<FLGUIPrefabEditor*> FLGUIPrefabEditor::LGUIPrefabEditorInstanceCollection;

struct FLGUIPrefabEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName OutlinerID;
	static const FName PrefabRawDataViewerID;
};

const FName FLGUIPrefabEditorTabs::DetailsID(TEXT("Details"));
const FName FLGUIPrefabEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLGUIPrefabEditorTabs::OutlinerID(TEXT("Outliner"));
const FName FLGUIPrefabEditorTabs::PrefabRawDataViewerID(TEXT("PrefabRawDataViewer"));

FLGUIPrefabEditor::FLGUIPrefabEditor()
	:PreviewScene(FLGUIPrefabPreviewScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(false).SetEditor(true))
{
	PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Transactional);
	PrefabHelperObject->bIsInsidePrefabEditor = true;
	LGUIPrefabEditorInstanceCollection.Add(this);
}
FLGUIPrefabEditor::~FLGUIPrefabEditor()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(this->OnObjectPropertyChangedDelegateHandle);
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.Remove(this->OnPreObjectPropertyChangedDelegateHandle);
	FCoreUObjectDelegates::OnObjectModified.Remove(this->OnObjectModifiedDelegateHandle);

	PrefabHelperObject->MarkPendingKill();
	PrefabHelperObject = nullptr;

	LGUIPrefabEditorInstanceCollection.Remove(this);
}

FLGUIPrefabEditor* FLGUIPrefabEditor::GetEditorForPrefabIfValid(ULGUIPrefab* InPrefab)
{
	for (auto Instance : LGUIPrefabEditorInstanceCollection)
	{
		if (Instance->PrefabBeingEdited == InPrefab)
		{
			return Instance;
		}
	}
	return nullptr;
}

ULGUIPrefabHelperObject* FLGUIPrefabEditor::GetEditorPrefabHelperObjectForActor(AActor* InActor)
{
	for (auto Instance : LGUIPrefabEditorInstanceCollection)
	{
		if (InActor->GetWorld() == Instance->GetWorld())
		{
			return Instance->PrefabHelperObject;
		}
	}
	return nullptr;
}

bool FLGUIPrefabEditor::ActorIsRootAgent(AActor* InActor)
{
	for (auto Instance : LGUIPrefabEditorInstanceCollection)
	{
		if (InActor == Instance->GetPreviewScene().GetRootAgentActor())
		{
			return true;
		}
	}
	return false;
}

bool FLGUIPrefabEditor::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab)
{
	bCanNotifyDetachment = false;
	bool AnythingChange = false;

	auto OriginObjectContainsInSourcePrefabByGuid = [=](UObject* InObject, FLGUISubPrefabData& SubPrefabData) {
		FGuid ObjectGuid;
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (KeyValue.Value == InObject)
			{
				ObjectGuid = KeyValue.Key;
			}
		}
		FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
		return SubPrefabData.PrefabAsset->GetPrefabHelperObject()->MapGuidToObject.Contains(ObjectGuidInSubPrefab);
	};
	for (auto& SubPrefabKeyValue : PrefabHelperObject->SubPrefabMap)
	{
		auto SubPrefabRootActor = SubPrefabKeyValue.Key;
		auto& SubPrefabData = SubPrefabKeyValue.Value;
		if (SubPrefabData.PrefabAsset == InSubPrefab)
		{
			//store override parameter to data
			LGUIPrefabSystem3::ActorSerializer3 serailizer;
			auto OverrideData = serailizer.SaveOverrideParameterToData(SubPrefabData.ObjectOverrideParameterArray);

			TArray<AActor*> ChildrenActors;
			LGUIUtils::CollectChildrenActors(SubPrefabRootActor, ChildrenActors);

			TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
			for (auto& GuidToObject : PrefabHelperObject->MapGuidToObject)
			{
				if (auto ObjectGuidInSubPrefabPtr = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Find(GuidToObject.Key))
				{
					SubPrefabMapGuidToObject.Add(*ObjectGuidInSubPrefabPtr, GuidToObject.Value);
				}
			}

			TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
			InSubPrefab->LoadPrefabWithExistingObjects(GetPreviewScene().GetWorld()
				, SubPrefabRootActor->GetAttachParentActor()->GetRootComponent()
				, SubPrefabMapGuidToObject, SubSubPrefabMap
				, false
			);

			//delete extra actors
			for (auto& OldChild : ChildrenActors)
			{
				if (!OriginObjectContainsInSourcePrefabByGuid(OldChild, SubPrefabData))
				{
					LGUIUtils::DestroyActorWithHierarchy(OldChild, false);
					AnythingChange = true;
				}
			}
			//collect added object and guid
			auto FindOrAddSubPrefabObjectGuidInParentPrefab = [&](UObject* InObject) {
				for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
				{
					if (KeyValue.Value == InObject)
					{
						return KeyValue.Key;
					}
				}
				auto NewGuid = FGuid::NewGuid();
				PrefabHelperObject->MapGuidToObject.Add(NewGuid, InObject);
				AnythingChange = true;
				return NewGuid;
			};
			for (auto& SubPrefabGuidToObject : SubPrefabMapGuidToObject)
			{
				auto ObjectGuidInParentPrefab = FindOrAddSubPrefabObjectGuidInParentPrefab(SubPrefabGuidToObject.Value);
				if (!SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(ObjectGuidInParentPrefab))
				{
					SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(ObjectGuidInParentPrefab, SubPrefabGuidToObject.Key);
					AnythingChange = true;
				}
			}
			//no need to clear invalid objects, because when SavePrefab it will do the clear work
			//apply override parameter. 
			serailizer.RestoreOverrideParameterFromData(OverrideData, SubPrefabData.ObjectOverrideParameterArray);

			if (SubPrefabData.CheckParameters())
			{
				AnythingChange = true;
			}
		}
	}

	if (AnythingChange)
	{
		PrefabHelperObject->SavePrefab();
		PrefabHelperObject->PrefabAsset->MarkPackageDirty();
	}
	ULGUIEditorManagerObject::RefreshAllUI();
	bCanNotifyDetachment = true;
	return AnythingChange;
}

bool FLGUIPrefabEditor::GetSelectedObjectsBounds(FBoxSphereBounds& OutResult)
{
	USelection* Selection = GEditor->GetSelectedActors();
	TArray<AActor*> SelectedActors;
	for (int i = 0; i < Selection->Num(); i++)
	{
		if (auto Actor = Cast<AActor>(Selection->GetSelectedObject(i)))
		{
			if (Actor->GetWorld() == this->GetWorld())//only concern actors belongs to this prefab
			{
				SelectedActors.Add(Actor);
			}
		}
	}

	FBoxSphereBounds Bounds;
	bool IsFirstBounds = true;
	for (auto& Actor : SelectedActors)
	{
		auto Box = Actor->GetComponentsBoundingBox();
		if (IsFirstBounds)
		{
			IsFirstBounds = false;
			Bounds = Box;
		}
		else
		{
			Bounds = Bounds + Box;
		}
	}
	OutResult = Bounds;
	return IsFirstBounds == false;
}

FBoxSphereBounds FLGUIPrefabEditor::GetAllObjectsBounds()
{
	FBoxSphereBounds Bounds;
	bool IsFirstBounds = true;
	for (auto& Actor : PrefabHelperObject->AllLoadedActorArray)
	{
		auto Box = Actor->GetComponentsBoundingBox();
		if (IsFirstBounds)
		{
			IsFirstBounds = false;
			Bounds = Box;
		}
		else
		{
			Bounds = Bounds + Box;
		}
	}
	return Bounds;
}

bool FLGUIPrefabEditor::ActorBelongsToSubPrefab(AActor* InActor)
{
	return PrefabHelperObject->IsActorBelongsToSubPrefab(InActor);
}

bool FLGUIPrefabEditor::ActorIsSubPrefabRoot(AActor* InSubPrefabRootActor)
{
	return PrefabHelperObject->SubPrefabMap.Contains(InSubPrefabRootActor);
}

FLGUISubPrefabData FLGUIPrefabEditor::GetSubPrefabDataForActor(AActor* InSubPrefabActor)
{
	return PrefabHelperObject->GetSubPrefabData(InSubPrefabActor);
}

//@todo: cleanup these revert and apply code. is undo good?
#pragma region RevertAndApply
void FLGUIPrefabEditor::RevertPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
{
	GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperties", "Revert Prefab Override: {0}"), FText::FromString(InObject->GetName())));
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
	auto SubPrefabData = PrefabHelperObject->GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
	
	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(InObject, OriginObject);
				//delete item
				PrefabHelperObject->RemoveMemberPropertyFromSubPrefab(Actor, InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(InObject, Property);

				bAnythingDirty = true;
			}
		}
		DetailsPtr->RefreshOverrideParameter();
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
}
void FLGUIPrefabEditor::RevertPrefabOverride(UObject* InObject, FName InPropertyName)
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
	auto SubPrefabData = PrefabHelperObject->GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		GEditor->BeginTransaction(FText::Format(LOCTEXT("RevertPrefabOnObjectProperty", "Revert Prefab Override: {0}.{1}"), FText::FromString(InObject->GetName()), FText::FromName(InPropertyName)));
		InObject->Modify();
		PrefabHelperObject->Modify();

		if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), InPropertyName))
		{
			//set to default value
			Property->CopyCompleteValue_InContainer(InObject, OriginObject);
			//delete item
			PrefabHelperObject->RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			DetailsPtr->RefreshOverrideParameter();
			//notify
			LGUIUtils::NotifyPropertyChanged(InObject, Property);
			bAnythingDirty = true;
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}

void FLGUIPrefabEditor::RevertAllPrefabOverride(AActor* InSubPrefabRootActor)
{
	bCanCollectProperty = false;
	{
		auto& SubPrefabData = PrefabHelperObject->SubPrefabMap[InSubPrefabRootActor];

		GEditor->BeginTransaction(LOCTEXT("RevertPrefabOnAll", "Revert Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		PrefabHelperObject->Modify();

		auto SubPrefabAsset = SubPrefabData.PrefabAsset;
		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
				}
			}
			FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
			return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
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
		for (int i = SubPrefabData.ObjectOverrideParameterArray.Num() - 1; i > 0; i--)//Remove all but remain root component
		{
			SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
		}

		bAnythingDirty = true;
		GEditor->EndTransaction();
		DetailsPtr->RefreshOverrideParameter();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}

void FLGUIPrefabEditor::ApplyPrefabOverride(UObject* InObject, const TSet<FName>& InPropertyNameSet)
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
	auto SubPrefabData = PrefabHelperObject->GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

	bCanCollectProperty = false;
	{
		for (auto& PropertyName : InPropertyNameSet)
		{
			if (auto Property = FindFProperty<FProperty>(OriginObject->GetClass(), PropertyName))
			{
				//set to default value
				Property->CopyCompleteValue_InContainer(OriginObject, InObject);
				//delete item
				PrefabHelperObject->RemoveMemberPropertyFromSubPrefab(Actor, InObject, PropertyName);
				//notify
				LGUIUtils::NotifyPropertyChanged(OriginObject, Property);

				bAnythingDirty = true;
			}
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
			LGUIEditorTools::RefreshLevelLoadedPrefab(SubPrefabAsset);
			LGUIEditorTools::RefreshOpenedPrefabEditor(SubPrefabAsset);
			LGUIEditorTools::RefreshOnSubPrefabChange(SubPrefabAsset);
		}

		DetailsPtr->RefreshOverrideParameter();
	}
	bCanCollectProperty = true;
	GEditor->EndTransaction();
	ULGUIEditorManagerObject::RefreshAllUI();
}
void FLGUIPrefabEditor::ApplyPrefabOverride(UObject* InObject, FName InPropertyName)
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
	auto SubPrefabData = PrefabHelperObject->GetSubPrefabData(Actor);
	auto SubPrefabAsset = SubPrefabData.PrefabAsset;
	auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
	FGuid ObjectGuid;
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (KeyValue.Value == InObject)
		{
			ObjectGuid = KeyValue.Key;
		}
	}
	FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
	auto OriginObject = SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];

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
			PrefabHelperObject->RemoveMemberPropertyFromSubPrefab(Actor, InObject, InPropertyName);
			DetailsPtr->RefreshOverrideParameter();
			//notify
			LGUIUtils::NotifyPropertyChanged(OriginObject, Property);
			bAnythingDirty = true;
		}
		//save origin prefab
		if (bAnythingDirty)
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
			LGUIEditorTools::RefreshLevelLoadedPrefab(SubPrefabAsset);
			LGUIEditorTools::RefreshOpenedPrefabEditor(SubPrefabAsset);
			LGUIEditorTools::RefreshOnSubPrefabChange(SubPrefabAsset);
		}

		GEditor->EndTransaction();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
void FLGUIPrefabEditor::ApplyAllOverrideToPrefab(AActor* InSubPrefabRootActor)
{
	bCanCollectProperty = false;
	{
		auto& SubPrefabData = PrefabHelperObject->SubPrefabMap[InSubPrefabRootActor];

		GEditor->BeginTransaction(LOCTEXT("ApplyPrefabOnAll", "Apply Prefab Override"));
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
			DataItem.Object->Modify();
		}
		PrefabHelperObject->Modify();

		auto SubPrefabAsset = SubPrefabData.PrefabAsset;
		auto SubPrefabHelperObject = SubPrefabAsset->GetPrefabHelperObject();
		auto FindOriginObjectInSourcePrefab = [&](UObject* InObject) {
			FGuid ObjectGuid;
			for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
			{
				if (KeyValue.Value == InObject)
				{
					ObjectGuid = KeyValue.Key;
				}
			}
			FGuid ObjectGuidInSubPrefab = SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab[ObjectGuid];
			return SubPrefabHelperObject->MapGuidToObject[ObjectGuidInSubPrefab];
		};
		for (int i = 0; i < SubPrefabData.ObjectOverrideParameterArray.Num(); i++)
		{
			auto& DataItem = SubPrefabData.ObjectOverrideParameterArray[i];
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
		for (int i = SubPrefabData.ObjectOverrideParameterArray.Num() - 1; i > 0; i--)//Remove all but remain root component
		{
			SubPrefabData.ObjectOverrideParameterArray.RemoveAt(i);
		}
		//save origin prefab
		{
			SubPrefabAsset->Modify();
			SubPrefabAsset->GetPrefabHelperObject()->SavePrefab();
			LGUIEditorTools::RefreshLevelLoadedPrefab(SubPrefabAsset);
			LGUIEditorTools::RefreshOpenedPrefabEditor(SubPrefabAsset);
			LGUIEditorTools::RefreshOnSubPrefabChange(SubPrefabAsset);
		}

		bAnythingDirty = true;
		GEditor->EndTransaction();
		DetailsPtr->RefreshOverrideParameter();
	}
	bCanCollectProperty = true;
	ULGUIEditorManagerObject::RefreshAllUI();
}
#pragma endregion RevertAndApply

void FLGUIPrefabEditor::OpenSubPrefab(AActor* InSubPrefabActor)
{
	if (auto SubPrefabAsset = PrefabHelperObject->GetSubPrefabAsset(InSubPrefabActor))
	{
		auto PrefabEditor = FLGUIPrefabEditor::GetEditorForPrefabIfValid(SubPrefabAsset);
		if (!PrefabEditor)
		{
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditorSubsystem->OpenEditorForAsset(SubPrefabAsset);
		}
	}
}
void FLGUIPrefabEditor::SelectSubPrefab(AActor* InSubPrefabActor)
{
	if (auto SubPrefabAsset = PrefabHelperObject->GetSubPrefabAsset(InSubPrefabActor))
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(SubPrefabAsset);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
}

void FLGUIPrefabEditor::CloseWithoutCheckDataDirty()
{
	bAnythingDirty = false;
	this->CloseWindow();
}

bool FLGUIPrefabEditor::OnRequestClose()
{
	if (bAnythingDirty)
	{
		auto WarningMsg = LOCTEXT("LoseDataOnCloseEditor", "Are you sure you want to close prefab editor window? Property will lose if not hit Apply!");
		auto Result = FMessageDialog::Open(EAppMsgType::YesNo, WarningMsg);
		if (Result == EAppReturnType::Yes)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	return true;
}

void FLGUIPrefabEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_LGUIPrefabEditor", "LGUIPrefab Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::OutlinerID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_Outliner))
		.SetDisplayName(LOCTEXT("OutlinerTabLabel", "Outliner"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Outliner"));

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::PrefabRawDataViewerID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_PrefabRawDataViewer))
		.SetDisplayName(LOCTEXT("PrefabRawDataViewerTabLabel", "PrefabRawDataViewer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		;
}
void FLGUIPrefabEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::OutlinerID);
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::PrefabRawDataViewerID);
}

void FLGUIPrefabEditor::InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost >& InitToolkitHost, ULGUIPrefab* InPrefab)
{
	PrefabBeingEdited = InPrefab;
	PrefabHelperObject->PrefabAsset = PrefabBeingEdited;

	FLGUIPrefabEditorCommand::Register();

	PrefabHelperObject->LoadPrefab(GetPreviewScene().GetWorld(), GetPreviewScene().GetParentComponentForPrefab(PrefabBeingEdited));

	TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SLGUIPrefabEditorViewport, PrefabEditorPtr);
	
	DetailsPtr = SNew(SLGUIPrefabEditorDetails, PrefabEditorPtr);

	PrefabRawDataViewer = SNew(SLGUIPrefabRawDataViewer, PrefabEditorPtr, PrefabBeingEdited);

	OutlinerPtr = MakeShared<FLGUIPrefabEditorOutliner>();
	OutlinerPtr->ActorFilter = FOnShouldFilterActor::CreateRaw(this, &FLGUIPrefabEditor::IsFilteredActor);
	OutlinerPtr->OnActorPickedDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerPickedChanged);
	OutlinerPtr->OnActorDoubleClickDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerActorDoubleClick);
	OutlinerPtr->InitOutliner(GetPreviewScene().GetWorld(), PrefabEditorPtr);

	BindCommands();
	ExtendToolbar();

	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_LGUIPrefabEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FLGUIPrefabEditorTabs::OutlinerID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.6f)
					->AddTab(FLGUIPrefabEditorTabs::ViewportID, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(FLGUIPrefabEditorTabs::DetailsID, ETabState::OpenedTab)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, PrefabEditorAppName, StandaloneDefaultLayout, true, true, PrefabBeingEdited);

	this->OnObjectPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FLGUIPrefabEditor::OnObjectPropertyChanged);
	this->OnPreObjectPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnPreObjectPropertyChanged.AddRaw(this, &FLGUIPrefabEditor::OnPreObjectPropertyChanged);
	this->OnObjectModifiedDelegateHandle = FCoreUObjectDelegates::OnObjectModified.AddRaw(this, &FLGUIPrefabEditor::OnObjectModified);
	GEditor->OnLevelActorAttached().AddSP(this, &FLGUIPrefabEditor::OnLevelActorAttached);
	GEditor->OnLevelActorDetached().AddSP(this, &FLGUIPrefabEditor::OnLevelActorDetached);
	GEditor->OnComponentTransformChanged().AddSP(this, &FLGUIPrefabEditor::OnComponentTransformChanged);
}

void FLGUIPrefabEditor::DeleteActors(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray)
{
	for (auto Item : InSelectedActorArray)
	{
		if (Item == PrefabHelperObject->LoadedRootActor)
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
		if (Item == GetPreviewScene().GetRootAgentActor())
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root agent actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
		if (PrefabHelperObject->IsActorBelongsToSubPrefab(Item.Get()) && !PrefabHelperObject->SubPrefabMap.Contains(Item.Get()))
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy sub prefab's actor!"));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
	}

	auto confirmMsg = FString::Printf(TEXT("Destroy selected actors? This will also destroy the children attached to selected actors."));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	GEditor->BeginTransaction(LOCTEXT("DeleteActors", "Delete actors"));
	GEditor->GetSelectedActors()->DeselectAll();
	TArray<AActor*> SelectedActorArray;
	for (auto Item : InSelectedActorArray)
	{
		if (Item.IsValid())
		{
			SelectedActorArray.Add(Item.Get());
		}
	}
	for (auto Item : SelectedActorArray)
	{
		if (PrefabHelperObject->SubPrefabMap.Contains(Item))
		{
			PrefabHelperObject->SubPrefabMap.Remove(Item);
		}
	}
	auto RootActorArray = LGUIEditorTools::GetRootActorListFromSelection(SelectedActorArray);
	for (auto Item : RootActorArray)
	{
		LGUIUtils::DestroyActorWithHierarchy(Item);
	}
	GEditor->EndTransaction();
}

void FLGUIPrefabEditor::DeleteActor(AActor* InActor)
{
	if (PrefabHelperObject->SubPrefabMap.Contains(InActor))
	{
		PrefabHelperObject->SubPrefabMap.Remove(InActor);
	}
	LGUIUtils::DestroyActorWithHierarchy(InActor);
}

void FLGUIPrefabEditor::SaveAsset_Execute()
{
	if (CheckBeforeSaveAsset())
	{
		//refresh parameter, remove invalid
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			KeyValue.Value.CheckParameters();
		}

		PrefabHelperObject->SavePrefab();
		LGUIEditorTools::RefreshLevelLoadedPrefab(PrefabHelperObject->PrefabAsset);
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
		FAssetEditorToolkit::SaveAsset_Execute();
		bAnythingDirty = false;
		ULGUIEditorManagerObject::RefreshAllUI();
	}
}
void FLGUIPrefabEditor::OnApply()
{
	if (CheckBeforeSaveAsset())
	{
		//refresh parameter, remove invalid
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			KeyValue.Value.CheckParameters();
		}

		PrefabHelperObject->SavePrefab();
		LGUIEditorTools::RefreshLevelLoadedPrefab(PrefabHelperObject->PrefabAsset);
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
		bAnythingDirty = false;
		ULGUIEditorManagerObject::RefreshAllUI();
	}
}

void FLGUIPrefabEditor::OnOpenRawDataViewerPanel()
{
	this->InvokeTab(FLGUIPrefabEditorTabs::PrefabRawDataViewerID);
}

void FLGUIPrefabEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrefabBeingEdited);
	Collector.AddReferencedObject(PrefabHelperObject);
}

bool FLGUIPrefabEditor::CheckBeforeSaveAsset()
{
	auto RootUIAgentActor = GetPreviewScene().GetRootAgentActor();
	//All actor should attach to prefab's root actor
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		if (AActor* ItemActor = *ActorItr)
		{
			if (ItemActor == PrefabHelperObject->LoadedRootActor)continue;
			if (ItemActor == RootUIAgentActor)continue;
			if (GetPreviewScene().IsWorldDefaultActor(ItemActor))continue;
			if (!ItemActor->IsAttachedTo(PrefabHelperObject->LoadedRootActor))
			{
				auto MsgText = LOCTEXT("Error_AllActor", "All prefab's actors must attach to prefab's root actor!");
				FMessageDialog::Open(EAppMsgType::Ok, MsgText);
				return false;
			}
		}
	}
	
	//If is UI prefab, then root actor must be child of [UIRootAgent]
	if (PrefabHelperObject->LoadedRootActor->IsA(AUIBaseActor::StaticClass()))
	{
		if (PrefabHelperObject->LoadedRootActor->GetAttachParentActor() != RootUIAgentActor)
		{
			auto MsgText = LOCTEXT("Error_PrefabRootMustMustBeChildOfRootUIAgent", "Prefab's root actor must be child of [UIRootAgent]");
			FMessageDialog::Open(EAppMsgType::Ok, MsgText);
			return false;
		}
	}

	return true;
}

FLGUIPrefabPreviewScene& FLGUIPrefabEditor::GetPreviewScene()
{ 
	return PreviewScene;
}

UWorld* FLGUIPrefabEditor::GetWorld()
{
	return PreviewScene.GetWorld();
}

void FLGUIPrefabEditor::BindCommands()
{
	const FLGUIPrefabEditorCommand& PrefabEditorCommands = FLGUIPrefabEditorCommand::Get();
	ToolkitCommands->MapAction(
		PrefabEditorCommands.Apply,
		FExecuteAction::CreateSP(this, &FLGUIPrefabEditor::OnApply),
		FCanExecuteAction(),
		FIsActionChecked()
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.RawDataViewer,
		FExecuteAction::CreateSP(this, &FLGUIPrefabEditor::OnOpenRawDataViewerPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);
}
void FLGUIPrefabEditor::ExtendToolbar()
{
	const FName MenuName = GetToolMenuToolbarName();
	if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
	{
		UToolMenu* ToolBar = UToolMenus::Get()->RegisterMenu(MenuName, "AssetEditor.DefaultToolBar", EMultiBoxType::ToolBar);

		FToolMenuInsert InsertAfterAssetSection("Asset", EToolMenuInsertType::After);
		{
			FToolMenuSection& Section = ToolBar->AddSection("LGUIPrefabCommands", TAttribute<FText>(), InsertAfterAssetSection);
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().Apply));
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().RawDataViewer));
		}
	}
}

TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			SNew(SOverlay)

			// The sprite editor viewport
			+SOverlay::Slot()
			[
				ViewportPtr.ToSharedRef()
			]

			// Bottom-right corner text indicating the preview nature of the sprite editor
			+SOverlay::Slot()
			.Padding(10)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			[
				SNew(STextBlock)
				.Visibility(EVisibility::HitTestInvisible)
				.TextStyle(FEditorStyle::Get(), "Graph.CornerText")
				//.Text(this, &FSpriteEditor::GetCurrentModeCornerText)
			]
		];
}
TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			DetailsPtr.ToSharedRef()
		];
}
TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_Outliner(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OutlinerTab_Title", "Outliner"))
		[
			OutlinerPtr->GetOutlinerWidget().ToSharedRef()
		];
}

TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OverrideParameterTab_Title", "PrefabRawData"))
		[
			PrefabRawDataViewer.ToSharedRef()
		];
}

bool FLGUIPrefabEditor::IsFilteredActor(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (!Actor->IsListedInSceneOutliner())
	{
		return false;
	}
	return true;
}

void FLGUIPrefabEditor::OnOutlinerPickedChanged(AActor* Actor)
{
	CurrentSelectedActor = Actor;
}

void FLGUIPrefabEditor::OnOutlinerActorDoubleClick(AActor* Actor)
{
	// Create a bounding volume of all of the selected actors.
	FBox BoundingBox(ForceInit);

	TArray<AActor*> Actors;
	Actors.Add(Actor);

	for (int32 ActorIdx = 0; ActorIdx < Actors.Num(); ActorIdx++)
	{
		AActor* TempActor = Actors[ActorIdx];

		if (TempActor)
		{
			TInlineComponentArray<UPrimitiveComponent*> PrimitiveComponents(TempActor);

			for (int32 ComponentIndex = 0; ComponentIndex < PrimitiveComponents.Num(); ++ComponentIndex)
			{
				UPrimitiveComponent* PrimitiveComponent = PrimitiveComponents[ComponentIndex];

				if (PrimitiveComponent->IsRegistered())
				{
					// Some components can have huge bounds but are not visible.  Ignore these components unless it is the only component on the actor 
					const bool bIgnore = PrimitiveComponents.Num() > 1 && PrimitiveComponent->IgnoreBoundsForEditorFocus();

					if (!bIgnore)
					{
						FBox LocalBox(ForceInit);
						if (GLevelEditorModeTools().ComputeBoundingBoxForViewportFocus(TempActor, PrimitiveComponent, LocalBox))
						{
							BoundingBox += LocalBox;
						}
						else
						{
							BoundingBox += PrimitiveComponent->Bounds.GetBox();
						}
					}
				}
			}
		}
	}

	ViewportPtr->GetViewportClient()->FocusViewportOnBox(BoundingBox);
}

FName FLGUIPrefabEditor::GetToolkitFName() const
{
	return FName("LGUIPrefabEditor");
}
FText FLGUIPrefabEditor::GetBaseToolkitName() const
{
	return LOCTEXT("LGUIPrefabEditorAppLabel", "LGUI Prefab Editor");
}
FText FLGUIPrefabEditor::GetToolkitName() const
{
	return FText::FromString(PrefabBeingEdited->GetName());
}
FText FLGUIPrefabEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(PrefabBeingEdited);
}
FLinearColor FLGUIPrefabEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
FString FLGUIPrefabEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LGUIPrefabEditor");
}
FString FLGUIPrefabEditor::GetDocumentationLink() const
{
	return TEXT("");
}
void FLGUIPrefabEditor::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{

}
void FLGUIPrefabEditor::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{

}

void FLGUIPrefabEditor::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (!IsValid(InObject))return;
	if (InPropertyChangedEvent.MemberProperty == nullptr || InPropertyChangedEvent.Property == nullptr)return;
	if (InPropertyChangedEvent.MemberProperty->HasAnyPropertyFlags(CPF_Transient))return;
	if (InPropertyChangedEvent.Property->HasAnyPropertyFlags(CPF_Transient))return;

	TryCollectPropertyToOverride(InObject, InPropertyChangedEvent.MemberProperty);
}
void FLGUIPrefabEditor::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
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

void FLGUIPrefabEditor::TryCollectPropertyToOverride(UObject* InObject, FProperty* InMemberProperty)
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
			if (PrefabHelperObject->IsActorBelongsToSubPrefab(Actor))
			{
				PropertyActor = Actor;
			}
		}
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			if (auto Actor = Component->GetOwner())
			{
				if (PrefabHelperObject->IsActorBelongsToSubPrefab(Actor))
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
				PrefabHelperObject->AddMemberPropertyToSubPrefab(PropertyActor, InObject, PropertyName);
				if (auto UIItem = Cast<UUIItem>(InObject))
				{
					if (PropertyName == USceneComponent::GetRelativeLocationPropertyName())//if UI's relative location change, then record anchor data too
					{
						PrefabHelperObject->AddMemberPropertyToSubPrefab(PropertyActor, InObject, UUIItem::GetAnchorDataPropertyName());
					}
					else if (PropertyName == UUIItem::GetAnchorDataPropertyName())//if UI's anchor data change, then record relative location too
					{
						PrefabHelperObject->AddMemberPropertyToSubPrefab(PropertyActor, InObject, USceneComponent::GetRelativeLocationPropertyName());
					}
				}
				//refresh override parameter
				DetailsPtr->RefreshOverrideParameter();
			}
		}
	}
}

void FLGUIPrefabEditor::OnComponentTransformChanged(USceneComponent* InComponent, ETeleportType TeleportType)
{

}
void FLGUIPrefabEditor::OnObjectModified(UObject* InObject)
{

}
void FLGUIPrefabEditor::OnLevelActorAttached(AActor* Actor, const AActor* AttachTo)
{
	if (!bCanNotifyDetachment)return;

	if (PrefabHelperObject->SubPrefabMap.Contains(AttachTo))//not allowed attach to sub prefab's root actor
	{
		auto InfoText = LOCTEXT("NotAllowAttachToSubPrefab", "Trying to change hierarchy of sub prefab's actor, this is not allowed! Please undo this operation!");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
	}

	if (AttachTo == GetPreviewScene().GetRootAgentActor())
	{
		auto InfoText = LOCTEXT("NotAllowAttachToRootAgent", "Trying to attach actor to UIRootAgent, this is not allowed! Please undo this operation!");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
	}
}
void FLGUIPrefabEditor::OnLevelActorDetached(AActor* Actor, const AActor* DetachFrom)
{
	if (!bCanNotifyDetachment)return;

	if (PrefabHelperObject->IsActorBelongsToSubPrefab(Actor) && !PrefabHelperObject->SubPrefabMap.Contains(Actor))//allow sub prefab's root actor detach, but not sub prefab's children actor
	{
		auto InfoText = LOCTEXT("NotAllowDetachFromSubPrefab", "Trying to change hierarchy of sub prefab's actor, this is not allowed! Please undo this operation!");
		FMessageDialog::Open(EAppMsgType::Ok, InfoText);
	}
}

FReply FLGUIPrefabEditor::TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid() && Operation->IsOfType<FAssetDragDropOp>())
	{
		TArray< FAssetData > DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(Operation);
		const int32 NumAssets = DroppedAssetData.Num();

		if (NumAssets > 0)
		{
			TArray<ULGUIPrefab*> PrefabsToLoad;
			for (int32 DroppedAssetIdx = 0; DroppedAssetIdx < NumAssets; ++DroppedAssetIdx)
			{
				const FAssetData& AssetData = DroppedAssetData[DroppedAssetIdx];

				if (!AssetData.IsAssetLoaded())
				{
					GWarn->StatusUpdate(DroppedAssetIdx, NumAssets, FText::Format(LOCTEXT("LoadingAsset", "Loading Asset {0}"), FText::FromName(AssetData.AssetName)));
				}

				UObject* Asset = AssetData.GetAsset();
				auto PrefabAsset = Cast<ULGUIPrefab>(Asset);
				if (PrefabAsset)
				{
					if (PrefabAsset->IsPrefabBelongsToThisSubPrefab(this->PrefabBeingEdited, true))
					{
						auto MsgText = LOCTEXT("Error_EndlessNestedPrefab", "Operation error! Target prefab have this prefab as child prefab, which will result in cyclic nested prefab!");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}
					if (this->PrefabBeingEdited == PrefabAsset)
					{
						auto MsgText = LOCTEXT("Error_SelfPrefabAsSubPrefab", "Operation error! Target prefab is same of this one, self cannot be self's child!");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}

					PrefabsToLoad.Add(PrefabAsset);
				}
			}

			if (PrefabsToLoad.Num() > 0)
			{
				if (CurrentSelectedActor == nullptr)
				{
					auto MsgText = LOCTEXT("Error_NeedParentNode", "Please select a actor as parent actor");
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (CurrentSelectedActor == GetPreviewScene().GetRootAgentActor())
				{
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "{0} cannot be parent actor of child prefab, please choose another actor."), FText::FromString(GetPreviewScene().UIRootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (PrefabHelperObject->SubPrefabMap.Contains(CurrentSelectedActor.Get()))
				{
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "Selected actor belongs to child prefab, which cannot be parent of other child prefab, please choose another actor."), FText::FromString(GetPreviewScene().UIRootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}

				for (auto& PrefabAsset : PrefabsToLoad)
				{
					TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
					TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
					auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabWithExistingObjects(GetPreviewScene().GetWorld()
						, CurrentSelectedActor->GetRootComponent()
						, SubPrefabMapGuidToObject, SubSubPrefabMap
					);

					MakePrefabAsSubPrefab(PrefabAsset, LoadedSubPrefabRootActor, SubPrefabMapGuidToObject, false);
				}
				OnApply();

				if (OutlinerPtr.IsValid())
				{
					OutlinerPtr->FullRefresh();
				}
			}
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}
void FLGUIPrefabEditor::MakePrefabAsSubPrefab(ULGUIPrefab* InPrefab, AActor* InActor, TMap<FGuid, UObject*> InSubMapGuidToObject, bool InApplyChanges)
{
	FLGUISubPrefabData SubPrefabData;
	SubPrefabData.PrefabAsset = InPrefab;

	TArray<AActor*> ChildrenActors;
	LGUIUtils::CollectChildrenActors(InActor, ChildrenActors, false);

	auto FindOrAddSubPrefabObjectGuidInParentPrefab = [&](UObject* InObject) {
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (KeyValue.Value == InObject)
			{
				return KeyValue.Key;
			}
		}
		auto NewGuid = FGuid::NewGuid();
		PrefabHelperObject->MapGuidToObject.Add(NewGuid, InObject);
		return NewGuid;
	};
	for (auto& KeyValue : InSubMapGuidToObject)
	{
		auto GuidInParentPrefab = FindOrAddSubPrefabObjectGuidInParentPrefab(KeyValue.Value);
		if (!SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Contains(GuidInParentPrefab))
		{
			SubPrefabData.MapObjectGuidFromParentPrefabToSubPrefab.Add(GuidInParentPrefab, KeyValue.Key);
		}
	}
	PrefabHelperObject->SubPrefabMap.Add(InActor, SubPrefabData);
	//mark AnchorData, RelativeLocation to default override parameter
	auto RootComp = InActor->GetRootComponent();
	auto RootUIComp = Cast<UUIItem>(RootComp);
	if (RootUIComp)
	{
		PrefabHelperObject->AddMemberPropertyToSubPrefab(InActor, RootUIComp, UUIItem::GetHierarchyIndexPropertyName());
		PrefabHelperObject->AddMemberPropertyToSubPrefab(InActor, RootUIComp, UUIItem::GetAnchorDataPropertyName());
	}
	PrefabHelperObject->AddMemberPropertyToSubPrefab(InActor, RootComp, USceneComponent::GetRelativeLocationPropertyName());

	bAnythingDirty = true;
	if (InApplyChanges)
	{
		//apply prefab immediatelly
		OnApply();
	}
}

#undef LOCTEXT_NAMESPACE