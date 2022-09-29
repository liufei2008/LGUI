// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/LGUIEventSystem.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerDragDropInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "Event/LGUIBaseRaycaster.h"
#include "Event/LGUIPointerEventData.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "Utils/LGUIUtils.h"
#include "LGUI.h"

#define LOCTEXT_NAMESPACE "LGUIEventSystemActor"

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
			FString errorMsg = FString::Printf(TEXT("LGUIEventSystem component is already exist in actor:%s, pathName:%s, world:%s, multiple LGUIEventSystem in same world is not allowed!"), *actorName, *instance->GetPathName(), *world->GetPathName());
			UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg);
#if WITH_EDITOR
			LGUIUtils::EditorNotification(FText::FromString(errorMsg), 10);
#endif
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
		auto CurrentInputModule = LGUIManagerActor->GetCurrentInputModule();
		if (CurrentInputModule.IsValid())
		{
			SCOPE_CYCLE_COUNTER(STAT_RayAndEvent);
			CurrentInputModule->ProcessInput();
		}
	}
}

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
		auto CurrentInputModule = LGUIManagerActor->GetCurrentInputModule();
		if (CurrentInputModule.IsValid())
		{
			CurrentInputModule->ClearEvent();
		}
	}
}

ULGUIPointerEventData* ULGUIEventSystem::GetPointerEventData(int pointerID, bool createIfNotExist)const
{
	if (auto foundPtr = pointerEventDataMap.Find(pointerID))
	{
		return *foundPtr;
	}
	auto newEventData = NewObject<ULGUIPointerEventData>(const_cast<ULGUIEventSystem*>(this));
	newEventData->pointerID = pointerID;
	newEventData->inputType = defaultInputType;
	pointerEventDataMap.Add(pointerID, newEventData);
	return newEventData;
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
bool ULGUIEventSystem::IsPointerOverUI(int pointerID)
{
	if (auto foundPtr = pointerEventDataMap.Find(pointerID))
	{
		auto pointer = *foundPtr;
		if (pointer->enterComponentStack.Num() > 0)
		{
			auto firstEnterComp = pointer->enterComponentStack[0];
			if (auto UIItem = Cast<UUIItem>(firstEnterComp))
			{
				return true;
			}
		}
	}
	return false;
}

void ULGUIEventSystem::SetHighlightedComponentForNavigation(USceneComponent* InComp, int InPointerID)
{
	if (auto eventData = GetPointerEventData(InPointerID, true))
	{
		eventData->highlightComponentForNavigation = InComp;
	}
}
USceneComponent* ULGUIEventSystem::GetHighlightedComponentForNavigation(int InPointerID)const
{
	if (auto eventData = GetPointerEventData(InPointerID, false))
	{
		return eventData->highlightComponentForNavigation.Get();
	}
	return nullptr;
}

bool ULGUIEventSystem::SetPointerInputType(int InPointerID, ELGUIPointerInputType InInputType)
{
	if (auto eventData = GetPointerEventData(InPointerID, false))
	{
		return SetPointerInputType(eventData, InInputType);
	}
	return false;
}
bool ULGUIEventSystem::SetPointerInputType(class ULGUIPointerEventData* InPointerEventData, ELGUIPointerInputType InInputType)
{
	if (InPointerEventData->inputType != InInputType)
	{
		InPointerEventData->inputType = InInputType;
		inputChangeDelegate.Broadcast(InPointerEventData->pointerID, InPointerEventData->inputType);
		return true;
	}
	return false;
}
void ULGUIEventSystem::ActivateNavigationInput(int InPointerID, USceneComponent* InDefaultHighlightedComponent)
{
	if (auto eventData = GetPointerEventData(InPointerID, false))
	{
		SetPointerInputType(eventData, ELGUIPointerInputType::Navigation);
		eventData->highlightComponentForNavigation = InDefaultHighlightedComponent;
	}
}
void ULGUIEventSystem::RegisterInputChangeEvent(const FLGUIPointerInputChange_Delegate& pointerInputChange)
{
	inputChangeDelegate.Add(pointerInputChange);
}
void ULGUIEventSystem::UnregisterInputChangeEvent(const FDelegateHandle& delegateHandle)
{
	inputChangeDelegate.Remove(delegateHandle);
}
FLGUIDelegateHandleWrapper ULGUIEventSystem::RegisterInputChangeEvent(const FLGUIPointerInputChange_DynamicDelegate& pointerInputChange)
{
	auto delegateHandle = inputChangeDelegate.AddLambda([pointerInputChange](int pointerID, ELGUIPointerInputType pointerInputType) {
		if (pointerInputChange.IsBound())pointerInputChange.Execute(pointerID, pointerInputType);
		});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void ULGUIEventSystem::UnregisterInputChangeEvent(FLGUIDelegateHandleWrapper delegateHandle)
{
	inputChangeDelegate.Remove(delegateHandle.DelegateHandle);
}


void ULGUIEventSystem::SetSelectComponent(USceneComponent* InSelectComp, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType)
{
	if (eventData->selectedComponent != InSelectComp)//select new object
	{
		auto oldSelectedComp = eventData->selectedComponent;
		eventData->selectedComponent = InSelectComp;
		if (IsValid(oldSelectedComp))
		{
			CallOnPointerDeselect(oldSelectedComp, eventData, eventFireType);
		}
		if (IsValid(eventData->selectedComponent))
		{
			CallOnPointerSelect(eventData->selectedComponent, eventData, eventFireType);
		}
		eventData->selectedComponentEventFireType = eventFireType;
	}
}

USceneComponent* ULGUIEventSystem::GetCurrentSelectedComponent(int InPointerID)const
{
	if (auto eventData = GetPointerEventData(InPointerID, false))
	{
		return eventData->selectedComponent;
	}
	return nullptr;
}

void ULGUIEventSystem::SetSelectComponentWithDefault(USceneComponent* InSelectComp)
{
	auto eventData = GetPointerEventData(0, true);
	SetSelectComponent(InSelectComp, eventData, eventData->pressComponentEventFireType);
}
ULGUIBaseInputModule* ULGUIEventSystem::GetCurrentInputModule()
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		return LGUIManagerActor->GetCurrentInputModule().Get();
	}
	return nullptr;
}


