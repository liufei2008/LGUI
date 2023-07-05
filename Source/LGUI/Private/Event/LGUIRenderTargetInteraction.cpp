// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LGUIRenderTargetInteraction.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Event/LGUIWorldSpaceRaycaster.h"
#include "Event/LGUIRenderTargetGeometrySource.h"
#include "Event/LGUIScreenSpaceRaycaster.h"
#include "Engine/World.h"

#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDragDropInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"

#define LOCTEXT_NAMESPACE "LGUIRenderTargetInteraction"

ULGUIRenderTargetInteraction::ULGUIRenderTargetInteraction()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void ULGUIRenderTargetInteraction::BeginPlay()
{
	Super::BeginPlay();
	PointerEventData = NewObject<ULGUIPointerEventData>(this);
}

void ULGUIRenderTargetInteraction::OnRegister()
{
	Super::OnRegister();
}

void ULGUIRenderTargetInteraction::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GeometrySource.IsValid())
	{
		GeometrySource = GetOwner()->FindComponentByClass<ULGUIRenderTargetGeometrySource>();
		if (!GeometrySource.IsValid())
		{
			auto ErrorMsg = LOCTEXT("GeometrySourceNotValid", "[ULGUIRenderTargetInteraction::TickComponent] GeometrySource is not valid! LGUIRenderTargetInteraction need a valid LGUIRenderTargetGeometrySource component on the same actor!");
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg.ToString());
			return;
		}
	}
	if (!TargetCanvas.IsValid())
	{
		TargetCanvas = GeometrySource->GetCanvas();
		if (!TargetCanvas.IsValid())
		{
			auto ErrorMsg = LOCTEXT("TargetCanvasNotValid", "[ULGUIRenderTargetInteraction::TickComponent] TargetCanvas is not valid! LGUIRenderTargetInteraction need to get a vaild LGUICanvas from GeometrySource!");
			UE_LOG(LGUI, Error, TEXT("%s"), *ErrorMsg.ToString());
			return;
		}
	}
	if (!InputPointerEventData.IsValid())
		return;

	FHitResultContainerStruct hitResultContainer;
	bool lineTraceHitSomething = LineTrace(hitResultContainer);
	ProcessPointerEvent(PointerEventData, lineTraceHitSomething, hitResultContainer);
}

void ULGUIRenderTargetInteraction::ActivateRaycaster()
{
	//skip Activate && Deactivate, because ULGUIRenderTargetInteraction will process input and interaction by itself
}
void ULGUIRenderTargetInteraction::DeactivateRaycaster()
{
	
}

bool ULGUIRenderTargetInteraction::LineTrace(FHitResultContainerStruct& hitResult)
{
	if (InputPointerEventData->raycaster == nullptr)return false;
	auto RayOrigin = InputPointerEventData->raycaster->GetRayOrigin();
	auto RayDirection = InputPointerEventData->raycaster->GetRayDirection();

	auto RayEnd = RayOrigin + RayDirection * rayLength;

	FVector2D HitUV;
	if (GeometrySource->LineTraceHitUV(InputPointerEventData->faceIndex, InputPointerEventData->worldPoint, RayOrigin, RayEnd, HitUV))
	{
		auto ViewProjectionMatrix = TargetCanvas->GetViewProjectionMatrix();
		FVector2D mousePos01 = HitUV;
		PointerEventData->pointerPosition = FVector(mousePos01 * TargetCanvas->GetViewportSize(), 0);

		FVector OutRayOrigin, OutRayDirection;
		ULGUIScreenSpaceRaycaster::DeprojectViewPointToWorld(ViewProjectionMatrix, mousePos01, OutRayOrigin, OutRayDirection);

		FHitResult hitResultItem;
		TArray<USceneComponent*> hoverArray;//temp array for store hover components
		if (this->Raycast(PointerEventData, OutRayOrigin, OutRayDirection, RayEnd, hitResultItem, hoverArray))
		{
			FHitResultContainerStruct container;
			container.hitResult = hitResultItem;
			container.eventFireType = this->GetEventFireType();
			container.raycaster = this;
			container.rayOrigin = OutRayOrigin;
			container.rayDirection = OutRayDirection;
			container.rayEnd = RayEnd;
			container.hoverArray = hoverArray;

			hitResult = container;
		}

		return true;
	}
	return false;
}

