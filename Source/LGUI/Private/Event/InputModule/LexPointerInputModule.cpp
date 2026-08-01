// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/InputModule/LexPointerInputModule.h"

#include "LGUI.h"
#include "Event/LexPointerEventData.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexWidget.h"
#include "Event/LexEventSystem.h"
#include "Event/LexBaseRaycaster.h"
#include "Event/LexScreenSpaceRaycaster.h"
#include "Event/Interface/LexNavigationInterface.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Interaction/UISelectable.h"

bool ULexPointerInputModule::LineTrace(ULexPointerEventData* InPointerEventData, FLexUIHitResultContainer& OutLexHitResult)
{
	MultiHitResult.Reset();
	auto World = this->GetWorld();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(World))
	{
		auto bIsGamePaused = World->IsPaused();
		auto& AllRaycasterArray = LexUIManager->GetAllRaycasterArray();
		InPointerEventData->HoverWidgetArray.Reset();

		FVector RayOrigin(0, 0, 0), RayDir(1, 0, 0), RayEnd(1, 0, 0);
		for (int i = 0; i < AllRaycasterArray.Num(); i++)
		{
			auto& RaycasterItem = AllRaycasterArray[i];
			if (!RaycasterItem.IsValid())continue;
			if (RaycasterItem->GetUserIndex() != EventSystem->GetUserIndex())continue;
			if (RaycasterItem->GetPointerID() != INDEX_NONE && RaycasterItem->GetPointerID() != InPointerEventData->PointerID)continue;
			if (bIsGamePaused && RaycasterItem->GetAffectByGamePause())continue;
			
			TArray<FLexUIHitResult> HitResultArray;
			RaycasterItem->Raycast(InPointerEventData, RayOrigin, RayDir, RayEnd, HitResultArray);
			if (HitResultArray.Num() > 0)
			{
				if (!HitResultArray[0].Widget->GetInteractableInHierarchy())
				{
					return false;
				}
				FLexUIHitResultContainer LexHitResult;
				LexHitResult.HitResult = HitResultArray[0];
				LexHitResult.Raycaster = RaycasterItem.Get();
				LexHitResult.RayOrigin = RayOrigin;
				LexHitResult.RayDirection = RayDir;
				LexHitResult.RayEnd = RayEnd;
				for (auto& HitItem : HitResultArray)
				{
					LexHitResult.HoverArray.Add(HitItem.Widget.Get());
				}
				MultiHitResult.Add(LexHitResult);
			}
		}
		if (MultiHitResult.Num() == 0)
		{
			return false;
		}
		else if (MultiHitResult.Num() > 1)
		{
			//sort only on distance (not depth), because multiHitResult only store hit result of same depth
			MultiHitResult.Sort([](const FLexUIHitResultContainer& A, const FLexUIHitResultContainer& B)
			{
				auto AIsScreenSpace = A.Raycaster->IsA(ULexScreenSpaceRaycaster::StaticClass());
				auto BIsScreenSpace = B.Raycaster->IsA(ULexScreenSpaceRaycaster::StaticClass());
				if (AIsScreenSpace && !BIsScreenSpace)
				{
					return true;
				}
				if (BIsScreenSpace && !AIsScreenSpace)
				{
					return false;
				}
				return A.HitResult.Distance < B.HitResult.Distance;
			});
			for (auto& hitResultItem : MultiHitResult)
			{
				for (auto& hoverItem : hitResultItem.HoverArray)
				{
					InPointerEventData->HoverWidgetArray.Add(hoverItem);
				}
			}
		}
		else
		{
			for (auto hoverItem : MultiHitResult[0].HoverArray)
			{
				InPointerEventData->HoverWidgetArray.Add(hoverItem);
			}
		}
		OutLexHitResult = MultiHitResult[0];
		return true;
	}
	return false;
}

