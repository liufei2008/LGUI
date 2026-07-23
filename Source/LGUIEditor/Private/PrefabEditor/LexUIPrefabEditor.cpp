// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditor.h"
#include "LexUIPrefabEditorViewport.h"
#include "LexUIPrefabEditorDetails.h"
#include "LexUIPrefabRawDataViewer.h"
#include "EditorModeManager.h"
#include "GameFramework/Actor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"
#include "LexUIPrefabEditorCommand.h"
#include "LexUIEditorTools.h"
#include "ToolMenus.h"
#include "Editor.h"
#include "LexWidgetEditorHierarchyView.h"
#include "UMGStyle.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexWidget.h"
#include "Framework/Commands/GenericCommands.h"
#include "PrefabSystem/LexUIPrefabInstanceScene.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "PrefabAnimation/LexUIPrefabSequenceEditor.h"

#define LOCTEXT_NAMESPACE "LexUIPrefabEditor"

const FName PrefabEditorAppName = FName(TEXT("LexUIPrefabEditorApp"));

TArray<FLexUIPrefabEditor*> FLexUIPrefabEditor::PrefabEditorInstanceCollection;

struct FLexUIPrefabEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName OutlinerID;
	static const FName SequencerID;
	static const FName PrefabRawDataViewerID;
};

const FName FLexUIPrefabEditorTabs::DetailsID(TEXT("Details"));
const FName FLexUIPrefabEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLexUIPrefabEditorTabs::OutlinerID(TEXT("Outliner"));
const FName FLexUIPrefabEditorTabs::SequencerID(TEXT("Sequencer"));
const FName FLexUIPrefabEditorTabs::PrefabRawDataViewerID(TEXT("PrefabRawDataViewer"));

FName GetPrefabWorldName()
{
	static uint32 NameSuffix = 0;
	return FName(*FString::Printf(TEXT("PrefabEditorWorld_%d"), NameSuffix++));
}
FLexUIPrefabEditor::FLexUIPrefabEditor()
{
	PrefabEditorInstanceCollection.Add(this);

	GEditor->RegisterForUndo(this);
}
FLexUIPrefabEditor::~FLexUIPrefabEditor()
{
	PrefabEditorInstanceCollection.Remove(this);

 	ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->EventOnOutlineChanged.RemoveAll(this);
	ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->bShouldTickInEditor = false;
	ULexUISelection::GetInstance(GetWorld())->OnSelectionChanged.RemoveAll(this);
	ULexUISelection::GetInstance(GetWorld())->SelectNone();

	GEditor->UnregisterForUndo(this);
}

FLexUIPrefabEditor* FLexUIPrefabEditor::GetEditorForPrefabIfValid(ULexUIPrefab* InPrefab)
{
	for (auto Instance : PrefabEditorInstanceCollection)
	{
		if (Instance->PrefabBeingEdited == InPrefab)
		{
			return Instance;
		}
	}
	return nullptr;
}

bool FLexUIPrefabEditor::WorldIsPrefabEditor(UWorld* InWorld)
{
	for (auto Instance : PrefabEditorInstanceCollection)
	{
		if (Instance->GetWorld() == InWorld)
		{
			return true;
		}
	}
	return false;
}

TWeakPtr<FLexUIPrefabEditor> FLexUIPrefabEditor::GetEditorByWorld(UWorld* InWorld)
{
	for (auto Instance : PrefabEditorInstanceCollection)
	{
		if (Instance->GetWorld() == InWorld)
		{
			return SharedThis(Instance);
		}
	}
	return nullptr;
}

bool FLexUIPrefabEditor::WidgetIsRootAgent(ULexWidget* InWidget)
{
	for (auto Instance : PrefabEditorInstanceCollection)
	{
		if (InWidget == Instance->GetPreviewScene()->GetRootAgent())
		{
			return true;
		}
	}
	return false;
}

void FLexUIPrefabEditor::IterateAllPrefabEditor(const TFunction<void(FLexUIPrefabEditor*)>& InFunction)
{
	for (auto Instance : PrefabEditorInstanceCollection)
	{
		InFunction(Instance);
	}
}

