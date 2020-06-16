﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIEventSystem.h"
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
#include "Event/LGUIPointereventData.h"
#include "Interaction/UISelectableComponent.h"
#include "LGUIBPLibrary.h"

ALGUIEventSystemActor::ALGUIEventSystemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	EventSystem = CreateDefaultSubobject<ULGUIEventSystem>(TEXT("EventSystem"));
}

DECLARE_CYCLE_STAT(TEXT("EventSystem RayAndEvent"), STAT_RayAndEvent, STATGROUP_LGUI);

ULGUIEventSystem* ULGUIEventSystem::Instance = nullptr;
ULGUIEventSystem::ULGUIEventSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}
ULGUIEventSystem* ULGUIEventSystem::GetLGUIEventSystemInstance()
{
	return Instance;
}
void ULGUIEventSystem::BeginPlay()
{
	if (Instance != nullptr)
	{
		FString actorName =
#if WITH_EDITOR
			Instance->GetOwner()->GetActorLabel();
#else
			Instance->GetOwner()->GetName();
#endif
		UE_LOG(LGUI, Error, TEXT("LGUIEventSystem component is already exist in actor:%s, multiple LGUIEventSystem is not allowed!"), *actorName);
		this->SetComponentTickEnabled(false);
		Super::BeginPlay();
	}
	else
	{
		Instance = this;
		Super::BeginPlay();
	}
}

void ULGUIEventSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRayEventEnable)
	{
		LineTraceAndEvent();
	}
}

#pragma region Navigation
void ULGUIEventSystem::InputNavigationLeft()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnLeft();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationLeft]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationRight()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnRight();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationRight]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationUp()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnUp();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationUp]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationDown()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnDown();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationDown]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationNext()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnNext();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationDown]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationPrev()
{
	if (isNavigationActive)
	{
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = selectableComponent->FindSelectableOnPrev();
			if (selectableComponent != newSelectable)
			{
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent());
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::InputNavigationDown]Navigation not activated, call InputNavigationBegin before this"));
	}
}
void ULGUIEventSystem::InputNavigationBegin()
{
	if (!isNavigationActive)
	{
		isNavigationActive = true;
		if (!IsValid(selectableComponent))
		{
			BeginNavigation();
		}
	}
}
void ULGUIEventSystem::BeginNavigation()
{
	if (ALGUIManagerActor::Instance != nullptr)
	{
		auto& selectables = ALGUIManagerActor::Instance->GetSelectables();
		if (selectables.Num() > 0)
		{
			int index = 0;
			do
			{
				selectableComponent = selectables[index];
			} while (!IsValid(selectableComponent));
			SetSelectComponent(selectableComponent->GetOwner()->GetRootComponent());
		}
	}
}
void ULGUIEventSystem::InputNavigationEnd()
{
	if (isNavigationActive)
	{
		isNavigationActive = false;
	}
}
#pragma endregion

void ULGUIEventSystem::SetRaycastEnable(bool enable, bool clearEvent)
{
	bRayEventEnable = enable;
	if (bRayEventEnable == false && clearEvent)
	{
		ClearEvent();
	}
}

void ULGUIEventSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Instance = nullptr;
}

