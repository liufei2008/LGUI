// Copyright 2019 LexLiu. All Rights Reserved.

#include "Event/LGUIEventSystemActor.h"
#include "Event/LGUIPointerClickInterface.h"
#include "Event/LGUIPointerEnterExitInterface.h"
#include "Event/LGUIPointerDownUpInterface.h"
#include "Event/LGUIPointerDragInterface.h"
#include "Event/LGUIPointerScrollInterface.h"
#include "Event/LGUIPointerDragEnterExitInterface.h"
#include "Event/LGUIPointerDragDropInterface.h"
#include "Event/LGUIPointerSelectDeselectInterface.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Event/Raycaster/LGUIBaseRaycaster.h"
#include "Event/Rayemitter/LGUIBaseRayemitter.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIBPLibrary.h"

DECLARE_CYCLE_STAT(TEXT("EventSystem RayAndEvent"), STAT_RayAndEvent, STATGROUP_LGUI);

ALGUIEventSystemActor* ALGUIEventSystemActor::Instance = nullptr;
ALGUIEventSystemActor::ALGUIEventSystemActor()
{
	PrimaryActorTick.bCanEverTick = true;
	if (auto world = this->GetWorld())
	{
		if (world->IsGameWorld())
		{
			Instance = this;
		}
	}
}
ALGUIEventSystemActor* ALGUIEventSystemActor::GetInstance()
{
	return Instance;
}
void ALGUIEventSystemActor::BeginPlay()
{
	Super::BeginPlay();
	//double check
	Instance = this;
}

void ALGUIEventSystemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRayEventEnable)
	{
		LineTraceAndEvent();
	}
}

void ALGUIEventSystemActor::SetRaycastEnable(bool enable, bool clearEvent)
{
	bRayEventEnable = enable;
	if (bRayEventEnable == false && clearEvent)
	{
		ClearEvent();
	}
}

void ALGUIEventSystemActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance = nullptr;
}

