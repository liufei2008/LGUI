// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Engine/World.h"
#include "Interaction/UISelectableComponent.h"
#include "Core/LGUISettings.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "PrefabSystem/ActorSerializer.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIRenderable.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
#include "EditorViewportClient.h"
#endif


ULGUIEditorManagerObject* ULGUIEditorManagerObject::Instance = nullptr;
ULGUIEditorManagerObject::ULGUIEditorManagerObject()
{

}
void ULGUIEditorManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnSelectionChangedDelegateHandle.IsValid())
	{
		USelection::SelectObjectEvent.Remove(OnSelectionChangedDelegateHandle);
	}
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULGUIEditorManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITORONLY_DATA
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
#endif
#if WITH_EDITOR
	CheckEditorViewportIndexAndKey();
#endif
}
TStatId ULGUIEditorManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}
#if WITH_EDITORONLY_DATA
bool ULGUIEditorManagerObject::CanExecuteSelectionConvert = true;
#endif
#if WITH_EDITOR
bool ULGUIEditorManagerObject::InitCheck(UWorld* InWorld)
{
	if (Instance == nullptr)
	{
		if (IsValid(InWorld))
		{
			Instance = NewObject<ULGUIEditorManagerObject>();
			Instance->AddToRoot();
			UE_LOG(LGUI, Log, TEXT("[ULGUIManagerObject::InitCheck]No Instance for LGUIManagerObject, create!"));
			//selection
			{
				Instance->OnSelectionChangedDelegateHandle = USelection::SelectionChangedEvent.AddUObject(Instance, &ULGUIEditorManagerObject::OnSelectionChanged);
			}
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool ULGUIEditorManagerObject::IsSelected(AActor* InObject)
{
	TArray<UObject*> selection;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator()); itr; ++itr)
	{
		auto itrActor = Cast<AActor>(*itr);
		if (itrActor == InObject)
		{
			return true;
		}
	}
	return false;
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
		Instance->allUIItem.RemoveSingle(InItem);
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
		Instance->allCanvas.RemoveSingle(InCanvas);
	}
}


void ULGUIEditorManagerObject::CheckEditorViewportIndexAndKey()
{
	auto& viewportClients = GEditor->GetAllViewportClients();
	if (PrevEditorViewportCount != viewportClients.Num())
	{
		PrevEditorViewportCount = viewportClients.Num();
		EditorViewportIndexToKeyMap.Reset();
		for (FEditorViewportClient* viewportClient : viewportClients)
		{
			auto viewKey = viewportClient->ViewState.GetReference()->GetViewKey();
			EditorViewportIndexToKeyMap.Add(viewportClient->ViewIndex, viewKey);
		}

		if (EditorViewportIndexAndKeyChange.IsBound())
		{
			EditorViewportIndexAndKeyChange.Broadcast();
		}
	}

	if (auto viewport = GEditor->GetActiveViewport())
	{
		if (auto viewportClient = viewport->GetClient())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				auto editorViewportClient = (FEditorViewportClient*)viewportClient;
				CurrentActiveViewportIndex = editorViewportClient->ViewIndex;
				CurrentActiveViewportKey = ULGUIEditorManagerObject::Instance->GetViewportKeyFromIndex(editorViewportClient->ViewIndex);
			}
		}
	}
}
uint32 ULGUIEditorManagerObject::GetViewportKeyFromIndex(int32 InViewportIndex)
{
	if (auto key = EditorViewportIndexToKeyMap.Find(InViewportIndex))
	{
		return *key;
	}
	return 0;
}



void ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(UWorld* InWorld)
{
	if (InitCheck(InWorld))
	{
		Instance->AllActors_PrefabSystemProcessing.Reset();
	}
}
void ULGUIEditorManagerObject::EndPrefabSystemProcessingActor()
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.Reset();
	}
}
void ULGUIEditorManagerObject::AddActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.AddUnique(InActor);
	}
}
void ULGUIEditorManagerObject::RemoveActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.RemoveSingle(InActor);
	}
}
bool ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(AActor* InActor)
{
	if (Instance != nullptr)
	{
		return Instance->AllActors_PrefabSystemProcessing.Contains(InActor);
	}
	return false;
}
void ULGUIEditorManagerObject::OnSelectionChanged(UObject* newSelection)
{
	if (!ULGUIEditorManagerObject::CanExecuteSelectionConvert)return;
	if (IsCalculatingSelection)return;//incase infinite recursive, because selection can change inside this function
	IsCalculatingSelection = true;

	AUIBaseActor* selectedActor = nullptr;
	ULGUICanvas* selectedCanvas = nullptr;
	if (USelection* selection = Cast<USelection>(newSelection))
	{
		if (selection->Num() > 0)
		{
			selectedActor = Cast<AUIBaseActor>(selection->GetSelectedObject(0));
			if (IsValid(selectedActor))
			{
				selectedCanvas = selectedActor->FindComponentByClass<ULGUICanvas>();
			}
		}
	}
	if (IsValid(selectedCanvas))
	{
		auto world = selectedCanvas->GetWorld();
		if (!world->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				auto& allUIItems = ULGUIEditorManagerObject::Instance->GetAllUIItem();
				if (GEditor)
				{
					if (auto viewport = GEditor->GetActiveViewport())
					{
						auto mouseX = viewport->GetMouseX();
						auto mouseY = viewport->GetMouseY();
						FVector rayOrigin, rayDirection;
						auto client = (FEditorViewportClient*)viewport->GetClient();
						FSceneView::DeprojectScreenToWorld(FVector2D(mouseX, mouseY), UUIItemEditorHelperComp::viewRect, UUIItemEditorHelperComp::viewMatrices.GetInvViewMatrix(), UUIItemEditorHelperComp::viewMatrices.GetInvProjectionMatrix(), rayOrigin, rayDirection);
						float lineTraceLength = 10000;
						//find hit UIRenderable
						auto lineStart = rayOrigin;
						auto lineEnd = rayOrigin + rayDirection * lineTraceLength;
						CacheHitResultArray.Reset();
						for (auto uiItem : allUIItems)
						{
							if (uiItem->GetWorld() == world)
							{
								if (auto uiRenderable = Cast<UUIRenderable>(uiItem))
								{
									FHitResult hitInfo;
									auto originRaycastComplex = uiRenderable->GetRaycastComplex();
									auto originRaycastTarget = uiRenderable->IsRaycastTarget();
									uiRenderable->SetRaycastComplex(true);//in editor selection, make the ray hit actural triangle
									uiRenderable->SetRaycastTarget(true);
									if (uiRenderable->LineTraceUI(hitInfo, lineStart, lineEnd))
									{
										if (uiRenderable->GetRenderCanvas()->IsPointVisible(hitInfo.Location))
										{
											CacheHitResultArray.Add(hitInfo);
										}
									}
									uiRenderable->SetRaycastComplex(originRaycastComplex);
									uiRenderable->SetRaycastTarget(originRaycastTarget);
								}
							}
						}
						if (CacheHitResultArray.Num() > 0)//hit something
						{
							CacheHitResultArray.Sort([](const FHitResult& A, const FHitResult& B)
								{
									auto AUIRenderable = (UUIRenderable*)(A.Component.Get());
									auto BUIRenderable = (UUIRenderable*)(B.Component.Get());
									if (AUIRenderable->GetDepth() == BUIRenderable->GetDepth())
									{
										return A.Distance < B.Distance;
									}
									else
									{
										return AUIRenderable->GetDepth() > BUIRenderable->GetDepth();
									}
								});
							if (auto uiRenderableComp = Cast<UUIRenderable>(CacheHitResultArray[0].Component.Get()))//target need to select
							{
								if (LastSelectTarget.Get() == uiRenderableComp)//if selection not change, then select hierarchy up
								{
									if (auto parentActor = LastSelectedActor->GetAttachParentActor())
									{
										LastSelectedActor = parentActor;
									}
									else//not have parent, loop back to origin
									{
										LastSelectedActor = uiRenderableComp->GetOwner();
									}
								}
								else
								{
									LastSelectedActor = uiRenderableComp->GetOwner();
								}
								GEditor->SelectNone(true, true);
								GEditor->SelectActor(LastSelectedActor.Get(), true, true);
								LastSelectTarget = uiRenderableComp;
								goto END;
							}
						}
					}
				}
			}
		}
	}

	LastSelectTarget.Reset();
	LastSelectedActor.Reset();

	END:
	IsCalculatingSelection = false;
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