//@todo: these logs is just for editor testing, remove them when ready
#define LOG_ENTER_EXIT 0
void ULexPointerInputModule::ProcessPointerEnterExit(ULexEventSystem* EventSystem, ULexPointerEventData* EventData, ULexWidget* OldObj, ULexWidget* NewObj)
{
	if (OldObj == NewObj)return;
	if (IsValid(OldObj) && IsValid(NewObj))
	{
		auto CommonRoot = FindCommonRoot(OldObj, NewObj);
#if LOG_ENTER_EXIT
		UE_LOG(LGUI, Error, TEXT("-----begin exit 000, commonRoot:%s"), CommonRoot != nullptr ? *(CommonRoot->GetDisplayName()) : TEXT("null"));
#endif
		//exit old
		for (int i = EventData->EnterWidgetStack.Num() - 1; i >= 0; i--)
		{
			if (CommonRoot == EventData->EnterWidgetStack[i])
			{
				break;
			}
			if (!EventData->bIsExitFiredAtCurrentFrame)
			{
				if (EventSystem == nullptr)
				{
					ULexEventSystem::ExecuteEvent_OnPointerExit(EventData->EnterWidgetStack[i], EventData);
				}
				else
				{
					EventSystem->CallOnPointerExit(EventData->EnterWidgetStack[i], EventData);
				}
			}
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("	%s"), *(EventData->EnterWidgetStack[i]->GetDisplayName()));
#endif
			EventData->EnterWidgetStack.RemoveAt(i);
		}
		EventData->EnterWidget = nullptr;
		EventData->bIsExitFiredAtCurrentFrame = true;
#if LOG_ENTER_EXIT
		UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), EventData->EnterWidgetStack.Num());
