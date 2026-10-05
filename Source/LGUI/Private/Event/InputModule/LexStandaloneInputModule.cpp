// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Event/InputModule/LexStandaloneInputModule.h"
#include "LGUI.h"
#include "Event/LexEventSystem.h"
#include "Event/LexPointerEventData.h"
#include "Engine/GameViewportClient.h"

void ULexStandaloneInputModule::ProcessInput()
{
	if (!EventSystem.IsValid())return;

	if (StandaloneInputDataArray.Num() > 0)
	{
		for (auto& InputData : StandaloneInputDataArray)//handle multiple click in one frame
		{
			auto EventData = EventSystem->GetPointerEventData(InputData.PointerID, true);
			EventData->PointerPosition = InputData.PointerPosition;
			EventData->bNowIsTriggerPressed = InputData.bTriggerPress;
			if (InputData.bTriggerPress)
			{
				EventData->PressTime = InputData.PressTime;
				EventData->PressPointerPosition = InputData.PointerPosition;
			}
			else
			{
				EventData->ReleaseTime = InputData.ReleaseTime;
			}
			EventData->MouseButtonType = InputData.MouseButtonType;

			FLexUIHitResultContainer LexHitResult;
			bool bLineTraceHitSomething = LineTrace(EventData, LexHitResult);
			bool bResultHitSomething = false;
			FLexUIHitResult HitResult;
			ProcessPointerEvent(EventSystem.Get(), EventData, bLineTraceHitSomething, LexHitResult, bResultHitSomething, HitResult);

			auto TempHitComp = HitResult.Widget.Get();
			EventSystem->RaiseHitEvent(bResultHitSomething, HitResult, TempHitComp);
		}
		StandaloneInputDataArray.Reset();
	}
	else
	{
		for (auto& keyValue : EventSystem->GetPointerEventDataMap())
		{
			auto& EventData = keyValue.Value;
			switch (EventData->InputType)
			{
			default:
			case ELexUIPointerInputType::Pointer:
				{
					FLexUIHitResultContainer LexHitResult;
					bool bLineTraceHitSomething = LineTrace(EventData, LexHitResult);
					bool bResultHitSomething = false;
					FLexUIHitResult HitResult;
					ProcessPointerEvent(EventSystem.Get(), EventData, bLineTraceHitSomething, LexHitResult, bResultHitSomething, HitResult);

					auto TempHitComp = HitResult.Widget.Get();
					EventSystem->RaiseHitEvent(bResultHitSomething, HitResult, TempHitComp);
				}
				break;
			case ELexUIPointerInputType::Navigation:
				{
					ProcessInputForNavigation(EventData);
				}
				break;
			}
		}
	}
}
void ULexStandaloneInputModule::InputScroll(const FVector2D& InAxisValue)
{
	if (!EventSystem.IsValid())return;

	auto EventData = EventSystem->GetPointerEventData(0, true);
	if (IsValid(EventData->EnterWidget))
	{
		if (InAxisValue != FVector2D::ZeroVector || EventData->ScrollAxisValue != InAxisValue)
		{
			EventData->ScrollAxisValue = InAxisValue;
			EventSystem->CallOnPointerScroll(EventData->EnterWidget, EventData);
		}
	}
}

void ULexStandaloneInputModule::InputTrigger(const FVector& InMousePosition, bool InTriggerPress, ELexUIMouseButtonType InMouseButtonType)
{
	if (!EventSystem.IsValid())return;
	CommonInputTrigger(InMousePosition, InTriggerPress, 0, InMouseButtonType);
}
void ULexStandaloneInputModule::GetMousePosition(FVector2D& OutMousePos)const
{
	if (auto Viewport = this->GetWorld()->GetGameViewport())
	{
		Viewport->GetMousePosition(OutMousePos);
	}
}

void ULexStandaloneInputModule::CommonInputTrigger(const FVector& InPointerPosition, bool InTriggerPress,
	int InPointerID, ELexUIMouseButtonType InMouseButtonType)
{
	auto EventData = EventSystem->GetPointerEventData(InPointerID, true);
	if (EventSystem->SetPointerInputType(EventData, ELexUIPointerInputType::Pointer))
	{
		StandaloneInputDataArray.Reset();//input type change, clear cached input data
	}

	StandaloneInputData InputData;
	InputData.PointerID = InPointerID;
	InputData.MouseButtonType = InMouseButtonType;
	InputData.bTriggerPress = InTriggerPress;
	InputData.PointerPosition = InPointerPosition;

	if (InTriggerPress)
	{
		InputData.PressTime = GetWorld()->TimeSeconds;
	}
	else
	{
		InputData.ReleaseTime = GetWorld()->TimeSeconds;
	}
	StandaloneInputDataArray.Add(InputData);
}

void ULexStandaloneInputModule::InputMouseMove(const FVector& InMousePosition)
{
	auto EventData = EventSystem->GetPointerEventData(0, true);
	EventData->PointerPosition = InMousePosition;
}

void ULexStandaloneInputModule::InputTouchTrigger(bool InTouchPress, int InTouchID, const FVector& InTouchPointPosition)
{
	if (!EventSystem.IsValid())return;
	CommonInputTrigger(InTouchPointPosition, InTouchPress, InTouchID);
}

void ULexStandaloneInputModule::InputTouchMoved(int InTouchID, const FVector& InTouchPointPosition)
{
	if (!EventSystem.IsValid())return;
	
	auto EventData = EventSystem->GetPointerEventData(InTouchID, true);
	EventData->PointerPosition = InTouchPointPosition;
}

void ULexStandaloneInputModule::InputNavigation(ELexUINavigationDirection InDirection, bool InPressOrRelease, int InPointerID)
{
	auto EventData = EventSystem->GetPointerEventData(InPointerID, true);
	if (InPressOrRelease)
	{
		if (EventData->InputType != ELexUIPointerInputType::Navigation)//if we just activate navigation input, then no need to move selection to next
		{
			InDirection = ELexUINavigationDirection::None;
		}
		EventSystem->SetPointerInputType(EventData, ELexUIPointerInputType::Navigation);
		EventData->NavigateDirection = InDirection;
	}
	else
	{
		EventData->NavigateDirection = ELexUINavigationDirection::None;
	}
	EventData->NavigateTickTime = 0;
}
void ULexStandaloneInputModule::InputTriggerForNavigation(bool InTriggerPress, int InPointerID)
{
	auto EventData = EventSystem->GetPointerEventData(InPointerID, true);
	if (InTriggerPress)
	{
		EventSystem->SetPointerInputType(EventData, ELexUIPointerInputType::Navigation);
	}
	EventData->NavigateTickTime = 0;
	EventData->bNowIsTriggerPressed = InTriggerPress;
}