bool FLexUIPrefabEditor::GetSelectedObjectsBounds(FBoxSphereBounds& OutResult)
{
	FBoxSphereBounds Bounds = FBoxSphereBounds(EForceInit::ForceInitToZero);
	bool IsFirstBounds = true;
	for (auto& Widget : SelectedWidgets)
	{
		if (!Widget->GetWidgetActiveInHierarchy())
		{
			continue;
		}
		FBox LocalBounds;
		if (auto Visual = Widget->GetVisual())
		{
			FVector Min, Max;
			Visual->GetGeometryBounds3DInLocalSpace(Min, Max);
			LocalBounds = FBox(Min, Max);
		}
		else
		{
			auto Min2D = Widget->GetLocalSpaceLeftBottomPoint();
			auto Max2D = Widget->GetLocalSpaceRightTopPoint();
			LocalBounds = FBox(FVector(0, Min2D.X, Min2D.Y), FVector(0, Max2D.X, Max2D.Y));
		}
		auto Box = LocalBounds.TransformBy(Widget->GetWorldTransform());
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

FBoxSphereBounds FLexUIPrefabEditor::GetAllObjectsBounds()
{
	FBoxSphereBounds Bounds;
	bool IsFirstBounds = true;
	for (auto& KeyValue : GetPrefabHelperObject()->MapGuidToObject)
	{
		FBox Box; bool bIsValidBox = false;
		if (auto Widget = Cast<ULexWidget>(KeyValue.Value))
		{
			if (Widget->GetWidgetActiveInHierarchy())
			{
				FBox LocalBounds;
				if (auto Visual = Widget->GetVisual())
				{
					FVector Min, Max;
					Visual->GetGeometryBounds3DInLocalSpace(Min, Max);
					LocalBounds = FBox(Min, Max);
				}
				else
				{
					auto Min2D = Widget->GetLocalSpaceLeftBottomPoint();
					auto Max2D = Widget->GetLocalSpaceRightTopPoint();
					LocalBounds = FBox(FVector(0, Min2D.X, Min2D.Y), FVector(0, Max2D.X, Max2D.Y));
				}
				Box = LocalBounds.TransformBy(Widget->GetWorldTransform());
				bIsValidBox = true;
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

bool FLexUIPrefabEditor::WidgetBelongsToSubPrefab(ULexWidget* InWidget)
{
	return GetPrefabHelperObject()->IsWidgetBelongsToSubPrefab(InWidget);
}

bool FLexUIPrefabEditor::WidgetIsSubPrefabRoot(ULexWidget* InSubPrefabRootWidget)
{
	return GetPrefabHelperObject()->SubPrefabMap.Contains(InSubPrefabRootWidget);
}

FLexUISubPrefabData FLexUIPrefabEditor::GetSubPrefabDataForActor(ULexWidget* InSubPrefabWidget)
{
	return GetPrefabHelperObject()->GetSubPrefabData(InSubPrefabWidget);
}

void FLexUIPrefabEditor::OpenSubPrefab(ULexWidget* InSubPrefabWidget)
{
	if (auto SubPrefabAsset = GetPrefabHelperObject()->GetSubPrefabAsset(InSubPrefabWidget))
	{
		auto PrefabEditor = FLexUIPrefabEditor::GetEditorForPrefabIfValid(SubPrefabAsset);
		if (!PrefabEditor)
		{
			UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
			AssetEditorSubsystem->OpenEditorForAsset(SubPrefabAsset);
		}
	}
}
void FLexUIPrefabEditor::SelectSubPrefab(ULexWidget* InSubPrefabWidget)
{
	if (auto SubPrefabAsset = GetPrefabHelperObject()->GetSubPrefabAsset(InSubPrefabWidget))
	{
		TArray<UObject*> ObjectsToSync;
		ObjectsToSync.Add(SubPrefabAsset);
		GEditor->SyncBrowserToObjects(ObjectsToSync);
	}
}

bool FLexUIPrefabEditor::GetAnythingDirty()const 
{ 
	return GetPrefabHelperObject()->GetAnythingDirty();
}

namespace
{
	void SyncWidgetRegisterStateAfterTransaction(FLexUIPrefabEditor* InEditor)
	{
		InEditor->GetPreviewScene()->GetRootAgent()->EnsureChildrenAfterTransaction();
		TArray<ULexWidget*> ReachableWidgets;
		ULexWidget::CollectChildrenWidgets(InEditor->GetPreviewScene()->GetRootAgent(), ReachableWidgets);

		TSet<ULexWidget*> AllWidgets;
		ForEachObjectOfClass(ULexWidget::StaticClass(), [&](UObject* Object)
		{
			if (auto Widget = Cast<ULexWidget>(Object))
			{
				if (Widget->GetWorld() == InEditor->GetWorld())
				{
					AllWidgets.Add(Widget);
				}
			}
		});
		for (auto Widget : AllWidgets)
		{
			if (!ReachableWidgets.Contains(Widget))
			{
				if (Widget->HasRegistered())
				{
					Widget->OnUnregister();
				}
			}
		}
	}
}

void FLexUIPrefabEditor::SyncSelection()
{
	SelectedWidgets = ULexUISelection::GetInstance(GetWorld())->GetSelectedWidgets();
	OnSelectionChanged.Broadcast();
	OutlinerPtr->RequestRefresh();
}

void FLexUIPrefabEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_LexUIPrefabEditor", "LexUIPrefab Editor"));
	auto WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(FLexUIPrefabEditorTabs::ViewportID, FOnSpawnTab::CreateSP(this, &FLexUIPrefabEditor::SpawnTab_Viewport))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));

	InTabManager->RegisterTabSpawner(FLexUIPrefabEditorTabs::DetailsID, FOnSpawnTab::CreateSP(this, &FLexUIPrefabEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTabLabel", "Details"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(FLexUIPrefabEditorTabs::OutlinerID, FOnSpawnTab::CreateSP(this, &FLexUIPrefabEditor::SpawnTab_Outliner))
		.SetDisplayName(LOCTEXT("OutlinerTabLabel", "Outliner"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Outliner"));

	InTabManager->RegisterTabSpawner(FLexUIPrefabEditorTabs::SequencerID, FOnSpawnTab::CreateSP(this, &FLexUIPrefabEditor::SpawnTab_Sequencer))
		.SetDisplayName(LOCTEXT("SequencerTabLabel", "Sequencer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FUMGStyle::GetStyleSetName(), "Animations.TabIcon"));

	InTabManager->RegisterTabSpawner(FLexUIPrefabEditorTabs::PrefabRawDataViewerID, FOnSpawnTab::CreateSP(this, &FLexUIPrefabEditor::SpawnTab_PrefabRawDataViewer))
		.SetDisplayName(LOCTEXT("PrefabRawDataViewerTabLabel", "PrefabRawDataViewer"))
		.SetGroup(WorkspaceMenuCategoryRef)
		;
}
void FLexUIPrefabEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FLexUIPrefabEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FLexUIPrefabEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FLexUIPrefabEditorTabs::OutlinerID);
	InTabManager->UnregisterTabSpawner(FLexUIPrefabEditorTabs::SequencerID);
	InTabManager->UnregisterTabSpawner(FLexUIPrefabEditorTabs::PrefabRawDataViewerID);
}

void FLexUIPrefabEditor::PostUndo(bool bSuccess)
{
	SyncWidgetRegisterStateAfterTransaction(this);
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->EventOnOutlineChanged.Broadcast();
	SelectedWidgets = ULexUISelection::GetInstance(GetWorld())->GetSelectedWidgets();
	OnSelectionChanged.Broadcast();
	OutlinerPtr->RequestRefresh();
}
void FLexUIPrefabEditor::PostRedo(bool bSuccess)
{
	SyncWidgetRegisterStateAfterTransaction(this);
	ULexUIManagerWorldSubsystem::RefreshAllUI();
	SelectedWidgets = ULexUISelection::GetInstance(GetWorld())->GetSelectedWidgets();
	OnSelectionChanged.Broadcast();
	OutlinerPtr->RequestRefresh();
}

void FLexUIPrefabEditor::InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost >& InitToolkitHost, ULexUIPrefab* InPrefab)
{
	PrefabBeingEdited = InPrefab;
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

	FLexUIPrefabEditorCommand::Register();

	PrefabBeingEdited->EnsureInstanceObjects();

	TSharedPtr<FLexUIPrefabEditor> PrefabEditorPtr = SharedThis(this);

	ViewportPtr = SNew(SLexUIPrefabEditorViewport, PrefabEditorPtr, PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode);
	
	DetailsPtr = SNew(SLexUIPrefabEditorDetails, GetWorld());

	PrefabRawDataViewer = SNew(SLexUIPrefabRawDataViewer, PrefabEditorPtr, PrefabBeingEdited);
	
	ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->EventOnOutlineChanged.AddSPLambda(this, [=, this]()
	{
		OutlinerPtr->RequestRefresh();
	});
	ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->bShouldTickInEditor = true;
	ULexUISelection::GetInstance(GetWorld())->OnSelectionChanged.AddSPLambda(this, [=, this]
	{
		SyncSelection();
	});
	
	OutlinerPtr = SNew(SLexWidgetEditorHierarchyView, GetWorld());

	SequencerPtr = SNew(SLexUIPrefabSequenceEditor);
	
	BindCommands();
	ExtendToolbar();

	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_LexUIPrefabEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.85f)
				->Split
				(
					FTabManager::NewSplitter()
					->SetOrientation(Orient_Horizontal)
					->SetSizeCoefficient(0.9f)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FLexUIPrefabEditorTabs::OutlinerID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.6f)
						->AddTab(FLexUIPrefabEditorTabs::ViewportID, ETabState::OpenedTab)
					)
					->Split
					(
						FTabManager::NewStack()
						->SetSizeCoefficient(0.2f)
						->AddTab(FLexUIPrefabEditorTabs::DetailsID, ETabState::OpenedTab)
					)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->SetForegroundTab(FLexUIPrefabEditorTabs::SequencerID)
					->AddTab(FLexUIPrefabEditorTabs::SequencerID, ETabState::ClosedTab)
				)
			)
		);

	InitAssetEditor(Mode, InitToolkitHost, PrefabEditorAppName, StandaloneDefaultLayout, true, true, PrefabBeingEdited);

	// After opening a prefab, broadcast event to LexUIPrefabSequencerEditor
	FLexUIEditorTools::OnEditingPrefabChanged.Broadcast(GetPreviewScene()->GetRootAgent());
}

void FLexUIPrefabEditor::GetInitialViewSetting(FVector& OutLocation, FRotator& OutRotation, FVector& OutOrbitLocation, ELevelViewportType& OutViewType)
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
	OutViewType = (ELevelViewportType)PrefabEditorData.ViewportType;
}

