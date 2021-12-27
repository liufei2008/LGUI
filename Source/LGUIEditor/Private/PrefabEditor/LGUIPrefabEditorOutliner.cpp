// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabEditorOutliner.h"
#include "SceneOutlinerModule.h"
#include "Modules/ModuleManager.h"
#include "GameFramework/Actor.h"
#include "Widgets/Layout/SBox.h"
#include "SceneOutlinerDelegates.h"
#include "ICustomSceneOutliner.h"
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




PRAGMA_ENABLE_OPTIMIZATION

