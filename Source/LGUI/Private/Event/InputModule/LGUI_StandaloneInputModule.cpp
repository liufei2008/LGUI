// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_StandaloneInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"

void ULGUI_StandaloneInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	auto leftButtonEventData = GetPointerEventData(0, true);
	auto viewport = this->GetWorld()->GetGameViewport();
	if (viewport == nullptr)return;
	FVector2D mousePos;
	viewport->GetMousePosition(mousePos);
	leftButtonEventData->pointerPosition = FVector(mousePos, 0);

	FHitResultContainerStruct hitResultContainer;
	bool lineTraceHitSomething = LineTrace(leftButtonEventData, hitResultContainer);
	bool resultHitSomething = false;
	FHitResult hitResult;
	ProcessPointerEvent(leftButtonEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

	auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
	eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
}
void ULGUI_StandaloneInputModule::InputScroll(const float& inAxisValue)
{
	auto eventData = GetPointerEventData(0, true);
	if (IsValid(eventData->enterComponent))
	{
		if (inAxisValue != 0 || eventData->scrollAxisValue != inAxisValue)
		{
			eventData->scrollAxisValue = inAxisValue;
			if (CheckEventSystem())
			{
				eventSystem->CallOnPointerScroll(eventData->enterComponent, eventData, eventData->enterComponentEventFireOnAllOrOnlyTarget);
			}
		}
	}
}

void ULGUI_StandaloneInputModule::InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType)
{
	auto eventData = GetPointerEventData(0, true);
	eventData->nowIsTriggerPressed = inTriggerPress;
	if (eventData->nowIsTriggerPressed)
	{
		if (eventData->prevPressTriggerType != inMouseButtonType)
		{
			//clear event, because input trigger change
			ClearEventByID(eventData->pointerID);
			eventData->prevPressTriggerType = inMouseButtonType;
		}
	}
	eventData->mouseButtonType = inMouseButtonType;
}
