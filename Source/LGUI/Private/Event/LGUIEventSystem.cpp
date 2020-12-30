// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/LGUIEventSystem.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDragEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDragDropInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"
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

TMap<UWorld*, ULGUIEventSystem*> ULGUIEventSystem::WorldToInstanceMap;
ULGUIEventSystem::ULGUIEventSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}
ULGUIEventSystem* ULGUIEventSystem::GetLGUIEventSystemInstance(UObject* WorldContextObject)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (auto result = WorldToInstanceMap.Find(world))
	{
		return *result;
	}
	else
	{
		return nullptr;
	}
}
void ULGUIEventSystem::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LGUI, Error, TEXT("LGUIEventSystem, world:%d, this:%d, isClientOrServer:%d, worldPath:%s"), this->GetWorld(), this, this->GetWorld()->IsClient(), *this->GetWorld()->GetPathName());
	if (auto world = this->GetWorld())
	{
		if (auto instancePtr = WorldToInstanceMap.Find(world))
		{
			auto instance = *instancePtr;
			FString actorName =
#if WITH_EDITOR
				instance->GetOwner()->GetActorLabel();
#else
				instance->GetOwner()->GetName();
#endif
			UE_LOG(LGUI, Warning, TEXT("LGUIEventSystem component is already exist in actor:%s, pathName:%s, world:%s, multiple LGUIEventSystem in same world is not allowed!"), *actorName, *instance->GetPathName(), *world->GetPathName());
			this->SetComponentTickEnabled(false);
		}
		else
		{
			WorldToInstanceMap.Add(world, this);
			existInInstanceMap = true;
		}
	}
}

void ULGUIEventSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRayEventEnable)
	{
		ProcessInputEvent();
	}
}

void ULGUIEventSystem::ProcessInputEvent()
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		if (auto inputModule = LGUIManagerActor->GetInputModule())
		{
			SCOPE_CYCLE_COUNTER(STAT_RayAndEvent);
			inputModule->ProcessInput();
		}
	}
}

#pragma region Navigation
void ULGUIEventSystem::InputNavigationLeft()
{
	if (isNavigationActive)
	{
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnLeft();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnRight();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnUp();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnDown();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnNext();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
		else
		{
			auto newSelectable = navigationSelectableComponent->FindSelectableOnPrev();
			if (navigationSelectableComponent != newSelectable)
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
		if (!IsValid(navigationSelectableComponent))
		{
			BeginNavigation();
		}
	}
}
void ULGUIEventSystem::BeginNavigation()
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		if (!IsValid(defaultEventData))
		{
			defaultEventData = NewObject<ULGUIBaseEventData>(this);
		}
		auto& selectables = LGUIManagerActor->GetSelectables();
		if (selectables.Num() > 0)
		{
			int index = 0;
			do
			{
				navigationSelectableComponent = selectables[index];
			} while (!IsValid(navigationSelectableComponent));
			SetSelectComponent(navigationSelectableComponent->GetOwner()->GetRootComponent(), defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
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
}
void ULGUIEventSystem::BeginDestroy()
{
	Super::BeginDestroy();
	if (WorldToInstanceMap.Num() > 0 && existInInstanceMap)
	{
		bool removed = false;
		if (auto world = this->GetWorld())
		{
			WorldToInstanceMap.Remove(world);
			//UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::BeginDestroy]Remove instance:%d"), this);
			removed = true;
		}
		else
		{
			world = nullptr;
			for (auto keyValue : WorldToInstanceMap)
			{
				if (keyValue.Value == this)
				{
					world = keyValue.Key;
				}
			}
			if (world != nullptr)
			{
				WorldToInstanceMap.Remove(world);
				//UE_LOG(LGUI, Error, TEXT("[ULGUIEventSystem::BeginDestroy]Remove instance:%d"), this);
				removed = true;
			}
		}
		if (removed)
		{
			existInInstanceMap = false;
		}
		else
		{
			UE_LOG(LGUI, Warning, TEXT("[ULGUIEventSystem::BeginDestroy]Instance not removed!"));
		}
	}
	if(WorldToInstanceMap.Num() <= 0)
	{
		UE_LOG(LGUI, Log, TEXT("[ULGUIEventSystem::BeginDestroy]All instance removed."));
	}
}

void ULGUIEventSystem::ClearEvent()
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		if (auto inputModule = LGUIManagerActor->GetInputModule())
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
				eventData->selectedComponent = selectedComponent;
				CallOnPointerDeselect(oldSelectedComp, eventData, eventData->selectedComponentEventFireOnAllOrOnlyTarget);
			}
		}
		if (selectedComponent != nullptr)
		{
			if (auto tempComp = selectedComponent->GetOwner()->FindComponentByClass<UUISelectableComponent>())
			{
				navigationSelectableComponent = tempComp;
			}
			eventData->selectedComponent = selectedComponent;
			CallOnPointerSelect(selectedComponent, eventData, eventFireOnAllOrOnlyTarget);
		}
		eventData->selectedComponentEventFireOnAllOrOnlyTarget = eventFireOnAllOrOnlyTarget;
	}
}
void ULGUIEventSystem::SetSelectComponentWithDefault(USceneComponent* InSelectComp)
{
	if (!IsValid(defaultEventData))
	{
		defaultEventData = NewObject<ULGUIBaseEventData>(this);
	}
	SetSelectComponent(InSelectComp, defaultEventData, defaultEventData->selectedComponentEventFireOnAllOrOnlyTarget);
}
ULGUIBaseInputModule* ULGUIEventSystem::GetCurrentInputModule()
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		return LGUIManagerActor->GetInputModule();
	}
	return nullptr;
}


void ULGUIEventSystem::LogEventData(ULGUIBaseEventData* inEventData, bool eventFireOnAll)
{
#if WITH_EDITORONLY_DATA
	if (outputLog == false)return;
	UE_LOG(LGUI, Log, TEXT("%s"), *inEventData->ToString());
#endif
}


#define CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, interface, function)\
{\
	inEventData->eventType = EPointerEventType::function;\
	LogEventData(inEventData, eventFireOnAll);\
	bool eventAllowBubble = true;\
	if (eventFireOnAll)\
	{\
		auto ownerActor = component->GetOwner();\
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
	else\
	{\
		if (component->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
		{\
			if (ILGUIPointer##interface##Interface::Execute_OnPointer##function(component, inEventData) == false)\
			{\
				eventAllowBubble = false;\
			}\
		}\
	}\
	if (eventAllowBubble)\
	{\
		if (auto parentActor = component->GetOwner()->GetAttachParentActor())\
		{\
			BubbleOnPointer##function(parentActor, inEventData);\
		}\
	}\
	if (globalListenerPointerEvent.IsBound())\
	{\
		globalListenerPointerEvent.Broadcast(inEventData);\
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