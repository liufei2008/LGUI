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

PRAGMA_DISABLE_OPTIMIZATION

FLGUIPrefabEditorOutliner::~FLGUIPrefabEditorOutliner()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}
#include "ActorMode.h"
#include "SceneOutlinerPublicTypes.h"
class FLGUIPrefabEditorOutlinerMode : public FActorMode
{
public:
	FLGUIPrefabEditorOutlinerMode(SSceneOutliner* InSceneOutliner, FLGUIPrefabEditorOutliner* InPrefabEditorOutlinerPtr, TWeakObjectPtr<UWorld> InSpecifiedWorldToDisplay)
		: FActorMode(FActorModeParams(InSceneOutliner, InSpecifiedWorldToDisplay, true, true))
		, PrefabEditorOutlinerPtr(InPrefabEditorOutlinerPtr)
	{}

	virtual bool CanRenameItem(const ISceneOutlinerTreeItem& Item) const override { return true; }
	virtual ESelectionMode::Type GetSelectionMode() const override { return ESelectionMode::Multi; }

	virtual void OnItemSelectionChanged(FSceneOutlinerTreeItemPtr Item, ESelectInfo::Type SelectionType, const FSceneOutlinerItemSelection& Selection) override
	{
		PrefabEditorOutlinerPtr->OnSceneOutlinerSelectionChanged(Item, SelectionType);
	}
private:
	FLGUIPrefabEditorOutliner* PrefabEditorOutlinerPtr;
};
void FLGUIPrefabEditorOutliner::InitOutliner(UWorld* World, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr)
{
	CurrentWorld = World;
	PrefabEditorPtr = InPrefabEditorPtr;
	if (CurrentWorld == nullptr)
	{
		return;
	}

	FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::Get().LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	FSceneOutlinerInitializationOptions InitOptions;
	InitOptions.bShowTransient = false;
	InitOptions.ModeFactory = FCreateSceneOutlinerMode::CreateLambda([SpecifiedWorld = TWeakObjectPtr<UWorld>(World), PrefabEditorOutliner = this](SSceneOutliner* SceneOutliner) {
		return new FLGUIPrefabEditorOutlinerMode(SceneOutliner, PrefabEditorOutliner, SpecifiedWorld);
		});
	InitOptions.bFocusSearchBoxWhenOpened = false;
	InitOptions.bShowCreateNewFolder = false;
	InitOptions.ColumnMap.Add(LGUISceneOutliner::FLGUISceneOutlinerInfoColumn::GetID(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 2));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Gutter(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 0));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 1));
	InitOptions.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::ActorInfo(), FSceneOutlinerColumnInfo(ESceneOutlinerColumnVisibility::Visible, 10));
	if (ActorFilter.IsBound())
	{
		InitOptions.Filters->AddFilterPredicate<FActorTreeItem>(ActorFilter);
	}
	InitOptions.CustomDelete = FCustomSceneOutlinerDeleteDelegate::CreateRaw(this, &FLGUIPrefabEditorOutliner::OnDelete);

	TSharedRef<ISceneOutliner> SceneOutlinerRef = SceneOutlinerModule.CreateSceneOutliner(InitOptions);
	SceneOutlinerPtr = StaticCastSharedRef<SSceneOutliner>(SceneOutlinerRef->AsShared());

	SceneOutlinerPtr->GetOnItemSelectionChanged().AddRaw(this, &FLGUIPrefabEditorOutliner::OnSceneOutlinerSelectionChanged);
	SceneOutlinerPtr->GetDoubleClickEvent().AddRaw(this, &FLGUIPrefabEditorOutliner::OnSceneOutlinerDoubleClick);
	GEditor->OnLevelActorListChanged().AddLambda([SceneOutlinerWeak = TWeakPtr<SSceneOutliner>(SceneOutlinerPtr)]() {//UE5 not auto refresh the actor label display, so manually refresh it
		if (SceneOutlinerWeak.IsValid())
		{
			SceneOutlinerWeak.Pin()->Refresh();
		}
		});
	FCoreDelegates::OnActorLabelChanged.AddLambda([SceneOutlinerWeak = TWeakPtr<SSceneOutliner>(SceneOutlinerPtr)](AActor* actor) {//UE5 not auto refresh the actor label display, so manually refresh it
		if (SceneOutlinerWeak.IsValid())
		{
			SceneOutlinerWeak.Pin()->FullRefresh();
		}
	});

	OutlinerWidget =
		SNew(SBox)
		.WidthOverride(OutlinerWidth)
		.HeightOverride(OutlinerHeight)
		[
			SceneOutlinerRef
		];



	USelection::SelectionChangedEvent.AddRaw(this, &FLGUIPrefabEditorOutliner::OnEditorSelectionChanged);
}

void FLGUIPrefabEditorOutliner::OnDelete(const TArray<TWeakPtr<ISceneOutlinerTreeItem>>& InSelectedTreeItemArray)
{
	TArray<TWeakObjectPtr<AActor>> InSelectedActorArray;
	for (auto Item : InSelectedTreeItemArray)
	{
		if (Item.IsValid())
		{
			if (auto Actor = GetActorFromTreeItem(Item.Pin()))
			{
				InSelectedActorArray.Add(Actor);
			}
		}
	}
	PrefabEditorPtr.Pin()->DeleteActors(InSelectedActorArray);
}

AActor* FLGUIPrefabEditorOutliner::GetActorFromTreeItem(FSceneOutlinerTreeItemPtr TreeItem)const
{
	if (auto ActorTreeItem = TreeItem->CastTo<FActorTreeItem>())
	{
		if (ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
		{
			if (ActorTreeItem->Actor->GetWorld())
			{
				return Cast<AActor>(ActorTreeItem->Actor.Get());
			}
		}
	}
	return nullptr;
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerSelectionChanged(FSceneOutlinerTreeItemPtr ItemPtr, ESelectInfo::Type SelectionMode)
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

		if (auto ActorTree = CachedItemPtr->CastTo<FActorTreeItem>())
		{
			SelectedActor = ActorTree->Actor;
			OnActorPickedDelegate.ExecuteIfBound(SelectedActor.Get());
		}

		GEditor->GetSelectedActors()->BeginBatchSelectOperation();
		for (auto& Item : SelectedTreeItems)
		{
			auto ActorTreeItem = Item->CastTo<FActorTreeItem>();

			GEditor->SelectActor(ActorTreeItem->Actor.Get(), true, false);
		}

		// Commit selection changes
		GEditor->GetSelectedActors()->EndBatchSelectOperation(/*bNotify*/false);
		// Fire selection changed event
		GEditor->NoteSelectionChange();
	}
	else
	{
		GEditor->SelectNone(false, true, true);
	}
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerDoubleClick(FSceneOutlinerTreeItemPtr ItemPtr)
{
	if (OnActorDoubleClickDelegate.IsBound())
	{
		FActorTreeItem* ActorTreeItem = (FActorTreeItem*)ItemPtr.Get();
		if (ActorTreeItem)
		{
			OnActorDoubleClickDelegate.ExecuteIfBound(ActorTreeItem->Actor.Get());
		}
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

			SceneOutlinerPtr->SetSelection([=](ISceneOutlinerTreeItem& TreeItem) {
				if (auto ActorTree = TreeItem.CastTo<FActorTreeItem>())
				{
					return ActorTree->Actor.Get() == Actor;
				}
				return false;
				});

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
PRAGMA_ENABLE_OPTIMIZATION

