// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIEditorModule.h"
#include "LGUIPrefabEditorViewport.h"
#include "LGUIPrefabPreviewScene.h"
#include "LGUIPrefabEditorDetails.h"
#include "LGUIPrefabEditorOutliner.h"
#include "LGUIPrefabOverrideParameterEditor.h"
#include "LGUIPrefabRawDataViewer.h"
#include "UnrealEdGlobals.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#include "LGUIPrefabEditorCommand.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "LGUIEditorTools.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "Engine/Selection.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditor"

const FName PrefabEditorAppName = FName(TEXT("LGUIPrefabEditorApp"));

TArray<FLGUIPrefabEditor*> FLGUIPrefabEditor::LGUIPrefabEditorInstanceCollection;

struct FLGUIPrefabEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName OutlinerID;
	static const FName OverrideParameterID;
	static const FName PrefabRawDataViewerID;
};

const FName FLGUIPrefabEditorTabs::DetailsID(TEXT("Details"));
const FName FLGUIPrefabEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLGUIPrefabEditorTabs::OutlinerID(TEXT("Outliner"));
const FName FLGUIPrefabEditorTabs::OverrideParameterID(TEXT("OverrideParameter"));
const FName FLGUIPrefabEditorTabs::PrefabRawDataViewerID(TEXT("PrefabRawDataViewer"));

FLGUIPrefabEditor::FLGUIPrefabEditor()
	:PreviewScene(FLGUIPrefabPreviewScene::ConstructionValues().AllowAudioPlayback(true).ShouldSimulatePhysics(false).SetEditor(true))
{
	PrefabHelperObject = NewObject<ULGUIPrefabHelperObject>(this->GetWorld());
	PrefabHelperObject->AddToRoot();
	PrefabHelperObject->bIsInsidePrefabEditor = true;
	LGUIPrefabEditorInstanceCollection.Add(this);
}
FLGUIPrefabEditor::~FLGUIPrefabEditor()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(this->OnObjectPropertyChangedDelegateHandle);
	FCoreUObjectDelegates::OnPreObjectPropertyChanged.Remove(this->OnPreObjectPropertyChangedDelegateHandle);

	PrefabHelperObject->RemoveFromRoot();
	PrefabHelperObject->ConditionalBeginDestroy();

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

void FLGUIPrefabEditor::RefreshOnSubPrefabDirty(ULGUIPrefab* InSubPrefab)
{
	bool AnythingChange = false;
	if (PrefabHelperObject->PrefabAsset->ReferenceAssetList.Contains(InSubPrefab))
	{
		for (auto& KeyValue : PrefabHelperObject->SubPrefabMap)
		{
			if (KeyValue.Value.PrefabAsset == InSubPrefab)
			{
				if (KeyValue.Value.OverrideParameterObject->RefreshParameterOnTemplate(InSubPrefab->AgentOverrideParameterObject))
				{
					AnythingChange = true;
				}
				if (KeyValue.Value.OverrideParameterObject->RefreshAutomaticParameter())
				{
					AnythingChange = true;
				}
			}
		}
	}
	if (AnythingChange)
	{
		PrefabHelperObject->SavePrefab();
		PrefabHelperObject->PrefabAsset->MarkPackageDirty();
	}
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

	InTabManager->RegisterTabSpawner(FLGUIPrefabEditorTabs::OverrideParameterID, FOnSpawnTab::CreateSP(this, &FLGUIPrefabEditor::SpawnTab_OverrideParameter))
		.SetDisplayName(LOCTEXT("OverrideParameterTabLabel", "OverrideParameter"))
		.SetGroup(WorkspaceMenuCategoryRef)
		;
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
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::OverrideParameterID);
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

	OverrideParameterPtr = SNew(SLGUIPrefabOverrideParameterEditor, PrefabEditorPtr);
	OverrideParameterPtr->SetTargetObject(PrefabHelperObject->PrefabOverrideParameterObject, PrefabHelperObject->LoadedRootActor);
	OverrideParameterPtr->SetTipText(PrefabHelperObject->LoadedRootActor->GetActorLabel());

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
}