#endif
		//enter new
		EventData->EnterWidget = NewObj;
		auto EnterObj = NewObj;
		if (CommonRoot != EnterObj)
		{
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("-----begin enter 111"));
#endif
			int InsertIndex = EventData->EnterWidgetStack.Num();
			if (EventSystem == nullptr)
			{
				ULexEventSystem::ExecuteEvent_OnPointerEnter(NewObj, EventData);
			}
			else
			{
				EventSystem->CallOnPointerEnter(NewObj, EventData);
			}
			EventData->HighlightWidgetForNavigation = NewObj;
			EventData->EnterWidgetStack.Add(NewObj);
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("	:%s"), *(EnterObj->GetDisplayName()));
#endif
			EnterObj = EnterObj->GetParent();
			while (EnterObj != nullptr)
			{
				if (CommonRoot == EnterObj)
				{
					break;
				}
				if (EventSystem == nullptr)
				{
					ULexEventSystem::ExecuteEvent_OnPointerEnter(EnterObj, EventData);
				}
				else
				{
					EventSystem->CallOnPointerEnter(EnterObj, EventData);
				}
				EventData->EnterWidgetStack.Insert(EnterObj, InsertIndex);
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("	:%s"), *(EnterObj->GetDisplayName()));
#endif
				EnterObj = EnterObj->GetParent();
			}
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), EventData->EnterWidgetStack.Num());
#endif
		}
	}
	else
	{
		if (IsValid(OldObj) || EventData->EnterWidgetStack.Num() > 0)
		{
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("-----begin exit 222"));
#endif
			//exit old
			for (int i = EventData->EnterWidgetStack.Num() - 1; i >= 0; i--)
			{
				if (IsValid(EventData->EnterWidgetStack[i]))
				{
					if (!EventData->bIsExitFiredAtCurrentFrame)
					{
						if (EventSystem == nullptr)
						{
							ULexEventSystem::ExecuteEvent_OnPointerExit(EventData->EnterWidgetStack[i], EventData);
						}
						else
						{
							EventSystem->CallOnPointerExit(EventData->EnterWidgetStack[i], EventData);
						}
					}
#if LOG_ENTER_EXIT
					UE_LOG(LGUI, Error, TEXT("	%s"), *(EventData->EnterWidgetStack[i]->GetDisplayName()));
#endif
				}
				EventData->EnterWidgetStack.RemoveAt(i);
			}
			EventData->EnterWidget = nullptr;
			EventData->bIsExitFiredAtCurrentFrame = true;
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), EventData->EnterWidgetStack.Num());
#endif
			EventData->EnterWidgetStack.Reset();
		}
		if (IsValid(NewObj))
		{
			//enter new
			if (!EventData->EnterWidgetStack.Contains(NewObj))
			{
				auto EnterObj = NewObj;
				int InsertIndex = EventData->EnterWidgetStack.Num();
				EventData->EnterWidget = NewObj;
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("-----begin enter 333"));
				UE_LOG(LGUI, Error, TEXT("	%s"), *(EnterObj->GetDisplayName()));
#endif
				if (EventSystem == nullptr)
				{
					ULexEventSystem::ExecuteEvent_OnPointerEnter(NewObj, EventData);
				}
				else
				{
					EventSystem->CallOnPointerEnter(NewObj, EventData);
				}
				EventData->HighlightWidgetForNavigation = NewObj;
				EventData->EnterWidgetStack.Add(NewObj);
				EnterObj = EnterObj->GetParent();
				while (EnterObj != nullptr)
				{
#if LOG_ENTER_EXIT
					UE_LOG(LGUI, Error, TEXT("	:%s"), *(EnterObj->GetDisplayName()));
#endif
					if (EventSystem == nullptr)
					{
						ULexEventSystem::ExecuteEvent_OnPointerEnter(EnterObj, EventData);
					}
					else
					{
						EventSystem->CallOnPointerEnter(EnterObj, EventData);
					}
					EventData->EnterWidgetStack.Insert(EnterObj, InsertIndex);
					EnterObj = EnterObj->GetParent();
				}
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), EventData->EnterWidgetStack.Num());
#endif
			}
		}
	}
}
ULexWidget* ULexPointerInputModule::FindCommonRoot(ULexWidget* A, ULexWidget* B)
{
	if (A == nullptr || B == nullptr)return nullptr;

	while (A != nullptr)
	{
		ULexWidget* TempB = B;
		while (TempB != nullptr)
		{
			if (A == TempB)
				return A;
			TempB = TempB->GetParent();
		}
		A = A->GetParent();
	}
	return nullptr;
}
void ULexPointerInputModule::ProcessPointerEvent(ULexEventSystem* EventSystem, ULexPointerEventData* EventData, bool bLineTraceHitSomething, const FLexUIHitResultContainer& LexHitResult, bool& OutIsHitSomething, FLexUIHitResult& OutHitResult)
{
	EventData->bIsUpFiredAtCurrentFrame = false;
	EventData->bIsExitFiredAtCurrentFrame = false;
	EventData->bIsEndDragFiredAtCurrentFrame = false;

	EventData->FaceIndex = LexHitResult.HitResult.FaceIndex;
	EventData->Raycaster = LexHitResult.Raycaster;
	OutHitResult = LexHitResult.HitResult;
	OutIsHitSomething = bLineTraceHitSomething;

	if (bLineTraceHitSomething)
	{
		auto nowHitComponent = OutHitResult.Widget.Get();
		//fire event
		EventData->WorldPoint = OutHitResult.Location;
		EventData->WorldNormal = OutHitResult.Normal;
		if (EventData->EnterWidget != nowHitComponent)//hit different object
		{
			ProcessPointerEnterExit(EventSystem, EventData, EventData->EnterWidget, nowHitComponent);
		}
	}
	else
	{
		if (IsValid(EventData->EnterWidget) || EventData->EnterWidgetStack.Num() > 0)//prev object
		{
			ProcessPointerEnterExit(EventSystem, EventData, EventData->EnterWidget, nullptr);
		}
	}

	if (EventData->bNowIsTriggerPressed && EventData->bPrevIsTriggerPressed)//if trigger keep pressing
	{
		if (EventData->bIsDragging)//if dragging
		{
			//trigger drag event
			if (IsValid(EventData->DragWidget))
			{
				if (EventSystem == nullptr)
				{
					ULexEventSystem::ExecuteEvent_OnPointerDrag(EventData->DragWidget, EventData, true);
				}
				else
				{
					EventSystem->CallOnPointerDrag(EventData->DragWidget, EventData);
				}

				OutHitResult.Distance = EventData->PressDistance;
				OutIsHitSomething = true;//always hit a plane when drag
			}
			else
			{
				EventData->bIsDragging = false;
			}
		}
		else//trigger press but not dragging, only concern if trigger drag event
		{
			if (IsValid(EventData->EnterWidget))//if hit something when press
			{
				if (IsValid(EventData->PressRaycaster))
				{
					if (EventData->PressRaycaster->ShouldStartDrag(EventData))
					{
						if (auto CurrentDragWidget = ULexEventSystem::GetEventHandler(EventData->EnterWidget, ULexPointerDragInterface::StaticClass()))
						{
							EventData->bIsDragging = true;
							EventData->DragWidget = CurrentDragWidget;
							if (EventSystem == nullptr)
							{
								ULexEventSystem::ExecuteEvent_OnPointerBeginDrag(EventData->DragWidget, EventData, true);
							}
							else
							{
								EventSystem->CallOnPointerBeginDrag(EventData->DragWidget, EventData);
							}
						}
					}
				}
				OutHitResult.Distance = EventData->PressDistance;
				OutIsHitSomething = true;
			}
		}
	}
	else if (!EventData->bNowIsTriggerPressed && !EventData->bPrevIsTriggerPressed)//is trigger keep release, only concern Enter/Exit event
	{
		
	}
	else//trigger state change
	{
		if (EventData->bNowIsTriggerPressed)//now is press, prev is release
		{
			if (bLineTraceHitSomething)
			{
				if (IsValid(EventData->EnterWidget))//now object
				{
					EventData->WorldPoint = OutHitResult.Location;
					EventData->WorldNormal = OutHitResult.Normal;
					EventData->PressDistance = OutHitResult.Distance;
					EventData->PressRayOrigin = LexHitResult.RayOrigin;
					EventData->PressRayDirection = LexHitResult.RayDirection;
					EventData->PressWorldPoint = OutHitResult.Location;
					EventData->PressWorldNormal = OutHitResult.Normal;
					EventData->PressRaycaster = LexHitResult.Raycaster;
					EventData->PressWorldToLocalTransform = EventData->EnterWidget->GetWorldTransform().Inverse();
					EventData->PressWidget = EventData->EnterWidget;
					DeselectIfSelectionChanged(EventSystem, EventData->PressWidget, EventData);
					if (EventSystem == nullptr)
					{
						ULexEventSystem::ExecuteEvent_OnPointerDown(EventData->PressWidget, EventData, true);
					}
					else
					{
						EventSystem->CallOnPointerDown(EventData->PressWidget, EventData);
					}
				}
			}
		}
		else//now is release, prev is press
		{
			if (EventData->bIsDragging)//is dragging
			{
				EventData->bIsDragging = false;
				if (IsValid(EventData->PressWidget))
				{
					if (!EventData->bIsUpFiredAtCurrentFrame)
					{
						EventData->bIsUpFiredAtCurrentFrame = true;
						if (EventSystem == nullptr)
						{
							ULexEventSystem::ExecuteEvent_OnPointerUp(EventData->PressWidget, EventData, true);
						}
						else
						{
							EventSystem->CallOnPointerUp(EventData->PressWidget, EventData);
						}
					}
					EventData->PressWidget = nullptr;
				}
				if (bLineTraceHitSomething)//hit something when stop drag
				{
					//if enter an object when drag, and after one frame trigger release and hit new object, then old object need to call DragExit
					if (IsValid(EventData->EnterWidget) && EventData->EnterWidget != EventData->DragWidget)
					{
						if (EventSystem == nullptr)
						{
							ULexEventSystem::ExecuteEvent_OnPointerDrop(EventData->EnterWidget, EventData, true);
						}
						else
						{
							EventSystem->CallOnPointerDrop(EventData->EnterWidget, EventData);
						}
					}
				}
				//drag end
				if (IsValid(EventData->DragWidget))
				{
					if (!EventData->bIsEndDragFiredAtCurrentFrame)
					{
						EventData->bIsEndDragFiredAtCurrentFrame = true;
						if (EventSystem == nullptr)
						{
							ULexEventSystem::ExecuteEvent_OnPointerEndDrag(EventData->DragWidget, EventData, true);
						}
						else
						{
							EventSystem->CallOnPointerEndDrag(EventData->DragWidget, EventData);
						}
					}
					EventData->DragWidget = nullptr;
				}
			}
			else//not dragging
			{
				if (IsValid(EventData->PressWidget))
				{
					if (!EventData->bIsUpFiredAtCurrentFrame)
					{
						EventData->bIsUpFiredAtCurrentFrame = true;
						if (EventSystem == nullptr)
						{
							ULexEventSystem::ExecuteEvent_OnPointerUp(EventData->PressWidget, EventData, true);
						}
						else
						{
							EventSystem->CallOnPointerUp(EventData->PressWidget, EventData);
						}
					}
					EventData->ClickTime = EventData->GetWorld()->GetTimeSeconds();
					if (EventSystem == nullptr)
					{
						ULexEventSystem::ExecuteEvent_OnPointerClick(EventData->PressWidget, EventData, true);
					}
					else
					{
						EventSystem->CallOnPointerClick(EventData->PressWidget, EventData);
					}
					EventData->PressWidget = nullptr;
				}
			}
		}
	}

	EventData->bPrevIsTriggerPressed = EventData->bNowIsTriggerPressed;
}
bool ULexPointerInputModule::Navigate(ELexUINavigationDirection InDirection, ULexPointerEventData* InPointerEventData, FLexUIHitResultContainer& OutLexUIHitResult)
{
	auto CurrentHover = InPointerEventData->HighlightWidgetForNavigation.Get();
	ULexUIBehaviour* CurrentNavigateObject = nullptr;
	if (IsValid(CurrentHover))
	{
		auto SearchWidget = CurrentHover;
		auto FindNavigationInterface = [](ULexWidget* InWidget) {
			auto& Components = InWidget->GetAllComponents();
			for (auto& Comp : Components)
			{
				if (IsValid(Comp) && Comp->GetClass()->ImplementsInterface(ULexNavigationInterface::StaticClass()))
				{
					if (ILexNavigationInterface::Execute_CanNavigateHere(Comp))
					{
						return Comp;
					}
				}
			}
			return (ULexUIBehaviour*)nullptr;
		};
		while (IsValid(SearchWidget))
		{
			CurrentNavigateObject = FindNavigationInterface(SearchWidget);
			if (CurrentNavigateObject != nullptr)
			{
				break;
			}
			SearchWidget = SearchWidget->GetParent();
		}
	}
	
	if (CurrentNavigateObject == nullptr)//not find valid selectable object, use default one
	{
		CurrentNavigateObject = UUISelectable::FindDefaultSelectable(this);//@todo: don't reference UISelectableComponent directly
	}
	else//find valid selectable, do navigation
	{
		TScriptInterface<ILexNavigationInterface> NextNavigateInterface = nullptr;
		if (ILexNavigationInterface::Execute_OnNavigate(CurrentNavigateObject, InDirection, NextNavigateInterface))
		{
			if (auto NextNavigateObject = Cast<ULexUIBehaviour>(NextNavigateInterface.GetObject()))
			{
				CurrentNavigateObject = NextNavigateObject;
			}
		}
	}
	if (CurrentNavigateObject != nullptr)
	{
		OutLexUIHitResult.HitResult.Widget = CurrentNavigateObject->GetWidget();//this convert is incorrect, but I need this pointer
		OutLexUIHitResult.HitResult.Location = OutLexUIHitResult.HitResult.Widget->GetWorldLocation();
		OutLexUIHitResult.HitResult.Normal = OutLexUIHitResult.HitResult.Widget->GetWorldTransform().TransformVector(FVector(0, 0, 1));
		OutLexUIHitResult.HitResult.Normal.Normalize();
		OutLexUIHitResult.Raycaster = nullptr;
		OutLexUIHitResult.HoverArray.Reset();

		InPointerEventData->HighlightWidgetForNavigation = CurrentNavigateObject->GetWidget();
		return true;
	}
	return false;
}

