// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
		if (standaloneInputDataArray.Num() == 0)
		{
			FVector2D mousePos;
			if (GetMousePosition(mousePos))
			{
				leftButtonEventData->pointerPosition = FVector(mousePos, 0);

				FHitResultContainerStruct hitResultContainer;
				bool lineTraceHitSomething = LineTrace(leftButtonEventData, hitResultContainer);
				bool resultHitSomething = false;
				FHitResult hitResult;
				ProcessPointerEvent(leftButtonEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

				auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
				eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
			}
		}
		else
		{
			for (auto inputData : standaloneInputDataArray)//handle multiple click in one frame
			{
				leftButtonEventData->pointerPosition = FVector(inputData.mousePosition, 0);
				leftButtonEventData->nowIsTriggerPressed = inputData.triggerPress;
				if (inputData.triggerPress)
				{
					leftButtonEventData->pressTime = inputData.pressTime;
				}
				else
				{
					leftButtonEventData->releaseTime = inputData.releaseTime;
				}
				leftButtonEventData->mouseButtonType = inputData.mouseButtonType;

				FHitResultContainerStruct hitResultContainer;
				bool lineTraceHitSomething = LineTrace(leftButtonEventData, hitResultContainer);
				bool resultHitSomething = false;
				FHitResult hitResult;
				ProcessPointerEvent(leftButtonEventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

				auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
				eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
			}
			standaloneInputDataArray.Reset();
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
		standaloneInputDataArray.Reset();//input type change, clear cached input data
		if (inputChangeDelegate.IsBound())
		{
			inputChangeDelegate.Broadcast(eventData->inputType);
		}
	}

	StandaloneInputData inputData;
	inputData.mouseButtonType = inMouseButtonType;
	inputData.triggerPress = inTriggerPress;
	if (inTriggerPress)
	{
		inputData.pressTime = GetWorld()->TimeSeconds;
	}
	else
	{
		inputData.releaseTime = GetWorld()->TimeSeconds;
	}
	if (bOverrideMousePosition)
	{
		inputData.mousePosition = overrideMousePosition;
		standaloneInputDataArray.Add(inputData);
	}
	else
	{
		FVector2D mousePos;
		if (GetMousePosition(mousePos))
		{
			inputData.mousePosition = mousePos;
			standaloneInputDataArray.Add(inputData);
		}
	}
}
bool ULGUI_StandaloneInputModule::GetMousePosition(FVector2D& OutMousePos)
{
	if (auto viewport = this->GetWorld()->GetGameViewport())
	{
		return viewport->GetMousePosition(OutMousePos);
	}
	return false;
}

void ULGUI_StandaloneInputModule::InputOverrideMousePosition(const FVector2D& inMousePosition)
{
	if (!bOverrideMousePosition)
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUI_StandaloneInputModule::InputOverrideMousePosition]Check parameter bOverrideMousePosition if you need to use custom mouse position. Or custom mouse position will not work!"));
		return;
	}
	if (!CheckEventSystem())return;

	overrideMousePosition = inMousePosition;
}