void FLGUIPrefabEditor::ApplySubPrefabParameterChange(AActor* InSubPrefabActor)
{
	check(PrefabHelperObject->SubPrefabMap.Contains(InSubPrefabActor));
	//refresh data
	auto& SubPrefabData = PrefabHelperObject->SubPrefabMap[InSubPrefabActor];
	SubPrefabData.OverrideParameterObject->ApplyParameter();

	ULGUIEditorManagerObject::RefreshAllUI();
}

void FLGUIPrefabEditor::DeleteActors(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray)
{
	for (auto Item : InSelectedActorArray)
	{
		if (Item == PrefabHelperObject->LoadedRootActor)
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root actor!."));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
		if (Item == GetPreviewScene().GetRootAgentActor())
		{
			auto WarningMsg = FString::Printf(TEXT("Cannot destroy root agent actor!."));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(WarningMsg));
			return;
		}
	}

	auto confirmMsg = FString::Printf(TEXT("Destroy selected actors? This will also destroy the children attached to selected actors."));
	auto confirmResult = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(confirmMsg));
	if (confirmResult == EAppReturnType::No)return;

	GEditor->BeginTransaction(LOCTEXT("DeleteActors", "FLGUIPrefabEditor DeleteActors"));
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

void FLGUIPrefabEditor::SaveAsset_Execute()
{
	if (CheckBeforeSaveAsset())
	{
		PrefabHelperObject->SavePrefab();
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
		FAssetEditorToolkit::SaveAsset_Execute();
		bAnythingDirty = false;
	}
}
void FLGUIPrefabEditor::OnApply()
{
	if (CheckBeforeSaveAsset())
	{
		PrefabHelperObject->SavePrefab();
		LGUIEditorTools::RefreshOnSubPrefabChange(PrefabHelperObject->PrefabAsset);
		bAnythingDirty = false;
	}
}

void FLGUIPrefabEditor::OnOpenOverrideParameterPanel()
{
	this->InvokeTab(FLGUIPrefabEditorTabs::OverrideParameterID);
}
void FLGUIPrefabEditor::OnOpenRawDataViewerPanel()
{
	this->InvokeTab(FLGUIPrefabEditorTabs::PrefabRawDataViewerID);
}

void FLGUIPrefabEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrefabBeingEdited);
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

	if (PrefabHelperObject->PrefabOverrideParameterObject->HasRepeatedParameter())
	{
		auto MsgText = LOCTEXT("Error_HaveRepeatedParameter", "Repeated override parameter! More than one override parameter point to the same parameter, this is not allowed! Please remove repeated parameter!");
		FMessageDialog::Open(EAppMsgType::Ok, MsgText);
		return false;
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
		PrefabEditorCommands.OverrideParameter,
		FExecuteAction::CreateSP(this, &FLGUIPrefabEditor::OnOpenOverrideParameterPanel),
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
			Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLGUIPrefabEditorCommand::Get().OverrideParameter));
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

TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_OverrideParameter(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OverrideParameterTab_Title", "OverrideParameter"))
		[
			OverrideParameterPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLGUIPrefabEditor::SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OverrideParameterTab_Title", "OverrideParameter"))
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
	if (PrefabHelperObject->SubPrefabMap.Contains(Actor))
	{
		PrefabHelperObject->SubPrefabMap[Actor].OverrideParameterObject->SetParameterDisplayType(false);
		OverrideParameterPtr->SetTargetObject(PrefabHelperObject->SubPrefabMap[Actor].OverrideParameterObject, Actor);
		OverrideParameterPtr->SetTipText(Actor->GetActorLabel());
	}
	else if (Actor == PrefabHelperObject->LoadedRootActor)
	{
		PrefabHelperObject->PrefabOverrideParameterObject->SetParameterDisplayType(true);
		OverrideParameterPtr->SetTargetObject(PrefabHelperObject->PrefabOverrideParameterObject, Actor);
		OverrideParameterPtr->SetTipText(Actor->GetActorLabel());
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

void FLGUIPrefabEditor::OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent)
{
	if (InObject->GetWorld() == this->GetWorld())
	{
		bAnythingDirty = true;

		AActor* SubPrefabRootActor = nullptr;
		if (auto Actor = Cast<AActor>(InObject))
		{
			if (PrefabHelperObject->SubPrefabMap.Contains(Actor))//if is subprefab's root actor
			{
				SubPrefabRootActor = Actor;
			}
		}
		if (auto Component = Cast<UActorComponent>(InObject))
		{
			if (auto Actor = Component->GetOwner())
			{
				if (PrefabHelperObject->SubPrefabMap.Contains(Actor))//if is subprefab's root actor's component
				{
					SubPrefabRootActor = Actor;
				}
			}
		}
		if (SubPrefabRootActor)//automatic record root actor's property
		{
			auto ChangedProperty = InPropertyChangedEvent.MemberProperty;
			ELGUIPrefabOverrideParameterType ParamType;
			if (ULGUIPrefabOverrideParameterHelper::IsSupportedProperty(ChangedProperty, ParamType))
			{
				PrefabHelperObject->PrefabOverrideParameterObject->AddOrUpdateParameterToAutomaticParameters(SubPrefabRootActor, InObject, ChangedProperty, ParamType);
			}
		}
	}
}
void FLGUIPrefabEditor::OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& InEditPropertyChain)
{

}

FReply FLGUIPrefabEditor::TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent)
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

	struct Local
	{
	private:
		static bool ContainsOtherPrefabAsSubPrefab(const TArray<UObject*>& ReferenceAssetList, ULGUIPrefab* InTestPrefab)
		{
			if (ReferenceAssetList.Contains(InTestPrefab))
			{
				return true;
			}
			for (auto Asset : ReferenceAssetList)
			{
				if (auto PrefabAsset = Cast<ULGUIPrefab>(Asset))
				{
					if (ContainsOtherPrefabAsSubPrefab(PrefabAsset->ReferenceAssetList, InTestPrefab))
					{
						return true;
					}
				}
			}
			return false;
		}
	public:
		static bool ContainsOtherPrefabAsSubPrefab(ULGUIPrefab* InPrefab, ULGUIPrefab* InTestPrefab)
		{
			return ContainsOtherPrefabAsSubPrefab(InPrefab->ReferenceAssetList, InTestPrefab);
		}
	};

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
					if (Local::ContainsOtherPrefabAsSubPrefab(PrefabAsset, this->PrefabBeingEdited))
					{
						auto MsgText = LOCTEXT("Error_EndlessNestedPrefab", "Operation error! Target prefab have this prefab as child prefab, which will result in endless nested prefab!");
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

			GWarn->BeginSlowTask(LOCTEXT("LoadingAssets", "Loading Asset(s)"), true);
			for (auto& PrefabAsset : PrefabsToLoad)
			{
				FLGUISubPrefabData SubPrefabData;
				SubPrefabData.PrefabAsset = PrefabAsset;

				TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
				TMap<AActor*, FLGUISubPrefabData> SubSubPrefabMap;
				ULGUIPrefabOverrideParameterObject* SubPrefabOverrideParameterObject = nullptr;
				auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabForEdit(GetPreviewScene().GetWorld()
					, CurrentSelectedActor->GetRootComponent()
					, SubPrefabMapGuidToObject, SubSubPrefabMap
					, PrefabAsset->OverrideParameterData, SubPrefabOverrideParameterObject
				);
				SubPrefabData.OverrideParameterObject = SubPrefabOverrideParameterObject;
				SubPrefabData.OverrideParameterData = PrefabAsset->OverrideParameterData;

				for (auto KeyValue : SubPrefabMapGuidToObject)
				{
					if (auto ActorItem = Cast<AActor>(KeyValue.Value))
					{
						if (LoadedSubPrefabRootActor != ActorItem)
						{
							ULGUIPrefabHelperObject::SetActorPropertyInOutliner(ActorItem, false);
						}
					}
				}

				PrefabHelperObject->SubPrefabMap.Add(LoadedSubPrefabRootActor, SubPrefabData);
			}

			if (OutlinerPtr.IsValid())
			{
				OutlinerPtr->FullRefresh();
			}

			GWarn->EndSlowTask();
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE