﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
#include "UIAnimation/LGUIAnimationUI.h"
#include "UIAnimation/LGUIAnimationListUI.h"
#include "Core/ActorComponent/UIAnimationComp.h"

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

FLGUIPrefabEditor::FLGUIPrefabEditor()
	:PreviewScene(FLGUIPrefabPreviewScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(false).SetEditor(true))
{
	PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Transactional);
	LGUIPrefabEditorInstanceCollection.Add(this);
}
FLGUIPrefabEditor::~FLGUIPrefabEditor()
{
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
	for (auto& KeyValue : PrefabHelperObject->MapGuidToObject)
	{
		if (auto Actor = Cast<AActor>(KeyValue.Value))
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

class AUIBaseActor* FLGUIPrefabEditor::GetUIMainContainer() const
{
	if (PrefabHelperObject)
	{
		return Cast<AUIBaseActor>(PrefabHelperObject->LoadedRootActor);
	}
	return nullptr;
}

void FLGUIPrefabEditor::SetPrefabModify()
{
	if (PrefabHelperObject && PrefabHelperObject->PrefabAsset)
	{
		PrefabHelperObject->PrefabAsset->Modify();
	}
}

class AActor* FLGUIPrefabEditor::GetSelectedActor() const
{
	return CurrentSelectedActor.Get();
}

UUIAnimationComp* FLGUIPrefabEditor::GetAnimationComp() const
{
	if (AUIBaseActor* MainContainer = GetUIMainContainer())
	{
		return MainContainer->FindComponentByClass<UUIAnimationComp>();
	}
	return nullptr;
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
	PrefabHelperObject->RootAgentActorForPrefabEditor = GetPreviewScene().GetRootAgentActor();
	PrefabHelperObject->MarkAsManagerObject();

	TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SLGUIPrefabEditorViewport, PrefabEditorPtr);
	
	DetailsPtr = SNew(SLGUIPrefabEditorDetails, PrefabEditorPtr);
	AnimationUI = SNew(SLGUIAnimationUI, this);
	AnimationListUI = SNew(SLGUIAnimationListUI, this);

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

	// focus to LGUI RootActor
	OnOutlinerActorDoubleClick(PrefabHelperObject->LoadedRootActor);
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
		ULGUIEditorManagerObject::RefreshAllUI();
	}
}

void FLGUIPrefabEditor::RefreshMainUI()
{
	if (AnimationListUI)
	{
		AnimationListUI->RefreshAnimationListUI();
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
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.VAlign(VAlign_Fill)
				[
					SNew(SSplitter)
					.Orientation(Orient_Vertical)
					+ SSplitter::Slot()
					.Value(0.7f)
					[
						// 3D UI预览
						ViewportPtr.ToSharedRef()
					]
					+ SSplitter::Slot()
					.Value(0.3f)
					[
						// Animation动画编辑器
						SNew(SSplitter)
						+ SSplitter::Slot()
						.Value(0.2f)
						[
							AnimationListUI.ToSharedRef()
						]
						+ SSplitter::Slot()
						.Value(0.8f)
						[
							AnimationUI.ToSharedRef()
						]
					]
				]
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

	if (AnimationUI)
	{
		AnimationUI->SelectLGUIBaseActor(Cast<AUIBaseActor>(Actor));
	}
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
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "{0} cannot be parent actor of child prefab, please choose another actor."), FText::FromString(FLGUIPrefabPreviewScene::RootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (PrefabHelperObject->IsActorBelongsToSubPrefab(CurrentSelectedActor.Get()))
				{
					auto MsgText = FText::Format(LOCTEXT("Error_SubPrefabCannotBeParentNode", "Selected actor belongs to child prefab, which cannot be parent of other child prefab, please choose another actor."), FText::FromString(FLGUIPrefabPreviewScene::RootAgentActorName));
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

					PrefabHelperObject->MakePrefabAsSubPrefab(PrefabAsset, LoadedSubPrefabRootActor, SubPrefabMapGuidToObject, {});
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

PRAGMA_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE