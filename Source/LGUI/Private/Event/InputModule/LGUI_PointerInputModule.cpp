// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_PointerInputModule.h"
#include "LGUI.h"
#include "Event/LGUIPointerEventData.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Event/LGUIEventSystem.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Event/Rayemitter/LGUIBaseRayemitter.h"

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
bool ULGUI_PointerInputModule::LineTrace(FHitResultContainerStruct& hitResult)
{
	multiHitResult.Reset();
	if (ALGUIManagerActor::Instance != nullptr)
	{
		auto& raycasterArray = ALGUIManagerActor::Instance->GetRaycasters();

		int32 prevRaycasterDepth = 0;
		for (int i = 0; i < raycasterArray.Num(); i++)
		{
			ULGUIBaseRaycaster* raycasterItem = raycasterArray[i];
			FHitResult hitResultItem;
			if (IsValid(raycasterItem))
			{
				if (raycasterItem->depth < prevRaycasterDepth && multiHitResult.Num() != 0)//if this raycaster's depth less than prev raycaster's depth, and prev hit test is true, then we dont need to raycast more, because of raycaster's depth
				{
					break;
				}
				FVector rayOrigin(0, 0, 0), rayDir(1, 0, 0), rayEnd(1, 0, 0);
				if (raycasterItem->Raycast(rayOrigin, rayDir, rayEnd, hitResultItem))
				{
					FHitResultContainerStruct container;
					container.hitResult = hitResultItem;
					container.eventFireOnAll = raycasterItem->eventFireType == ELGUIEventFireType::TargetActorAndAllItsComponents;
					container.raycaster = raycasterItem;
					container.rayOrigin = rayOrigin;
					container.rayDirection = rayDir;
					container.rayEnd = rayEnd;

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
		}
		hitResult = multiHitResult[0];
		return true;
	}
	return false;
}
void ULGUI_PointerInputModule::ProcessPointerEvent(ULGUIPointerEventData* eventData, bool lineTraceHitSomething, const FHitResultContainerStruct& hitResultContainer, bool& isHitSomething)
{
	auto eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance();
	if (eventSystem == nullptr)return;

	eventData->isUpFiredAtCurrentFrame = false;
	eventData->isExitFiredAtCurrentFrame = false;
	eventData->isEndDragFiredAtCurrentFrame = false;
	eventData->isDragExitFiredAtCurrentFrame = false;

	if (!IsValid(eventData))
	{
		eventData = NewObject<ULGUIPointerEventData>(this);
	}
	eventData->raycaster = hitResultContainer.raycaster;
	eventData->rayOrigin = hitResultContainer.rayOrigin;
	eventData->rayDirection = hitResultContainer.rayDirection;
	FHitResult hitResult = hitResultContainer.hitResult;
	isHitSomething = lineTraceHitSomething;
	if (eventData->nowIsTriggerPressed && eventData->prevIsTriggerPressed)//if trigger keep pressing
	{
		if (IsValid(eventData->pressComponent))
		{
			if (eventData->dragging)//if is dragging
			{
				if (lineTraceHitSomething)//hit something during drag
				{
					auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
					//fire event
					eventData->worldPoint = hitResult.Location;
					eventData->worldNormal = hitResult.Normal;
					if (eventData->dragEnterComponent != nowHitComponent)//drag and hit different object
					{
						if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)//prev object, not dragging object
						{
							eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
							eventData->dragEnterComponent = nullptr;
						}
						if (IsValid(nowHitComponent) && nowHitComponent != eventData->dragComponent)//current object, not dragging object
						{
							eventData->dragEnterComponent = nowHitComponent;
							eventData->dragEnterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
							eventSystem->CallOnPointerDragEnter(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
						}
					}
					eventData->enterComponent = nowHitComponent;
					isHitSomething = true;
				}
				else//hit nothing during drag
				{
					if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)//prev object
					{
						eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
						eventData->dragEnterComponent = nullptr;
					}
					eventData->enterComponent = nullptr;
					isHitSomething = false;
				}

				FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
					+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;
				//trigger drag event
				auto prevHitPoint = eventData->worldPoint;
				eventData->worldPoint = approximatHitPosition;
				eventData->worldNormal = hitResult.Normal;
				eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
				eventData->moveDelta = approximatHitPosition - prevHitPoint;
				eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
				eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
				eventSystem->CallOnPointerDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);

				hitResult.Location = approximatHitPosition;
				hitResult.ImpactPoint = approximatHitPosition;
				hitResult.Distance = eventData->pressDistance;
				isHitSomething = true;//always hit a plane when drag
			}
			else//trigger press but not dragging, only consern if trigger drag event
			{
				if (IsValid(eventData->pressComponent))//if hit something when press
				{
					FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;
					eventData->worldPoint = approximatHitPosition;
					eventData->worldNormal = hitResult.Normal;
					eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
					eventData->moveDelta = approximatHitPosition - eventData->pressWorldPoint;
					if (eventData->pressRaycaster->rayEmitter->ShouldStartDrag(eventData))
					{
						eventData->dragging = true;
						eventData->dragComponent = eventData->pressComponent;
						eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
						eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
						eventData->dragComponentEventFireOnAllOrOnlyTarget = eventData->enterComponentEventFireOnAllOrOnlyTarget;
						eventSystem->CallOnPointerBeginDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
					}
					hitResult.Location = approximatHitPosition;
					hitResult.ImpactPoint = approximatHitPosition;
					hitResult.Distance = eventData->pressDistance;
					isHitSomething = true;
				}
			}
		}
	}
	else if (!eventData->nowIsTriggerPressed && !eventData->prevIsTriggerPressed)//is trigger keep release, only concern Enter/Exit event
	{
		if (lineTraceHitSomething)
		{
			auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
			//fire event
			eventData->worldPoint = hitResult.Location;
			eventData->worldNormal = hitResult.Normal;
			if (eventData->enterComponent != nowHitComponent)//hit differenct object
			{
				if (IsValid(eventData->enterComponent))//prev object
				{
					eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
				}
				if (IsValid(nowHitComponent))//current object
				{
					eventSystem->CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
				}
			}
			eventData->enterComponent = nowHitComponent;
			eventData->enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
		}
		else
		{
			if (IsValid(eventData->enterComponent))//prev object
			{
				eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
			}
			eventData->enterComponent = nullptr;
		}
	}
	else//trigger state change
	{
		if (eventData->nowIsTriggerPressed)//now is press, prev is release
		{
			if (lineTraceHitSomething)
			{
				hitResultContainer.raycaster->rayEmitter->MarkPress(eventData);
				auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
				eventData->worldPoint = hitResult.Location;
				eventData->worldNormal = hitResult.Normal;
				eventData->pressDistance = hitResult.Distance;
				eventData->pressRayOrigin = hitResultContainer.rayOrigin;
				eventData->pressRayDirection = hitResultContainer.rayDirection;
				eventData->pressWorldPoint = hitResult.Location;
				eventData->pressWorldNormal = hitResult.Normal;
				eventData->pressRaycaster = hitResultContainer.raycaster;
				eventData->pressComponent = nowHitComponent;
				eventData->pressWorldToLocalTransform = nowHitComponent->GetComponentTransform().Inverse();
				if (eventData->enterComponent != nowHitComponent)//hit different object
				{
					if (IsValid(eventData->enterComponent))//prev object
					{
						eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
					}
					if (nowHitComponent != nullptr)//current object
					{
						eventSystem->CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
					}
				}
				if (nowHitComponent != nullptr)//now object
				{
					eventSystem->CallOnPointerDown(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
				}
				eventData->enterComponent = nowHitComponent;
				eventData->enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
			}
			else
			{
				if (IsValid(eventData->enterComponent))//prev object
				{
					eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
				}
				eventData->enterComponent = nullptr;
				eventData->pressComponent = nullptr;
			}
		}
		else//now is release, prev is press
		{
			if (IsValid(eventData->pressComponent))
			{
				auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();

				if (eventData->dragging)//is dragging
				{
					auto prevHitPoint = eventData->worldPoint;
					eventData->dragging = false;
					auto approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;//calculated approximate hit position
					eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
					eventData->worldPoint = approximatHitPosition;
					eventData->worldNormal = hitResult.Normal;
					eventData->moveDelta = approximatHitPosition - prevHitPoint;
					eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
					eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
					eventSystem->CallOnPointerUp(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
					if (lineTraceHitSomething)//hit something when stop drag
					{
						//if DragEnter an object when press, and after one frame trigger release and hit new object, then old object need to call DragExit
						if (IsValid(eventData->dragEnterComponent) && eventData->dragEnterComponent != eventData->dragComponent)
						{
							eventSystem->CallOnPointerDragDrop(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
							eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
							eventData->dragEnterComponent = nullptr;
						}

						if (nowHitComponent != eventData->dragComponent)//hit object is not dragging object
						{
							//Enter/Exit will not fire during drag, so we need to call Enter/Exit at drag end
							eventSystem->CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
							eventSystem->CallOnPointerExit(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
						}
						//drag end
						eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
						eventData->dragComponent = nullptr;
						eventData->enterComponent = nowHitComponent;
						eventData->enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
					}
					else//hit nothing when stop drag
					{
						//drag end
						eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
						eventSystem->CallOnPointerExit(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
						eventData->enterComponent = nullptr;
					}
				}
				else//not dragging
				{
					eventData->worldPoint = hitResult.Location;
					eventData->worldNormal = hitResult.Normal;
					if (lineTraceHitSomething)//hit something when release
					{
						if (IsValid(eventData->enterComponent))
						{
							eventSystem->CallOnPointerUp(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
							eventData->clickTime = GetWorld()->TimeSeconds;
							eventSystem->CallOnPointerClick(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
						}
						if (eventData->enterComponent != nowHitComponent)//if hit different object when release
						{
							if (IsValid(eventData->enterComponent))
							{
								eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
							}
							eventSystem->CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
							eventData->enterComponent = nowHitComponent;
							eventData->enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
						}
					}
					else
					{
						if (IsValid(eventData->enterComponent))
						{
							eventSystem->CallOnPointerUp(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
							eventData->clickTime = GetWorld()->TimeSeconds;
							eventSystem->CallOnPointerClick(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
							eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
							eventData->enterComponent = nullptr;
						}
					}
				}
			}
			eventData->pressComponent = nullptr;
		}
	}

	eventData->prevIsTriggerPressed = eventData->nowIsTriggerPressed;
}
void ULGUI_PointerInputModule::ClearEventByID(int pointerID)
{
	auto eventData = GetPointerEventData(pointerID, false);
	if (eventData == nullptr)return;
	auto eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance();
	if (eventSystem == nullptr)return;

	if (eventData->prevIsTriggerPressed)//if trigger is pressed
	{
		if (eventData->dragging)
		{
			if (!eventData->isUpFiredAtCurrentFrame)
			{
				eventData->isUpFiredAtCurrentFrame = true;
				if (IsValid(eventData->dragComponent))
				{
					eventSystem->CallOnPointerUp(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!eventData->isEndDragFiredAtCurrentFrame)
			{
				eventData->isEndDragFiredAtCurrentFrame = true;
				if (IsValid(eventData->dragComponent))
				{
					eventSystem->CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!eventData->isExitFiredAtCurrentFrame)
			{
				eventData->isExitFiredAtCurrentFrame = true;
				if (IsValid(eventData->dragComponent))
				{
					eventSystem->CallOnPointerExit(eventData->dragComponent, eventData, eventData->dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!eventData->isDragExitFiredAtCurrentFrame)
			{
				eventData->isDragExitFiredAtCurrentFrame = true;
				if (IsValid(eventData->dragEnterComponent))
				{
					eventSystem->CallOnPointerDragExit(eventData->dragEnterComponent, eventData, eventData->dragEnterComponentEventFireOnAllOrOnlyTarget);
					eventData->dragEnterComponent = nullptr;
				}
			}
			eventData->dragging = false;
			eventData->dragComponent = nullptr;
		}
		else
		{
			if (!eventData->isUpFiredAtCurrentFrame)
			{
				if (IsValid(eventData->enterComponent))
				{
					eventSystem->CallOnPointerUp(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!eventData->isExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData->enterComponent))
				{
					eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			eventData->enterComponent = nullptr;
			eventData->pressComponent = nullptr;
		}

		eventData->prevIsTriggerPressed = false;
	}
	else
	{
		if (!eventData->isExitFiredAtCurrentFrame)
		{
			if (IsValid(eventData->enterComponent))
			{
				eventSystem->CallOnPointerExit(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
			}
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
