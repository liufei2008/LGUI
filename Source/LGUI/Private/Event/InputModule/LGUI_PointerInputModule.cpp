// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "LGUI.h"
#include "Event/LGUIPointerEventData.h"
#include "Core/LGUIManager.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIBaseRaycaster.h"
#include "Event/Interface/LGUINavigationInterface.h"
#include "Interaction/UISelectableComponent.h"
#include "Utils/LGUIUtils.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

bool ULGUI_PointerInputModule::CheckEventSystem()
{
	if (IsValid(eventSystem))
	{
		return true;
	}
	else
	{
		eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance(this);
		return IsValid(eventSystem);
	}
}

bool ULGUI_PointerInputModule::LineTrace(ULGUIPointerEventData* InPointerEventData, FLGUIHitResult& hitResult)
{
	multiHitResult.Reset();
	auto World = this->GetWorld();
	if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(World))
	{
		auto bIsGamePaused = World->IsPaused();
		auto& AllRaycasterArray = LGUIManager->GetAllRaycasterArray();
		InPointerEventData->hoverComponentArray.Reset();

		FVector rayOrigin(0, 0, 0), rayDir(1, 0, 0), rayEnd(1, 0, 0);
		int32 prevRaycasterDepth = 0;
		for (int i = 0; i < AllRaycasterArray.Num(); i++)
		{
			auto& RaycasterItem = AllRaycasterArray[i];
			FHitResult hitResultItem;
			if (RaycasterItem.IsValid()
				&& (RaycasterItem->GetPointerID() == InPointerEventData->pointerID || RaycasterItem->GetPointerID() == INDEX_NONE)
				&& (!bIsGamePaused || (bIsGamePaused && !RaycasterItem->GetAffectByGamePause()))
				)
			{
				if (RaycasterItem->GetDepth() < prevRaycasterDepth && multiHitResult.Num() != 0)//if this raycaster's depth not equal than prev raycaster's depth, and prev hit test is true, then we dont need to raycast more, because of raycaster's depth
				{
					break;
				}
				TArray<USceneComponent*> hoverArray;//temp array for store hover components
				if (RaycasterItem->Raycast(InPointerEventData, rayOrigin, rayDir, rayEnd, hitResultItem, hoverArray))
				{
					FLGUIHitResult LGUIHitResult;
					LGUIHitResult.hitResult = hitResultItem;
					LGUIHitResult.eventFireType = RaycasterItem->GetEventFireType();
					LGUIHitResult.raycaster = RaycasterItem.Get();
					LGUIHitResult.rayOrigin = rayOrigin;
					LGUIHitResult.rayDirection = rayDir;
					LGUIHitResult.rayEnd = rayEnd;
					LGUIHitResult.hoverArray = hoverArray;

					multiHitResult.Add(LGUIHitResult);
					prevRaycasterDepth = RaycasterItem->GetDepth();
				}
			}
		}
		if (multiHitResult.Num() == 0)
		{
			return false;
		}
		else if (multiHitResult.Num() > 1)
		{
			//sort only on distance (not depth), because multiHitResult only store hit result of same depth
			multiHitResult.Sort([](const FLGUIHitResult& A, const FLGUIHitResult& B)
				{
					return A.hitResult.Distance < B.hitResult.Distance;
				});
			for (auto& hitResultItem : multiHitResult)
			{
				for (auto& hoverItem : hitResultItem.hoverArray)
				{
					InPointerEventData->hoverComponentArray.Add(hoverItem);
				}
			}
		}
		else
		{
			for (auto hoverItem : multiHitResult[0].hoverArray)
			{
				InPointerEventData->hoverComponentArray.Add(hoverItem);
			}
		}
		hitResult = multiHitResult[0];
		return true;
	}
	return false;
}