ULexWidget* FLexUIPrefabEditor::GetRootAgentWidget()
{
	return GetPreviewScene()->GetRootAgent();
}

ULexWidget* FLexUIPrefabEditor::GetLoadedRootWidget()
{
	return GetPrefabHelperObject()->LoadedRootWidget;
}

FName FLexUIPrefabEditor::GetSequencerTabID()
{
	return FLexUIPrefabEditorTabs::SequencerID;
}

void FLexUIPrefabEditor::SaveAsset_Execute()
{
	SaveEditorState();
	FAssetEditorToolkit::SaveAsset_Execute();//save asset
	
	FLexUIEditorTools::OnBeforeApplyPrefab.Broadcast(GetPrefabHelperObject());

	FLexUIEditorTools::RefreshLoadedPrefab();
	FLexUIEditorTools::RefreshOnSubPrefabChange(GetPrefabHelperObject()->PrefabAsset);
}

void FLexUIPrefabEditor::OnOpenRawDataViewerPanel()
{
	this->InvokeTab(FLexUIPrefabEditorTabs::PrefabRawDataViewerID);
}
void FLexUIPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel()
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(GetPrefabHelperObject());
}

void FLexUIPrefabEditor::SaveEditorState()
{
	//save view location and rotation
	auto ViewTransform = ViewportPtr->GetViewportClient()->GetViewTransform();
	PrefabBeingEdited->PrefabDataForPrefabEditor.ViewLocation = ViewTransform.GetLocation();
	PrefabBeingEdited->PrefabDataForPrefabEditor.ViewRotation = ViewTransform.GetRotation();
	PrefabBeingEdited->PrefabDataForPrefabEditor.ViewOrbitLocation = ViewTransform.GetLookAt();
	PrefabBeingEdited->PrefabDataForPrefabEditor.ViewportType = ViewportPtr->GetViewportClient()->GetViewportType();
	auto RootAgentWidget = GetPreviewScene()->GetRootAgent();
	PrefabBeingEdited->PrefabDataForPrefabEditor.CanvasSize = FIntPoint(RootAgentWidget->GetWidth(), RootAgentWidget->GetHeight());
	auto RootCanvas = RootAgentWidget->GetComponent<ULexCanvas>();
	PrefabBeingEdited->PrefabDataForPrefabEditor.CanvasRenderMode = (uint8)RootCanvas->GetRenderMode();
	PrefabBeingEdited->PrefabDataForPrefabEditor.ViewMode = ViewportPtr->GetViewportClient()->GetViewMode();

	TSet<TWeakObjectPtr<ULexWidget>> ExpandWidgetSet;
	OutlinerPtr->GetExpandWidgets(ExpandWidgetSet);
	TSet<FGuid> UnexpandWidgetGuidArray;
	for (auto& KeyValue : GetPrefabHelperObject()->MapGuidToObject)
	{
		if (auto Widget = Cast<ULexWidget>(KeyValue.Value))
		{
			if (!ExpandWidgetSet.Contains(Widget))
			{
				UnexpandWidgetGuidArray.Add(KeyValue.Key);
			}
		}
	}
	PrefabBeingEdited->PrefabDataForPrefabEditor.UnexpandedWidgetSet = UnexpandWidgetGuidArray;
	PrefabBeingEdited->bThumbnailDirty = true;

	//refresh parameter, remove invalid
	for (auto& KeyValue : GetPrefabHelperObject()->SubPrefabMap)
	{
		KeyValue.Value.CheckParameters();
	}
}

