// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_StandaloneInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"
#include "Engine/GameViewportClient.h"

void ULGUI_StandaloneInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	auto leftButtonEventData = eventSystem->GetPointerEventData(0, true);
	switch (leftButtonEventData->inputType)
	{
	default:
	case ELGUIPointerInputType::Pointer:
	{
		auto viewport = this->GetWorld()->GetGameViewport();
		if (viewport == nullptr)return;
		FVector2D mousePos;
		if (!viewport->GetMousePosition(mousePos))return;
		leftButtonEventData->pointerPosition = FVector(mousePos, 0);

		FHitResultContainerStruct hitResultContainer;
		bool lineTraceHitSomething = LineTrace(leftButtonEventData, hitResultContainer);
		bool resultHitSomething = false;
		FHitResult hitResult;
		ProcessPointerEvent(leftButtonEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

		auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
		eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
	}
	break;
	case ELGUIPointerInputType::Navigation:
	{
		ProcessInputForNavigation();
	}
	break;
	}
}
void ULGUI_StandaloneInputModule::InputScroll(const float& inAxisValue)
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

void ULGUI_StandaloneInputModule::InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	if (eventData->inputType != ELGUIPointerInputType::Pointer)
	{
		eventData->inputType = ELGUIPointerInputType::Pointer;
		if (inputChangeDelegate.IsBound())
		{
			inputChangeDelegate.Broadcast(eventData->inputType);
		}
	}
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
