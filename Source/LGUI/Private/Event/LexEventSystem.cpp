// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/LexEventSystem.h"
#include "Event/Interface/LexPointerClickInterface.h"
#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Event/Interface/LexPointerScrollInterface.h"
#include "Event/Interface/LexPointerDropInterface.h"
#include "Event/Interface/LexSelectDeselectInterface.h"
#include "Core/LexUIManager.h"
#include "Event/LexPointerEventData.h"
#include "Event/InputModule/LexBaseInputModule.h"
#include "LGUI.h"
#include "Core/Components/LexWidget.h"

#define LOCTEXT_NAMESPACE "LGUIEventSystemActor"

ALexEventSystemActor::ALexEventSystemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	EventSystem = CreateDefaultSubobject<ULexEventSystem>(TEXT("EventSystem"));
}

DECLARE_CYCLE_STAT(TEXT("EventSystem"), STAT_EventSystem, STATGROUP_LGUI);

ULexEventSystem::ULexEventSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = true;
}
ULexEventSystem* ULexEventSystem::GetLexEventSystemInstance(UObject* WorldContextObject, int UserIndex)
{
	if (auto World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(World))
		{
			return LexUIManager->GetEventSystemByUserIndex(UserIndex);
		}
	}
	return nullptr;
}
void ULexEventSystem::BeginPlay()
{
	Super::BeginPlay();
	auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld());
	check(LexUIManager != nullptr);
	LexUIManager->AddEventSystem(this);
}

void ULexEventSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRayEventEnable)
	{
		ProcessInputEvent();
	}
}

void ULexEventSystem::ProcessInputEvent()
{
	if (CurrentInputModule.IsValid())
	{
		SCOPE_CYCLE_COUNTER(STAT_EventSystem);
		CurrentInputModule->ProcessInput();
	}
}

void ULexEventSystem::SetRaycastEnable(bool bEnable, bool bClearEvent)
{
	bRayEventEnable = bEnable;
	if (bRayEventEnable == false && bClearEvent)
	{
		ClearEvent();
	}
}

void ULexEventSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
void ULexEventSystem::BeginDestroy()
{
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
		LexUIManager->RemoveEventSystem(this);
	}
	Super::BeginDestroy();
}

void ULexEventSystem::SetInputModule(ULexBaseInputModule* InputModule)
{
	CurrentInputModule = InputModule;
}

void ULexEventSystem::ClearInputModule()
{
	CurrentInputModule = nullptr;
}

void ULexEventSystem::ClearEvent()
{
	if (CurrentInputModule.IsValid())
	{
		CurrentInputModule->ClearEvent();
	}
}

ULexPointerEventData* ULexEventSystem::GetPointerEventData(int PointerID, bool bCreateIfNotExist)const
{
	if (auto foundPtr = PointerEventDataMap.Find(PointerID))
	{
		return *foundPtr;
	}
	auto newEventData = NewObject<ULexPointerEventData>(const_cast<ULexEventSystem*>(this));
	newEventData->PointerID = PointerID;
	newEventData->InputType = DefaultInputType;
	PointerEventDataMap.Add(PointerID, newEventData);
	return newEventData;
}
void ULexEventSystem::RemovePointerEventData(int PointerID)
{
	PointerEventDataMap.Remove(PointerID);
}

void ULexEventSystem::RaiseHitEvent(bool bHitOrNot, const FLexUIHitResult& HitResult, ULexWidget* HitComponent)
{
	if (bRayEventEnable)
	{
		RaycastHitEvent.Broadcast(bHitOrNot, HitResult, HitComponent);
		RaycastHitEventBP.Broadcast(bHitOrNot, HitResult, HitComponent);
	}
}
bool ULexEventSystem::IsPointerOverUIByPointerID(int PointerID)
{
	if (auto foundPtr = PointerEventDataMap.Find(PointerID))
	{
		return (*foundPtr)->IsPointerOverUI();
	}
	return false;
}

