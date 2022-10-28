// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "SceneOutliner/LGUINativeSceneOutlinerExtension.h"
#include "LGUIEditorModule.h"
#include "Editor.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Views/STreeView.h"
#include "ISceneOutliner.h"
#include "ITreeItem.h"
#include "ActorTreeItem.h"
#include "FolderTreeItem.h"
#include "WorldTreeItem.h"
#include "SceneOutlinerStandaloneTypes.h"
#include "EditorActorFolders.h"
#include "Core/LGUISettings.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUINativeSceneOutlinerExtension"
FLGUINativeSceneOutlinerExtension::FLGUINativeSceneOutlinerExtension()
{
	OnPreSaveWorldDelegateHandle = FEditorDelegates::PreSaveWorld.AddRaw(this, &FLGUINativeSceneOutlinerExtension::OnPreSaveWorld);
	OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddRaw(this, &FLGUINativeSceneOutlinerExtension::OnMapOpened);
	OnPreBeginPIEDelegateHandle = FEditorDelegates::PreBeginPIE.AddRaw(this, &FLGUINativeSceneOutlinerExtension::OnPreBeginPIE);
	OnBeginPIEDelegateHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FLGUINativeSceneOutlinerExtension::OnBeginPIE);
	OnEndPIEDelegateHandle = FEditorDelegates::EndPIE.AddRaw(this, &FLGUINativeSceneOutlinerExtension::OnEndPIE);
	OnLGUIEditorPreserveHierarchyStateChangeDelegateHandle = ULGUIEditorSettings::LGUIEditorSetting_PreserveHierarchyStateChange.AddRaw(this, &FLGUINativeSceneOutlinerExtension::PreserveHierarchyStateChange);
}
FLGUINativeSceneOutlinerExtension::~FLGUINativeSceneOutlinerExtension()
{
	FEditorDelegates::PreSaveWorld.Remove(OnPreSaveWorldDelegateHandle);
	FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	FEditorDelegates::PreBeginPIE.Remove(OnPreBeginPIEDelegateHandle);
	FEditorDelegates::BeginPIE.Remove(OnBeginPIEDelegateHandle);
	FEditorDelegates::EndPIE.Remove(OnEndPIEDelegateHandle);
}
void FLGUINativeSceneOutlinerExtension::Tick(float DeltaTime)
{
	if (needToRestore)
	{
		delayRestoreTime += DeltaTime;
		if (delayRestoreTime > ULGUIEditorSettings::GetDelayRestoreHierarchyTime())
		{
			RestoreSceneOutlinerState();
			delayRestoreTime = 0;
			needToRestore = false;
		}
	}
}
TStatId FLGUINativeSceneOutlinerExtension::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}

void FLGUINativeSceneOutlinerExtension::OnPreSaveWorld(uint32 SaveFlags, UWorld* World)
{
	SaveSceneOutlinerState();
}
void FLGUINativeSceneOutlinerExtension::OnMapOpened(const FString& FileName, bool AsTemplate)
{
	SetDelayRestore(true, false);
}
void FLGUINativeSceneOutlinerExtension::OnPreBeginPIE(const bool IsSimulating)
{
	SaveSceneOutlinerState();
}
void FLGUINativeSceneOutlinerExtension::OnBeginPIE(const bool IsSimulating)
{
	SetDelayRestore(false, true);
}
void FLGUINativeSceneOutlinerExtension::OnEndPIE(const bool IsSimulating)
{
	SetDelayRestore(true, false);
}
void FLGUINativeSceneOutlinerExtension::SetDelayRestore(bool RestoreTemporarilyHidden, bool RestoreUseFName)
{
	shouldRestoreTemporarilyHidden = RestoreTemporarilyHidden;
	needToRestore = true;
	delayRestoreTime = 0;
	shouldRestoreUseFNameData = RestoreUseFName;
}
void FLGUINativeSceneOutlinerExtension::PreserveHierarchyStateChange()
{
	if (!ULGUIEditorSettings::GetPreserveHierarchyState())//no need to preseve it, just delete the actor
	{
		auto storageActor = FindDataStorageActor(false);
		LGUIUtils::DestroyActorWithHierarchy(storageActor);
	}
}