//@todo: these logs is just for editor testing, remove them when ready
#define LOG_ENTER_EXIT 0
void ULGUI_PointerInputModule::ProcessPointerEnterExit(ULGUIEventSystem* eventSystem, ULGUIPointerEventData* eventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType)
{
	if (oldObj == newObj)return;
	if (IsValid(oldObj) && IsValid(newObj))
	{
		auto commonRoot = FindCommonRoot(oldObj->GetOwner(), newObj->GetOwner());
#if LOG_ENTER_EXIT
		UE_LOG(LGUI, Error, TEXT("-----begin exit 000, commonRoot:%s"), commonRoot != nullptr ? *(commonRoot->GetActorLabel()) : TEXT("null"));
#endif
		//exit old
		for (int i = eventData->enterComponentStack.Num() - 1; i >= 0; i--)
		{
			if (commonRoot == eventData->enterComponentStack[i]->GetOwner())
			{
				break;
			}
			if (!eventData->isExitFiredAtCurrentFrame)
			{
				if (eventSystem == nullptr)
				{
					ULGUIEventSystem::ExecuteEvent_OnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType, false);
				}
				else
				{
					eventSystem->CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
				}
			}
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("	%s"), *(eventData->enterComponentStack[i]->GetOwner()->GetActorLabel()));
#endif
			eventData->enterComponentStack.RemoveAt(i);
		}
		eventData->enterComponent = nullptr;
		eventData->isExitFiredAtCurrentFrame = true;
#if LOG_ENTER_EXIT
		UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
		//enter new
		eventData->enterComponent = newObj;
		eventData->enterComponentEventFireType = enterFireType;
		AActor* enterObjectActor = newObj->GetOwner();
		if (commonRoot != enterObjectActor)
		{
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("-----begin enter 111"));
#endif
			int insertIndex = eventData->enterComponentStack.Num();
			if (eventSystem == nullptr)
			{
				ULGUIEventSystem::ExecuteEvent_OnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType, false);
			}
			else
			{
				eventSystem->CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
			}
			eventData->highlightComponentForNavigation = newObj;
			eventData->enterComponentStack.Add(newObj);
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
			enterObjectActor = enterObjectActor->GetAttachParentActor();
			while (enterObjectActor != nullptr)
			{
				if (commonRoot == enterObjectActor)
				{
					break;
				}
				if (eventSystem == nullptr)
				{
					ULGUIEventSystem::ExecuteEvent_OnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType, false);
				}
				else
				{
					eventSystem->CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
				}
				eventData->enterComponentStack.Insert(enterObjectActor->GetRootComponent(), insertIndex);
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
				enterObjectActor = enterObjectActor->GetAttachParentActor();
			}
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
		}
	}
	else
	{
		if (IsValid(oldObj) || eventData->enterComponentStack.Num() > 0)
		{
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("-----begin exit 222"));
#endif
			//exit old
			for (int i = eventData->enterComponentStack.Num() - 1; i >= 0; i--)
			{
				if (IsValid(eventData->enterComponentStack[i]))
				{
					if (!eventData->isExitFiredAtCurrentFrame)
					{
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType, false);
						}
						else
						{
							eventSystem->CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
						}
					}
#if LOG_ENTER_EXIT
					UE_LOG(LGUI, Error, TEXT("	%s, fireType:%d"), *(eventData->enterComponentStack[i]->GetOwner()->GetActorLabel()), (int)(eventData->enterComponentEventFireType));
#endif
				}
				eventData->enterComponentStack.RemoveAt(i);
			}
			eventData->enterComponent = nullptr;
			eventData->isExitFiredAtCurrentFrame = true;
#if LOG_ENTER_EXIT
			UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
			eventData->enterComponentStack.Reset();
		}
		if (IsValid(newObj))
		{
			//enter new
			if (!eventData->enterComponentStack.Contains(newObj))
			{
				AActor* enterObjectActor = newObj->GetOwner();
				int insertIndex = eventData->enterComponentStack.Num();
				eventData->enterComponent = newObj;
				eventData->enterComponentEventFireType = enterFireType;
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("-----begin enter 333"));
				UE_LOG(LGUI, Error, TEXT("	%s"), *(enterObjectActor->GetActorLabel()));
#endif
				if (eventSystem == nullptr)
				{
					ULGUIEventSystem::ExecuteEvent_OnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType, false);
				}
				else
				{
					eventSystem->CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
				}
				eventData->highlightComponentForNavigation = newObj;
				eventData->enterComponentStack.Add(newObj);
				enterObjectActor = enterObjectActor->GetAttachParentActor();
				while (enterObjectActor != nullptr)
				{
#if LOG_ENTER_EXIT
					UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
					if (eventSystem == nullptr)
					{
						ULGUIEventSystem::ExecuteEvent_OnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType, false);
					}
					else
					{
						eventSystem->CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
					}
					eventData->enterComponentStack.Insert(enterObjectActor->GetRootComponent(), insertIndex);
					enterObjectActor = enterObjectActor->GetAttachParentActor();
				}