void ULexEventSystem::SetHighlightedComponentForNavigation(ULexWidget* InComp, int InPointerID)
{
	if (auto EventData = GetPointerEventData(InPointerID, true))
	{
		EventData->SetHighlightedWidgetForNavigation(InComp);
	}
}
ULexWidget* ULexEventSystem::GetHighlightedComponentForNavigation(int InPointerID)const
{
	if (auto EventData = GetPointerEventData(InPointerID, false))
	{
		return EventData->GetHighlightedComponentForNavigation();
	}
	return nullptr;
}

bool ULexEventSystem::SetPointerInputTypeByPointerID(int InPointerID, ELexUIPointerInputType InInputType)
{
	if (auto EventData = GetPointerEventData(InPointerID, false))
	{
		return SetPointerInputType(EventData, InInputType);
	}
	return false;
}
bool ULexEventSystem::SetPointerInputType(ULexPointerEventData* InPointerEventData, ELexUIPointerInputType InInputType)
{
	if (InPointerEventData->InputType != InInputType)
	{
		InPointerEventData->InputType = InInputType;
		PointerInputTypedChangedEvent.Broadcast(InPointerEventData->PointerID, InPointerEventData->InputType);
		return true;
	}
	return false;
}
void ULexEventSystem::ActivateNavigationInput(int InPointerID, ULexWidget* InDefaultHighlightedComponent)
{
	if (auto EventData = GetPointerEventData(InPointerID, false))
	{
		SetPointerInputType(EventData, ELexUIPointerInputType::Navigation);
		EventData->SetHighlightedWidgetForNavigation(InDefaultHighlightedComponent);
	}
}

void ULexEventSystem::SetSelectWidget(ULexWidget* InSelectWidget, ULexBaseEventData* EventData)
{
	if (EventData->SelectedComponent != InSelectWidget)//select new object
	{
		auto oldSelectedComp = EventData->SelectedComponent;
		EventData->SelectedComponent = InSelectWidget;
		if (IsValid(oldSelectedComp))
		{
			CallOnDeselect(oldSelectedComp, EventData);
		}
		if (IsValid(EventData->SelectedComponent))
		{
			CallOnSelect(EventData->SelectedComponent, EventData);
		}
	}
}

void ULexEventSystem::SetSelectWidget(ULexEventSystem* InEventSystem, ULexWidget* InSelectWidget, ULexBaseEventData* EventData)
{
	if (InEventSystem != nullptr)
	{
		InEventSystem->SetSelectWidget(InSelectWidget, EventData);
	}
	else
	{
		if (EventData->SelectedComponent != InSelectWidget)//select new object
		{
			auto oldSelectedComp = EventData->SelectedComponent;
			EventData->SelectedComponent = InSelectWidget;
			if (IsValid(oldSelectedComp))
			{
				ExecuteEvent_OnDeselect(oldSelectedComp, EventData, false);
			}
			if (IsValid(EventData->SelectedComponent))
			{
				ExecuteEvent_OnSelect(EventData->SelectedComponent, EventData, false);
			}
		}
	}
}

ULexWidget* ULexEventSystem::GetCurrentSelectedComponent(int InPointerID)const
{
	if (auto EventData = GetPointerEventData(InPointerID, false))
	{
		return EventData->SelectedComponent;
	}
	return nullptr;
}

void ULexEventSystem::SetSelectComponentWithDefault(ULexWidget* InSelectWidget)
{
	auto EventData = GetPointerEventData(0, true);
	SetSelectWidget(InSelectWidget, EventData);
}

void ULexEventSystem::LogEventData(ULexBaseEventData* inEventData)
{
#if WITH_EDITORONLY_DATA
	if (bOutputLog == false)return;
	UE_LOG(LGUI, Log, TEXT("%s"), *inEventData->ToString());
#endif
}

ULexWidget* ULexEventSystem::GetEventHandler(ULexWidget* Widget, UClass* InterfaceClass)
{
	auto ParentWidget = Widget;
	while (ParentWidget != nullptr)
	{
		if (ParentWidget->GetComponentByInterface(InterfaceClass))
		{
			return ParentWidget;
		}
		ParentWidget = ParentWidget->GetParent();
	}
	return nullptr;
}

