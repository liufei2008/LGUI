// Copyright 2019 LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"
#include "LGUIEditorModule.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Views/STreeView.h"
#include "ISceneOutliner.h"
#include "ITreeItem.h"
#include "ActorTreeItem.h"

void ULGUINativeSceneOutlinerExtension::Init()
{
	FEditorDelegates::PreSaveWorld.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnPreSaveWorld);
	FEditorDelegates::OnMapOpened.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnMapOpened);
	FEditorDelegates::PreBeginPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnBeginPIE);
	FEditorDelegates::BeginPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::DelayRestoreHierarchy);
	FEditorDelegates::EndPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::DelayRestoreHierarchy);
}
void ULGUINativeSceneOutlinerExtension::Tick(float DeltaTime)
{
	if (needToRestore)
	{
		if (frameCount >= 1)
		{
			frameCount = 0;
			needToRestore = false;
			RestoreSceneOutlinerTreeFolder();
		}
		frameCount++;
	}
}
TStatId ULGUINativeSceneOutlinerExtension::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}

void ULGUINativeSceneOutlinerExtension::OnPreSaveWorld(uint32 SaveFlags, UWorld* World)
{

}
void ULGUINativeSceneOutlinerExtension::OnMapOpened(const FString& FileName, bool AsTemplate)
{
	
}
void ULGUINativeSceneOutlinerExtension::OnBeginPIE(const bool IsSimulating)
{
	SaveSceneOutlinerTreeFolder();
}
void ULGUINativeSceneOutlinerExtension::DelayRestoreHierarchy(const bool IsSimulating)
{
	needToRestore = true;
	frameCount = 0;
}

struct FToggleExpansionVisitor : SceneOutliner::IMutableTreeItemVisitor
{
	const bool bSetExpansion;

	FToggleExpansionVisitor(bool bInSetExpansion) : bSetExpansion(bInSetExpansion) {}

	virtual void Visit(SceneOutliner::FActorTreeItem& ActorItem) const override
	{
		AActor* Actor = ActorItem.Actor.Get();
		if (Actor)
		{
			ActorItem.Flags.bIsExpanded = bSetExpansion;
		}
	}
};

void ULGUINativeSceneOutlinerExtension::SaveSceneOutlinerTreeFolder()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	TArray<AActor*> ExpandedActorTreeItemArray;
	if (LevelEditorTabManager.IsValid())
	{
		TSharedPtr<SDockTab> SceneOutlinerTab = LevelEditorTabManager->FindExistingLiveTab(FTabId("LevelEditorSceneOutliner"));
		if (SceneOutlinerTab.IsValid())
		{
			auto BorderWidget = StaticCastSharedRef<SBorder>(SceneOutlinerTab->GetContent());
			auto SceneOutlinerWidget = StaticCastSharedRef<ISceneOutliner>(BorderWidget->GetContent());
			const auto& TreeView = SceneOutlinerWidget->GetTree();
			TSet<SceneOutliner::FTreeItemPtr> VisitingItems;
			TreeView.GetExpandedItems(VisitingItems);
			for (SceneOutliner::FTreeItemPtr& Item : VisitingItems)
			{
				TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
				if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
				{
					if (ActorTreeItem->Actor->IsValidLowLevel())
					{
						ExpandedActorTreeItemArray.Add(ActorTreeItem->Actor.Get());
					}
				}
			}
		}
	}
	UnexpandedActorArray.Reset();
	if (auto world = GEditor->GetEditorWorldContext().World())
	{
		int count = 0;
		for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if ((*ActorItr)->IsPendingKill())continue;
			if (!(*ActorItr)->IsValidLowLevel())continue;
			if (!ExpandedActorTreeItemArray.Contains(*ActorItr))
			{
				UnexpandedActorArray.Add((*ActorItr)->GetFName());
			}
			count++;
		}
	}
}

void ULGUINativeSceneOutlinerExtension::RestoreSceneOutlinerTreeFolder()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	if (LevelEditorTabManager.IsValid())
	{
		TSharedPtr<SDockTab> SceneOutlinerTab = LevelEditorTabManager->FindExistingLiveTab(FTabId("LevelEditorSceneOutliner"));
		if (SceneOutlinerTab.IsValid())
		{
			auto BorderWidget = StaticCastSharedRef<SBorder>(SceneOutlinerTab->GetContent());
			auto SceneOutlinerWidget = StaticCastSharedRef<ISceneOutliner>(BorderWidget->GetContent());
			const auto& TreeView = SceneOutlinerWidget->GetTree();
			TSet<SceneOutliner::FTreeItemPtr> VisitingItems;
			TreeView.GetExpandedItems(VisitingItems);
			for (SceneOutliner::FTreeItemPtr& Item : VisitingItems)
			{
				TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
				if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid() && !ActorTreeItem->Actor->IsPendingKillPending())
				{
					bool needToExpand = NeedToExpand(ActorTreeItem->Actor.Get());
					FToggleExpansionVisitor ExpansionVisitor(needToExpand);
					ActorTreeItem->Visit(ExpansionVisitor);
				}
			}
			SceneOutlinerWidget->Refresh();
			GEditor->RedrawLevelEditingViewports();
		}
	}
}

bool ULGUINativeSceneOutlinerExtension::NeedToExpand(AActor* InActor)
{
	return !UnexpandedActorArray.Contains(InActor->GetFName());
}