void FLGUINativeSceneOutlinerExtension::SaveSceneOutlinerState()
{
	if (!ULGUIEditorSettings::GetPreserveHierarchyState())return;
	auto storageActor = FindDataStorageActor();
	if (!storageActor)return;
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	storageActor->ExpandedFolderArray.Reset();
	TArray<AActor*> ExpandedActorArray;
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
			for (auto& Item : VisitingItems)
			{
				switch (Item->GetTypeSortPriority())
				{
				default:
				case SceneOutliner::ETreeItemSortOrder::Actor:
				{
					auto ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
					if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid())
					{
						ExpandedActorArray.Add(ActorTreeItem->Actor.Get());
					}
				}
				break;
				case SceneOutliner::ETreeItemSortOrder::Folder:
				{
					auto FolderTreeItem = StaticCastSharedPtr<SceneOutliner::FFolderTreeItem>(Item);
					if (FolderTreeItem.IsValid())
					{
						storageActor->ExpandedFolderArray.Add(FolderTreeItem->Path);
					}
				}
				break;
				case SceneOutliner::ETreeItemSortOrder::World:
				{
					auto WorldTreeItem = StaticCastSharedPtr<SceneOutliner::FWorldTreeItem>(Item);
					if (WorldTreeItem.IsValid() && WorldTreeItem->World.IsValid())
					{
						
					}
				}
				break;
				}
			}
		}
	}
	storageActor->TemporarilyHiddenActorArray.Reset();
	storageActor->TemporarilyHiddenSoftActorArray.Reset();
	storageActor->UnexpandedActorArray.Reset();
	storageActor->UnexpandedSoftActorArray.Reset();
	UnexpandedActorArray.Reset();
	if (auto world = GEditor->GetEditorWorldContext().World())
	{
		for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if(!IsValid(*ActorItr))continue;
			if (ActorItr->GetClass() == ALGUIEditorLevelDataStorageActor::StaticClass())continue;//skip it-self
			if (ActorItr->HasAnyFlags(EObjectFlags::RF_Transient))continue;//skip transient
			if (!ExpandedActorArray.Contains(*ActorItr))
			{
				if (storageActor->GetLevel() == (*ActorItr)->GetLevel())
				{
					storageActor->UnexpandedActorArray.Add(*ActorItr);
				}
				else
				{
					storageActor->UnexpandedSoftActorArray.Add(*ActorItr);
				}
				UnexpandedActorArray.Add((*ActorItr)->GetFName());
			}
			if ((*ActorItr)->IsTemporarilyHiddenInEditor())
			{
				if (storageActor->GetLevel() == (*ActorItr)->GetLevel())
				{
					storageActor->TemporarilyHiddenActorArray.Add(*ActorItr);
				}
				else
				{
					storageActor->TemporarilyHiddenSoftActorArray.Add(*ActorItr);
				}
			}
		}
	}
}
ALGUIEditorLevelDataStorageActor* FLGUINativeSceneOutlinerExtension::FindDataStorageActor(bool CreateIfNotExist)
{
	ALGUIEditorLevelDataStorageActor* result = nullptr;
	if (auto world = GEditor->GetEditorWorldContext().World())
	{
		auto baseLevel = world->GetLevel(0);
		TArray<ALGUIEditorLevelDataStorageActor*> needToDelete;
		for (TActorIterator<ALGUIEditorLevelDataStorageActor> ActorItr(world); ActorItr; ++ActorItr)
		{
			if (!IsValid(*ActorItr))continue;
			if (ActorItr->GetLevel() == baseLevel)
			{
				if (result == nullptr)
				{
					result = *ActorItr;
				}
				else
				{
					needToDelete.Add(*ActorItr);
				}
			}
		}
		if (needToDelete.Num() > 1)
		{
			auto msg = FText::Format(LOCTEXT("MultipleDataStorageActorError", "[ULGUINativeSceneOutlinerExtension::FindOrCreateDataStorageActor]There are {0} count of LGUIEditorLevelDataStorageActor, this is weird..."), needToDelete.Num());
			UE_LOG(LGUIEditor, Error, TEXT("%s"), *msg.ToString());
			LGUIUtils::EditorNotification(msg);
			for (auto item : needToDelete)
			{
				item->Destroy();
			}
			needToDelete.Empty();
		}
		if (!result && CreateIfNotExist)
		{
			world->SetCurrentLevel(baseLevel);
			result = world->SpawnActor<ALGUIEditorLevelDataStorageActor>();
		}
	}
	return result;
}