void ULGUIEventSystem::LogEventData(ULGUIBaseEventData* inEventData)
{
#if WITH_EDITORONLY_DATA
	if (outputLog == false)return;
	UE_LOG(LGUI, Log, TEXT("%s"), *inEventData->ToString());
#endif
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
void ULGUIEventSystem::CallOnPointerEnter(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, EnterExit, Enter, false);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerExit(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, EnterExit, Exit, false);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, DownUp, Down, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	if (!inEventData->isUpFiredAtCurrentFrame)
	{
		inEventData->isUpFiredAtCurrentFrame = true;
		LogEventData(inEventData);
		CALL_LGUIINTERFACE(component, inEventData, eventFireType, DownUp, Up, true);
		if (globalListenerPointerEvent.IsBound())
		{
			globalListenerPointerEvent.Broadcast(inEventData);
		}
	}
}
void ULGUIEventSystem::CallOnPointerClick(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, Click, Click, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerBeginDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, Drag, BeginDrag, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, Drag, Drag, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerEndDrag(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	if (!inEventData->isEndDragFiredAtCurrentFrame)
	{
		inEventData->isEndDragFiredAtCurrentFrame = true;
		LogEventData(inEventData);
		CALL_LGUIINTERFACE(component, inEventData, eventFireType, Drag, EndDrag, true);
		if (globalListenerPointerEvent.IsBound())
		{
			globalListenerPointerEvent.Broadcast(inEventData);
		}
	}
}

void ULGUIEventSystem::CallOnPointerScroll(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, Scroll, Scroll, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}

void ULGUIEventSystem::CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, DragDrop, DragDrop, true);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}

void ULGUIEventSystem::CallOnPointerSelect(USceneComponent* component, ULGUIBaseEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, SelectDeselect, Select, false);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
void ULGUIEventSystem::CallOnPointerDeselect(USceneComponent* component, ULGUIBaseEventData* inEventData, ELGUIEventFireType eventFireType)
{
	LogEventData(inEventData);
	CALL_LGUIINTERFACE(component, inEventData, eventFireType, SelectDeselect, Deselect, false);
	if (globalListenerPointerEvent.IsBound())
	{
		globalListenerPointerEvent.Broadcast(inEventData);
	}
}
#pragma endregion

#undef LOCTEXT_NAMESPACE