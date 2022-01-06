// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
#include "LGUIPrefabEditor.h"

PRAGMA_DISABLE_OPTIMIZATION

FLGUIPrefabEditorOutliner::~FLGUIPrefabEditorOutliner()
{
	USelection::SelectionChangedEvent.RemoveAll(this);
}
#if ENGINE_MAJOR_VERSION >= 5
#include "ActorMode.h"
#include "SceneOutlinerPublicTypes.h"
class FLGUIPrefabEditorOutlinerMode : public FActorMode
{
public:
	FLGUIPrefabEditorOutlinerMode(SSceneOutliner* InSceneOutliner, FLGUIPrefabEditorOutliner* InPrefabEditorOutlinerPtr, TWeakObjectPtr<UWorld> InSpecifiedWorldToDisplay)
		: FActorMode(FActorModeParams(InSceneOutliner, InSpecifiedWorldToDisplay, true, true))
		, PrefabEditorOutlinerPtr(InPrefabEditorOutlinerPtr)
	{}

	virtual bool CanRenameItem(const ISceneOutlinerTreeItem& Item) const override { return false; }
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
	CachedItemPtr = ItemPtr;

	if (OnActorPickedDelegate.IsBound())
	{
		FActorTreeItem* ActorTreeItem = (FActorTreeItem*)ItemPtr.Get();
		if (ActorTreeItem)
		{
			OnActorPickedDelegate.ExecuteIfBound(ActorTreeItem->Actor.Get());
		}
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
	if (!SceneOutlinerPtr.IsValid())
	{
		return;
	}



	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			// zachma todo:
			//UE_LOG(LGUI, Log, TEXT("LGUIPreivewOutliner::OnEditorSelectionChanged, Actor=%s"), *GetNameSafe(Actor));

			if (Actor->GetWorld() != CurrentWorld.Get())
			{
				return;
			}
			OnActorPickedDelegate.ExecuteIfBound(Actor);

			//SceneOutlinerPtr->ClearSelection();
			//SceneOutlinerPtr->AddObjectToSelection(Actor);

			SelectedActor = Actor;

			//SceneOutlinerPtr	
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
#else
void FLGUIPrefabEditorOutliner::InitOutliner(UWorld* World, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr)
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
	CachedItemPtr = ItemPtr;

	if (OnActorPickedDelegate.IsBound())
	{
		SceneOutliner::FActorTreeItem* ActorTreeItem = (SceneOutliner::FActorTreeItem*)ItemPtr.Get();
		if (ActorTreeItem)
		{
			OnActorPickedDelegate.ExecuteIfBound(ActorTreeItem->Actor.Get());
		}
	}
}

void FLGUIPrefabEditorOutliner::OnSceneOutlinerDoubleClick(SceneOutliner::FTreeItemPtr ItemPtr)
{
	if (OnActorDoubleClickDelegate.IsBound())
	{
		SceneOutliner::FActorTreeItem* ActorTreeItem = (SceneOutliner::FActorTreeItem*)ItemPtr.Get();
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
	if (!SceneOutlinerPtr.IsValid())
	{
		return;
	}



	USelection* Selection = Cast<USelection>(Object);
	if (Selection)
	{
		if (AActor* Actor = Selection->GetTop<AActor>())
		{
			// zachma todo:
			//UE_LOG(LGUI, Log, TEXT("LGUIPreivewOutliner::OnEditorSelectionChanged, Actor=%s"), *GetNameSafe(Actor));

			if (Actor->GetWorld() != CurrentWorld.Get())
			{
				return;
			}
			OnActorPickedDelegate.ExecuteIfBound(Actor);

			//SceneOutlinerPtr->ClearSelection();
			//SceneOutlinerPtr->AddObjectToSelection(Actor);

			SelectedActor = Actor;

			//SceneOutlinerPtr	
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
#endif
PRAGMA_ENABLE_OPTIMIZATION

