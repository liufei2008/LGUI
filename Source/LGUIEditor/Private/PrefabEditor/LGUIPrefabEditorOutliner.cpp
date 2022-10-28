// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorOutliner.h"
#include "SceneOutlinerModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"
#include "Widgets/Layout/SBox.h"
#include "SceneOutlinerDelegates.h"
#include "ActorTreeItem.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "SceneOutliner/LGUISceneOutlinerInfoColumn.h"
#include "SOutlinerTreeView.h"
#include "Editor/GroupActor.h"
#include "LGUIPrefabEditor.h"
#include "LGUIEditorModule.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "SceneOutlinerDragDrop.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"

PRAGMA_DISABLE_OPTIMIZATION

FLGUIPrefabEditorOutliner::~FLGUIPrefabEditorOutliner()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}
void FLGUIPrefabEditorOutliner::InitOutliner(UWorld* World, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr, const TSet<AActor*>& InUnexpendActorSet)
{
	CurrentWorld = World;
	PrefabEditorPtr = InPrefabEditorPtr;
	if (CurrentWorld == nullptr)
	{
		return;
	}

	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	SceneOutliner::FInitializationOptions InitOptions;
	InitOptions.bShowTransient = false;
	InitOptions.Mode = ESceneOutlinerMode::ActorBrowsing;
	InitOptions.ModifyContextMenu = FSceneOutlinerModifyContextMenu::CreateLambda([](FName& MenuName, FToolMenuContext& MenuContext) {

		});
	InitOptions.bFocusSearchBoxWhenOpened = false;
	InitOptions.SpecifiedWorldToDisplay = World;
	InitOptions.bShowCreateNewFolder = false;
	InitOptions.ColumnMap.Add(LGUISceneOutliner::FLGUISceneOutlinerInfoColumn::GetID(), SceneOutliner::FColumnInfo(SceneOutliner::EColumnVisibility::Visible, 2));
	InitOptions.ColumnMap.Add(SceneOutliner::FBuiltInColumnTypes::Gutter(), SceneOutliner::FColumnInfo(SceneOutliner::EColumnVisibility::Visible, 0));
	InitOptions.ColumnMap.Add(SceneOutliner::FBuiltInColumnTypes::Label(), SceneOutliner::FColumnInfo(SceneOutliner::EColumnVisibility::Visible, 1));
	InitOptions.ColumnMap.Add(SceneOutliner::FBuiltInColumnTypes::ActorInfo(), SceneOutliner::FColumnInfo(SceneOutliner::EColumnVisibility::Visible, 10));
	if (ActorFilter.IsBound())
	{
		InitOptions.Filters->AddFilterPredicate(ActorFilter);
	}
	InitOptions.CustomDelete = FCustomSceneOutlinerDeleteDelegate::CreateRaw(this, &FLGUIPrefabEditorOutliner::OnDelete);

	TSharedRef<ISceneOutliner> SceneOutlinerRef = SceneOutlinerModule.CreateSceneOutliner(InitOptions, FOnSceneOutlinerItemPicked::CreateLambda([this](TSharedRef<SceneOutliner::ITreeItem> ItemPtr)
		{ this->OnSceneOutlinerItemPicked(ItemPtr); }));
	SceneOutlinerPtr = StaticCastSharedRef<ICustomSceneOutliner>(SceneOutlinerRef);

	SceneOutlinerPtr->GetOnItemSelectionChanged().AddRaw(this, &FLGUIPrefabEditorOutliner::OnSceneOutlinerSelectionChanged);
	SceneOutlinerPtr->GetDoubleClickEvent().AddRaw(this, &FLGUIPrefabEditorOutliner::OnSceneOutlinerDoubleClick);
	//SceneOutlinerPtr->SetOnItemDragDetected([=](const SceneOutliner::ITreeItem& TreeItem) {
	//	auto Operation = SceneOutliner::CreateDragDropOperation(SceneOutlinerPtr->GetTree().GetSelectedItems());
	//	return FReply::Handled().BeginDragDrop(Operation.ToSharedRef());
	//	});
	//SceneOutlinerPtr->SetOnDropOnItem([=](const FDragDropEvent& DragDropEvent, const SceneOutliner::ITreeItem& TreeItem) {
	//	UE_LOG(LGUIEditor, Error, TEXT(""));
	//	return FReply::Handled();
	//	});
	SceneOutlinerPtr->SetSelectionMode(ESelectionMode::Multi);
	

	auto& TreeView = SceneOutlinerPtr->GetTree();
	TSet<SceneOutliner::FTreeItemPtr> VisitingItems;
	TreeView.GetExpandedItems(VisitingItems);
	for (auto& Item : VisitingItems)
	{
		switch (Item->GetTypeSortPriority())
		{
		case SceneOutliner::ETreeItemSortOrder::Actor:
		{
			auto ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
			if (ActorTreeItem->Actor.IsValid())
			{
				if (InUnexpendActorSet.Contains(ActorTreeItem->Actor.Get()))
				{
					ActorTreeItem->Flags.bIsExpanded = false;
				}
			}
		}
		break;
		}
	}
	SceneOutlinerPtr->Refresh();

	OutlinerWidget =
		SNew(SBox)
		.WidthOverride(OutlinerWidth)
		.HeightOverride(OutlinerHeight)
		[
			SceneOutlinerRef
		];



	USelection::SelectionChangedEvent.AddRaw(this, &FLGUIPrefabEditorOutliner::OnEditorSelectionChanged);
}