#if LOG_ENTER_EXIT
				UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
			}
		}
	}
}
AActor* ULGUI_PointerInputModule::FindCommonRoot(AActor* actorA, AActor* actorB)
{
	if (actorA == nullptr || actorB == nullptr)return nullptr;

	while (actorA != nullptr)
	{
		AActor* tempActorB = actorB;
		while (tempActorB != nullptr)
		{
			if (actorA == tempActorB)
				return actorA;
			tempActorB = tempActorB->GetAttachParentActor();
		}
		actorA = actorA->GetAttachParentActor();
	}
	return nullptr;
}
void ULGUI_PointerInputModule::ProcessPointerEvent(ULGUIEventSystem* eventSystem, ULGUIPointerEventData* eventData, bool lineTraceHitSomething, const FLGUIHitResult& LGUIHitResult, bool& outIsHitSomething, FHitResult& outHitResult)
{
	eventData->isUpFiredAtCurrentFrame = false;
	eventData->isExitFiredAtCurrentFrame = false;
	eventData->isEndDragFiredAtCurrentFrame = false;

	eventData->faceIndex = LGUIHitResult.hitResult.FaceIndex;
	eventData->raycaster = LGUIHitResult.raycaster;
	outHitResult = LGUIHitResult.hitResult;
	outIsHitSomething = lineTraceHitSomething;

	if (lineTraceHitSomething)
	{
		auto nowHitComponent = (USceneComponent*)outHitResult.Component.Get();
		//fire event
		eventData->worldPoint = outHitResult.Location;
		eventData->worldNormal = outHitResult.Normal;
		if (eventData->enterComponent != nowHitComponent)//hit differenct object
		{
			ProcessPointerEnterExit(eventSystem, eventData, eventData->enterComponent, nowHitComponent, LGUIHitResult.eventFireType);
		}
	}
	else
	{
		if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)//prev object
		{
			ProcessPointerEnterExit(eventSystem, eventData, eventData->enterComponent, nullptr, LGUIHitResult.eventFireType);
		}
	}

	if (eventData->nowIsTriggerPressed && eventData->prevIsTriggerPressed)//if trigger keep pressing
	{
		if (eventData->isDragging)//if is dragging
		{
			//trigger drag event
			if (IsValid(eventData->dragComponent))
			{
				if (eventSystem == nullptr)
				{
					ULGUIEventSystem::ExecuteEvent_OnPointerDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType, true);
				}
				else
				{
					eventSystem->CallOnPointerDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
				}

				outHitResult.Distance = eventData->pressDistance;
				outIsHitSomething = true;//always hit a plane when drag
			}
			else
			{
				eventData->isDragging = false;
			}
		}
		else//trigger press but not dragging, only consern if trigger drag event
		{
			if (IsValid(eventData->pressComponent))//if hit something when press
			{
				if (IsValid(eventData->pressRaycaster))
				{
					if (eventData->pressRaycaster->ShouldStartDrag(eventData))
					{
						eventData->isDragging = true;
						eventData->dragComponent = eventData->pressComponent;
						eventData->dragComponentEventFireType = eventData->pressComponentEventFireType;
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerBeginDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType, true);
						}
						else
						{
							eventSystem->CallOnPointerBeginDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
						}
					}
				}
				outHitResult.Distance = eventData->pressDistance;
				outIsHitSomething = true;
			}
		}
	}
	else if (!eventData->nowIsTriggerPressed && !eventData->prevIsTriggerPressed)//is trigger keep release, only concern Enter/Exit event
	{
		
	}
	else//trigger state change
	{
		if (eventData->nowIsTriggerPressed)//now is press, prev is release
		{
			if (lineTraceHitSomething)
			{
				if (IsValid(eventData->enterComponent))//now object
				{
					eventData->worldPoint = outHitResult.Location;
					eventData->worldNormal = outHitResult.Normal;
					eventData->pressDistance = outHitResult.Distance;
					eventData->pressRayOrigin = LGUIHitResult.rayOrigin;
					eventData->pressRayDirection = LGUIHitResult.rayDirection;
					eventData->pressWorldPoint = outHitResult.Location;
					eventData->pressWorldNormal = outHitResult.Normal;
					eventData->pressRaycaster = LGUIHitResult.raycaster;
					eventData->pressWorldToLocalTransform = eventData->enterComponent->GetComponentTransform().Inverse();
					eventData->pressComponent = eventData->enterComponent;
					eventData->pressComponentEventFireType = eventData->enterComponentEventFireType;
					DeselectIfSelectionChanged(eventSystem, eventData->pressComponent, eventData);
					if (eventSystem == nullptr)
					{
						ULGUIEventSystem::ExecuteEvent_OnPointerDown(eventData->pressComponent, eventData, eventData->pressComponentEventFireType, true);
					}
					else
					{
						eventSystem->CallOnPointerDown(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
					}
				}
			}
		}
		else//now is release, prev is press
		{
			if (eventData->isDragging)//is dragging
			{
				eventData->isDragging = false;
				if (IsValid(eventData->pressComponent))
				{
					if (!eventData->isUpFiredAtCurrentFrame)
					{
						eventData->isUpFiredAtCurrentFrame = true;
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType, true);
						}
						else
						{
							eventSystem->CallOnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
						}
					}
					eventData->pressComponent = nullptr;
				}
				if (lineTraceHitSomething)//hit something when stop drag
				{
					//if enter an object when drag, and after one frame trigger release and hit new object, then old object need to call DragExit
					if (IsValid(eventData->enterComponent) && eventData->enterComponent != eventData->dragComponent)
					{
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerDragDrop(eventData->enterComponent, eventData, eventData->enterComponentEventFireType, true);
						}
						else
						{
							eventSystem->CallOnPointerDragDrop(eventData->enterComponent, eventData, eventData->enterComponentEventFireType);
						}
					}
				}
				//drag end
				if (IsValid(eventData->dragComponent))
				{
					if (!eventData->isEndDragFiredAtCurrentFrame)
					{
						eventData->isEndDragFiredAtCurrentFrame = true;
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType, true);
						}
						else
						{
							eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
						}
					}
					eventData->dragComponent = nullptr;
				}
			}
			else//not dragging
			{
				if (IsValid(eventData->pressComponent))
				{
					if (!eventData->isUpFiredAtCurrentFrame)
					{
						eventData->isUpFiredAtCurrentFrame = true;
						if (eventSystem == nullptr)
						{
							ULGUIEventSystem::ExecuteEvent_OnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType, true);
						}
						else
						{
							eventSystem->CallOnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
						}
					}
					eventData->clickTime = eventData->GetWorld()->GetTimeSeconds();
					if (eventSystem == nullptr)
					{
						ULGUIEventSystem::ExecuteEvent_OnPointerClick(eventData->pressComponent, eventData, eventData->pressComponentEventFireType, true);
					}
					else
					{
						eventSystem->CallOnPointerClick(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
					}
					eventData->pressComponent = nullptr;
				}
			}
		}
	}

	eventData->prevIsTriggerPressed = eventData->nowIsTriggerPressed;
}
bool ULGUI_PointerInputModule::Navigate(ELGUINavigationDirection direction, ULGUIPointerEventData* InPointerEventData, FLGUIHitResult& LGUIHitResult)
{
	if (!CheckEventSystem())return false;

	auto currentHover = InPointerEventData->highlightComponentForNavigation.Get();
	UActorComponent* currentNavigateObject = nullptr;
	if (IsValid(currentHover))
	{
		AActor* searchActor = currentHover->GetOwner();
		auto FindNavigationInterface = [](AActor* InActor) {
			auto& Components = InActor->GetComponents();
			for (auto& Comp : Components)
			{
				if (IsValid(Comp) && Comp->GetClass()->ImplementsInterface(ULGUINavigationInterface::StaticClass()))
				{
					return Comp;
				}
			}
			return (UActorComponent*)nullptr;
		};
		while (IsValid(searchActor))
		{
			currentNavigateObject = FindNavigationInterface(searchActor);
			if (currentNavigateObject != nullptr)
			{
				break;
			}
			searchActor = searchActor->GetAttachParentActor();
		}
	}
	
	if (currentNavigateObject == nullptr)//not find valid selectable object, use default one
	{
		currentNavigateObject = UUISelectableComponent::FindDefaultSelectable(this);//@todo: don't reference UISelectableComponent directly
	}
	else//find valid selectable, do navigation
	{
		TScriptInterface<ILGUINavigationInterface> nextNavigateInterface = nullptr;
		if (ILGUINavigationInterface::Execute_OnNavigate(currentNavigateObject, direction, nextNavigateInterface))
		{
			auto nextNavigateObject = Cast<UActorComponent>(nextNavigateInterface.GetObject());
			if (nextNavigateObject)
			{
				currentNavigateObject = nextNavigateObject;
			}
		}
	}
	if (currentNavigateObject != nullptr)
	{
		LGUIHitResult.hitResult.Component = (UPrimitiveComponent*)currentNavigateObject->GetOwner()->GetRootComponent();//this convert is incorrect, but I need this pointer
		LGUIHitResult.hitResult.Location = LGUIHitResult.hitResult.Component->GetComponentLocation();
		LGUIHitResult.hitResult.Normal = LGUIHitResult.hitResult.Component->GetComponentTransform().TransformVector(FVector(0, 0, 1));
		LGUIHitResult.hitResult.Normal.Normalize();
		LGUIHitResult.eventFireType = eventSystem->eventFireTypeForNavigation;
		LGUIHitResult.raycaster = nullptr;
		LGUIHitResult.hoverArray.Reset();

		InPointerEventData->highlightComponentForNavigation = currentNavigateObject->GetOwner()->GetRootComponent();
		return true;
	}
	return false;
}

