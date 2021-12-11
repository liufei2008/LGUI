// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditor.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIEditorModule.h"
#include "LGUIPrefabEditorViewport.h"
#include "LGUIPrefabPreviewScene.h"
#include "LGUIPrefabEditorDetails.h"
#include "LGUIPrefabEditorOutliner.h"
#include "UnrealEdGlobals.h"
#include "EditorModeManager.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "AssetSelection.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Misc/FeedbackContext.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditor"

const FName PrefabEditorAppName = FName(TEXT("LGUIPrefabEditorApp"));

struct FLGUIPrefabEditorTabs
{
	// Tab identifiers
	static const FName DetailsID;
	static const FName ViewportID;
	static const FName OutlinerID;
};

const FName FLGUIPrefabEditorTabs::DetailsID(TEXT("Details"));
const FName FLGUIPrefabEditorTabs::ViewportID(TEXT("Viewport"));
const FName FLGUIPrefabEditorTabs::OutlinerID(TEXT("Outliner"));

FLGUIPrefabEditor::FLGUIPrefabEditor()
	:PreviewScene(FLGUIPrefabPreviewScene::ConstructionValues().AllowAudioPlayback(false).ShouldSimulatePhysics(false).SetEditor(true))
{

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
}
void FLGUIPrefabEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::ViewportID);
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::DetailsID);
	InTabManager->UnregisterTabSpawner(FLGUIPrefabEditorTabs::OutlinerID);
}

void FLGUIPrefabEditor::InitPrefabEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost >& InitToolkitHost, ULGUIPrefab* InPrefab)
{
	PrefabBeingEdited = InPrefab;

	TMap<FGuid, UObject*> TempMapGuidToObject;
	for (auto KeyValue : MapGuidToObject)
	{
		if (KeyValue.Value.IsValid())
		{
			TempMapGuidToObject.Add(KeyValue.Key, KeyValue.Value.Get());
		}
	}
	TMap<AActor*, ULGUIPrefab*> TempSubPrefabMap;
	LoadedRootActor = PrefabBeingEdited->LoadPrefabForEdit(PreviewScene.GetWorld(), PreviewScene.GetParentComponentForPrefab(PrefabBeingEdited), TempMapGuidToObject, TempSubPrefabMap);
	MapGuidToObject.Reset();
	TArray<AActor*> LoadedActorArray;
	for (auto KeyValue : TempMapGuidToObject)
	{
		MapGuidToObject.Add(KeyValue.Key, KeyValue.Value);
		if (auto ActorItem = Cast<AActor>(KeyValue.Value))
		{
			LoadedActorArray.Add(ActorItem);
		}
	}
	SubPrefabMap.Reset();
	for (auto KeyValue : TempSubPrefabMap)
	{
		SubPrefabMap.Add(KeyValue.Key, KeyValue.Value);
	}

	TArray<AActor*> AllChildrenActorArray;
	LGUIUtils::CollectChildrenActors(LoadedRootActor, AllChildrenActorArray, true);
	for (auto ActorItem : AllChildrenActorArray)
	{
		if (!LoadedActorArray.Contains(ActorItem))
		{
			SetActorNotListInOutliner(ActorItem);
		}
	}

	TSharedPtr<FLGUIPrefabEditor> PrefabEditorPtr = SharedThis(this);
	ViewportPtr = SNew(SLGUIPrefabEditorViewport, PrefabEditorPtr);

	OutlinerPtr = MakeShared<FLGUIPrefabEditorOutliner>();
	OutlinerPtr->ActorFilter = FOnShouldFilterActor::CreateRaw(this, &FLGUIPrefabEditor::IsFilteredActor);
	OutlinerPtr->OnActorPickedDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerPickedChanged);
	OutlinerPtr->OnActorDoubleClickDelegate = FOnActorPicked::CreateRaw(this, &FLGUIPrefabEditor::OnOutlinerActorDoubleClick);
	OutlinerPtr->InitOutliner(PreviewScene.GetWorld());

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
}

void FLGUIPrefabEditor::SaveAsset_Execute()
{
	if (CheckBeforeSaveAsset())
	{
		TMap<UObject*, FGuid> MapObjectToGuid;
		for (auto KeyValue : MapGuidToObject)
		{
			if (KeyValue.Value.IsValid())
			{
				MapObjectToGuid.Add(KeyValue.Value.Get(), KeyValue.Key);
			}
		}
		TMap<AActor*, ULGUIPrefab*> TempSubPrefabMap;
		for (auto KeyValue : SubPrefabMap)
		{
			if (KeyValue.Key.IsValid() && KeyValue.Value.IsValid())
			{
				TempSubPrefabMap.Add(KeyValue.Key.Get(), KeyValue.Value.Get());
			}
		}
		PrefabBeingEdited->SavePrefab(LoadedRootActor, MapObjectToGuid, TempSubPrefabMap);
		MapGuidToObject.Empty();
		for (auto KeyValue : MapObjectToGuid)
		{
			MapGuidToObject.Add(KeyValue.Value, KeyValue.Key);
		}

		FAssetEditorToolkit::SaveAsset_Execute();
	}
}