bool ALGUIManagerActor::IsPlaying = false;
void ALGUIManagerActor::BeginPlay()
{
	Super::BeginPlay();
	IsPlaying = true;
}
void ALGUIManagerActor::BeginDestroy()
{
	Instance = nullptr;
	Super::BeginDestroy();
	IsPlaying = false;
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
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::Awake;
	for (int i = 0; i < LGUIBehavioursForAwake.Num(); i++)
	{
		auto item = LGUIBehavioursForAwake[i];
		//UE_LOG(LGUI, Error, TEXT("Awake count:%d, i:%d, item:%s"), LGUIBehavioursForAwake.Num(), i, *(item->GetOwner()->GetActorLabel()));
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				//add to Enable array
				{
					if (!LGUIBehavioursForEnable.Contains(item))
					{
						AddLGUIBehaviourToArrayWithOrder(item, LGUIBehavioursForEnable);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to enable array but already exist! comp:%s"), *(item->GetPathName()));
					}
				}
				//remove from Awake array
				{
					LGUIBehavioursForAwake.RemoveAt(i);
					i--;
				}

				item->Awake();
			}
		}
	}
	firstAwakeExecuted = true;
	//enable
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::OnEnable;
	for (int i = 0; i < LGUIBehavioursForEnable.Num(); i++)
	{
		auto item = LGUIBehavioursForEnable[i];
		//UE_LOG(LGUI, Error, TEXT("Enable count:%d, i:%d, item:%s"), LGUIBehavioursForEnable.Num(), i, *(item->GetOwner()->GetActorLabel()));
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				//add to Start array
				{
					if (!LGUIBehavioursForStart.Contains(item))
					{
						AddLGUIBehaviourToArrayWithOrder(item, LGUIBehavioursForStart);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to start array but already exist! comp:%s"), *(item->GetPathName()));
					}
				}
				//remove from Enable array
				{
					LGUIBehavioursForEnable.RemoveAt(i);
					i--;
				}

				item->OnEnable();
			}
		}
	}
	//disabled after update called
	for (int i = 0; i < LGUIBehavioursDisabled.Num(); i++)
	{
		auto item = LGUIBehavioursDisabled[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				//add to Update array
				{
					if (!LGUIBehavioursForUpdate.Contains(item))
					{
						AddLGUIBehaviourToArrayWithOrder(item, LGUIBehavioursForUpdate);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to start array but already exist! comp:%s"), *(item->GetPathName()));
					}
				}
				//remove from Disabled array
				{
					LGUIBehavioursDisabled.RemoveAt(i);
					i--;
				}
				item->OnEnable();
			}
		}
	}
	//start
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::Start;
	for (int i = 0; i < LGUIBehavioursForStart.Num(); i++)
	{
		auto item = LGUIBehavioursForStart[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				//add to Update array
				{
					if (!LGUIBehavioursForUpdate.Contains(item))
					{
						AddLGUIBehaviourToArrayWithOrder(item, LGUIBehavioursForUpdate);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to start array but already exist! comp:%s"), *(item->GetPathName()));
					}
				}
				//remove from Start array
				{
					LGUIBehavioursForStart.RemoveAt(i);
					i--;
				}

				item->Start();
			}
		}
	}
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::AfterStart;
}

DECLARE_CYCLE_STAT(TEXT("LGUIBehaviour Update"), STAT_LGUIBehaviourUpdate, STATGROUP_LGUI);

void ALGUIManagerActor::Tick_DuringPhysics(float deltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_LGUIBehaviourUpdate);
	//update
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::Update;
	for (int i = 0; i < LGUIBehavioursForUpdate.Num(); i++)
	{
		auto item = LGUIBehavioursForUpdate[i];
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				item->Update(deltaTime);
			}
			else
			{
				//add to enable array
				{
					if (!LGUIBehavioursDisabled.Contains(item))
					{
						LGUIBehavioursDisabled.Add(item);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_DuringPhysics]trying to add to disabled array but already exist!"));
					}
				}
				//remove form update array
				{
					LGUIBehavioursForUpdate.RemoveAt(i);
					i--;
				}

				item->OnDisable();
			}
		}
	}
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::None;
}