void ULGUI_PointerInputModule::ProcessInputForNavigation()
{
	for (auto& keyValue : eventSystem->pointerEventDataMap)
	{
		ProcessInputForNavigation(keyValue.Value);
	}
}
void ULGUI_PointerInputModule::ProcessInputForNavigation(ULGUIPointerEventData* eventData)
{
	auto timeSeconds = this->GetWorld()->GetTimeSeconds();
	while (timeSeconds > eventData->navigateTickTime)
	{
		bool isFirstPressInSequence = eventData->navigateTickTime == 0.0f;
		auto timeInterval = isFirstPressInSequence ? eventSystem->navigateInputIntervalForFirstTime : eventSystem->navigateInputInterval;
		if (isFirstPressInSequence)
		{
			eventData->navigateTickTime = timeSeconds + timeInterval;
		}
		else
		{
			eventData->navigateTickTime += timeInterval;
		}
		FLGUIHitResult LGUIHitResult;
		bool selectValid = Navigate(eventData->navigateDirection, eventData, LGUIHitResult);
		bool resultHitSomething = false;
		FHitResult hitResult;
		ProcessPointerEvent(eventSystem, eventData, selectValid, LGUIHitResult, resultHitSomething, hitResult);
		if (resultHitSomething)
		{
			eventSystem->SetSelectComponent((USceneComponent*)hitResult.Component.Get(), eventData, eventSystem->eventFireTypeForNavigation);
		}

		auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
		eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
	}
}
void ULGUI_PointerInputModule::InputNavigation(ELGUINavigationDirection direction, bool pressOrRelease, int pointerID)
{
	auto eventData = eventSystem->GetPointerEventData(pointerID, true);
	if (pressOrRelease)
	{
		eventSystem->SetPointerInputType(eventData, ELGUIPointerInputType::Navigation);
		eventData->navigateDirection = direction;
	}
	else
	{
		eventData->navigateDirection = ELGUINavigationDirection::None;
	}
	eventData->navigateTickTime = 0;
}
void ULGUI_PointerInputModule::InputTriggerForNavigation(bool inTriggerPress, int pointerID)
{
	auto eventData = eventSystem->GetPointerEventData(pointerID, true);
	if (inTriggerPress)
	{
		eventSystem->SetPointerInputType(eventData, ELGUIPointerInputType::Navigation);
	}
	eventData->navigateTickTime = 0;
	eventData->nowIsTriggerPressed = inTriggerPress;
}

