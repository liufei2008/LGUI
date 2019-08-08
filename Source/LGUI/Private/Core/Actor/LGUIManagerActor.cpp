// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIPanel.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Engine/World.h"
#if WITH_EDITOR
#include "Editor.h"
#endif


ULGUIEditorManagerObject* ULGUIEditorManagerObject::Instance = nullptr;
ULGUIEditorManagerObject::ULGUIEditorManagerObject()
{

}
void ULGUIEditorManagerObject::BeginDestroy()
{
	Instance = nullptr;
	Super::BeginDestroy();
}

bool ULGUIEditorManagerObject::InitCheck(UWorld* InWorld)
{
	if (Instance == nullptr)
	{
		if (IsValid(InWorld))
		{
			Instance = NewObject<ULGUIEditorManagerObject>();
			Instance->AddToRoot();
			UE_LOG(LGUI, Log, TEXT("[ULGUIManagerObject::InitCheck]No Instance for LGUIManagerObject, create!"));
		}
		else
		{
			return false;
		}
	}
	return true;
}

void ULGUIEditorManagerObject::AddUIItem(UUIItem* InItem)
{
	if (InitCheck(InItem->GetWorld()))
	{
		Instance->allUIItem.Add(InItem);
	}
}
void ULGUIEditorManagerObject::RemoveUIItem(UUIItem* InItem)
{
	if (Instance != nullptr)
	{
		Instance->allUIItem.Remove(InItem);
	}
}
const TArray<UUIItem*>& ULGUIEditorManagerObject::GetAllUIItem()
{
	return allUIItem;
}

void ULGUIEditorManagerObject::AddUIPanel(UUIPanel* InPanel, bool InSortDepth)
{
	if (InitCheck(InPanel->GetWorld()))
	{
		auto& uiPanelArray = Instance->allUIPanel;
		uiPanelArray.AddUnique(InPanel);
		//sort on depth
		uiPanelArray.Sort([](const UUIPanel& A, const UUIPanel& B)
		{
			return A.GetDepth() < B.GetDepth();
		});
	}
}
void ULGUIEditorManagerObject::SortUIPanelOnDepth()
{
	if (Instance != nullptr)
	{
		//sort on depth
		Instance->allUIPanel.Sort([](const UUIPanel& A, const UUIPanel& B)
		{
			return A.GetDepth() < B.GetDepth();
		});
	}
}
void ULGUIEditorManagerObject::RemoveUIPanel(UUIPanel* InPanel)
{
	if (Instance != nullptr)
	{
		Instance->allUIPanel.Remove(InPanel);
	}
}
const TArray<UUIPanel*>& ULGUIEditorManagerObject::GetAllUIPanel()
{
	return allUIPanel;
}

void ULGUIEditorManagerObject::Tick(float DeltaTime)
{
	for (auto item : allUIPanel)
	{
		if (IsValid(item))
		{
			item->CustomTick(DeltaTime);
		}
	}
	if (EditorTick.IsBound())
	{
		EditorTick.Broadcast(DeltaTime);
	}
}
TStatId ULGUIEditorManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}



ALGUIManagerActor* ALGUIManagerActor::Instance = nullptr;
ALGUIManagerActor::ALGUIManagerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR
	FEditorDelegates::BeginPIE.AddLambda([=](const bool isSimulating) {
		if (Instance != nullptr)
		{
			LGUIUtils::DeleteActor(Instance);//delete any instance before begin play
			Instance = nullptr;
		}
	});
#endif
}

void ALGUIManagerActor::BeginDestroy()
{
	Instance = nullptr;
	Super::BeginDestroy();
}

bool ALGUIManagerActor::InitCheck(UWorld* InWorld)
{
	if (Instance == nullptr)
	{
		if (IsValid(InWorld))
		{
			FActorSpawnParameters param = FActorSpawnParameters();
			param.ObjectFlags = RF_Transient;
			Instance = InWorld->SpawnActor<ALGUIManagerActor>(param);
			UE_LOG(LGUI, Log, TEXT("[ALGUIManagerActor::InitCheck]No Instance for LGUIManagerActor, create!"));
		}
		else
		{
			return false;
		}
	}
	return true;
}


DECLARE_CYCLE_STAT(TEXT("LGUIManagerTick"), STAT_LGUIManagerTick, STATGROUP_LGUI);
void ALGUIManagerActor::Tick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LGUIManagerTick);
	for (auto item : allUIPanel)
	{
		if (IsValid(item))
		{
			item->CustomTick(DeltaTime);
		}
	}
}


void ALGUIManagerActor::AddUIItem(UUIItem* InItem)
{
	if (InitCheck(InItem->GetWorld()))
	{
		Instance->allUIItem.Add(InItem);
	}
}
void ALGUIManagerActor::RemoveUIItem(UUIItem* InItem)
{
	if (Instance != nullptr)
	{
		Instance->allUIItem.Remove(InItem);
	}
}
const TArray<UUIItem*>& ALGUIManagerActor::GetAllUIItem()
{
	return allUIItem;
}