void FLexUIPrefabEditor::OnApply()
{
	GetPrefabHelperObject()->SavePrefab();
}

void FLexUIPrefabEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PrefabBeingEdited);
}

void FLexUIPrefabEditor::SelectWidgets(const TSet<ULexWidget*>& Widgets, bool bAppendOrToggle, bool bNotifyGEditor)
{
	if (bIsSelecting)return;
	bIsSelecting = true;
	const FScopedTransaction Transaction(LOCTEXT("SelectionChanged_Transaction", "Select Widgets"));
	ULexUISelection::GetInstance(GetWorld())->Modify();
	
	TSet<ULexWidget*> TempSelection;
	for (auto& Widget : Widgets)
	{
		if (IsValid(Widget))
		{
			TempSelection.Add(Widget);
		}
	}

	if (!bAppendOrToggle)
	{
		SelectedWidgets.Empty();
		if (bNotifyGEditor)
		{
			ULexUISelection::GetInstance(GetWorld())->SelectNone();
		}
	}

	for ( const auto& Widget : TempSelection )
	{
		if ( bAppendOrToggle && SelectedWidgets.Contains(Widget) )
		{
			SelectedWidgets.Remove(Widget);
		}
		else
		{
			SelectedWidgets.Add(Widget);
		}
		if (bNotifyGEditor)
		{
			ULexUISelection::GetInstance(GetWorld())->SelectWidget(Widget);
		}
	}
	
	OnSelectionChanged.Broadcast();
	bIsSelecting = false;
}

FLexUIPrefabInstanceScene* FLexUIPrefabEditor::GetPreviewScene()
{ 
	return PrefabBeingEdited->GetPrefabInstanceScene();
}

UWorld* FLexUIPrefabEditor::GetWorld()
{
	return PrefabBeingEdited->GetPrefabInstanceScene()->GetWorld();
}

void FLexUIPrefabEditor::BindCommands()
{
	const FLexUIPrefabEditorCommand& PrefabEditorCommands = FLexUIPrefabEditorCommand::Get();
	ToolkitCommands->MapAction(
		PrefabEditorCommands.Apply,
		FExecuteAction::CreateSP(this, &FLexUIPrefabEditor::OnApply),
		FCanExecuteAction(),
		FIsActionChecked()
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.RawDataViewer,
		FExecuteAction::CreateSP(this, &FLexUIPrefabEditor::OnOpenRawDataViewerPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);
	ToolkitCommands->MapAction(
		PrefabEditorCommands.OpenPrefabHelperObject,
		FExecuteAction::CreateSP(this, &FLexUIPrefabEditor::OnOpenPrefabHelperObjectDetailsPanel),
		FCanExecuteAction(),
		FIsActionChecked()
	);

	TFunction<ULexWidget*()> GetSelectedWidget = [this]()
	{
		if (this->GetSelectedWidgets().Num() == 1)
		{
			auto Actor = this->GetSelectedWidgets()[0];
			if (Actor.IsValid())
			{
				return Actor.Get();
			}
		}
		return (ULexWidget*)nullptr;
	};
	TFunction<TArray<ULexWidget*>()> GetSelectedWidgetArray = [this]()
	{
		TArray<ULexWidget*> TempSelectedActors;
		if (this->GetSelectedWidgets().Num() > 0)
		{
			for (auto Actor : this->GetSelectedWidgets())
			{
				if (Actor.IsValid())
				{
					TempSelectedActors.Add(Actor.Get());
				}
			}
			return TempSelectedActors;
		}
		return TempSelectedActors;
	};
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateStatic(&FLexUIEditorTools::CopyWidgets, GetSelectedWidgetArray),
		FCanExecuteAction::CreateStatic(&FLexUIEditorTools::CanCopyWidget, GetSelectedWidgetArray),
		FGetActionCheckState(),
		FIsActionButtonVisible()
	);
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateSPLambda(this, [=, this]()
		{
			FLexUIEditorTools::CutWidgets(GetSelectedWidgetArray);
			OutlinerPtr->RequestRefresh();
		}),
		FCanExecuteAction::CreateStatic(&FLexUIEditorTools::CanCutWidget, GetSelectedWidgetArray),
		FGetActionCheckState(),
		FIsActionButtonVisible()
	);
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateSPLambda(this, [=, this]()
		{
			FLexUIEditorTools::PasteWidgets(GetSelectedWidgetArray);
			OutlinerPtr->RequestRefresh();
		}),
		FCanExecuteAction::CreateStatic(&FLexUIEditorTools::CanPasteWidget, GetSelectedWidget),
		FGetActionCheckState(),
		FIsActionButtonVisible()
	);
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSPLambda(this, [=, this]()
		{
			FLexUIEditorTools::DuplicateWidgets(GetSelectedWidgetArray);
			OutlinerPtr->RequestRefresh();
		}),
		FCanExecuteAction::CreateStatic(&FLexUIEditorTools::CanDuplicateWidget, GetSelectedWidgetArray),
		FGetActionCheckState(),
		FIsActionButtonVisible()
	);
	ToolkitCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSPLambda(this, [=, this]()
		{
			FLexUIEditorTools::DeleteWidgets(GetSelectedWidgetArray);
			OutlinerPtr->RequestRefresh();
		}),
		FCanExecuteAction::CreateStatic(&FLexUIEditorTools::CanDeleteWidget, GetSelectedWidgetArray),
		FGetActionCheckState(),
		FIsActionButtonVisible()
	);
}
void FLexUIPrefabEditor::ExtendToolbar()
{
	const FName MenuName = GetToolMenuToolbarName();
	if (!UToolMenus::Get()->IsMenuRegistered(MenuName))
	{
		UToolMenus::Get()->RegisterMenu(MenuName, "AssetEditor.DefaultToolBar", EMultiBoxType::ToolBar);
	}

	UToolMenu* ToolBar = UToolMenus::Get()->FindMenu(MenuName);

	FToolMenuInsert InsertAfterAssetSection("Asset", EToolMenuInsertType::After);
	{
		auto ApplyButtonMenuEntry = FToolMenuEntry::InitToolBarButton(FLexUIPrefabEditorCommand::Get().Apply
			, LOCTEXT("Apply", "Apply")
			, TAttribute<FText>(this, &FLexUIPrefabEditor::GetApplyButtonStatusTooltip)
			, TAttribute<FSlateIcon>(this, &FLexUIPrefabEditor::GetApplyButtonStatusImage));
		
		FToolMenuSection& Section = ToolBar->AddSection("LexUIPrefabCommands", TAttribute<FText>(), InsertAfterAssetSection);
		Section.AddEntry(ApplyButtonMenuEntry);
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLexUIPrefabEditorCommand::Get().RawDataViewer));
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(FLexUIPrefabEditorCommand::Get().OpenPrefabHelperObject));
	}
}