void FLGUIPrefabEditorOutliner::OnDelete(const TArray<TWeakObjectPtr<AActor>>& InSelectedActorArray)
{
	PrefabEditorPtr.Pin()->DeleteActors(InSelectedActorArray);
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerItemPicked(TSharedRef<SceneOutliner::ITreeItem> ItemPtr)
{
	SceneOutliner::FActorTreeItem* ActorTreeItem = (SceneOutliner::FActorTreeItem*)(&ItemPtr.Get());

	if (ActorTreeItem)
	{
	}
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerSelectionChanged(SceneOutliner::FTreeItemPtr ItemPtr, ESelectInfo::Type SelectionMode)
{
	if (bIsReentrant)return;
	TGuardValue<bool> ReentrantGuard(bIsReentrant, true);

	CachedItemPtr = ItemPtr;
	auto SelectedTreeItems = SceneOutlinerPtr->GetTree().GetSelectedItems();

	const FScopedTransaction Transaction(NSLOCTEXT("UnrealEd", "ClickingOnActors", "Clicking on Actors"));
	GEditor->GetSelectedActors()->Modify();

	if (SelectedTreeItems.Num() > 0)
	{
		// Clear the selection.
		GEditor->SelectNone(false, true, true);

		if (ItemPtr->GetTypeSortPriority() == SceneOutliner::ETreeItemSortOrder::Actor)
		{
			auto ActorTree = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(CachedItemPtr);
			if (ActorTree.IsValid() && ActorTree->Actor.IsValid())
			{
				SelectedActor = ActorTree->Actor;
				OnActorPickedDelegate.ExecuteIfBound(SelectedActor.Get());
			}
			GEditor->GetSelectedActors()->BeginBatchSelectOperation();
			for (auto& Item : SelectedTreeItems)
			{
				if (Item->GetTypeSortPriority() == SceneOutliner::ETreeItemSortOrder::Actor)
				{
					auto ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
					GEditor->SelectActor(ActorTreeItem->Actor.Get(), true, false);
				}
			}

			// Commit selection changes
			GEditor->GetSelectedActors()->EndBatchSelectOperation(/*bNotify*/false);
			// Fire selection changed event
			GEditor->NoteSelectionChange();
		}
	}
	else
	{
		GEditor->SelectNone(false, true, true);
	}
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerDoubleClick(SceneOutliner::FTreeItemPtr ItemPtr)
{
	SceneOutliner::FActorTreeItem* ActorTreeItem = (SceneOutliner::FActorTreeItem*)ItemPtr.Get();
	if (ActorTreeItem)
	{
		OnActorDoubleClickDelegate.ExecuteIfBound(ActorTreeItem->Actor.Get());
	}
}

void FLGUIPrefabEditorOutliner::RenameSelectedActor()
{
	if (CachedItemPtr.IsValid())
	{
		CachedItemPtr->RenameRequestEvent.ExecuteIfBound();
	}
	else
	{
	}
}

void FLGUIPrefabEditorOutliner::OnEditorSelectionChanged(UObject* Object)
{
	if (bIsReentrant)
	{
		return;
	}
	if (!SceneOutlinerPtr.IsValid())
	{
		return;
	}

	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			if (Actor->GetWorld() != CurrentWorld.Get())
			{
				return;
			}
			OnActorPickedDelegate.ExecuteIfBound(Actor);

			SceneOutlinerPtr->ClearSelection();
			SceneOutlinerPtr->AddObjectToSelection(Actor);

			SelectedActor = Actor;
		}
		else
		{
			SceneOutlinerPtr->ClearSelection();
			SelectedActor = nullptr;
		}
	}
}

void FLGUIPrefabEditorOutliner::Refresh()
{
	if (SceneOutlinerPtr.IsValid())
	{
		SceneOutlinerPtr->Refresh();
	}
	else
	{
	}
}

void FLGUIPrefabEditorOutliner::FullRefresh()
{
	if (SceneOutlinerPtr.IsValid())
	{
		SceneOutlinerPtr->FullRefresh();

	}
	else
	{
	}
}

void FLGUIPrefabEditorOutliner::ClearSelectedActor()
{
	SelectedActor = nullptr;
}

void FLGUIPrefabEditorOutliner::GetUnexpendActor(TArray<AActor*>& InOutAllActors)const
{
	auto& TreeView = SceneOutlinerPtr->GetTree();
	TSet<SceneOutliner::FTreeItemPtr> VisitingItems;
	TreeView.GetExpandedItems(VisitingItems);
	for (auto& Item : VisitingItems)
	{
		switch (Item->GetTypeSortPriority())
		{
		case SceneOutliner::ETreeItemSortOrder::Actor:
		{
			auto ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
			if (ActorTreeItem->Actor.IsValid())
			{
				if (ActorTreeItem->Flags.bIsExpanded)
				{
					InOutAllActors.Remove(ActorTreeItem->Actor.Get());
				}
			}
		}
		break;
		}
	}
}

PRAGMA_ENABLE_OPTIMIZATION