#pragma region CallEvent
void ULexEventSystem::ExecuteEvent_OnPointerEnter(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData)
{
	PointerEventData->EventType = ELexUIPointerEventType::Enter;
	for (auto& Comp : TargetWidget->GetAllComponents())
	{
		if (Comp->GetClass()->ImplementsInterface(ULexPointerEnterExitInterface::StaticClass()))
		{
			ILexPointerEnterExitInterface::Execute_OnPointerEnter(Comp, PointerEventData);
		}
	}
}
void ULexEventSystem::ExecuteEvent_OnPointerExit(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData)
{
	PointerEventData->EventType = ELexUIPointerEventType::Exit;
	for (auto& Comp : TargetWidget->GetAllComponents())
	{
		if (Comp->GetClass()->ImplementsInterface(ULexPointerEnterExitInterface::StaticClass()))
		{
			ILexPointerEnterExitInterface::Execute_OnPointerExit(Comp, PointerEventData);
		}
	}
}
void ULexEventSystem::ExecuteEvent_OnPointerDown(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Down; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDownUpInterface::StaticClass(),
		ILexPointerDownUpInterface::Execute_OnPointerDown, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerUp(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Up; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDownUpInterface::StaticClass(),
		ILexPointerDownUpInterface::Execute_OnPointerUp, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerClick(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Click; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerClickInterface::StaticClass(),
		ILexPointerClickInterface::Execute_OnPointerClick, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerBeginDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::BeginDrag; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDragInterface::StaticClass(),
		ILexPointerDragInterface::Execute_OnPointerBeginDrag, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Drag; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDragInterface::StaticClass(),
		ILexPointerDragInterface::Execute_OnPointerDrag, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerEndDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::EndDrag; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDragInterface::StaticClass(),
		ILexPointerDragInterface::Execute_OnPointerEndDrag, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerScroll(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Scroll; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerScrollInterface::StaticClass(),
		ILexPointerScrollInterface::Execute_OnPointerScroll, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnPointerDrop(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp)
{
	PointerEventData->EventType = ELexUIPointerEventType::Drop; 
	ExecuteLexUIInterface(TargetWidget,
		PointerEventData,
		ULexPointerDropInterface::StaticClass(),
		ILexPointerDropInterface::Execute_OnPointerDrop, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnSelect(ULexWidget* TargetWidget, ULexBaseEventData* EventData, bool AllowEventBubbleUp)
{
	EventData->EventType = ELexUIPointerEventType::Select; 
	ExecuteLexUIInterface(TargetWidget,
		EventData,
		ULexSelectDeselectInterface::StaticClass(),
		ILexSelectDeselectInterface::Execute_OnSelect, AllowEventBubbleUp);
}
void ULexEventSystem::ExecuteEvent_OnDeselect(ULexWidget* TargetWidget, ULexBaseEventData* EventData, bool AllowEventBubbleUp)
{
	EventData->EventType = ELexUIPointerEventType::Deselect; 
	ExecuteLexUIInterface(TargetWidget,
		EventData,
		ULexSelectDeselectInterface::StaticClass(),
		ILexSelectDeselectInterface::Execute_OnDeselect, AllowEventBubbleUp);
}


void ULexEventSystem::CallOnPointerEnter(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerEnter(TargetWidget, EventData);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerExit(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerExit(TargetWidget, EventData);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerDown(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerDown(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerUp(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerUp(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerClick(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerClick(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerBeginDrag(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerBeginDrag(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerDrag(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerDrag(TargetWidget, EventData, true);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnPointerEndDrag(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerEndDrag(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}

void ULexEventSystem::CallOnPointerScroll(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerScroll(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}

void ULexEventSystem::CallOnPointerDrop(ULexWidget* TargetWidget, ULexPointerEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnPointerDrop(TargetWidget, EventData, true);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}

void ULexEventSystem::CallOnSelect(ULexWidget* TargetWidget, ULexBaseEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnSelect(TargetWidget, EventData, false);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
void ULexEventSystem::CallOnDeselect(ULexWidget* TargetWidget, ULexBaseEventData* EventData)
{
	LogEventData(EventData);
	ExecuteEvent_OnDeselect(TargetWidget, EventData, false);
	InputEvent.Broadcast(EventData);
	InputEventBP.Broadcast(EventData);
}
#pragma endregion

#undef LOCTEXT_NAMESPACE