void ALGUIManagerActor::AddUIPanel(UUIPanel* InPanel, bool InSortDepth)
{
	if (InitCheck(InPanel->GetWorld()))
	{
		auto& uiPanelArray = Instance->allUIPanel;
		uiPanelArray.AddUnique(InPanel);
		//sort on depth
		uiPanelArray.Sort([](const UUIPanel& A, const UUIPanel& B)
		{
			return A.GetDepth() < B.GetDepth();
		});
	}
}
void ALGUIManagerActor::SortUIPanelOnDepth()
{
	if (Instance != nullptr)
	{
		//sort on depth
		Instance->allUIPanel.Sort([](const UUIPanel& A, const UUIPanel& B)
		{
			return A.GetDepth() < B.GetDepth();
		});
	}
}
void ALGUIManagerActor::RemoveUIPanel(UUIPanel* InPanel)
{
	if (Instance != nullptr)
	{
		Instance->allUIPanel.Remove(InPanel);
	}
}
const TArray<UUIPanel*>& ALGUIManagerActor::GetAllUIPanel()
{
	return allUIPanel;
}

const TArray<ULGUIBaseRaycaster*>& ALGUIManagerActor::GetRaycasters()
{
	return raycasterArray;
}
void ALGUIManagerActor::AddRaycaster(ULGUIBaseRaycaster* InRaycaster)
{
	if (InitCheck(InRaycaster->GetWorld()))
	{
		auto& raycasterArray = Instance->raycasterArray;
		if (raycasterArray.Contains(InRaycaster))return;
		raycasterArray.Add(InRaycaster);
		//sort depth
		raycasterArray.Sort([](const ULGUIBaseRaycaster& A, const ULGUIBaseRaycaster& B)
		{
			return A.depth > B.depth;
		});
	}
}
void ALGUIManagerActor::RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster)
{
	if (Instance != nullptr)
	{
		int32 index;
		if (Instance->raycasterArray.Find(InRaycaster, index))
		{
			Instance->raycasterArray.RemoveAt(index);
		}
	}
}



bool LGUIManager::IsManagerValid(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			return ALGUIManagerActor::Instance != nullptr;
		}
		else if (InWorld->IsEditorWorld())
		{
			return ULGUIEditorManagerObject::Instance != nullptr;
		}
#else
		return ALGUIManagerActor::Instance != nullptr;
#endif
	}
	return false;
}
void LGUIManager::AddUIItem(UUIItem* InItem)
{
	if (IsValid(InItem->GetWorld()))
	{
#if WITH_EDITOR
		if (InItem->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::AddUIItem(InItem);
		}
		else if (InItem->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::AddUIItem(InItem);
		}
#else
		ALGUIManagerActor::AddUIItem(InItem);
#endif
	}
}
void LGUIManager::RemoveUIItem(UUIItem* InItem)
{
	if (IsValid(InItem->GetWorld()))
	{
#if WITH_EDITOR
		if (InItem->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::RemoveUIItem(InItem);
		}
		else if (InItem->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::RemoveUIItem(InItem);
		}
#else
		ALGUIManagerActor::RemoveUIItem(InItem);
#endif
	}
}
const TArray<UUIItem*>& LGUIManager::GetAllUIItem(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				return ALGUIManagerActor::Instance->GetAllUIItem();
			}
		}
		else if (InWorld->IsEditorWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetAllUIItem();
			}
		}
#else
		if (ALGUIManagerActor::Instance != nullptr)
		{
			return ALGUIManagerActor::Instance->GetAllUIItem();
		}
#endif
	}
	return ALGUIManagerActor::Instance->GetAllUIItem();
}

void LGUIManager::AddUIPanel(UUIPanel* InPanel, bool InSortDepth)
{
	if (IsValid(InPanel->GetWorld()))
	{
#if WITH_EDITOR
		if (InPanel->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::AddUIPanel(InPanel);
		}
		else if (InPanel->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::AddUIPanel(InPanel);
		}
#else
		ALGUIManagerActor::AddUIPanel(InPanel);
#endif
	}
}
void LGUIManager::SortUIPanelOnDepth(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			ALGUIManagerActor::SortUIPanelOnDepth();
		}
		else if (InWorld->IsEditorWorld())
		{
			ULGUIEditorManagerObject::SortUIPanelOnDepth();
		}
#else
		ALGUIManagerActor::SortUIPanelOnDepth();
#endif
	}
}
void LGUIManager::RemoveUIPanel(UUIPanel* InPanel)
{
	if (IsValid(InPanel->GetWorld()))
	{
#if WITH_EDITOR
		if (InPanel->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::RemoveUIPanel(InPanel);
		}
		else if (InPanel->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::RemoveUIPanel(InPanel);
		}
#else
		ALGUIManagerActor::RemoveUIPanel(InPanel);
#endif
	}
}
const TArray<UUIPanel*>& LGUIManager::GetAllUIPanel(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				return ALGUIManagerActor::Instance->GetAllUIPanel();
			}
		}
		else if (InWorld->IsEditorWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetAllUIPanel();
			}
		}
#else
		if (ALGUIManagerActor::Instance != nullptr)
		{
			return ALGUIManagerActor::Instance->GetAllUIPanel();
		}
#endif
	}
	return ALGUIManagerActor::Instance->GetAllUIPanel();
}