bool ULGUIRenderTargetInteraction::ShouldSkipCanvas(class ULGUICanvas* UICanvas)
{
	if (TargetCanvas.IsValid())
	{
		return !UICanvas->IsRenderToRenderTarget() || TargetCanvas->GetRenderTarget() != UICanvas->GetActualRenderTarget();
	}
	return true;
}
bool ULGUIRenderTargetInteraction::ShouldStartDrag(ULGUIPointerEventData* InPointerEventData)
{
	if (ShouldStartDrag_HoldToDrag(InPointerEventData))return true;
	FVector2D mousePos = FVector2D(InPointerEventData->pointerPosition);
	FVector2D pressMousePos = FVector2D(InPointerEventData->pressPointerPosition);
	return FVector2D::DistSquared(pressMousePos, mousePos) > clickThresholdSquare;
}
bool ULGUIRenderTargetInteraction::Raycast(ULGUIPointerEventData* InPointerEventData, FVector& OutRayOrigin, FVector& OutRayDirection, FVector& OutRayEnd, FHitResult& OutHitResult, TArray<USceneComponent*>& OutHoverArray)
{
	return Super::RaycastUI(InPointerEventData, OutRayOrigin, OutRayDirection, OutRayEnd, OutHitResult, OutHoverArray);
}


bool ULGUIRenderTargetInteraction::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	InputPointerEventData = eventData;
	return bAllowEventBubbleUp;
}
bool ULGUIRenderTargetInteraction::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	return bAllowEventBubbleUp;
}
bool ULGUIRenderTargetInteraction::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	PointerEventData->pressPointerPosition = PointerEventData->pointerPosition;
	PointerEventData->pressTime = GetWorld()->TimeSeconds;
	PointerEventData->nowIsTriggerPressed = true;
	PointerEventData->mouseButtonType = eventData->mouseButtonType;
	return bAllowEventBubbleUp;
}
bool ULGUIRenderTargetInteraction::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	PointerEventData->releaseTime = GetWorld()->TimeSeconds;
	PointerEventData->nowIsTriggerPressed = false;
	return bAllowEventBubbleUp;
}
bool ULGUIRenderTargetInteraction::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	auto inAxisValue = eventData->scrollAxisValue;
	if (IsValid(PointerEventData->enterComponent))
	{
		if (inAxisValue != FVector2D::ZeroVector || PointerEventData->scrollAxisValue != inAxisValue)
		{
			PointerEventData->scrollAxisValue = inAxisValue;
			CallOnPointerScroll(PointerEventData->enterComponent, PointerEventData, PointerEventData->enterComponentEventFireType);
		}
	}
	return bAllowEventBubbleUp;
}


