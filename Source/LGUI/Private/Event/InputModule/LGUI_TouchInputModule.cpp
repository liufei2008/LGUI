﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_TouchInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"

void ULGUI_TouchInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	for (auto keyValue : eventSystem->pointerEventDataMap)
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
void ULGUI_TouchInputModule::InputScroll(const float& inAxisValue)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	if (IsValid(eventData->enterComponent))
	{
		if (inAxisValue != 0 || eventData->scrollAxisValue != inAxisValue)
		{
			eventData->scrollAxisValue = inAxisValue;
			if (CheckEventSystem())
			{
				eventSystem->CallOnPointerScroll(eventData->enterComponent, eventData, eventData->enterComponentEventFireType);
			}
		}
	}
}

void ULGUI_TouchInputModule::InputTouchTrigger(bool inTouchPress, int inTouchID, const FVector& inTouchPointPosition)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(inTouchID, true);
	eventData->nowIsTriggerPressed = inTouchPress;
	eventData->pointerPosition = inTouchPointPosition;
}

void ULGUI_TouchInputModule::InputTouchMoved(int inTouchID, const FVector& inTouchPointPosition)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(inTouchID, true);
	eventData->pointerPosition = inTouchPointPosition;
}