FText FLexUIPrefabEditor::GetApplyButtonStatusTooltip()const
{
	return GetAnythingDirty() ? LOCTEXT("Apply_Tooltip", "Dirty, need to apply") : LOCTEXT("Apply_Tooltip", "Good to go");
}
FSlateIcon FLexUIPrefabEditor::GetApplyButtonStatusImage()const
{
	static const FName CompileStatusBackground("Blueprint.CompileStatus.Background");
	static const FName CompileStatusUnknown("Blueprint.CompileStatus.Overlay.Unknown");
	static const FName CompileStatusGood("Blueprint.CompileStatus.Overlay.Good");

	return FSlateIcon(FAppStyle::GetAppStyleSetName(), CompileStatusBackground, NAME_None, GetAnythingDirty() ? CompileStatusUnknown : CompileStatusGood);
}

TSharedRef<SDockTab> FLexUIPrefabEditor::SpawnTab_Viewport(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("ViewportTab_Title", "Viewport"))
		[
			ViewportPtr.ToSharedRef()
		];
}
TSharedRef<SDockTab> FLexUIPrefabEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			DetailsPtr.ToSharedRef()
		];
}
TSharedRef<SDockTab> FLexUIPrefabEditor::SpawnTab_Outliner(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("OutlinerTab_Title", "Outliner"))
		[
			OutlinerPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLexUIPrefabEditor::SpawnTab_Sequencer(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("SequencerTab_Title", "Sequencer"))
		[
			SequencerPtr.ToSharedRef()
		];
}

