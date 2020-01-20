﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"
#include "LGUIEditorModule.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Views/STreeView.h"
#include "ISceneOutliner.h"
#include "ITreeItem.h"
#include "ActorTreeItem.h"
#include "JsonObjectConverter.h"

bool ULGUINativeSceneOutlinerExtension::active = true;

void ULGUINativeSceneOutlinerExtension::Init()
{
	FEditorDelegates::PreSaveWorld.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnPreSaveWorld);
	FEditorDelegates::OnMapOpened.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnMapOpened);
	FEditorDelegates::PreBeginPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnPreBeginPIE);
	FEditorDelegates::BeginPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnBeginPIE);
	FEditorDelegates::EndPIE.AddUObject(this, &ULGUINativeSceneOutlinerExtension::OnEndPIE);
}
void ULGUINativeSceneOutlinerExtension::Tick(float DeltaTime)
{
	if (needToRestore)
	{
		if (frameCount <= 1)
		{
			RestoreSceneOutlinerTreeFolder();
		}
		else
		{
			frameCount = 0;
			needToRestore = false;
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
	SaveSceneOutlinerTreeFolder();
	//save to file
	static FString readmeString = TEXT("This file is generated by LGUI Editor, used to restore SceneOutliner actor's folder state, only valid for editor");
	static FString readmeStringCH = TEXT("此文件是LGUI编辑器自动生成的，用来还原场景中的Actor层级的打开和关闭的状态，只在编辑器里用");
	SceneOutlinerStateStruct.readme = readmeString;
	SceneOutlinerStateStruct.readmeCH = readmeStringCH;
	auto worldPathName = World->GetPathName();
	auto& arrayValue = SceneOutlinerStateStruct.WorldNameToUnexpandedActor.FindOrAdd(worldPathName);
	arrayValue.UnexpandedActorArray = UnexpandedActorArray;
	FString jsonStr;
	FJsonObjectConverter::UStructToJsonObjectString(SceneOutlinerStateStruct, jsonStr);
	FFileHelper::SaveStringToFile(jsonStr, *GetLGUIDataFilePath());
}
void ULGUINativeSceneOutlinerExtension::OnMapOpened(const FString& FileName, bool AsTemplate)
{
	//load from file
	FString jsonStr;
	FFileHelper::LoadFileToString(jsonStr, *GetLGUIDataFilePath());
	FJsonObjectConverter::JsonObjectStringToUStruct(jsonStr, &SceneOutlinerStateStruct, 0, 0);
	auto worldPathName = GEditor->GetEditorWorldContext().World()->GetPathName();
	if (auto arrayPtr = SceneOutlinerStateStruct.WorldNameToUnexpandedActor.Find(worldPathName))
	{
		UnexpandedActorArray = arrayPtr->UnexpandedActorArray;
	}
	else
	{
		UnexpandedActorArray.Empty();
	}

	SetDelayRestore();
}
const FString& ULGUINativeSceneOutlinerExtension::GetLGUIDataFilePath()
{
	static FString jsonFilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("LGUI_Saved_Data.json"));
	return jsonFilePath;
}
void ULGUINativeSceneOutlinerExtension::OnPreBeginPIE(const bool IsSimulating)
{
	SaveSceneOutlinerTreeFolder();
}
void ULGUINativeSceneOutlinerExtension::OnBeginPIE(const bool IsSimulating)
{
	SetDelayRestore();
}
void ULGUINativeSceneOutlinerExtension::OnEndPIE(const bool IsSimulating)
{
	SetDelayRestore();
}
void ULGUINativeSceneOutlinerExtension::SetDelayRestore()
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
	if (!active)return;
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
		for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if ((*ActorItr)->IsPendingKill())continue;
			if (!(*ActorItr)->IsValidLowLevel())continue;
			if (!ExpandedActorTreeItemArray.Contains(*ActorItr))
			{
				UnexpandedActorArray.Add((*ActorItr)->GetFName());
			}
		}
	}
}

void ULGUINativeSceneOutlinerExtension::RestoreSceneOutlinerTreeFolder()
{
	if (!active)return;
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
			GEngine->BroadcastLevelActorListChanged();
		}
	}
}

bool ULGUINativeSceneOutlinerExtension::NeedToExpand(AActor* InActor)
{
	return !UnexpandedActorArray.Contains(InActor->GetFName());
}