void ULexPointerInputModule::ProcessInputForNavigation()
{
	for (auto& keyValue : EventSystem->GetPointerEventDataMap())
	{
		ProcessInputForNavigation(keyValue.Value);
	}
}
void ULexPointerInputModule::ProcessInputForNavigation(ULexPointerEventData* EventData)
{
	auto TimeSeconds = this->GetWorld()->GetTimeSeconds();
	while (TimeSeconds > EventData->NavigateTickTime)
	{
		bool bIsFirstPressInSequence = EventData->NavigateTickTime == 0.0f;
		auto TimeInterval = bIsFirstPressInSequence ? EventSystem->GetNavigateInputIntervalForFirstTime() : EventSystem->GetNavigateInputInterval();
		if (bIsFirstPressInSequence)
		{
			EventData->NavigateTickTime = TimeSeconds + TimeInterval;
		}
		else
		{
			EventData->NavigateTickTime += TimeInterval;
		}
		FLexUIHitResultContainer LexUIHitResult;
		bool bSelectValid = Navigate(EventData->NavigateDirection, EventData, LexUIHitResult);
		bool bResultHitSomething = false;
		FLexUIHitResult HitResult;
		ProcessPointerEvent(EventSystem.Get(), EventData, bSelectValid, LexUIHitResult, bResultHitSomething, HitResult);
		if (bResultHitSomething)
		{
			EventSystem->SetSelectWidget(HitResult.Widget.Get(), EventData);
		}

		auto TempHitComp = HitResult.Widget.Get();
		EventSystem->RaiseHitEvent(bResultHitSomething, HitResult, TempHitComp);
	}
}
void ULexPointerInputModule::ClearEventByID(int pointerID)
{
	auto EventData = EventSystem->GetPointerEventData(pointerID, false);
	if (EventData == nullptr)return;
	if (!EventSystem.IsValid())return;

	if (EventData->bPrevIsTriggerPressed)//if trigger is pressed
	{
		if (EventData->bIsDragging)
		{
			EventData->bIsDragging = false;
			if (!EventData->bIsEndDragFiredAtCurrentFrame)
			{
				EventData->bIsEndDragFiredAtCurrentFrame = true;
				if (IsValid(EventData->DragWidget))
				{
					EventSystem->CallOnPointerEndDrag(EventData->DragWidget, EventData);
					EventData->DragWidget = nullptr;
				}
			}
		}

		if (!EventData->bIsUpFiredAtCurrentFrame)
		{
			EventData->bIsUpFiredAtCurrentFrame = true;
			if (IsValid(EventData->PressWidget))
			{
				auto OldPressComponent = EventData->PressWidget;
				EventData->PressWidget = nullptr;
				EventSystem->CallOnPointerUp(OldPressComponent, EventData);
			}
		}
		if (!EventData->bIsExitFiredAtCurrentFrame)
		{
			if (IsValid(EventData->EnterWidget) || EventData->EnterWidgetStack.Num() > 0)
			{
				ProcessPointerEnterExit(EventSystem.Get(), EventData, EventData->EnterWidget, nullptr);
			}
			EventData->bIsExitFiredAtCurrentFrame = true;
		}

		EventData->bPrevIsTriggerPressed = false;
	}
	else
	{
		if (!EventData->bIsExitFiredAtCurrentFrame)
		{
			if (IsValid(EventData->EnterWidget) || EventData->EnterWidgetStack.Num() > 0)
			{
				ProcessPointerEnterExit(EventSystem.Get(), EventData, EventData->EnterWidget, nullptr);
			}
			EventData->bIsExitFiredAtCurrentFrame = true;
		}
	}
}