TSharedRef<SDockTab> FLexUIPrefabEditor::SpawnTab_PrefabRawDataViewer(const FSpawnTabArgs& Args)
{
	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("OverrideParameterTab_Title", "PrefabRawData"))
		[
			PrefabRawDataViewer.ToSharedRef()
		];
}

bool FLexUIPrefabEditor::IsFilteredActor(const AActor* Actor)
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

void FLexUIPrefabEditor::OnOutlinerActorDoubleClick(AActor* Actor)
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
					const bool bIgnore = PrimitiveComponents.Num() > 1 && PrimitiveComponent->GetIgnoreBoundsForEditorFocus();

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

FName FLexUIPrefabEditor::GetToolkitFName() const
{
	return FName("LexUIPrefabEditor");
}
FText FLexUIPrefabEditor::GetBaseToolkitName() const
{
	return LOCTEXT("LexUIPrefabEditorAppLabel", "LexUI Prefab Editor");
}
FText FLexUIPrefabEditor::GetToolkitName() const
{
	return FText::FromString(PrefabBeingEdited->GetName());
}
FText FLexUIPrefabEditor::GetToolkitToolTipText() const
{
	return FAssetEditorToolkit::GetToolTipTextForObject(PrefabBeingEdited);
}
FLinearColor FLexUIPrefabEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor::White;
}
FString FLexUIPrefabEditor::GetWorldCentricTabPrefix() const
{
	return TEXT("LexUIPrefabEditor");
}
FString FLexUIPrefabEditor::GetDocumentationLink() const
{
	return TEXT("");
}
void FLexUIPrefabEditor::OnToolkitHostingStarted(const TSharedRef<IToolkit>& Toolkit)
{

}
void FLexUIPrefabEditor::OnToolkitHostingFinished(const TSharedRef<IToolkit>& Toolkit)
{

}