void FLGUINativeSceneOutlinerExtension::RestoreSceneOutlinerStateForTreeItem(SceneOutliner::FTreeItemPtr& Item, ALGUIEditorLevelDataStorageActor* storageActor)
{
	switch (Item->GetTypeSortPriority())
	{
	default:
	case SceneOutliner::ETreeItemSortOrder::Actor:
	{
		TSharedPtr<SceneOutliner::FActorTreeItem> ActorTreeItem = StaticCastSharedPtr<SceneOutliner::FActorTreeItem>(Item);
		if (ActorTreeItem.IsValid() && ActorTreeItem->Actor.IsValid())
		{
			//expend
			if (shouldRestoreUseFNameData)
			{
				bool needToExpand = !UnexpandedActorArray.Contains(ActorTreeItem->Actor->GetFName());
				ActorTreeItem->Flags.bIsExpanded = needToExpand;
			}
			else
			{
				if (storageActor->GetLevel() == ActorTreeItem->Actor->GetLevel())
				{
					bool needToExpand = !storageActor->UnexpandedActorArray.Contains(ActorTreeItem->Actor);
					ActorTreeItem->Flags.bIsExpanded = needToExpand;
				}
				else
				{
					bool needToExpand = !storageActor->UnexpandedSoftActorArray.Contains(ActorTreeItem->Actor.Get());
					ActorTreeItem->Flags.bIsExpanded = needToExpand;
				}
			}
		}
	}
	break;
	case SceneOutliner::ETreeItemSortOrder::Folder:
	{
		TSharedPtr<SceneOutliner::FFolderTreeItem> FolderTreeItem = StaticCastSharedPtr<SceneOutliner::FFolderTreeItem>(Item);
		if (FolderTreeItem.IsValid())
		{
			//expend
			bool needToExpand = storageActor->ExpandedFolderArray.Contains(FolderTreeItem->Path);
			FolderTreeItem->Flags.bIsExpanded = needToExpand;
		}
	}
	break;
	case SceneOutliner::ETreeItemSortOrder::World:
	{
		TSharedPtr<SceneOutliner::FWorldTreeItem> WorldTreeItem = StaticCastSharedPtr<SceneOutliner::FWorldTreeItem>(Item);
		if (WorldTreeItem.IsValid() && WorldTreeItem->World.IsValid())
		{
			
		}
	}
	break;
	}
}

void FLGUINativeSceneOutlinerExtension::RestoreSceneOutlinerState()
{
	if (!ULGUIEditorSettings::GetPreserveHierarchyState())return;
	auto storageActor = FindDataStorageActor();
	if (!storageActor)return;
	FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<FTabManager> LevelEditorTabManager = LevelEditorModule.GetLevelEditorTabManager();
	if (LevelEditorTabManager.IsValid())
	{
		TSharedPtr<SDockTab> SceneOutlinerTab = LevelEditorTabManager->FindExistingLiveTab(FTabId("LevelEditorSceneOutliner"));
		if (SceneOutlinerTab.IsValid())
		{
			auto BorderWidget = StaticCastSharedRef<SBorder>(SceneOutlinerTab->GetContent());
			auto SceneOutlinerWidget = StaticCastSharedRef<ISceneOutliner>(BorderWidget->GetContent());
			auto& TreeView = SceneOutlinerWidget->GetTree();
			TSet<SceneOutliner::FTreeItemPtr> VisitingItems;
			TreeView.GetExpandedItems(VisitingItems);
			for (auto& Item : VisitingItems)
			{
				RestoreSceneOutlinerStateForTreeItem(Item, storageActor);
			}
			GEditor->BroadcastLevelActorListChanged();
		}
	}
	if (shouldRestoreTemporarilyHidden)
	{
		//hidden
		if (auto world = GEditor->GetEditorWorldContext().World())
		{
			for (TActorIterator<AActor> ActorItr(world); ActorItr; ++ActorItr)
			{
				if (!IsValid(*ActorItr))continue;
				if (storageActor->GetLevel() == (ActorItr->GetLevel()))
				{
					if (storageActor->TemporarilyHiddenActorArray.Contains((*ActorItr)))
					{
						(*ActorItr)->SetIsTemporarilyHiddenInEditor(true);
					}
				}
				else
				{
					if (storageActor->TemporarilyHiddenSoftActorArray.Contains((*ActorItr)))
					{
						(*ActorItr)->SetIsTemporarilyHiddenInEditor(true);
					}
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE