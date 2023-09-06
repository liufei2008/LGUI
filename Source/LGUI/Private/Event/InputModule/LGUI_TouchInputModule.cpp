// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_TouchInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"

void ULGUI_TouchInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	for (auto& keyValue : eventSystem->pointerEventDataMap)
	{
		auto& eventData = keyValue.Value;
		switch (eventData->inputType)
		{
		default:
		case ELGUIPointerInputType::Pointer:
		{
			if (IsValid(eventData))
			{
				if (eventData->nowIsTriggerPressed || eventData->prevIsTriggerPressed)
				{
					FLGUIHitResult LGUIHitResult;
					bool lineTraceHitSomething = LineTrace(eventData, LGUIHitResult);
					bool resultHitSomething = false;
					FHitResult hitResult;
					ProcessPointerEvent(eventSystem, eventData, lineTraceHitSomething, LGUIHitResult, resultHitSomething, hitResult);

					auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
					eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
				}
			}
		}
		break;
		case ELGUIPointerInputType::Navigation:
		{
			ProcessInputForNavigation(eventData);
		}
		break;
		}
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