//@todo: these logs is just for editor testing, remove them when ready
#define LOG_ENTER_EXIT 0
void ULGUIRenderTargetInteraction::ProcessPointerEnterExit(ULGUIPointerEventData* eventData, USceneComponent* oldObj, USceneComponent* newObj, ELGUIEventFireType enterFireType)
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
				CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
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
			CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
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
				CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
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
						CallOnPointerExit(eventData->enterComponentStack[i], eventData, eventData->enterComponentEventFireType);
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
				CallOnPointerEnter(newObj, eventData, eventData->enterComponentEventFireType);
				eventData->enterComponentStack.Add(newObj);
				enterObjectActor = enterObjectActor->GetAttachParentActor();
				while (enterObjectActor != nullptr)
				{
#if LOG_ENTER_EXIT
					UE_LOG(LGUI, Error, TEXT("	:%s"), *(enterObjectActor->GetActorLabel()));
#endif
					CallOnPointerEnter(enterObjectActor->GetRootComponent(), eventData, eventData->enterComponentEventFireType);
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
AActor* ULGUIRenderTargetInteraction::FindCommonRoot(AActor* actorA, AActor* actorB)
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
void ULGUIRenderTargetInteraction::ProcessPointerEvent(ULGUIPointerEventData* eventData, bool lineTraceHitSomething, const FHitResultContainerStruct& hitResultContainer)
{
	eventData->isUpFiredAtCurrentFrame = false;
	eventData->isExitFiredAtCurrentFrame = false;
	eventData->isEndDragFiredAtCurrentFrame = false;

	eventData->raycaster = hitResultContainer.raycaster;
	auto outHitResult = hitResultContainer.hitResult;

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

	if (eventData->nowIsTriggerPressed && eventData->prevIsTriggerPressed)//if trigger keep pressing
	{
		if (eventData->isDragging)//if is dragging
		{
			//trigger drag event
			if (IsValid(eventData->dragComponent))
			{
				CallOnPointerDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);

				outHitResult.Distance = eventData->pressDistance;
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
						CallOnPointerBeginDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
					}
				}
				outHitResult.Distance = eventData->pressDistance;
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
					eventData->pressRayOrigin = hitResultContainer.rayOrigin;
					eventData->pressRayDirection = hitResultContainer.rayDirection;
					eventData->pressWorldPoint = outHitResult.Location;
					eventData->pressWorldNormal = outHitResult.Normal;
					eventData->pressRaycaster = hitResultContainer.raycaster;
					eventData->pressWorldToLocalTransform = eventData->enterComponent->GetComponentTransform().Inverse();
					eventData->pressComponent = eventData->enterComponent;
					eventData->pressComponentEventFireType = eventData->enterComponentEventFireType;
					CallOnPointerDown(eventData->pressComponent, eventData, eventData->enterComponentEventFireType);
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
					CallOnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
					eventData->pressComponent = nullptr;
				}
				if (lineTraceHitSomething)//hit something when stop drag
				{
					//if enter an object when drag, and after one frame trigger release and hit new object, then old object need to call DragExit
					if (IsValid(eventData->enterComponent) && eventData->enterComponent != eventData->dragComponent)
					{
						CallOnPointerDragDrop(eventData->enterComponent, eventData, eventData->enterComponentEventFireType);
					}
				}
				//drag end
				if (IsValid(eventData->dragComponent))
				{
					CallOnPointerEndDrag(eventData->dragComponent, eventData, eventData->dragComponentEventFireType);
					eventData->dragComponent = nullptr;
				}
			}
			else//not dragging
			{
				if (IsValid(eventData->pressComponent))
				{
					CallOnPointerUp(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
					eventData->clickTime = GetWorld()->TimeSeconds;
					CallOnPointerClick(eventData->pressComponent, eventData, eventData->pressComponentEventFireType);
					eventData->pressComponent = nullptr;
				}
			}
		}
	}

	eventData->prevIsTriggerPressed = eventData->nowIsTriggerPressed;
}


#define CALL_LGUIINTERFACE(component, inEventData, eventFireType, interface, function, allowBubble)\
{\
	inEventData->eventType = EPointerEventType::function;\
	bool eventAllowBubble = allowBubble;\
	switch(eventFireType)\
	{\
		case ELGUIEventFireType::OnlyTargetActor:\
		{\
			auto ownerActor = component->GetOwner(); \
			if (ownerActor->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
			{\
				if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(ownerActor, inEventData) == false)\
				{\
					eventAllowBubble = false;\
				}\
			}\
		}\
		break;\
		case ELGUIEventFireType::OnlyTargetComponent:\
		{\
			if (component->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
			{\
				if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(component, inEventData) == false)\
				{\
					eventAllowBubble = false;\
				}\
			}\
		}\
		break;\
		case ELGUIEventFireType::TargetActorAndAllItsComponents:\
		{\
			auto ownerActor = component->GetOwner(); \
			if (ownerActor->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
			{\
				if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(ownerActor, inEventData) == false)\
				{\
					eventAllowBubble = false;\
				}\
			}\
			auto components = ownerActor->GetComponents();\
			for (auto item : components)\
			{\
				if (item->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
				{\
					if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(item, inEventData) == false)\
					{\
						eventAllowBubble = false;\
					}\
				}\
			}\
		}\
		break;\
	}\
	if (eventAllowBubble)\
	{\
		if (auto parentActor = component->GetOwner()->GetAttachParentActor())\
		{\
			BubbleOnPointer##function(parentActor, inEventData);\
		}\
	}\
}

