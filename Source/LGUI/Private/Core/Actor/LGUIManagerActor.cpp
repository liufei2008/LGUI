// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Engine/World.h"
#include "Interaction/UISelectableComponent.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
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

void ULGUIEditorManagerObject::AddCanvas(ULGUICanvas* InCanvas)
{
	if (InitCheck(InCanvas->GetWorld()))
	{
		auto& canvasArray = Instance->allCanvas;
		canvasArray.AddUnique(InCanvas);
		//sort on order
		canvasArray.Sort([](const ULGUICanvas& A, const ULGUICanvas& B)
		{
			return A.GetSortOrder() < B.GetSortOrder();
		});
	}
}
void ULGUIEditorManagerObject::SortCanvasOnOrder()
{
	if (Instance != nullptr)
	{
		//sort on order
		Instance->allCanvas.Sort([](const ULGUICanvas& A, const ULGUICanvas& B)
		{
			return A.GetSortOrder() < B.GetSortOrder();
		});
	}
}
void ULGUIEditorManagerObject::RemoveCanvas(ULGUICanvas* InCanvas)
{
	if (Instance != nullptr)
	{
		Instance->allCanvas.Remove(InCanvas);
	}
}

void ULGUIEditorManagerObject::Tick(float DeltaTime)
{
	for (auto item : allCanvas)
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
#if WITH_EDITOR
bool ULGUIEditorManagerObject::IsSelected(AActor* InObject)
{
	TArray<UObject*> selection;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator());itr;++itr)
	{
		auto itrActor = Cast<AActor>(*itr);
		if (itrActor == InObject)
		{
			return true;
		}
	}
	return false;
}
#endif



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
	CreateDefaultSubobject<ULGUIManagerComponent_PrePhysics>(TEXT("Tick_PrePhysics"))->ManagerActor = this;
	CreateDefaultSubobject<ULGUIManagerComponent_DuringPhysics>(TEXT("Tick_DuringPhysics"))->ManagerActor = this;
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


//DECLARE_CYCLE_STAT(TEXT("LGUIManagerTick"), STAT_LGUIManagerTick, STATGROUP_LGUI);
void ALGUIManagerActor::Tick(float DeltaTime)
{
	//SCOPE_CYCLE_COUNTER(STAT_LGUIManagerTick);
	for (auto item : allCanvas)
	{
		if (IsValid(item))
		{
			item->CustomTick(DeltaTime);
		}
	}
}
void ALGUIManagerActor::Tick_PrePhysics()
{
	//awake
	for (int i = uiComponentsForAwake.Num() - 1; i >= 0; i--)
	{
		auto item = uiComponentsForAwake[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				item->Awake();
				//add to enable array
				{
					if (!uiComponentsForEnable.Contains(item))
					{
						uiComponentsForEnable.Add(item);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to enable array but already exist!"));
					}
				}
				//remote from array
				{
					uiComponentsForAwake.RemoveAt(i);
				}
			}
		}
	}
	//enable
	for (int i = uiComponentsForEnable.Num() - 1; i >= 0; i--)
	{
		auto item = uiComponentsForEnable[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				item->OnEnable();
				//add to start array
				{
					if (!uiComponentsForStart.Contains(item))
					{
						uiComponentsForStart.Add(item);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to start array but already exist!"));
					}
				}
				//remote from array
				{
					uiComponentsForEnable.RemoveAt(i);
				}
			}
		}
	}
	//start
	for (int i = uiComponentsForStart.Num() - 1; i >= 0; i--)
	{
		auto item = uiComponentsForStart[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				item->Start();
				//add to start array
				{
					if (!uiComponentsForUpdate.Contains(item))
					{
						uiComponentsForUpdate.Add(item);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to start array but already exist!"));
					}
				}
				//remote from array
				{
					uiComponentsForStart.RemoveAt(i);
				}
			}
		}
	}
}
void ALGUIManagerActor::Tick_DuringPhysics(float deltaTime)
{
	//update
	for (int i = uiComponentsForUpdate.Num() - 1; i >= 0; i--)
	{
		auto item = uiComponentsForUpdate[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				item->Update(deltaTime);
			}
			else
			{
				//call disable
				item->OnDisable();
				//add to enable array
				{
					if (!uiComponentsForEnable.Contains(item))
					{
						uiComponentsForEnable.Add(item);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_DuringPhysics]trying to add to enable array but already exist!"));
					}
				}
				//remote form update array
				{
					uiComponentsForUpdate.RemoveAt(i);
				}
			}
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

void ALGUIManagerActor::AddCanvas(ULGUICanvas* InCanvas)
{
	if (InitCheck(InCanvas->GetWorld()))
	{
		auto& canvasArray = Instance->allCanvas;
		canvasArray.AddUnique(InCanvas);
		//sort on depth
		canvasArray.Sort([](const ULGUICanvas& A, const ULGUICanvas& B)
		{
			return A.GetSortOrder() < B.GetSortOrder();
		});
	}
}
void ALGUIManagerActor::SortCanvasOnOrder()
{
	if (Instance != nullptr)
	{
		//sort on depth
		Instance->allCanvas.Sort([](const ULGUICanvas& A, const ULGUICanvas& B)
		{
			return A.GetSortOrder() < B.GetSortOrder();
		});
	}
}
void ALGUIManagerActor::RemoveCanvas(ULGUICanvas* InCanvas)
{
	if (Instance != nullptr)
	{
		Instance->allCanvas.Remove(InCanvas);
	}
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

void ALGUIManagerActor::AddSelectable(UUISelectableComponent* InSelectable)
{
	if (InitCheck(InSelectable->GetWorld()))
	{
		auto& allSelectableArray = Instance->allSelectableArray;
		if (allSelectableArray.Contains(InSelectable))return;
		allSelectableArray.Add(InSelectable);
	}
}
void ALGUIManagerActor::RemoveSelectable(UUISelectableComponent* InSelectable)
{
	if (Instance != nullptr)
	{
		int32 index;
		if (Instance->allSelectableArray.Find(InSelectable, index))
		{
			Instance->allSelectableArray.RemoveAt(index);
		}
	}
}
void ALGUIManagerActor::AddLGUIComponent(ULGUIBehaviour* InComp)
{
	if (InitCheck(InComp->GetWorld()))
	{
		auto& uiComponentsForAwake = Instance->uiComponentsForAwake;
		if (uiComponentsForAwake.Contains(InComp))
		{
			UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddUIComponent]already contains!"));
			return;
		}
		uiComponentsForAwake.Add(InComp);
	}
}
void ALGUIManagerActor::RemoveLGUIComponent(ULGUIBehaviour* InComp)
{
	if (Instance != nullptr)
	{
		int32 index;
		if (Instance->uiComponentsForAwake.Find(InComp, index))
		{
			Instance->uiComponentsForAwake.RemoveAt(index);
		}
	}
}

ULGUIManagerComponent_PrePhysics::ULGUIManagerComponent_PrePhysics()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}
void ULGUIManagerComponent_PrePhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsValid(ManagerActor))
	{
		ManagerActor->Tick_PrePhysics();
	}
}

