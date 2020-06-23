// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
#include "Event/InputModule/LGUIBaseInputModule.h"

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
		if (ALGUIManagerActor::Instance != nullptr)
		{
			if (auto inputModule = ALGUIManagerActor::Instance->GetInputModule())
			{
				SCOPE_CYCLE_COUNTER(STAT_RayAndEvent);
				inputModule->ProcessInput();
			}
		}
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
				SetSelectComponent(newSelectable->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
		if (!IsValid(defaultEventData))
		{
			defaultEventData = NewObject<ULGUIBaseEventData>(this);
		}
		auto& selectables = ALGUIManagerActor::Instance->GetSelectables();
		if (selectables.Num() > 0)
		{
			int index = 0;
			do
			{
				selectableComponent = selectables[index];
			} while (!IsValid(selectableComponent));
			SetSelectComponent(selectableComponent->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
	if (ALGUIManagerActor::Instance != nullptr)
	{
		if (auto inputModule = ALGUIManagerActor::Instance->GetInputModule())
		{
			inputModule->ClearEvent();
		}
	}
}


void ULGUIEventSystem::RegisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Add(InEvent);
}
void ULGUIEventSystem::UnregisterHitEvent(const FLGUIHitDelegate& InEvent)
{
	hitEvent.Remove(InEvent.GetHandle());
}
void ULGUIEventSystem::RegisterGlobalListener(const FLGUIBaseEventDelegate& InEvent)
{
	globalListenerPointerEvent.Add(InEvent);
}
void ULGUIEventSystem::UnregisterGlobalListener(const FLGUIBaseEventDelegate& InEvent)
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
FLGUIDelegateHandleWrapper ULGUIEventSystem::RegisterGlobalListener(const FLGUIBaseEventDynamicDelegate& InDelegate)
{
	auto delegateHandle = globalListenerPointerEvent.AddLambda([InDelegate](ULGUIBaseEventData* eventData) {
		InDelegate.ExecuteIfBound(eventData);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIEventSystem::UnregisterGlobalListener(const FLGUIDelegateHandleWrapper& InHandle)
{
	globalListenerPointerEvent.Remove(InHandle.DelegateHandle);
}
void ULGUIEventSystem::RaiseHitEvent(bool hitOrNot, const FHitResult& hitResult, USceneComponent* hitComponent)
{
	if (hitEvent.IsBound() && bRayEventEnable)
	{
		hitEvent.Broadcast(hitOrNot, hitResult, hitComponent);
	}
}

void ULGUIEventSystem::SetSelectComponent(USceneComponent* InSelectComp, ULGUIBaseEventData* eventData, bool eventFireOnAllOrOnlyTarget)
{
	if (selectedComponent != InSelectComp)//select new object
	{
		auto oldSelectedComp = selectedComponent;
		selectedComponent = InSelectComp;
		if (oldSelectedComp != nullptr)
		{
			if (IsValid(oldSelectedComp))
			{
				eventData->selectedComponent = oldSelectedComp;
				CallOnPointerDeselect(selectedComponent, eventData, eventData->selectedComponentEventFireOnAllOrOnlyTarget);
			}
		}
		if (InSelectComp != nullptr)
		{
			if (auto tempComp = InSelectComp->GetOwner()->FindComponentByClass<UUISelectableComponent>())
			{
				selectableComponent = tempComp;
			}
			eventData->selectedComponent = InSelectComp;
			CallOnPointerSelect(selectedComponent, eventData, eventFireOnAllOrOnlyTarget);
		}
		eventData->selectedComponentEventFireOnAllOrOnlyTarget = eventFireOnAllOrOnlyTarget;
	}
}
void ULGUIEventSystem::SetSelectComponentWithDefault(USceneComponent* InSelectComp)
{
	SetSelectComponent(InSelectComp, defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
}
ULGUIBaseInputModule* ULGUIEventSystem::GetCurrentInputModule()
{
	if (ALGUIManagerActor::Instance != nullptr)
	{
		return ALGUIManagerActor::Instance->GetInputModule();
	}
}


void ULGUIEventSystem::LogEventData(ULGUIBaseEventData* inEventData, bool eventFireOnAll)
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

void ULGUIEventSystem::BubbleOnPointerSelect(AActor* actor, ULGUIBaseEventData* inEventData)
{
	BUBBLE_LGUIINTERFACE(actor, inEventData, SelectDeselect, Select);
}
void ULGUIEventSystem::BubbleOnPointerDeselect(AActor* actor, ULGUIBaseEventData* inEventData)
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
	if (!inEventData->isExitFiredAtCurrentFrame)
	{
		inEventData->isExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, EnterExit, Exit);
	}
}
void ULGUIEventSystem::CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DownUp, Down);
	SetSelectComponent(component, inEventData, inEventData->enterComponentEventFireOnAllOrOnlyTarget);
}
void ULGUIEventSystem::CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	if (!inEventData->isUpFiredAtCurrentFrame)
	{
		inEventData->isUpFiredAtCurrentFrame = true;
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
	if (!inEventData->isEndDragFiredAtCurrentFrame)
	{
		inEventData->isEndDragFiredAtCurrentFrame = true;
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
	if (!inEventData->isDragExitFiredAtCurrentFrame)
	{
		inEventData->isDragExitFiredAtCurrentFrame = true;
		CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragEnterExit, DragExit);
	}
}
void ULGUIEventSystem::CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, DragDrop, DragDrop);
}

void ULGUIEventSystem::CallOnPointerSelect(USceneComponent* component, ULGUIBaseEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Select);
}
void ULGUIEventSystem::CallOnPointerDeselect(USceneComponent* component, ULGUIBaseEventData* inEventData, bool eventFireOnAll)
{
	CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, SelectDeselect, Deselect);
}
#pragma endregion