#define BUBBLE_LGUIINTERFACE(actor, inEventData, interface, function)\
{\
	bool eventAllowBubble = true;\
	if (actor->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
	{\
		if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(actor, inEventData) == false)\
		{\
			eventAllowBubble = false;\
		}\
	}\
	auto components = actor->GetComponents();\
	for (auto item : components)\
	{\
		if (item->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
		{\
			if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(item, inEventData) == false)\
			{\
				eventAllowBubble = false;\
			}\
		}\
	}\
	if (eventAllowBubble)\
	{\
		if (auto parentActor = actor->GetAttachParentActor())\
		{\
			BubbleOnPointer##function(parentActor, inEventData);\
		}\
	}\
}


#pragma region BubbleEvent
void ULGUIRenderTargetInteraction::BubbleOnPointerEnter(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Enter);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerExit(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, EnterExit, Exit);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerDown(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Down);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerUp(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DownUp, Up);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerClick(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Click, Click);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerBeginDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, BeginDrag);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, Drag);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerEndDrag(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Drag, EndDrag);
}

void ULGUIRenderTargetInteraction::BubbleOnPointerScroll(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, Scroll, Scroll);
}

void ULGUIRenderTargetInteraction::BubbleOnPointerDragDrop(AActor* actor, ULGUIPointerEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, DragDrop, DragDrop);
}

void ULGUIRenderTargetInteraction::BubbleOnPointerSelect(AActor* actor, ULGUIBaseEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Select);
}
void ULGUIRenderTargetInteraction::BubbleOnPointerDeselect(AActor* actor, ULGUIBaseEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Deselect);
}
#pragma endregion

#pragma region CallEvent
void ULGUIRenderTargetInteraction::CallOnPointerEnter(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, EnterExit, Enter, false);
}
void ULGUIRenderTargetInteraction::CallOnPointerExit(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, EnterExit, Exit, false);
}
void ULGUIRenderTargetInteraction::CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, DownUp, Down, true);
}
void ULGUIRenderTargetInteraction::CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	if (!inEventData->isUpFiredAtCurrentFrame)
	{
		inEventData->isUpFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, inEventFireType, DownUp, Up, true);
	}
}
void ULGUIRenderTargetInteraction::CallOnPointerClick(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, Click, Click, true);
}
void ULGUIRenderTargetInteraction::CallOnPointerBeginDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, Drag, BeginDrag, true);
}
void ULGUIRenderTargetInteraction::CallOnPointerDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, Drag, Drag, true);
}
void ULGUIRenderTargetInteraction::CallOnPointerEndDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	if (!inEventData->isEndDragFiredAtCurrentFrame)
	{
		inEventData->isEndDragFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, inEventFireType, Drag, EndDrag, true);
	}
}

void ULGUIRenderTargetInteraction::CallOnPointerScroll(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, Scroll, Scroll, true);
}

void ULGUIRenderTargetInteraction::CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, DragDrop, DragDrop, true);
}

void ULGUIRenderTargetInteraction::CallOnPointerSelect(USceneComponent* component, ULGUIBaseEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, SelectDeselect, Select, false);
}
void ULGUIRenderTargetInteraction::CallOnPointerDeselect(USceneComponent* component, ULGUIBaseEventData* inEventData, ELGUIEventFireType inEventFireType)
{
	CALL_LGUIINTERFACE(component, inEventData, inEventFireType, SelectDeselect, Deselect, false);
}
#pragma endregion


#undef LOCTEXT_NAMESPACE