ULGUIManagerComponent_DuringPhysics::ULGUIManagerComponent_DuringPhysics()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_DuringPhysics;
}
void ULGUIManagerComponent_DuringPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (IsValid(ManagerActor))
	{
		ManagerActor->Tick_DuringPhysics(DeltaTime);
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

void LGUIManager::AddCanvas(ULGUICanvas* InCanvas)
{
	if (IsValid(InCanvas->GetWorld()))
	{
#if WITH_EDITOR
		if (InCanvas->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::AddCanvas(InCanvas);
		}
		else if (InCanvas->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::AddCanvas(InCanvas);
		}
#else
		ALGUIManagerActor::AddCanvas(InCanvas);
#endif
	}
}
void LGUIManager::SortCanvasOnOrder(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			ALGUIManagerActor::SortCanvasOnOrder();
		}
		else if (InWorld->IsEditorWorld())
		{
			ULGUIEditorManagerObject::SortCanvasOnOrder();
		}
#else
		ALGUIManagerActor::SortCanvasOnOrder();
#endif
	}
}
void LGUIManager::RemoveCanvas(ULGUICanvas* InCanvas)
{
	if (IsValid(InCanvas->GetWorld()))
	{
#if WITH_EDITOR
		if (InCanvas->GetWorld()->IsGameWorld())
		{
			ALGUIManagerActor::RemoveCanvas(InCanvas);
		}
		else if (InCanvas->GetWorld()->IsEditorWorld())
		{
			ULGUIEditorManagerObject::RemoveCanvas(InCanvas);
		}
#else
		ALGUIManagerActor::RemoveCanvas(InCanvas);
#endif
	}
}
const TArray<ULGUICanvas*>& LGUIManager::GetAllCanvas(UWorld* InWorld)
{
	if (IsValid(InWorld))
	{
#if WITH_EDITOR
		if (InWorld->IsGameWorld())
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				return ALGUIManagerActor::Instance->GetAllCanvas();
			}
		}
		else if (InWorld->IsEditorWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetAllCanvas();
			}
		}
#else
		if (ALGUIManagerActor::Instance != nullptr)
		{
			return ALGUIManagerActor::Instance->GetAllCanvas();
		}
#endif
	}
	return ALGUIManagerActor::Instance->GetAllCanvas();
}
#if WITH_EDITOR
bool LGUIManager::IsSelected_Editor(AActor* InItem)
{
	return ULGUIEditorManagerObject::IsSelected(InItem);
}
#endif
