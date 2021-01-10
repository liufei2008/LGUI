// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "LGUI.h"
#include "Event/LGUIPointerEventData.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Event/LGUIEventSystem.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Event/Rayemitter/LGUIBaseRayemitter.h"

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
ULGUIPointerEventData* ULGUI_PointerInputModule::GetPointerEventData(int pointerID, bool createIfNotExist)
{
	if (auto foundPtr = pointerEventDataMap.Find(pointerID))
	{
		return *foundPtr;
	}
	auto newEventData = NewObject<ULGUIPointerEventData>(this);
	newEventData->pointerID = pointerID;
	pointerEventDataMap.Add(pointerID, newEventData);
	return newEventData;
}
bool ULGUI_PointerInputModule::LineTrace(ULGUIPointerEventData* InPointerEventData, FHitResultContainerStruct& hitResult)
{
	multiHitResult.Reset();
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		auto& raycasterArray = LGUIManagerActor->GetRaycasters();
		InPointerEventData->hoverComponentArray.Reset();

		int32 prevRaycasterDepth = 0;
		for (int i = 0; i < raycasterArray.Num(); i++)
		{
			ULGUIBaseRaycaster* raycasterItem = raycasterArray[i];
			FHitResult hitResultItem;
			if (IsValid(raycasterItem))
			{
				if (raycasterItem->depth < prevRaycasterDepth && multiHitResult.Num() != 0)//if this raycaster's depth not equal than prev raycaster's depth, and prev hit test is true, then we dont need to raycast more, because of raycaster's depth
				{
					break;
				}
				FVector rayOrigin(0, 0, 0), rayDir(1, 0, 0), rayEnd(1, 0, 0);
				TArray<USceneComponent*> hoverArray;//temp array for store hover components
				if (raycasterItem->Raycast(InPointerEventData, rayOrigin, rayDir, rayEnd, hitResultItem, hoverArray))
				{
					FHitResultContainerStruct container;
					container.hitResult = hitResultItem;
					container.eventFireType = raycasterItem->eventFireType;
					container.raycaster = raycasterItem;
					container.rayOrigin = rayOrigin;
					container.rayDirection = rayDir;
					container.rayEnd = rayEnd;
					container.hoverArray = hoverArray;

					multiHitResult.Add(container);
					prevRaycasterDepth = raycasterItem->depth;
				}
			}
		}
		if (multiHitResult.Num() == 0)//no hit result
		{
			return false;
		}
		else if (multiHitResult.Num() > 1)
		{
			//sort only on distance (not depth), because multiHitResult only store hit result of same depth
			multiHitResult.Sort([](const FHitResultContainerStruct& A, const FHitResultContainerStruct& B)
				{
					return A.hitResult.Distance < B.hitResult.Distance;
				});
			for (auto hitResultItem : multiHitResult)
			{
				for (auto hoverItem : hitResultItem.hoverArray)
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
void ULGUI_PointerInputModule::ProcessPointerEnterExit(ULGUIPointerEventData* eventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType)
{
	if (oldObj == newObj)return;
	if (IsValid(oldObj) && IsValid(newObj))
	{
		auto commonRoot = FindCommonRoot(oldObj->GetOwner(), newObj->GetOwner());
#if WITH_EDITOR
		//@todo: these logs is just for editor testing, remove them when ready
		//UE_LOG(LGUI, Error, TEXT("-----begin exit 000, commonRoot:%s"), commonRoot != nullptr ? *(commonRoot->GetActorLabel()) : TEXT("null"));
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
				eventSystem->CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
			}
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("	%s"), *(eventData->enterComponentStack[i]->GetOwner()->GetActorLabel()));
#endif
			eventData->enterComponentStack.RemoveAt(i);
		}
		eventData->enterComponent = nullptr;
		eventData->isEndDragFiredAtCurrentFrame = true;
#if WITH_EDITOR
		//UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
		//enter new
		AActor* enterObjectActor = newObj->GetOwner();
		eventData->enterComponent = newObj;
		if (commonRoot != enterObjectActor)
		{
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("-----begin enter 111"));
#endif
			int insertIndex = eventData->enterComponentStack.Num();
			eventData->enterComponentEventFireType = enterFireType;
			eventSystem->CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
			eventData->enterComponentStack.Add(newObj);
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
			enterObjectActor = enterObjectActor->GetAttachParentActor();
			eventData->enterComponent = newObj;
			while (enterObjectActor != nullptr)
			{
				if (commonRoot == enterObjectActor)
				{
					break;
				}
				eventSystem->CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
				eventData->enterComponentStack.Insert(enterObjectActor->GetRootComponent(), insertIndex);
#if WITH_EDITOR
				//UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
				enterObjectActor = enterObjectActor->GetAttachParentActor();
			}
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
		}
	}
	else
	{
		if (IsValid(oldObj) || eventData->enterComponentStack.Num() > 0)
		{
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("-----begin exit 222"));
#endif
			//exit old
			for (int i = eventData->enterComponentStack.Num() - 1; i >= 0; i--)
			{
				if (IsValid(eventData->enterComponentStack[i]))
				{
					if (!eventData->isExitFiredAtCurrentFrame)
					{
						eventSystem->CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
					}
#if WITH_EDITOR
					//UE_LOG(LGUI, Error, TEXT("	%s, fireType:%d"), *(eventData->enterComponentStack[i]->GetOwner()->GetActorLabel()), (int)(eventData->enterComponentEventFireType));
#endif
				}
				eventData->enterComponentStack.RemoveAt(i);
			}
			eventData->enterComponent = nullptr;
			eventData->isExitFiredAtCurrentFrame = true;
#if WITH_EDITOR
			//UE_LOG(LGUI, Error, TEXT("*****end exit, stack count:%d\n"), eventData->enterComponentStack.Num());
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
#if WITH_EDITOR
				//UE_LOG(LGUI, Error, TEXT("-----begin enter 333"));
				//UE_LOG(LGUI, Error, TEXT("	%s"), *(enterObjectActor->GetActorLabel()));
#endif
				eventSystem->CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
				eventData->enterComponentStack.Add(newObj);
				enterObjectActor = enterObjectActor->GetAttachParentActor();
				while (enterObjectActor != nullptr)
				{
#if WITH_EDITOR
					//UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
					eventSystem->CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
					eventData->enterComponentStack.Insert(enterObjectActor->GetRootComponent(), insertIndex);
					enterObjectActor = enterObjectActor->GetAttachParentActor();
				}
#if WITH_EDITOR
				//UE_LOG(LGUI, Error, TEXT("*****end enter, stack count:%d\n"), eventData->enterComponentStack.Num());
#endif
			}
		}
	}
}
AActor* ULGUI_PointerInputModule::FindCommonRoot(AActor* actorA, AActor* actorB)
{
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
void ULGUI_PointerInputModule::ProcessPointerEvent(ULGUIPointerEventData* eventData, bool lineTraceHitSomething, const FHitResultContainerStruct& hitResultContainer, bool& outIsHitSomething, FHitResult& outHitResult)
{
	if (!CheckEventSystem())return;
	//@todo: fix enter/exit in hierarchy
	eventData->isUpFiredAtCurrentFrame = false;
	eventData->isExitFiredAtCurrentFrame = false;
	eventData->isEndDragFiredAtCurrentFrame = false;
	eventData->isDragExitFiredAtCurrentFrame = false;

	eventData->raycaster = hitResultContainer.raycaster;
	eventData->rayOrigin = hitResultContainer.rayOrigin;
	eventData->rayDirection = hitResultContainer.rayDirection;
	outHitResult = hitResultContainer.hitResult;
	outIsHitSomething = lineTraceHitSomething;

	if (eventData->nowIsTriggerPressed && eventData->prevIsTriggerPressed)//if trigger keep pressing
	{
		if (eventData->dragging)//if is dragging
		{
			if (lineTraceHitSomething)//hit something during drag
			{
				auto nowHitComponent = (USceneComponent*)outHitResult.Component.Get();
				//fire event
				eventData->worldPoint = outHitResult.Location;
				eventData->worldNormal = outHitResult.Normal;
				if (eventData->dragEnterComponent != nowHitComponent)//drag and hit different object
				{
					if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)//prev object, not dragging object
					{
						eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
						eventData->dragEnterComponent = nullptr;
					}
					if (IsValid(nowHitComponent) && nowHitComponent != eventData->dragComponent)//current object, not dragging object
					{
						eventData->dragEnterComponent = nowHitComponent;
						eventData->dragEnterComponentEventFireType = hitResultContainer.eventFireType;
						eventSystem->CallOnPointerDragEnter(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
					}
				}
			}
			else//hit nothing during drag
			{
				if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)//prev object
				{
					eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
					eventData->dragEnterComponent = nullptr;
				}
			}

			//trigger drag event
			if (IsValid(eventData->dragComponent))
			{
				FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
					+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;

				auto prevHitPoint = eventData->worldPoint;
				eventData->worldPoint = approximatHitPosition;
				eventData->worldNormal = outHitResult.Normal;
				eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
				eventData->moveDelta = approximatHitPosition - prevHitPoint;
				eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
				eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
				eventSystem->CallOnPointerDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);

				outHitResult.Location = approximatHitPosition;
				outHitResult.ImpactPoint = approximatHitPosition;
				outHitResult.Distance = eventData->pressDistance;
				outIsHitSomething = true;//always hit a plane when drag
			}
			else
			{
				eventData->dragging = false;
			}
		}
		else//trigger press but not dragging, only consern if trigger drag event
		{
			if (IsValid(eventData->pressComponent))//if hit something when press
			{
				FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
					+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;
				eventData->worldPoint = approximatHitPosition;
				eventData->worldNormal = outHitResult.Normal;
				eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
				eventData->moveDelta = approximatHitPosition - eventData->pressWorldPoint;
				if (eventData->pressRaycaster->rayEmitter->ShouldStartDrag(eventData))
				{
					eventData->dragging = true;
					eventData->dragComponent = eventData->pressComponent;
					eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
					eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
					eventData->dragComponentEventFireType = eventData->enterComponentEventFireType;
					eventSystem->CallOnPointerBeginDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
				}
				outHitResult.Location = approximatHitPosition;
				outHitResult.ImpactPoint = approximatHitPosition;
				outHitResult.Distance = eventData->pressDistance;
				outIsHitSomething = true;
			}
		}
	}
	else if (!eventData->nowIsTriggerPressed && !eventData->prevIsTriggerPressed)//is trigger keep release, only concern Enter/Exit event
	{
		if (lineTraceHitSomething)
		{
			auto nowHitComponent = (USceneComponent*)outHitResult.Component.Get();
			//fire event
			eventData->worldPoint = outHitResult.Location;
			eventData->worldNormal = outHitResult.Normal;
			if (eventData->enterComponent != nowHitComponent)//hit differenct object
			{
				ProcessPointerEnterExit(eventData, eventData->enterComponent, nowHitComponent, hitResultContainer.eventFireType);
			}
		}
		else
		{
			if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)//prev object
			{
				ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, hitResultContainer.eventFireType);
			}
		}
	}
	else//trigger state change
	{
		if (eventData->nowIsTriggerPressed)//now is press, prev is release
		{
			if (lineTraceHitSomething)
			{
				hitResultContainer.raycaster->rayEmitter->MarkPress(eventData);
				auto nowHitComponent = (USceneComponent*)outHitResult.Component.Get();
				eventData->worldPoint = outHitResult.Location;
				eventData->worldNormal = outHitResult.Normal;
				eventData->pressDistance = outHitResult.Distance;
				eventData->pressRayOrigin = hitResultContainer.rayOrigin;
				eventData->pressRayDirection = hitResultContainer.rayDirection;
				eventData->pressWorldPoint = outHitResult.Location;
				eventData->pressWorldNormal = outHitResult.Normal;
				eventData->pressRaycaster = hitResultContainer.raycaster;
				eventData->pressWorldToLocalTransform = nowHitComponent->GetComponentTransform().Inverse();
				if (eventData->enterComponent != nowHitComponent)//hit different object
				{
					ProcessPointerEnterExit(eventData, eventData->enterComponent, nowHitComponent, hitResultContainer.eventFireType);
				}
				if (IsValid(eventData->enterComponent))//now object
				{
					eventData->pressTime = GetWorld()->TimeSeconds;
					eventData->pressComponent = eventData->enterComponent;
					eventData->pressComponentEventFireType = eventData->enterComponentEventFireType;
					eventSystem->CallOnPointerDown(eventData->pressComponent, eventData, eventData->enterComponentEventFireType);
				}
			}
			else
			{
				if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)//prev object
				{
					ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, hitResultContainer.eventFireType);
				}
			}
		}
		else//now is release, prev is press
		{
			auto nowHitComponent = (USceneComponent*)outHitResult.Component.Get();

			if (eventData->dragging)//is dragging
			{
				eventData->dragging = false;
				if (IsValid(eventData->pressComponent))
				{
					auto prevHitPoint = eventData->worldPoint;
					auto approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;//calculated approximate hit position
					eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
					eventData->worldPoint = approximatHitPosition;
					eventData->worldNormal = outHitResult.Normal;
					eventData->moveDelta = approximatHitPosition - prevHitPoint;
					eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
					eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();

					auto oldPressComponent = eventData->pressComponent;
					eventData->pressComponent = nullptr;
					eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
				}
				if (lineTraceHitSomething)//hit something when stop drag
				{
					//if DragEnter an object when press, and after one frame trigger release and hit new object, then old object need to call DragExit
					if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)
					{
						eventSystem->CallOnPointerDragDrop(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
						eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
						eventData->dragEnterComponent = nullptr;
					}
					//drag end
					if (IsValid(eventData->dragComponent))
					{
						eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
						eventData->dragComponent = nullptr;
					}

					if (nowHitComponent != eventData->enterComponent)//hit different object
					{
						//Enter/Exit will not fire during drag, so we need to call Enter/Exit at drag end
						ProcessPointerEnterExit(eventData, eventData->enterComponent, nowHitComponent, hitResultContainer.eventFireType);
					}
				}
				else//hit nothing when stop drag
				{
					//drag end
					if (IsValid(eventData->dragComponent))
					{
						eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
						eventData->dragComponent = nullptr;
					}
					if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
					{
						ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, hitResultContainer.eventFireType);
					}
				}
			}
			else//not dragging
			{
				eventData->worldPoint = outHitResult.Location;
				eventData->worldNormal = outHitResult.Normal;
				if (lineTraceHitSomething)//hit something when release
				{
					if (IsValid(eventData->pressComponent))
					{
						auto oldPressComponent = eventData->pressComponent;
						eventData->pressComponent = nullptr;
						eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
						eventData->clickTime = GetWorld()->TimeSeconds;
						eventSystem->CallOnPointerClick(oldPressComponent, eventData, eventData->pressComponentEventFireType);
					}
					if (eventData->enterComponent != nowHitComponent)//if hit different object when release
					{
						ProcessPointerEnterExit(eventData, eventData->enterComponent, nowHitComponent, hitResultContainer.eventFireType);
					}
				}
				else
				{
					if (IsValid(eventData->pressComponent))
					{
						auto oldPressComponent = eventData->pressComponent;
						eventData->pressComponent = nullptr;
						eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
						eventData->clickTime = GetWorld()->TimeSeconds;
						eventSystem->CallOnPointerClick(oldPressComponent, eventData, eventData->pressComponentEventFireType);
					}
					if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
					{
						ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, hitResultContainer.eventFireType);
					}
				}
			}
		}
	}

	eventData->prevIsTriggerPressed = eventData->nowIsTriggerPressed;
}
void ULGUI_PointerInputModule::ClearEventByID(int pointerID)
{
	auto eventData = GetPointerEventData(pointerID, false);
	if (eventData == nullptr)return;
	if (!CheckEventSystem())return;

	if (eventData->prevIsTriggerPressed)//if trigger is pressed
	{
		if (eventData->dragging)
		{
			eventData->dragging = false;
			if (!eventData->isUpFiredAtCurrentFrame)
			{
				if (IsValid(eventData->pressComponent))
				{
					auto oldPressComponent = eventData->pressComponent;
					eventData->pressComponent = nullptr;
					eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
				}
				eventData->isUpFiredAtCurrentFrame = true;
			}
			if (!eventData->isEndDragFiredAtCurrentFrame)
			{
				if (IsValid(eventData->dragComponent))
				{
					eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
					eventData->dragComponent = nullptr;
				}
				eventData->isEndDragFiredAtCurrentFrame = true;
			}
			if (!eventData->isExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
				{
					ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, eventData->enterComponentEventFireType);
				}
				eventData->isExitFiredAtCurrentFrame = true;
			}
			if (!eventData->isDragExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData->dragEnterComponent))
				{
					eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireType);
					eventData->dragEnterComponent = nullptr;
				}
				eventData->isDragExitFiredAtCurrentFrame = true;
			}
		}
		else
		{
			if (!eventData->isUpFiredAtCurrentFrame)
			{
				if (IsValid(eventData->pressComponent))
				{
					auto oldPressComponent = eventData->pressComponent;
					eventData->pressComponent = nullptr;
					eventSystem->CallOnPointerUp(oldPressComponent, eventData, eventData->pressComponentEventFireType);
				}
				eventData->isUpFiredAtCurrentFrame = true;
			}
			if (!eventData->isExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
				{
					ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, eventData->enterComponentEventFireType);
				}
				eventData->isExitFiredAtCurrentFrame = true;
			}
		}

		eventData->prevIsTriggerPressed = false;
	}
	else
	{
		if (!eventData->isExitFiredAtCurrentFrame)
		{
			if (IsValid(eventData->enterComponent) || eventData->enterComponentStack.Num() > 0)
			{
				ProcessPointerEnterExit(eventData, eventData->enterComponent, nullptr, eventData->enterComponentEventFireType);
			}
			eventData->isExitFiredAtCurrentFrame = true;
		}
	}
}
void ULGUI_PointerInputModule::ClearEvent()
{
	for (auto keyValue : pointerEventDataMap)
	{
		ClearEventByID(keyValue.Key);
	}
}