void ALGUIManagerActor::AddLGUIComponent(ULGUIBehaviour* InComp)
{
	if (InitCheck(InComp->GetWorld()))
	{
		if (IsPrefabSystemProcessingActor(InComp->GetOwner()))//if this component is processing by prefab system(creating from prefab, or duplicating from ActorCopier), then we need to collect it to custom awake array
		{
			if (Instance->PrefabSystemProcessing_CurrentArrayIndex < 0 || Instance->PrefabSystemProcessing_CurrentArrayIndex >= Instance->LGUIBehaviours_PrefabSystemProcessing.Num())
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddLGUIComponent]array out of range, index:%d, arrayCount:%d"), Instance->PrefabSystemProcessing_CurrentArrayIndex, Instance->LGUIBehaviours_PrefabSystemProcessing.Num());
				return;
			}
			auto& compArray = Instance->LGUIBehaviours_PrefabSystemProcessing[Instance->PrefabSystemProcessing_CurrentArrayIndex].LGUIBehaviourArray;
			if (compArray.Contains(InComp))
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddLGUIComponent]already contains, comp:%s"), *(InComp->GetPathName()));
				return;
			}
			Instance->AddLGUIBehaviourToArrayWithOrder(InComp, compArray);
		}
		else
		{
			if (Instance->firstAwakeExecuted//if first awake is already executed, then just call awake
				&& InComp->GetIsActiveAndEnable()//should call awake now?
				)
			{
				//add to Enable array
				{
					if (!Instance->LGUIBehavioursForEnable.Contains(InComp))
					{
						Instance->AddLGUIBehaviourToArrayWithOrder(InComp, Instance->LGUIBehavioursForEnable);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::AddLGUIComponent]trying to add to enable array but already exist! comp:%s"), *(InComp->GetPathName()));
					}
				}

				InComp->Awake();
			}
			else
			{
				auto& LGUIBehavioursForAwake = Instance->LGUIBehavioursForAwake;
				if (LGUIBehavioursForAwake.Contains(InComp))
				{
					UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddLGUIComponent]already contains, comp:%s"), *(InComp->GetPathName()));
					return;
				}
				if (Instance->scriptExecutingType == ELGUIBehaviourScriptExecutingType::Awake)//this means script is created during Awake, then put the script at the end of array
				{
					LGUIBehavioursForAwake.Add(InComp);
				}
				else
				{
					Instance->AddLGUIBehaviourToArrayWithOrder(InComp, LGUIBehavioursForAwake);
				}
			}
		}
	}
}
void ALGUIManagerActor::AddLGUIBehaviourToArrayWithOrder(ULGUIBehaviour* InComp, TArray<ULGUIBehaviour*>& InArray)
{
#if WITH_EDITOR
	auto& lguiBehaviourExecuteOrders = ULGUISettings::GetLGUIBehaviourExecuteOrder();
#else
	static auto& lguiBehaviourExecuteOrders = ULGUISettings::GetLGUIBehaviourExecuteOrder();
#endif
	//UE_LOG(LGUI, Error, TEXT("executeOrderCount:%d"), lguiBehaviourExecuteOrders.Num());
	if (lguiBehaviourExecuteOrders.Num() > 0 && InArray.Num() > 0)
	{
		auto inCompClass = InComp->GetClass();
		int inCompIndex = INDEX_NONE;
		if (lguiBehaviourExecuteOrders.Find(inCompClass, inCompIndex))
		{
			bool addToArray = false;
			for (int i = 0; i < InArray.Num(); i++)
			{
				auto checkItemClass = InArray[i]->GetClass();
				int checkItemIndex = INDEX_NONE;
				if (lguiBehaviourExecuteOrders.Find(checkItemClass, checkItemIndex))//exist, check index
				{
					if (inCompIndex > checkItemIndex)
					{		
						//addToArray = false;
						continue;
					}
					else
					{
						InArray.Insert(InComp, i);
						addToArray = true;
						break;
					}
				}
				else//none exist
				{
					InArray.Insert(InComp, i);
					addToArray = true;
					break;
				}
			}
			if (!addToArray)
			{
				InArray.Add(InComp);
			}
		}
		else//class no need reorder
		{
			InArray.Add(InComp);
		}
	}
	else
	{
		InArray.Add(InComp);
	}
}
void ALGUIManagerActor::RemoveLGUIComponent(ULGUIBehaviour* InComp)
{
	if (Instance != nullptr)
	{
		int32 index = INDEX_NONE;
		if (Instance->LGUIBehavioursForAwake.Find(InComp, index))
		{
			Instance->LGUIBehavioursForAwake.RemoveAt(index);
			return;
		}
		if (Instance->LGUIBehavioursForEnable.Find(InComp, index))
		{
			Instance->LGUIBehavioursForEnable.RemoveAt(index);
			return;
		}
		if (Instance->LGUIBehavioursDisabled.Find(InComp, index))
		{
			Instance->LGUIBehavioursDisabled.RemoveAt(index);
			return;
		}
		if (Instance->LGUIBehavioursForStart.Find(InComp, index))
		{
			Instance->LGUIBehavioursForStart.RemoveAt(index);
			return;
		}
		if (Instance->LGUIBehavioursForUpdate.Find(InComp, index))
		{
			Instance->LGUIBehavioursForUpdate.RemoveAt(index);
			return;
		}
		UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::RemoveLGUIComponent]not exist, comp:%s"), *(InComp->GetPathName()));
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
		Instance->allUIItem.RemoveSingle(InItem);
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
		Instance->allCanvas.RemoveSingle(InCanvas);
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

void ALGUIManagerActor::SetInputModule(ULGUIBaseInputModule* InInputModule)
{
	if (InitCheck(InInputModule->GetWorld()))
	{
		Instance->currentInputModule = InInputModule;
	}
}
void ALGUIManagerActor::ClearInputModule()
{
	if (Instance != nullptr)
	{
		Instance->currentInputModule = nullptr;
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


void ALGUIManagerActor::EndPrefabSystemProcessingActor_Implement()
{
	auto LGUIBehaviourArray = LGUIBehaviours_PrefabSystemProcessing.Pop().LGUIBehaviourArray;
	PrefabSystemProcessing_CurrentArrayIndex--;

	//awake
	scriptExecutingType = ELGUIBehaviourScriptExecutingType::Awake;
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto item = LGUIBehaviourArray[i];
		//UE_LOG(LGUI, Error, TEXT("Awake count:%d, i:%d, item:%s"), LGUIBehaviourArray.Num(), i, *(item->GetOwner()->GetActorLabel()));
		if (IsValid(item))
		{
			if (item->GetIsActiveAndEnable())
			{
				//add to Enable array
				{
					if (!LGUIBehavioursForEnable.Contains(item))
					{
						AddLGUIBehaviourToArrayWithOrder(item, LGUIBehavioursForEnable);
					}
					else
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::Tick_PrePhysics]trying to add to enable array but already exist! comp:%s"), *(item->GetPathName()));
					}
				}
				item->Awake();
			}
			else
			{
				LGUIBehavioursForAwake.AddUnique(item);
			}
		}
	}

	scriptExecutingType = ELGUIBehaviourScriptExecutingType::None;
}
void ALGUIManagerActor::BeginPrefabSystemProcessingActor(UWorld* InWorld)
{
	if (InitCheck(InWorld))
	{
		Instance->LGUIBehaviours_PrefabSystemProcessing.Add({});
		Instance->PrefabSystemProcessing_CurrentArrayIndex++;
	}
}
void ALGUIManagerActor::EndPrefabSystemProcessingActor()
{
	if (Instance != nullptr)
	{
		Instance->EndPrefabSystemProcessingActor_Implement();
	}
}
void ALGUIManagerActor::AddActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.AddUnique(InActor);
	}
}
void ALGUIManagerActor::RemoveActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.RemoveSingle(InActor);
	}
}
bool ALGUIManagerActor::IsPrefabSystemProcessingActor(AActor* InActor)
{
	if (Instance != nullptr)
	{
		return Instance->AllActors_PrefabSystemProcessing.Contains(InActor);
	}
	return false;
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