void ULGUI_PointerInputModule::ClearEventByID(int pointerID)
{
	auto eventData = eventSystem->GetPointerEventData(pointerID, false);
	if (eventData == nullptr)return;
	if (!CheckEventSystem())return;

	if (eventData->prevIsTriggerPressed)//if trigger is pressed
	{
		if (eventData->isDragging)
		{
			eventData->isDragging = false;
			if (!eventData->isEndDragFiredAtCurrentFrame)
			{
				eventData->isEndDragFiredAtCurrentFrame = true;
				if (IsValid(eventData->dragComponent))
				{
					eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
					eventData->dragComponent = nullptr;
				}
			}
		}

		if (!eventData->isUpFiredAtCurrentFrame)
		{
			eventData->isUpFiredAtCurrentFrame = true;
			if (IsValid(eventData->pressComponent))
			{
				auto oldPressComponent = eventData->pressComponent;
				eventData->pressComponent = nullptr;
				eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
			}
		}
		if (!eventData->isExitFiredAtCurrentFrame)
		{
			if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
			{
				ProcessPointerEnterExit(eventSystem, eventData, eventData->enterComponent, nullptr, eventData->enterComponentEventFireType);
			}
			eventData->isExitFiredAtCurrentFrame = true;
		}

		eventData->prevIsTriggerPressed = false;
	}
	else
	{
		if (!eventData->isExitFiredAtCurrentFrame)
		{
			if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
			{
				ProcessPointerEnterExit(eventSystem, eventData, eventData->enterComponent, nullptr, eventData->enterComponentEventFireType);
			}
			eventData->isExitFiredAtCurrentFrame = true;
		}
	}
}