void ALGUIEventSystemActor::ClearEvent()
{
	if (prevIsTriggerPressed)//if trigger is pressed
	{
		if (startToDrag)
		{
			if (!isUpFiredAtCurrentFrame)
			{
				if (IsValid(eventData.dragComponent))
				{
					CallOnPointerUp(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isEndDragFiredAtCurrentFrame)
			{
				if (IsValid(eventData.dragComponent))
				{
					CallOnPointerEndDrag(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData.dragComponent))
				{
					CallOnPointerExit(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isDragExitFiredAtCurrentFrame)
			{
				if (IsValid(dragEnterComponent))
				{
					CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			eventData.dragComponent = nullptr;
			startToDrag = false;
		}
		else
		{
			if (!isUpFiredAtCurrentFrame)
			{
				if (IsValid(enterComponent))
				{
					CallOnPointerUp(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isExitFiredAtCurrentFrame)
			{
				if (IsValid(enterComponent))
				{
					CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			enterComponent = nullptr;
			eventData.pressComponent = nullptr;
		}

		prevIsTriggerPressed = false;
	}
	else
	{
		if (!isExitFiredAtCurrentFrame)
		{
			if (IsValid(enterComponent))
			{
				CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
			}
		}
	}
}

USceneComponent* ALGUIEventSystemActor::GetCurrentHitComponent()
{
	return enterComponent;
}

bool ALGUIEventSystemActor::LineTrace(FHitResultContainerStruct& hitResult)
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

bool ALGUIEventSystemActor::LineTraceAndEvent()
{
	SCOPE_CYCLE_COUNTER(STAT_RayAndEvent);
	isUpFiredAtCurrentFrame = false;
	isExitFiredAtCurrentFrame = false;
	isEndDragFiredAtCurrentFrame = false;
	isDragExitFiredAtCurrentFrame = false;

	FHitResultContainerStruct hitResultContainer;

	bool lineTraceHitSomething = LineTrace(hitResultContainer);
	eventData.raycaster = hitResultContainer.raycaster;
	eventData.rayOrigin = hitResultContainer.rayOrigin;
	eventData.rayDirection = hitResultContainer.rayDirection;
	FHitResult hitResult = hitResultContainer.hitResult;
	bool isHitSomething = lineTraceHitSomething;
	if (nowIsTriggerPressed && prevIsTriggerPressed)//if trigger keep pressing
	{
		if (pressHitSomething)
		{
			if (startToDrag)//if is dragging
			{
				if (lineTraceHitSomething)//hit something during drag
				{
					auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
					//fire event
					eventData.worldPoint = hitResult.Location;
					eventData.worldNormal = hitResult.Normal;
					if (dragEnterComponent != nowHitComponent)//drag and hit different object
					{
						if (IsValid(dragEnterComponent) && dragEnterComponent != eventData.dragComponent)//prev object, not dragging object
						{
							CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							dragEnterComponent = nullptr;
						}
						if (IsValid(nowHitComponent) && nowHitComponent != eventData.dragComponent)//current object, not dragging object
						{
							dragEnterComponent = nowHitComponent;
							dragEnterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
							CallOnPointerDragEnter(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
						}
					}
					enterComponent = nowHitComponent;
					isHitSomething = true;
				}
				else//hit nothing during drag
				{
					if (IsValid(dragEnterComponent) && dragEnterComponent != eventData.dragComponent)//prev object
					{
						CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
						dragEnterComponent = nullptr;
					}
					enterComponent = nullptr;
					isHitSomething = false;
				}

				FVector approximatHitPosition = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin()
					+ eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData.pressDistance;
				//trigger drag event
				eventData.worldPoint = approximatHitPosition;
				eventData.worldNormal = hitResult.Normal;
				eventData.cumulativeMoveDelta = approximatHitPosition - eventData.pressWorldPoint;
				eventData.moveDelta = approximatHitPosition - prevHitPoint;
				eventData.dragRayOrigin = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin();
				eventData.dragRayDirection = eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection();
				CallOnPointerDrag(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);

				prevHitPoint = approximatHitPosition;
				hitResult.Location = approximatHitPosition;
				hitResult.ImpactPoint = approximatHitPosition;
				hitResult.Distance = eventData.pressDistance;
				isHitSomething = true;//always hit a plane when drag
			}
			else//trigger press but not dragging, only consern if trigger drag event
			{
				if (pressHitSomething && IsValid(eventData.pressComponent))//if hit something when press
				{
					FVector approximatHitPosition = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData.pressDistance;
					eventData.worldPoint = approximatHitPosition;
					eventData.worldNormal = hitResult.Normal;
					eventData.cumulativeMoveDelta = approximatHitPosition - eventData.pressWorldPoint;
					eventData.moveDelta = approximatHitPosition - eventData.pressWorldPoint;
					if (eventData.pressRaycaster->rayEmitter->ShouldStartDrag(eventData))
					{
						startToDrag = true;
						eventData.dragComponent = eventData.pressComponent;
						eventData.dragRayOrigin = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin();
						eventData.dragRayDirection = eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection();
						dragComponentEventFireOnAllOrOnlyTarget = enterComponentEventFireOnAllOrOnlyTarget;
						CallOnPointerBeginDrag(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
					}
					prevHitPoint = approximatHitPosition;
					hitResult.Location = approximatHitPosition;
					hitResult.ImpactPoint = approximatHitPosition;
					hitResult.Distance = eventData.pressDistance;
					isHitSomething = true;
				}
			}
		}
	}
	else if (!nowIsTriggerPressed && !prevIsTriggerPressed)//is trigger keep release, only concern Enter/Exit event
	{
		if (lineTraceHitSomething)
		{
			auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
			//fire event
			eventData.worldPoint = hitResult.Location;
			eventData.worldNormal = hitResult.Normal;
			if (enterComponent != nowHitComponent)//hit differenct object
			{
				if (IsValid(enterComponent))//prev object
				{
					CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
				}
				if (IsValid(nowHitComponent))//current object
				{
					CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
				}
			}
			enterComponent = nowHitComponent;
			enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
		}
		else
		{
			if (IsValid(enterComponent))//prev object
			{
				CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
			}
			enterComponent = nullptr;
		}
	}
	else//trigger state change
	{
		if (nowIsTriggerPressed)//now is press, prev is release
		{
			if (lineTraceHitSomething)
			{
				hitResultContainer.raycaster->rayEmitter->MarkPress(eventData);
				auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();
				eventData.worldPoint = hitResult.Location;
				eventData.worldNormal = hitResult.Normal;
				eventData.pressDistance = hitResult.Distance;
				eventData.pressRayOrigin = hitResultContainer.rayOrigin;
				eventData.pressRayDirection = hitResultContainer.rayDirection;
				eventData.pressWorldPoint = hitResult.Location;
				eventData.pressWorldNormal = hitResult.Normal;
				eventData.pressRaycaster = hitResultContainer.raycaster;
				eventData.pressComponent = nowHitComponent;
				eventData.pressWorldToLocalTransform = nowHitComponent->GetComponentTransform().Inverse();
				if (enterComponent != nowHitComponent)//hit different object
				{
					if (IsValid(enterComponent))//prev object
					{
						CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
					}
					if (nowHitComponent != nullptr)//current object
					{
						CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
					}
				}
				if (nowHitComponent != nullptr)//now object
				{
					CallOnPointerDown(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
				}
				enterComponent = nowHitComponent;
				enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
				pressHitSomething = true;
			}
			else
			{
				if (IsValid(enterComponent))//prev object
				{
					CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
				}
				enterComponent = nullptr;
				pressHitSomething = false;
			}
		}
		else//now is release, prev is press
		{
			if (pressHitSomething)
			{
				auto nowHitComponent = (USceneComponent*)hitResult.Component.Get();

				if (startToDrag)//is dragging
				{
					startToDrag = false;
					auto approximatHitPosition = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData.pressDistance;//calculated approximate hit position
					eventData.cumulativeMoveDelta = approximatHitPosition - eventData.pressWorldPoint;
					eventData.worldPoint = approximatHitPosition;
					eventData.worldNormal = hitResult.Normal;
					eventData.moveDelta = approximatHitPosition - prevHitPoint;
					eventData.dragRayOrigin = eventData.pressRaycaster->rayEmitter->GetCurrentRayOrigin();
					eventData.dragRayDirection = eventData.pressRaycaster->rayEmitter->GetCurrentRayDirection();
					CallOnPointerUp(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
					if (lineTraceHitSomething)//hit something when stop drag
					{
						//if DragEnter an object when press, and after one frame trigger release and hit new object, then old object need to call DragExit
						if (IsValid(dragEnterComponent) && dragEnterComponent != eventData.dragComponent)
						{
							CallOnPointerDragDrop(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							dragEnterComponent = nullptr;
						}

						if (nowHitComponent != eventData.dragComponent)//hit object is not dragging object
						{
							//Enter/Exit will not fire during drag, so we need to call Enter/Exit at drag end
							CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
							CallOnPointerExit(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						}
						//drag end
						CallOnPointerEndDrag(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						eventData.dragComponent = nullptr;
						enterComponent = nowHitComponent;
						enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
					}
					else//hit nothing when stop drag
					{
						//drag end
						CallOnPointerEndDrag(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						CallOnPointerExit(eventData.dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						enterComponent = nullptr;
					}
				}
				else//not dragging
				{
					eventData.worldPoint = hitResult.Location;
					eventData.worldNormal = hitResult.Normal;
					if (lineTraceHitSomething)//hit something when release
					{
						if (IsValid(enterComponent))
						{
							CallOnPointerUp(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							eventData.clickTime = GetWorld()->TimeSeconds;
							CallOnPointerClick(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
						}
						if (enterComponent != nowHitComponent)//if hit different object when release
						{
							if (IsValid(enterComponent))
							{
								CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							}
							CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
							enterComponent = nowHitComponent;
							enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
						}
					}
					else
					{
						if (IsValid(enterComponent))
						{
							CallOnPointerUp(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							eventData.clickTime = GetWorld()->TimeSeconds;
							CallOnPointerClick(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							CallOnPointerExit(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							enterComponent = nullptr;
						}
					}
				}
			}
			pressHitSomething = false;
		}
	}
	
	prevIsTriggerPressed = nowIsTriggerPressed;
	if (hitEvent.IsBound() && bRayEventEnable)
	{
		auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
		hitResult.Component = nullptr;
		hitEvent.Broadcast(isHitSomething, hitResult, tempHitComp);
	}

	return isHitSomething;
}


void ALGUIEventSystemActor::RegisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Add(InEvent);
}
void ALGUIEventSystemActor::UnregisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Remove(InEvent.GetHandle());
}
void ALGUIEventSystemActor::RegisterGlobalListener(const FLGUIPointerEventDelegate& InEvent)
{
	globalListenerPointerEvent.Add(InEvent);
}
void ALGUIEventSystemActor::UnregisterGlobalListener(const FLGUIPointerEventDelegate& InEvent)
{
	globalListenerPointerEvent.Remove(InEvent.GetHandle());
}


FLGUIDelegateHandleWrapper ALGUIEventSystemActor::RegisterHitEvent(const FLGUIHitDynamicDelegate& InDelegate)
{
	auto delegateHandle = hitEvent.AddLambda([InDelegate](bool isHit, const FHitResult& hitResult, USceneComponent* hitComp) {
		InDelegate.ExecuteIfBound(isHit, hitResult, hitComp);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ALGUIEventSystemActor::UnregisterHitEvent(const FLGUIDelegateHandleWrapper& InHandle)
{
	hitEvent.Remove(InHandle.DelegateHandle);
}
FLGUIDelegateHandleWrapper ALGUIEventSystemActor::RegisterGlobalListener(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = globalListenerPointerEvent.AddLambda([InDelegate](const FLGUIPointerEventData& eventData) {
		InDelegate.ExecuteIfBound(eventData);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ALGUIEventSystemActor::UnregisterGlobalListener(const FLGUIDelegateHandleWrapper& InHandle)
{
	globalListenerPointerEvent.Remove(InHandle.DelegateHandle);
}


void ALGUIEventSystemActor::InputScroll(const float& inAxisValue)
{
	if (IsValid(enterComponent))
	{
		if (inAxisValue != 0)
		{
			eventData.scrollAxisValue = inAxisValue;
			CallOnPointerScroll(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
		}
	}
}

void ALGUIEventSystemActor::InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType)
{
	nowIsTriggerPressed = inTriggerPress;
	if (nowIsTriggerPressed)
	{
		if (prevPressTriggerType != inMouseButtonType)
		{
			//clear event, because input trigger change
			ClearEvent();
			prevPressTriggerType = inMouseButtonType;
		}
	}
	eventData.mouseButtonType = inMouseButtonType;
}

void ALGUIEventSystemActor::SetSelectComponent(USceneComponent* InSelectComp)
{
	if (selectedComponent != InSelectComp)//select new object
	{
		auto oldSelectedComp = selectedComponent;
		selectedComponent = InSelectComp;
		if (oldSelectedComp != nullptr)
		{
			if (IsValid(oldSelectedComp))
			{
				CallOnPointerDeselect(oldSelectedComp, eventData, selectedComponentEventFireOnAllOrOnlyTarget);
			}
		}
		if (InSelectComp != nullptr)
		{
			CallOnPointerSelect(InSelectComp, eventData, enterComponentEventFireOnAllOrOnlyTarget);
		}
		selectedComponentEventFireOnAllOrOnlyTarget = enterComponentEventFireOnAllOrOnlyTarget;
	}
}


void ALGUIEventSystemActor::LogEventData(FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
#if WITH_EDITORONLY_DATA
	if (outputLog == false)return;
	UE_LOG(LGUI, Log, TEXT("%s"), *inEventData.ToString());
#endif
}


#pragma region BubbleEvent
void ALGUIEventSystemActor::BubbleOnPointerEnter(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Enter);
}
void ALGUIEventSystemActor::BubbleOnPointerExit(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Exit);
}
void ALGUIEventSystemActor::BubbleOnPointerDown(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Down);
}
void ALGUIEventSystemActor::BubbleOnPointerUp(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Up);
}
void ALGUIEventSystemActor::BubbleOnPointerClick(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Click, Click);
}
void ALGUIEventSystemActor::BubbleOnPointerBeginDrag(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, BeginDrag);
}
void ALGUIEventSystemActor::BubbleOnPointerDrag(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, Drag);
}
void ALGUIEventSystemActor::BubbleOnPointerEndDrag(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, EndDrag);
}

void ALGUIEventSystemActor::BubbleOnPointerScroll(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Scroll, Scroll);
}

void ALGUIEventSystemActor::BubbleOnPointerDragEnter(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragEnterExit, DragEnter);
}
void ALGUIEventSystemActor::BubbleOnPointerDragExit(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragEnterExit, DragExit);
}
void ALGUIEventSystemActor::BubbleOnPointerDragDrop(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragDrop, DragDrop);
}

void ALGUIEventSystemActor::BubbleOnPointerSelect(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Select);
}
void ALGUIEventSystemActor::BubbleOnPointerDeselect(AActor* actor, FLGUIPointerEventData& inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Deselect);
}
#pragma endregion

#pragma region CallEvent
void ALGUIEventSystemActor::CallOnPointerEnter(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, EnterExit, Enter);
}
void ALGUIEventSystemActor::CallOnPointerExit(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	if (!isExitFiredAtCurrentFrame)
	{
		isExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, EnterExit, Exit);
	}
}
void ALGUIEventSystemActor::CallOnPointerDown(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	SetSelectComponent(component);
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DownUp, Down);
}
void ALGUIEventSystemActor::CallOnPointerUp(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	if (!isUpFiredAtCurrentFrame)
	{
		isUpFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DownUp, Up);
	}
}
void ALGUIEventSystemActor::CallOnPointerClick(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Click, Click);
}
void ALGUIEventSystemActor::CallOnPointerBeginDrag(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, BeginDrag);
}
void ALGUIEventSystemActor::CallOnPointerDrag(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, Drag);
}
void ALGUIEventSystemActor::CallOnPointerEndDrag(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	if (!isEndDragFiredAtCurrentFrame)
	{
		isEndDragFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, EndDrag);
	}
}

void ALGUIEventSystemActor::CallOnPointerScroll(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Scroll, Scroll);
}

void ALGUIEventSystemActor::CallOnPointerDragEnter(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragEnterExit, DragEnter);
}
void ALGUIEventSystemActor::CallOnPointerDragExit(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	if (!isDragExitFiredAtCurrentFrame)
	{
		isDragExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragEnterExit, DragExit);
	}
}
void ALGUIEventSystemActor::CallOnPointerDragDrop(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragDrop, DragDrop);
}

void ALGUIEventSystemActor::CallOnPointerSelect(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Select);
}
void ALGUIEventSystemActor::CallOnPointerDeselect(USceneComponent* component, FLGUIPointerEventData& inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Deselect);
}
#pragma endregion