FReply FLexUIPrefabEditor::TryHandleAssetDragDropOperation(const FDragDropEvent& DragDropEvent, ULexWidget* InParentWidget)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid() && Operation->IsOfType<FAssetDragDropOp>())
	{
		TArray< FAssetData > DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(Operation);
		const int32 NumAssets = DroppedAssetData.Num();

		if (NumAssets > 0)
		{
			TArray<ULexUIPrefab*> PrefabsToLoad;
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

				UObject* Asset = AssetData.GetAsset();
				if (auto PrefabAsset = Cast<ULexUIPrefab>(Asset))
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
					if (PrefabAsset->PrefabVersion <= (uint16)ELexUIPrefabVersion::OldVersion)
					{
						auto MsgText = LOCTEXT("Error_UnsupportOldPrefabVersion", "Operation error! Target prefab's version is too old! Please make it newer: open the prefab and hit \"Save\" button.");
						FMessageDialog::Open(EAppMsgType::Ok, MsgText);
						return FReply::Unhandled();
					}

					PrefabsToLoad.Add(PrefabAsset);
				}
			}

			auto CurrentSelectedWidget = InParentWidget != nullptr ? InParentWidget :
			(SelectedWidgets.Num() > 0 ? SelectedWidgets[0] : nullptr);
			if (PrefabsToLoad.Num() > 0)
			{
				if (CurrentSelectedWidget == nullptr)
				{
					auto MsgText = LOCTEXT("Error_NeedParentNode", "Please select a actor as parent actor");
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
				if (CurrentSelectedWidget == GetPreviewScene()->GetRootAgent())
				{
					auto MsgText = FText::Format(LOCTEXT("Error_RootCannotBeParentNode", "{0} cannot be parent actor of child prefab, please choose another actor."), FText::FromString(FLexUIPrefabInstanceScene::RootAgentActorName));
					FMessageDialog::Open(EAppMsgType::Ok, MsgText);
					return FReply::Unhandled();
				}
			}
			else
			{
				return FReply::Unhandled();
			}

			GEditor->BeginTransaction(LOCTEXT("CreateFromAssetDrop_Transaction", "LexUI Create from asset drop"));
			TArray<ULexWidget*> CreatedWidgetArray;
			if (PrefabsToLoad.Num() > 0)
			{
				for (auto& PrefabAsset : PrefabsToLoad)
				{
					TMap<FGuid, TObjectPtr<UObject>> SubPrefabMapGuidToObject;
					TMap<TObjectPtr<ULexWidget>, FLexUISubPrefabData> SubSubPrefabMap;
					auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabWithExistingObjects(
						GetPreviewScene()->GetWorld()
						, CurrentSelectedWidget->GetOuter()
						, CurrentSelectedWidget.Get()
						, SubPrefabMapGuidToObject, SubSubPrefabMap
					);

					GetPrefabHelperObject()->MakePrefabAsSubPrefab(PrefabAsset, LoadedSubPrefabRootActor, SubPrefabMapGuidToObject, {});
					CreatedWidgetArray.Add(LoadedSubPrefabRootActor);
				}

				if (OutlinerPtr.IsValid())
				{
					ULexUIManagerObject::AddOneShotTickFunction([=, this] {
						for (auto& Actor : CreatedWidgetArray)
						{
							//OutlinerPtr->UnexpandActorForDragDroppedPrefab(Actor);
						}
						OutlinerPtr->RequestRefresh();
						}, 1);//delay execute, because the outliner not create actor yet
				}
			}
			if (CreatedWidgetArray.Num() > 0)
			{
				ULexUISelection::GetInstance(GetWorld())->SelectNone();
				for (auto& Widget : CreatedWidgetArray)
				{
					ULexUISelection::GetInstance(GetWorld())->SelectWidget(Widget);
				}
			}
			GEditor->EndTransaction();
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}



#undef LOCTEXT_NAMESPACE