bool ULexPointerInputModule::CanHandleInterface(ULexWidget* targetComp, UClass* targetInterfaceClass)
{
	bool canSelectPressedComponent = false;
	auto components = targetComp->GetAllComponents();
	for (auto item : components)
	{
		if (item->GetClass()->ImplementsInterface(targetInterfaceClass))
		{
			canSelectPressedComponent = true;
			break;
		}
	}
	return canSelectPressedComponent;
}

ULexWidget* ULexPointerInputModule::GetEventHandle(ULexWidget* targetComp, UClass* targetInterfaceClass)
{
	if (!IsValid(targetComp))
	{
		return nullptr;
	}

	ULexWidget* rootComp = targetComp;
	while (rootComp != nullptr)
	{
		if (CanHandleInterface(rootComp, targetInterfaceClass))
		{
			return rootComp;
		}
		rootComp = rootComp->GetParent();
	}
	return nullptr;
}
void ULexPointerInputModule::DeselectIfSelectionChanged(ULexEventSystem* eventSystem, ULexWidget* currentPressed, ULexBaseEventData* EventData)
{
	auto SelectHandleComp = GetEventHandle(currentPressed, ULexSelectDeselectInterface::StaticClass());
	if (SelectHandleComp != EventData->SelectedComponent)
	{
		ULexEventSystem::SetSelectWidget(eventSystem, nullptr, EventData);
	}
}

void ULexPointerInputModule::ClearEvent()
{
	for (auto& keyValue : EventSystem->GetPointerEventDataMap())
	{
		ClearEventByID(keyValue.Key);
	}
}