bool FLGUIPrefabEditor::CheckBeforeSaveAsset()
{
	auto RootUIAgentActor = PreviewScene.GetRootUIAgentActor();
	//All actor should attach to prefab's root actor
	for (TActorIterator<AActor> ActorItr(PreviewScene.GetWorld()); ActorItr; ++ActorItr)
	{
		if (AActor* ItemActor = *ActorItr)
		{
			if (ItemActor == LoadedRootActor)continue;
			if (ItemActor == RootUIAgentActor)continue;
			if (PreviewScene.IsWorldDefaultActor(ItemActor))continue;
			if (!ItemActor->IsAttachedTo(LoadedRootActor))
			{
				auto MsgText = LOCTEXT("Error_AllActor", "All prefab's actors must attach to prefab's root actor!");
				FMessageDialog::Open(EAppMsgType::Ok, MsgText);
				return false;
			}
		}
	}
	
	//If is UI prefab, then root actor must be child of [UIRootAgent]
	if (LoadedRootActor->IsA(AUIBaseActor::StaticClass()))
	{
		if (LoadedRootActor->GetAttachParentActor() != RootUIAgentActor)
		{
			auto MsgText = LOCTEXT("Error_PrefabRootMustMustBeChildOfRootUIAgent", "Prefab's root actor must be child of [UIRootAgent]");
			FMessageDialog::Open(EAppMsgType::Ok, MsgText);
			return false;
		}
	}

	check(AllLoadedActorArray.Num() == AllLoadedActorGuidArrayInPrefab.Num());
	for (int i = AllLoadedActorArray.Num() - 1; i >= 0; i--)
	{
		auto ActorItem = AllLoadedActorArray[i];
		if (!IsValid(ActorItem)//not valid
			|| (!ActorItem->IsAttachedTo(LoadedRootActor) && ActorItem != LoadedRootActor)//not attached to this prefab
			)
		{
			AllLoadedActorArray.RemoveAt(i);
			AllLoadedActorGuidArrayInPrefab.RemoveAt(i);
		}
	}

	return true;
}

UWorld* FLGUIPrefabEditor::GetWorld()
{
	return PreviewScene.GetWorld();
}

void FLGUIPrefabEditor::ExtendToolbar()
{

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
	TSharedPtr<FLGUIPrefabEditor> DetailsPtr = SharedThis(this);

	// Spawn the tab
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Title", "Details"))
		[
			SNew(SLGUIPrefabEditorDetails, DetailsPtr)
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
	if (CurrentSelectedActor == nullptr)return FReply::Unhandled();//need a parent node

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if (Operation.IsValid() && Operation->IsOfType<FAssetDragDropOp>())
	{
		TArray< FAssetData > DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(Operation);
		const int32 NumAssets = DroppedAssetData.Num();

		if (NumAssets > 0)
		{
			GWarn->BeginSlowTask(LOCTEXT("LoadingAssets", "Loading Asset(s)"), true);
			bool bMarkBlueprintAsModified = false;

			for (int32 DroppedAssetIdx = 0; DroppedAssetIdx < NumAssets; ++DroppedAssetIdx)
			{
				const FAssetData& AssetData = DroppedAssetData[DroppedAssetIdx];

				if (!AssetData.IsAssetLoaded())
				{
					GWarn->StatusUpdate(DroppedAssetIdx, NumAssets, FText::Format(LOCTEXT("LoadingAsset", "Loading Asset {0}"), FText::FromName(AssetData.AssetName)));
				}

				UObject* Asset = AssetData.GetAsset();

				if (auto PrefabAsset = Cast<ULGUIPrefab>(Asset))
				{
					TMap<FGuid, UObject*> SubPrefabMapGuidToObject;
					TMap<AActor*, ULGUIPrefab*> SubSubPrefabMap;
					auto LoadedSubPrefabRootActor = PrefabAsset->LoadPrefabForEdit(PreviewScene.GetWorld()
						, CurrentSelectedActor->GetRootComponent()
						, SubPrefabMapGuidToObject, SubSubPrefabMap
						);

					for (auto KeyValue : SubPrefabMapGuidToObject)
					{
						if (auto ActorItem = Cast<AActor>(KeyValue.Value))
						{
							if (LoadedSubPrefabRootActor != ActorItem)
							{
								SetActorNotListInOutliner(ActorItem);
							}
						}
					}

					SubPrefabMap.Add(LoadedSubPrefabRootActor, PrefabAsset);
				}
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

void FLGUIPrefabEditor::SetActorNotListInOutliner(AActor* Actor)
{
	auto bEditable_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bEditable"));
	bEditable_Property->SetPropertyValue_InContainer(Actor, false);

	Actor->bHiddenEd = true;
	Actor->bHiddenEdLayer = true;
	Actor->bHiddenEdLevel = true;
	Actor->bLockLocation = true;

	auto bListedInSceneOutliner_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bListedInSceneOutliner"));
	bListedInSceneOutliner_Property->SetPropertyValue_InContainer(Actor, false);
}

#undef LOCTEXT_NAMESPACE