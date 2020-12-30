// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_TouchInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"

void ULGUI_TouchInputModule::ProcessInput()
{
	auto eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance();
	if (eventSystem == nullptr)return;

	for (auto keyValue : pointerEventDataMap)
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
	auto eventData = GetPointerEventData(0, true);
	if (IsValid(eventData->enterComponent))
	{
		if (inAxisValue != eventData->scrollAxisValue)
		{
			eventData->scrollAxisValue = inAxisValue;
			if (auto eventSystem = ULGUIEventSystem::GetLGUIEventSystemInstance())
			{
				eventSystem->CallOnPointerScroll(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
			}
		}
	}
}

void ULGUI_TouchInputModule::InputTouchTrigger(bool inTouchPress, int inTouchID, const FVector& inTouchPointPosition)
{
	auto eventData = GetPointerEventData(inTouchID, true);
	eventData->nowIsTriggerPressed = inTouchPress;
	eventData->pointerPosition = inTouchPointPosition;
}

void ULGUI_TouchInputModule::InputTouchMoved(int inTouchID, const FVector& inTouchPointPosition)
{
	auto eventData = GetPointerEventData(inTouchID, true);
	eventData->pointerPosition = inTouchPointPosition;
}
