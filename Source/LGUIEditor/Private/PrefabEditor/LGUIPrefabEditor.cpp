// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIPrefabEditor.h"
#include "LGUIEditorModule.h"
#include "LGUIPrefabEditorViewport.h"
#include "LGUIPrefabEditorScene.h"
#include "LGUIPrefabEditorDetails.h"
#include "LGUIPrefabEditorOutliner.h"
#include "LGUIPrefabRawDataViewer.h"
#include "UnrealEdGlobals.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/StaticMeshActor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"
#include "LGUIPrefabEditorCommand.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LGUIEditorTools.h"
#include "Engine/Selection.h"
#include "ToolMenus.h"
#include "LGUIEditorUtils.h"
#include "Editor.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "PrefabAnimation/LGUIPrefabSequenceEditor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Utils/LGUIUtils.h"
#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditor"

PRAGMA_DISABLE_OPTIMIZATION

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

FName GetPrefabWorldName()
{
	static uint32 NameSuffix = 0;
	return FName(*FString::Printf(TEXT("PrefabEditorWorld_%d"), NameSuffix++));
}
FLGUIPrefabEditor::FLGUIPrefabEditor()
	:PreviewScene(FLGUIPrefabEditorScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(false).SetEditor(true).SetName(GetPrefabWorldName()))
{
	PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Transactional);
	LGUIPrefabEditorInstanceCollection.Add(this);
}
FLGUIPrefabEditor::~FLGUIPrefabEditor()
{
	PrefabHelperObject->ConditionalBeginDestroy();
	PrefabHelperObject = nullptr;

	LGUIPrefabEditorInstanceCollection.Remove(this);

	GEditor->SelectNone(true, true);

	ULGUIPrefabManagerObject::MarkBroadcastLevelActorListChanged();
	FLGUIEditorModule::Get().GetNativeSceneOutlinerExtension()->Restore();
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

bool FLGUIPrefabEditor::WorldIsPrefabEditor(UWorld* InWorld)
{
	for (auto Instance : LGUIPrefabEditorInstanceCollection)
	{
		if (Instance->GetWorld() == InWorld)
		{
			return true;
		}
	}
	return false;
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

void FLGUIPrefabEditor::IterateAllPrefabEditor(const TFunction<void(FLGUIPrefabEditor*)>& InFunction)
{
	for (auto Instance : LGUIPrefabEditorInstanceCollection)
	{
		InFunction(Instance);
	}
}

bool FLGUIPrefabEditor::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab)
{
	return PrefabHelperObject->RefreshOnSubPrefabDirty(InSubPrefab);
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

	FBoxSphereBounds Bounds = FBoxSphereBounds(EForceInit::ForceInitToZero);
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
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		FBox Box; bool bIsValidBox = false;
		if (auto SceneComp = Cast<USceneComponent>(KeyValue.Value))
		{
			if (SceneComp->IsRegistered() && !SceneComp->IsVisualizationComponent())
			{
				if (!ULGUIPrefabManagerObject::OnPrefabEditor_GetBounds.ExecuteIfBound(SceneComp, Box, bIsValidBox))
				{
					if (auto PrimitiveComp = Cast<UPrimitiveComponent>(SceneComp))
					{
						Box = PrimitiveComp->Bounds.GetBox();
						bIsValidBox = true;
					}
				}
			}
		}
		if (bIsValidBox)
		{
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

bool FLGUIPrefabEditor::GetAnythingDirty()const 
{ 
	return PrefabHelperObject->GetAnythingDirty();
}

void FLGUIPrefabEditor::CloseWithoutCheckDataDirty()
{
	PrefabHelperObject->SetNothingDirty();
	this->CloseWindow();
}

bool FLGUIPrefabEditor::OnRequestClose()
{
	if (GetAnythingDirty())
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
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::OutlinerID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_Outliner))
		.SetDisplayName(LOCTEXT("OutlinerTabLabel", "Outliner"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Outliner"));

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
	if (PrefabBeingEdited->ReferenceClassList.Contains(nullptr))
	{
		auto MsgText = LOCTEXT("Error_PrefabMissingReferenceClass", "Prefab missing some class reference!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}
	if (PrefabBeingEdited->ReferenceAssetList.Contains(nullptr))
	{
		auto MsgText = LOCTEXT("Error_PrefabMissingReferenceAsset", "Prefab missing some asset reference!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}

	FLGUIPrefabEditorCommand::Register();

	PrefabHelperObject->LoadPrefab(GetPreviewScene().GetWorld(), GetPreviewScene().GetParentComponentForPrefab(PrefabBeingEdited));
	if (!IsValid(PrefabHelperObject->LoadedRootActor))
	{
		auto MsgText = LOCTEXT("Error_LoadPrefabFail", "Load prefab fail! Nothing loaded!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
	}
	PrefabHelperObject->RootAgentActorForPrefabEditor = GetPreviewScene().GetRootAgentActor();
	PrefabHelperObject->MarkAsManagerObject();

	TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SLGUIPrefabEditorViewport, PrefabEditorPtr, PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode);
	
	DetailsPtr = SNew(SLGUIPrefabEditorDetails, PrefabEditorPtr);

	PrefabRawDataViewer = SNew(SLGUIPrefabRawDataViewer, PrefabEditorPtr, PrefabBeingEdited);

	auto UnexpendActorGuidSet = PrefabBeingEdited->PrefabDataForPrefabEditor.UnexpendActorSet;
	TSet<AActor*> UnexpendActorSet;
	for (auto& ItemActorGuid : UnexpendActorGuidSet)
	{
		if (auto ObjectPtr = PrefabHelperObject->MapGuidToObject.Find(ItemActorGuid))
		{
			if (auto Actor = Cast<AActor>(*ObjectPtr))
			{
				UnexpendActorSet.Add(Actor);
			}
		}
	}
	OutlinerPtr = MakeShared<FLGUIPrefabEditorOutliner>();
	OutlinerPtr->ActorFilter = FOnShouldFilterActor::CreateRaw(this, &FLGUIPrefabEditor::IsFilteredActor);
	OutlinerPtr->OnActorPickedDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerPickedChanged);
	OutlinerPtr->OnActorDoubleClickDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerActorDoubleClick);
	OutlinerPtr->InitOutliner(GetPreviewScene().GetWorld(), PrefabEditorPtr, UnexpendActorSet);

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

	// After opening a prefab, broadcast event to LGUIPrefabSequencerEditor
	LGUIEditorTools::OnEditingPrefabChanged.Broadcast(GetPreviewScene().GetRootAgentActor());
}

TArray<AActor*> FLGUIPrefabEditor::GetAllActors()
{
	TArray<AActor*> AllActors;
	if (PrefabHelperObject->LoadedRootActor != nullptr)
	{
		LGUIUtils::CollectChildrenActors(PrefabHelperObject->LoadedRootActor, AllActors, true);
	}
	return AllActors;
}

void FLGUIPrefabEditor::GetInitialViewLocationAndRotation(FVector& OutLocation, FRotator& OutRotation, FVector& OutOrbitLocation)
{
	auto& PrefabEditorData = PrefabBeingEdited->PrefabDataForPrefabEditor;
	auto SceneBounds = this->GetAllObjectsBounds();
	if (PrefabEditorData.ViewLocation == FVector::ZeroVector && PrefabEditorData.ViewRotation == FRotator::ZeroRotator)
	{
		OutLocation = FVector(-SceneBounds.SphereRadius * 1.2f, SceneBounds.Origin.Y, SceneBounds.Origin.Z);
		OutRotation = FRotator::ZeroRotator;
	}
	else
	{
		OutLocation = PrefabEditorData.ViewLocation;
		OutRotation = PrefabEditorData.ViewRotation;
	}
	if (PrefabEditorData.ViewOrbitLocation == FVector::ZeroVector)
	{
		OutOrbitLocation = SceneBounds.Origin;
	}
	else
	{
		OutOrbitLocation = PrefabEditorData.ViewOrbitLocation;
	}
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

	TArray<AActor*> SelectedActorArray;
	for (auto Item : InSelectedActorArray)
	{
		if (Item.IsValid())
		{
			SelectedActorArray.Add(Item.Get());
		}
	}
	LGUIEditorTools::DeleteActors_Impl(SelectedActorArray);
}

void FLGUIPrefabEditor::ApplyPrefab()
{
	OnApply();
}

void FLGUIPrefabEditor::SaveAsset_Execute()
{
	if (CheckBeforeSaveAsset())
	{
		if (GetAnythingDirty())
		{
			OnApply();//apply change
		}
		FAssetEditorToolkit::SaveAsset_Execute();//save asset
	}
}
void FLGUIPrefabEditor::OnApply()
{
	if (CheckBeforeSaveAsset())
	{
		//save view location and rotation
		auto ViewTransform = ViewportPtr->GetViewportClient()->GetViewTransform();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewLocation = ViewTransform.GetLocation();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewRotation = ViewTransform.GetRotation();
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewOrbitLocation = ViewTransform.GetLookAt();
		if (auto RootAgentActor = GetPreviewScene().GetRootAgentActor())
		{
			if (!ULGUIPrefabManagerObject::OnPrefabEditor_SavePrefab.ExecuteIfBound(RootAgentActor, PrefabBeingEdited))
			{
				PrefabBeingEdited->PrefabDataForPrefabEditor.bNeedCanvas = false;
			}
		}
		PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode = ViewportPtr->GetViewportClient()->GetViewMode();
		TSet<FGuid> UnexpendActorGuidArray;
		TArray<AActor*> UnexpendActorArray;
		LGUIUtils::CollectChildrenActors(PrefabHelperObject->LoadedRootActor, UnexpendActorArray, true);
		OutlinerPtr->GetUnexpendActor(UnexpendActorArray);
		for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
		{
			if (UnexpendActorArray.Contains(KeyValue.Value))
			{
				UnexpendActorGuidArray.Add(KeyValue.Key);
			}
		}
		PrefabBeingEdited->PrefabDataForPrefabEditor.UnexpendActorSet = UnexpendActorGuidArray;

		//refresh parameter, remove invalid
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			KeyValue.Value.CheckParameters();
		}

		LGUIEditorTools::OnBeforeApplyPrefab.Broadcast(PrefabHelperObject);
		PrefabHelperObject->SavePrefab();
		LGUIEditorTools::RefreshLevelLoadedPrefab(PrefabHelperObject->PrefabAsset);
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
	}
}

void FLGUIPrefabEditor::OnOpenRawDataViewerPanel()
{
	this->InvokeTab(FLGUIPrefabEditorTabs::PrefabRawDataViewerID);
}
void FLGUIPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(PrefabHelperObject);
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

	return true;
}

FLGUIPrefabEditorScene& FLGUIPrefabEditor::GetPreviewScene()
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
	ToolkitCommands->MapAction(
		PrefabEditorCommands.OpenPrefabHelperObject,
		FExecuteAction::CreateSP(this, &FLGUIPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);

	ToolkitCommands->MapAction(
		PrefabEditorCommands.CopyActor,
		FExecuteAction::CreateStatic(&LGUIEditorTools::CopySelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanCopyActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanCopyActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.PasteActor,
		FExecuteAction::CreateStatic(&LGUIEditorTools::CutSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanCutActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanCutActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.PasteActor,
		FExecuteAction::CreateStatic(&LGUIEditorTools::PasteSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanPasteActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanPasteActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.DuplicateActor,
		FExecuteAction::CreateStatic(&LGUIEditorTools::DuplicateSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanDuplicateActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanDuplicateActor)
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.DestroyActor,
		FExecuteAction::CreateStatic(&LGUIEditorTools::DeleteSelectedActors_Impl),
		FCanExecuteAction::CreateStatic(&LGUIEditorTools::CanDeleteActor),
		FGetActionCheckState(),
		FIsActionButtonVisible::CreateStatic(&LGUIEditorTools::CanDeleteActor)
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
			auto ApplyButtonMenuEntry = FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().Apply
				, LOCTEXT("Apply", "Apply")
				, TAttribute<FText>::CreateLambda([this]() {return GetAnythingDirty() ? LOCTEXT("Apply_Tooltip", "Dirty, need to apply") : LOCTEXT("Apply_Tooltip", "Good to go"); })
				, TAttribute<FSlateIcon>(this, &FLGUIPrefabEditor::GetApplyButtonStatusImage));

			FToolMenuSection& Section = ToolBar->AddSection("LGUIPrefabCommands", TAttribute<FText>(), InsertAfterAssetSection);
			Section.AddEntry(ApplyButtonMenuEntry);
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().RawDataViewer));
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().OpenPrefabHelperObject));
		}
	}
}

FSlateIcon FLGUIPrefabEditor::GetApplyButtonStatusImage()const
{
	static const FName CompileStatusBackground("Blueprint.CompileStatus.Background");
	static const FName CompileStatusUnknown("Blueprint.CompileStatus.Overlay.Unknown");
	static const FName CompileStatusGood("Blueprint.CompileStatus.Overlay.Good");

	return FSlateIcon(FAppStyle::GetAppStyleSetName(), CompileStatusBackground, NAME_None, GetAnythingDirty() ? CompileStatusUnknown : CompileStatusGood);
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
				.TextStyle(FAppStyle::Get(), "Graph.CornerText")
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
			TArray<UClass*> PotentialActorClassesToLoad;
			TArray<UStaticMesh*> PotentialStaticMeshesToLoad;
			auto IsSupportedActorClass = [](UClass* ActorClass) {
				if (ActorClass->HasAnyClassFlags(EClassFlags::CLASS_NotPlaceable | EClassFlags::CLASS_Abstract))
					return false;
				if (!ActorClass->IsChildOf(AActor::StaticClass()))return false;
				return true;
			};
			for (int32 DroppedAssetIdx = 0; DroppedAssetIdx < NumAssets; ++DroppedAssetIdx)
			{
				const FAssetData& AssetData = DroppedAssetData[DroppedAssetIdx];

				if (!AssetData.IsAssetLoaded())
				{
					GWarn->StatusUpdate(DroppedAssetIdx, NumAssets, FText::Format(LOCTEXT("LoadingAsset", "Loading Asset {0}"), FText::FromName(AssetData.AssetName)));
				}

				UClass* AssetClass = AssetData.GetClass();
				UObject* Asset = AssetData.GetAsset();
				UBlueprint* BPClass = Cast<UBlueprint>(Asset);
				UClass* PotentialActorClass = nullptr;
				if ((BPClass != nullptr) && (BPClass->GeneratedClass != nullptr))
				{
					if (IsSupportedActorClass(BPClass->GeneratedClass))
					{
						PotentialActorClass = BPClass->GeneratedClass;
					}
				}
				else if (AssetClass->IsChildOf(UClass::StaticClass()))
				{
					UClass* AssetAsClass = CastChecked<UClass>(Asset);
					if (IsSupportedActorClass(AssetAsClass))
					{
						PotentialActorClass = AssetAsClass;
					}
				}
				if (auto PrefabAsset = Cast<ULGUIPrefab>(Asset))
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
					if (PrefabAsset->PrefabVersion <= (uint16)ELGUIPrefabVersion::OldVersion)
					{
						auto MsgText = LOCTEXT("Error_UnsupportOldPrefabVersion", "Operation error! Target prefab's version is too old! Please make it newer: open the prefab and hit \"Save\" button.");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}

					PrefabsToLoad.Add(PrefabAsset);
				}
				else if (auto StaticMeshAsset = Cast<UStaticMesh>(Asset))
				{
					PotentialStaticMeshesToLoad.Add(StaticMeshAsset);
				}
				else if (PotentialActorClass != nullptr)
				{
					PotentialActorClassesToLoad.Add(PotentialActorClass);
				}
			}

			if (PrefabsToLoad.Num() > 0 || PotentialActorClassesToLoad.Num() > 0 || PotentialStaticMeshesToLoad.Num() > 0)
			{
				if (CurrentSelectedActor == nullptr)
				{
					auto MsgText = LOCTEXT("Error_NeedParentNode", "Please select a actor as parent actor");
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (CurrentSelectedActor == GetPreviewScene().GetRootAgentActor())
				{
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "{0} cannot be parent actor of child prefab, please choose another actor."), FText::FromString(FLGUIPrefabEditorScene::RootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
			}
			else
			{
				return FReply::Unhandled();
			}

			GEditor->BeginTransaction(LOCTEXT("CreateFromAssetDrop_Transaction", "LGUI Create from asset drop"));
			TArray<AActor*> CreatedActorArray;
			if (PrefabsToLoad.Num() > 0)
			{
				PrefabHelperObject->SetCanNotifyAttachment(false);
				for (auto& PrefabAsset : PrefabsToLoad)
				{
					TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
					TMap<TObjectPtr<AActor>, FLGUISubPrefabData> SubSubPrefabMap;
					auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabWithExistingObjects(GetPreviewScene().GetWorld()
						, CurrentSelectedActor->GetRootComponent()
						, SubPrefabMapGuidToObject, SubSubPrefabMap
					);

					PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedSubPrefabRootActor, SubPrefabMapGuidToObject, {});
					CreatedActorArray.Add(LoadedSubPrefabRootActor);
				}
				OnApply();
				PrefabHelperObject->SetCanNotifyAttachment(true);

				if (OutlinerPtr.IsValid())
				{
					OutlinerPtr->FullRefresh();
					ULGUIPrefabManagerObject::AddOneShotTickFunction([=] {
						for (auto& Actor : CreatedActorArray)
						{
							OutlinerPtr->UnexpandActor(Actor);
						}
						}, 1);//delay execute, because the outliner not create actor yet
				}
			}
			if (PotentialActorClassesToLoad.Num() > 0)
			{
				for (auto& ActorClass : PotentialActorClassesToLoad)
				{
					if (auto Actor = this->GetWorld()->SpawnActor<AActor>(ActorClass, FActorSpawnParameters()))
					{
						if (auto RootComp = Actor->GetRootComponent())
						{
							RootComp->AttachToComponent(CurrentSelectedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
							CreatedActorArray.Add(Actor);
						}
						else
						{
							Actor->ConditionalBeginDestroy();
						}
					}
				}
			}
			if (PotentialStaticMeshesToLoad.Num() > 0)
			{
				for (auto& Mesh : PotentialStaticMeshesToLoad)
				{
					auto MeshActor = this->GetWorld()->SpawnActor<AStaticMeshActor>();
					MeshActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
					MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
					MeshActor->GetStaticMeshComponent()->AttachToComponent(CurrentSelectedActor->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
					MeshActor->SetActorLabel(Mesh->GetName());
					CreatedActorArray.Add(MeshActor);
				}
			}
			if (CreatedActorArray.Num() > 0)
			{
				GEditor->SelectNone(true, true);
				for (auto& Actor : CreatedActorArray)
				{
					GEditor->SelectActor(Actor, true, true, false, true);
				}
			}
			GEditor->EndTransaction();
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE