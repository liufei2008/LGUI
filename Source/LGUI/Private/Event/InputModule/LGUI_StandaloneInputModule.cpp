// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Event/InputModule/LGUI_StandaloneInputModule.h"
#include "LGUI.h"
#include "Event/LGUIEventSystem.h"
#include "Event/LGUIPointerEventData.h"
#include "Engine/GameViewportClient.h"

void ULGUI_StandaloneInputModule::ProcessInput()
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	switch (eventData->inputType)
	{
	default:
	case ELGUIPointerInputType::Pointer:
	{
		if (standaloneInputDataArray.Num() == 0)
		{
			FVector2D mousePos = FVector2D(eventData->pointerPosition);
			GetMousePosition(mousePos);
			eventData->pointerPosition = FVector(mousePos, 0);

			FHitResultContainerStruct hitResultContainer;
			bool lineTraceHitSomething = LineTrace(eventData, hitResultContainer);
			bool resultHitSomething = false;
			FHitResult hitResult;
			ProcessPointerEvent(eventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

			auto tempHitComp = (USceneComponent*)hitResult.Component.Get();
			eventSystem->RaiseHitEvent(resultHitSomething, hitResult, tempHitComp);
		}
		else
		{
			for (auto& inputData : standaloneInputDataArray)//handle multiple click in one frame
			{
				eventData->pointerPosition = FVector(inputData.mousePosition, 0);
				eventData->nowIsTriggerPressed = inputData.triggerPress;
				if (inputData.triggerPress)
				{
					eventData->pressTime = inputData.pressTime;
				}
				else
				{
					eventData->releaseTime = inputData.releaseTime;
				}
				eventData->mouseButtonType = inputData.mouseButtonType;

				FHitResultContainerStruct hitResultContainer;
				bool lineTraceHitSomething = LineTrace(eventData, hitResultContainer);
				bool resultHitSomething = false;
				FHitResult hitResult;
				ProcessPointerEvent(eventData, lineTraceHitSomething, hitResultContainer, resultHitSomething, hitResult);

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
void ULGUI_StandaloneInputModule::InputScroll(const FVector2D& inAxisValue)
{
	if (!CheckEventSystem())return;

	auto eventData = eventSystem->GetPointerEventData(0, true);
	if (IsValid(eventData->enterComponent))
	{
		if (inAxisValue != FVector2D::ZeroVector || eventData->scrollAxisValue != inAxisValue)
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
	if (eventSystem->SetPointerInputType(eventData, ELGUIPointerInputType::Pointer))
	{
		standaloneInputDataArray.Reset();//input type change, clear cached input data
	}

	StandaloneInputData inputData;
	inputData.mouseButtonType = inMouseButtonType;
	inputData.triggerPress = inTriggerPress;

	if (bOverrideMousePosition)
	{
		inputData.mousePosition = overrideMousePosition;
	}
	else
	{
		FVector2D mousePos = FVector2D(eventData->pointerPosition);
		GetMousePosition(mousePos);
		inputData.mousePosition = mousePos;
	}

	if (inTriggerPress)
	{
		inputData.pressTime = GetWorld()->TimeSeconds;
		eventData->pressPointerPosition = eventData->pointerPosition;
	}
	else
	{
		inputData.releaseTime = GetWorld()->TimeSeconds;
	}
	standaloneInputDataArray.Add(inputData);
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
