// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_TouchInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"

void ULGUI_TouchInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	switch (eventData->inputType)
	{
	default:
	case ELGUIPointerInputType::Pointer:
	{
		for (auto& keyValue : eventSystem->pointerEventDataMap)
		{
			auto touchPointerEventData = keyValue.Value;
			if (IsValid(touchPointerEventData))
			{
				if (touchPointerEventData->nowIsTriggerPressed || touchPointerEventData->prevIsTriggerPressed)
				{
					FHitResultContainerStruct hitResultContainer;
					bool lineTraceHitSomething = LineTrace(touchPointerEventData, hitResultContainer);
					bool resultHitSomething = false;
					FHitResult hitResult;
					ProcessPointerEvent(touchPointerEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

					auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
					eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
				}
			}
		}
	}
	break;
	case ELGUIPointerInputType::Navigation:
	{
		ProcessInputForNavigation();
	}
	break;
	}
}
void ULGUI_TouchInputModule::InputScroll(const FVector2D& inAxisValue)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	if (IsValid(eventData->enterComponent))
	{
		if (inAxisValue != FVector2D::ZeroVector || eventData->scrollAxisValue != inAxisValue)
		{
			eventData->scrollAxisValue = inAxisValue;
			eventSystem->CallOnPointerScroll(eventData->enterComponent, eventData, eventData->enterComponentEventFireType);
		}
	}
}

void ULGUI_TouchInputModule::InputTouchTrigger(bool inTouchPress, int inTouchID, const FVector& inTouchPointPosition)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(inTouchID, true);
	eventSystem->SetPointerInputType(eventData, ELGUIPointerInputType::Pointer);
	eventData->nowIsTriggerPressed = inTouchPress;
	eventData->pointerPosition = inTouchPointPosition;
	if (inTouchPress)
	{
		eventData->pressPointerPosition = eventData->pointerPosition;
	}
}

void ULGUI_TouchInputModule::InputTouchMoved(int inTouchID, const FVector& inTouchPointPosition)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(inTouchID, true);
	eventSystem->SetPointerInputType(eventData, ELGUIPointerInputType::Pointer);
	eventData->pointerPosition = inTouchPointPosition;
}