void ULGUIEventSystem::ClearEvent()
{
	if (prevIsTriggerPressed)//if trigger is pressed
	{
		if (startToDrag)
		{
			if (!isUpFiredAtCurrentFrame)
			{
				if (IsValid(eventData->dragComponent))
				{
					CallOnPointerUp(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isEndDragFiredAtCurrentFrame)
			{
				if (IsValid(eventData->dragComponent))
				{
					CallOnPointerEndDrag(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isExitFiredAtCurrentFrame)
			{
				if (IsValid(eventData->dragComponent))
				{
					CallOnPointerExit(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
				}
			}
			if (!isDragExitFiredAtCurrentFrame)
			{
				if (IsValid(dragEnterComponent))
				{
					CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
				}
			}
			eventData->dragComponent = nullptr;
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
			eventData->pressComponent = nullptr;
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

USceneComponent* ULGUIEventSystem::GetCurrentHitComponent()
{
	return enterComponent;
}

bool ULGUIEventSystem::LineTrace(FHitResultContainerStruct& hitResult)
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

bool ULGUIEventSystem::LineTraceAndEvent()
{
	SCOPE_CYCLE_COUNTER(STAT_RayAndEvent);
	isUpFiredAtCurrentFrame = false;
	isExitFiredAtCurrentFrame = false;
	isEndDragFiredAtCurrentFrame = false;
	isDragExitFiredAtCurrentFrame = false;

	FHitResultContainerStruct hitResultContainer;

	bool lineTraceHitSomething = LineTrace(hitResultContainer);
	if (!IsValid(eventData))
	{
		eventData = NewObject<ULGUIPointerEventData>(this);
	}
	eventData->raycaster = hitResultContainer.raycaster;
	eventData->rayOrigin = hitResultContainer.rayOrigin;
	eventData->rayDirection = hitResultContainer.rayDirection;
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
					eventData->worldPoint = hitResult.Location;
					eventData->worldNormal = hitResult.Normal;
					if (dragEnterComponent != nowHitComponent)//drag and hit different object
					{
						if (IsValid(dragEnterComponent) && dragEnterComponent != eventData->dragComponent)//prev object, not dragging object
						{
							CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							dragEnterComponent = nullptr;
						}
						if (IsValid(nowHitComponent) && nowHitComponent != eventData->dragComponent)//current object, not dragging object
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
					if (IsValid(dragEnterComponent) && dragEnterComponent != eventData->dragComponent)//prev object
					{
						CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
						dragEnterComponent = nullptr;
					}
					enterComponent = nullptr;
					isHitSomething = false;
				}

				FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
					+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;
				//trigger drag event
				eventData->worldPoint = approximatHitPosition;
				eventData->worldNormal = hitResult.Normal;
				eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
				eventData->moveDelta = approximatHitPosition - prevHitPoint;
				eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
				eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
				CallOnPointerDrag(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);

				prevHitPoint = approximatHitPosition;
				hitResult.Location = approximatHitPosition;
				hitResult.ImpactPoint = approximatHitPosition;
				hitResult.Distance = eventData->pressDistance;
				isHitSomething = true;//always hit a plane when drag
			}
			else//trigger press but not dragging, only consern if trigger drag event
			{
				if (pressHitSomething && IsValid(eventData->pressComponent))//if hit something when press
				{
					FVector approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;
					eventData->worldPoint = approximatHitPosition;
					eventData->worldNormal = hitResult.Normal;
					eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
					eventData->moveDelta = approximatHitPosition - eventData->pressWorldPoint;
					if (eventData->pressRaycaster->rayEmitter->ShouldStartDrag(eventData))
					{
						startToDrag = true;
						eventData->dragComponent = eventData->pressComponent;
						eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
						eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
						dragComponentEventFireOnAllOrOnlyTarget = enterComponentEventFireOnAllOrOnlyTarget;
						CallOnPointerBeginDrag(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
					}
					prevHitPoint = approximatHitPosition;
					hitResult.Location = approximatHitPosition;
					hitResult.ImpactPoint = approximatHitPosition;
					hitResult.Distance = eventData->pressDistance;
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
			eventData->worldPoint = hitResult.Location;
			eventData->worldNormal = hitResult.Normal;
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
					auto approximatHitPosition = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin()
						+ eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection() * eventData->pressDistance;//calculated approximate hit position
					eventData->cumulativeMoveDelta = approximatHitPosition - eventData->pressWorldPoint;
					eventData->worldPoint = approximatHitPosition;
					eventData->worldNormal = hitResult.Normal;
					eventData->moveDelta = approximatHitPosition - prevHitPoint;
					eventData->dragRayOrigin = eventData->pressRaycaster->rayEmitter->GetCurrentRayOrigin();
					eventData->dragRayDirection = eventData->pressRaycaster->rayEmitter->GetCurrentRayDirection();
					CallOnPointerUp(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
					if (lineTraceHitSomething)//hit something when stop drag
					{
						//if DragEnter an object when press, and after one frame trigger release and hit new object, then old object need to call DragExit
						if (IsValid(dragEnterComponent) && dragEnterComponent != eventData->dragComponent)
						{
							CallOnPointerDragDrop(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							CallOnPointerDragExit(dragEnterComponent, eventData, dragEnterComponentEventFireOnAllOrOnlyTarget);
							dragEnterComponent = nullptr;
						}

						if (nowHitComponent != eventData->dragComponent)//hit object is not dragging object
						{
							//Enter/Exit will not fire during drag, so we need to call Enter/Exit at drag end
							CallOnPointerEnter(nowHitComponent, eventData, hitResultContainer.eventFireOnAll);
							CallOnPointerExit(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						}
						//drag end
						CallOnPointerEndDrag(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						eventData->dragComponent = nullptr;
						enterComponent = nowHitComponent;
						enterComponentEventFireOnAllOrOnlyTarget = hitResultContainer.eventFireOnAll;
					}
					else//hit nothing when stop drag
					{
						//drag end
						CallOnPointerEndDrag(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						CallOnPointerExit(eventData->dragComponent, eventData, dragComponentEventFireOnAllOrOnlyTarget);
						enterComponent = nullptr;
					}
				}
				else//not dragging
				{
					eventData->worldPoint = hitResult.Location;
					eventData->worldNormal = hitResult.Normal;
					if (lineTraceHitSomething)//hit something when release
					{
						if (IsValid(enterComponent))
						{
							CallOnPointerUp(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
							eventData->clickTime = GetWorld()->TimeSeconds;
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
							eventData->clickTime = GetWorld()->TimeSeconds;
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


void ULGUIEventSystem::RegisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Add(InEvent);
}
void ULGUIEventSystem::UnregisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Remove(InEvent.GetHandle());
}
void ULGUIEventSystem::RegisterGlobalListener(const FLGUIPointerEventDelegate& InEvent)
{
	globalListenerPointerEvent.Add(InEvent);
}
void ULGUIEventSystem::UnregisterGlobalListener(const FLGUIPointerEventDelegate& InEvent)
{
	globalListenerPointerEvent.Remove(InEvent.GetHandle());
}


FLGUIDelegateHandleWrapper ULGUIEventSystem::RegisterHitEvent(const FLGUIHitDynamicDelegate& InDelegate)
{
	auto delegateHandle = hitEvent.AddLambda([InDelegate](bool isHit, const FHitResult& hitResult, USceneComponent* hitComp) {
		InDelegate.ExecuteIfBound(isHit, hitResult, hitComp);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIEventSystem::UnregisterHitEvent(const FLGUIDelegateHandleWrapper& InHandle)
{
	hitEvent.Remove(InHandle.DelegateHandle);
}
FLGUIDelegateHandleWrapper ULGUIEventSystem::RegisterGlobalListener(const FLGUIPointerEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = globalListenerPointerEvent.AddLambda([InDelegate](ULGUIPointerEventData* eventData) {
		InDelegate.ExecuteIfBound(eventData);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIEventSystem::UnregisterGlobalListener(const FLGUIDelegateHandleWrapper& InHandle)
{
	globalListenerPointerEvent.Remove(InHandle.DelegateHandle);
}


void ULGUIEventSystem::InputScroll(const float& inAxisValue)
{
	if (IsValid(enterComponent))
	{
		if (inAxisValue != 0)
		{
			eventData->scrollAxisValue = inAxisValue;
			CallOnPointerScroll(enterComponent, eventData, enterComponentEventFireOnAllOrOnlyTarget);
		}
	}
}

void ULGUIEventSystem::InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType)
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
	eventData->mouseButtonType = inMouseButtonType;
}

void ULGUIEventSystem::SetSelectComponent(USceneComponent* InSelectComp)
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
			if (auto tempComp = InSelectComp->GetOwner()->FindComponentByClass<UUISelectableComponent>())
			{
				selectableComponent = tempComp;
			}
			CallOnPointerSelect(InSelectComp, eventData, enterComponentEventFireOnAllOrOnlyTarget);
		}
		selectedComponentEventFireOnAllOrOnlyTarget = enterComponentEventFireOnAllOrOnlyTarget;
	}
}


void ULGUIEventSystem::LogEventData(ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
#if WITH_EDITORONLY_DATA
	if (outputLog == false)return;
	UE_LOG(LGUI, Log, TEXT("%s"), *inEventData->ToString());
#endif
}


#pragma region BubbleEvent
void ULGUIEventSystem::BubbleOnPointerEnter(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Enter);
}
void ULGUIEventSystem::BubbleOnPointerExit(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Exit);
}
void ULGUIEventSystem::BubbleOnPointerDown(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Down);
}
void ULGUIEventSystem::BubbleOnPointerUp(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Up);
}
void ULGUIEventSystem::BubbleOnPointerClick(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Click, Click);
}
void ULGUIEventSystem::BubbleOnPointerBeginDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, BeginDrag);
}
void ULGUIEventSystem::BubbleOnPointerDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, Drag);
}
void ULGUIEventSystem::BubbleOnPointerEndDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, EndDrag);
}

void ULGUIEventSystem::BubbleOnPointerScroll(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Scroll, Scroll);
}

void ULGUIEventSystem::BubbleOnPointerDragEnter(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragEnterExit, DragEnter);
}
void ULGUIEventSystem::BubbleOnPointerDragExit(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragEnterExit, DragExit);
}
void ULGUIEventSystem::BubbleOnPointerDragDrop(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragDrop, DragDrop);
}

void ULGUIEventSystem::BubbleOnPointerSelect(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Select);
}
void ULGUIEventSystem::BubbleOnPointerDeselect(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Deselect);
}
#pragma endregion

#pragma region CallEvent
void ULGUIEventSystem::CallOnPointerEnter(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, EnterExit, Enter);
}
void ULGUIEventSystem::CallOnPointerExit(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	if (!isExitFiredAtCurrentFrame)
	{
		isExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, EnterExit, Exit);
	}
}
void ULGUIEventSystem::CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	SetSelectComponent(component);
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DownUp, Down);
}
void ULGUIEventSystem::CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	if (!isUpFiredAtCurrentFrame)
	{
		isUpFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DownUp, Up);
	}
}
void ULGUIEventSystem::CallOnPointerClick(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Click, Click);
}
void ULGUIEventSystem::CallOnPointerBeginDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, BeginDrag);
}
void ULGUIEventSystem::CallOnPointerDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, Drag);
}
void ULGUIEventSystem::CallOnPointerEndDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	if (!isEndDragFiredAtCurrentFrame)
	{
		isEndDragFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Drag, EndDrag);
	}
}

void ULGUIEventSystem::CallOnPointerScroll(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, Scroll, Scroll);
}

void ULGUIEventSystem::CallOnPointerDragEnter(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragEnterExit, DragEnter);
}
void ULGUIEventSystem::CallOnPointerDragExit(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	if (!isDragExitFiredAtCurrentFrame)
	{
		isDragExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragEnterExit, DragExit);
	}
}
void ULGUIEventSystem::CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragDrop, DragDrop);
}

void ULGUIEventSystem::CallOnPointerSelect(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Select);
}
void ULGUIEventSystem::CallOnPointerDeselect(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Deselect);
}
#pragma endregion