bool ULGUI_PointerInputModule::CanHandleInterface(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType)
{
	bool canSelectPressedComponent = false;
	switch (eventFireType)
	{
	case ELGUIEventFireType::OnlyTargetActor:
	{
		if (targetComp->GetOwner()->GetClass()->ImplementsInterface(targetInterfaceClass))
		{
			canSelectPressedComponent = true;
		}
	}
	break;
	case ELGUIEventFireType::OnlyTargetComponent:
	{
		if (targetComp->GetClass()->ImplementsInterface(targetInterfaceClass))
		{
			canSelectPressedComponent = true;
		}
	}
	break;
	case ELGUIEventFireType::TargetActorAndAllItsComponents:
	{
		if (targetComp->GetOwner()->GetClass()->ImplementsInterface(targetInterfaceClass))
		{
			canSelectPressedComponent = true;
		}
		if (!canSelectPressedComponent)
		{
			auto components = targetComp->GetOwner()->GetComponents();
			for (auto item : components)
			{
				if (item->GetClass()->ImplementsInterface(targetInterfaceClass))
				{
					canSelectPressedComponent = true;
					break;
				}
			}
		}
	}
	break;
	}
	return canSelectPressedComponent;
}

USceneComponent* ULGUI_PointerInputModule::GetEventHandle(USceneComponent* targetComp, UClass* targetInterfaceClass, ELGUIEventFireType eventFireType)
{
	if (!IsValid(targetComp))
	{
		return nullptr;
	}

	USceneComponent* rootComp = targetComp;
	while (rootComp != nullptr)
	{
		if (CanHandleInterface(rootComp, targetInterfaceClass, eventFireType))
		{
			return rootComp;
		}
		rootComp = rootComp->GetAttachParent();
	}
	return nullptr;
}
void ULGUI_PointerInputModule::DeselectIfSelectionChanged(ULGUIEventSystem* eventSystem, USceneComponent* currentPressed, ULGUIBaseEventData* eventData)
{
	auto selectHandleComp = GetEventHandle(currentPressed, ULGUIPointerSelectDeselectInterface::StaticClass(), eventData->selectedComponentEventFireType);
	if (selectHandleComp != eventData->selectedComponent)
	{
		ULGUIEventSystem::SetSelectComponent(eventSystem, nullptr, eventData, eventData->selectedComponentEventFireType);
	}
}

void ULGUI_PointerInputModule::ClearEvent()
{
	for (auto& keyValue : eventSystem->pointerEventDataMap)
	{
		ClearEventByID(keyValue.Key);
	}
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
