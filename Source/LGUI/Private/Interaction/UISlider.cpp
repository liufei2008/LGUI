// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UISlider.h"
#include "Core/Components/LexWidget.h"

void UUISlider::Awake()
{
    Super::Awake();
}

void UUISlider::Start()
{
    Super::Start();
    ApplyValueToVisual();
}

bool UUISlider::CheckFill()
{
    if (Fill.IsValid() && FillArea.IsValid())
        return true;
    if (!Fill.IsValid())
        return false;
    FillArea = Fill->GetParent();
    if (Fill.IsValid() && FillArea.IsValid())
        return true;
    return false;
}
bool UUISlider::CheckHandle()
{
    if (Handle.IsValid() && HandleArea.IsValid())
        return true;
    if (!Handle.IsValid())
        return false;
    HandleArea = Handle->GetParent();
    if (HandleArea.IsValid())
        return true;
    return false;
}

#if WITH_EDITOR
void UUISlider::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (WholeNumbers)
    {
        Value = FMath::FloorToFloat(Value);
    }
    Value = FMath::Clamp(Value, MinValue, MaxValue);
    HandleArea = nullptr;//force re-check
    FillArea = nullptr;//force re-check
    ApplyValueToVisual();
}
#endif

void UUISlider::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
    Super::OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
    ApplyValueToVisual();
}

void UUISlider::SetValue(float InValue, bool FireEvent)
{
    InValue = FMath::Clamp(InValue, MinValue, MaxValue);
    if (Value != InValue)
    {
        Value = InValue;
        ApplyValueToVisual();
        if (FireEvent)
        {
            OnValueChangedCPP.Broadcast(Value);
            OnValueChanged.Broadcast(Value);
            OnValueChangedED.FireEvent((double)Value);
        }
    }
}

void UUISlider::SetValue(float InValue)
{
    SetValue(InValue, true);
}

void UUISlider::SetValueWithoutNotify(float InValue)
{
    SetValue(InValue, false);
}

void UUISlider::SetMinValue(float InMinValue, bool KeepRelativeValue, bool FireEvent)
{
    if (MinValue != InMinValue)
    {
		float value01 = (Value - MinValue) / (MaxValue - MinValue);
		MinValue = InMinValue;
        if (KeepRelativeValue)
        {
            Value = value01 * (MaxValue - MinValue) + MinValue;
        }
        else
        {
            Value = FMath::Clamp(Value, MinValue, MaxValue);
        }
        ApplyValueToVisual();
		if (FireEvent)
		{
			OnValueChangedCPP.Broadcast(Value);
			OnValueChangedED.FireEvent((double)Value);
		}
    }
}
void UUISlider::SetMaxValue(float InMaxValue, bool KeepRelativeValue, bool FireEvent)
{
	if (MaxValue != InMaxValue)
	{
		float value01 = (Value - MinValue) / (MaxValue - MinValue);
        MaxValue = InMaxValue;
		if (KeepRelativeValue)
		{
			Value = value01 * (MaxValue - MinValue) + MinValue;
		}
		else
		{
			Value = FMath::Clamp(Value, MinValue, MaxValue);
		}
		ApplyValueToVisual();
		if (FireEvent)
		{
			OnValueChangedCPP.Broadcast(Value);
			OnValueChangedED.FireEvent((double)Value);
		}
	}
}

void UUISlider::SetNavigationChangeInterval(float InValue)
{
    NavigationChangeInterval = InValue;
}

bool UUISlider::OnPointerDown_Implementation(ULexPointerEventData *EventData)
{
    Super::OnPointerDown_Implementation(EventData);
    if (EventData->InputType == ELexUIPointerInputType::Pointer)
    {
        CalculateInputValue(EventData);
    }
    return AllowEventBubbleUp;
}
bool UUISlider::OnPointerUp_Implementation(ULexPointerEventData *EventData)
{
    Super::OnPointerUp_Implementation(EventData);
    return AllowEventBubbleUp;
}
bool UUISlider::OnPointerBeginDrag_Implementation(ULexPointerEventData *EventData)
{
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUISlider::OnPointerDrag_Implementation(ULexPointerEventData *EventData)
{
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUISlider::OnPointerEndDrag_Implementation(ULexPointerEventData *EventData)
{
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUISlider::OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)
{
    float valueIntervalMultiply = 0.0f;
    if (
        (DirectionType == EUISliderDirectionType::LeftToRight && direction == ELexUINavigationDirection::Left)
        || (DirectionType == EUISliderDirectionType::RightToLeft && direction == ELexUINavigationDirection::Right)
        || (DirectionType == EUISliderDirectionType::BottomToTop && direction == ELexUINavigationDirection::Down)
        || (DirectionType == EUISliderDirectionType::TopToBottom && direction == ELexUINavigationDirection::Up))
    {
        valueIntervalMultiply = -NavigationChangeInterval;
    }
    else if (
        (DirectionType == EUISliderDirectionType::LeftToRight && direction == ELexUINavigationDirection::Right)
        || (DirectionType == EUISliderDirectionType::RightToLeft && direction == ELexUINavigationDirection::Left)
        || (DirectionType == EUISliderDirectionType::BottomToTop && direction == ELexUINavigationDirection::Up)
        || (DirectionType == EUISliderDirectionType::TopToBottom && direction == ELexUINavigationDirection::Down))
    {
        valueIntervalMultiply = NavigationChangeInterval;
    }
    if (valueIntervalMultiply == 0.0f)
    {
        return Super::OnNavigate_Implementation(direction, result);
    }
    else
    {
        auto tempValue = Value;
        tempValue += (MaxValue - MinValue) * valueIntervalMultiply;
        tempValue = FMath::Clamp(tempValue, MinValue, MaxValue);
        SetValue(tempValue);
        return false;
    }
}

void UUISlider::CalculateInputValue(ULexPointerEventData *EventData)
{
    ULexWidget *MainWidget = nullptr;
    ULexWidget *AreaWidget = nullptr;
    if (CheckHandle())
    {
        MainWidget = Handle.Get();
        AreaWidget = HandleArea.Get();
    }
    else
    {
        if (CheckFill())
        {
            MainWidget = Fill.Get();
            AreaWidget = FillArea.Get();
        }
    }
    if (MainWidget != nullptr && AreaWidget != nullptr)
    {
        //calculate value to 0-1 range
        auto localPointerPosition = AreaWidget->GetWorldTransform().InverseTransformPosition(EventData->GetWorldPointInPlane());
        float MinPosition = 0;
        float value01 = 0;
        switch (DirectionType)
        {
        case EUISliderDirectionType::LeftToRight:
        {
            MinPosition = -AreaWidget->GetPivot().X * AreaWidget->GetWidth();
            value01 = (localPointerPosition.Y - MinPosition) / AreaWidget->GetWidth();
        }
        break;
        case EUISliderDirectionType::RightToLeft:
        {
            MinPosition = -AreaWidget->GetPivot().X * AreaWidget->GetWidth();
            value01 = 1.0f - (localPointerPosition.Y - MinPosition) / AreaWidget->GetWidth();
        }
        break;
        case EUISliderDirectionType::BottomToTop:
        {
            MinPosition = -AreaWidget->GetPivot().Y * AreaWidget->GetHeight();
            value01 = (localPointerPosition.Z - MinPosition) / AreaWidget->GetHeight();
        }
        break;
        case EUISliderDirectionType::TopToBottom:
        {
            MinPosition = -AreaWidget->GetPivot().Y * AreaWidget->GetHeight();
            value01 = 1.0f - (localPointerPosition.Z - MinPosition) / AreaWidget->GetHeight();
        }
        break;
        }
        value01 = FMath::Clamp(value01, 0.0f, 1.0f);
        float value = (MaxValue - MinValue) * value01 + MinValue;
        if (WholeNumbers)
        {
            value = FMath::FloorToFloat(value);
        }
        SetValue(value, true);
    }
}
void UUISlider::ApplyValueToVisual()
{
    float value01 = (Value - MinValue) / (MaxValue - MinValue);
    value01 = FMath::Clamp(value01, 0.0f, 1.0f);

    if (CheckHandle() || CheckFill())
    {
        switch (DirectionType)
        {
        case EUISliderDirectionType::LeftToRight:
        {
            if (CheckHandle())
            {
                Handle->SetHorizontalAnchorMinMax(FVector2D(value01, value01));
            }
            if (CheckFill())
            {
                Fill->SetHorizontalAnchorMinMax(FVector2D(0, value01));
            }
        }
        break;
        case EUISliderDirectionType::RightToLeft:
        {
            if (CheckHandle())
            {
                float invValue01 = 1.0f - value01;
                Handle->SetHorizontalAnchorMinMax(FVector2D(invValue01, invValue01));
            }
            if (CheckFill())
            {
                Fill->SetHorizontalAnchorMinMax(FVector2D(1.0f - value01, 1));
            }
        }
        break;
        case EUISliderDirectionType::BottomToTop:
        {
            if (CheckHandle())
            {
                Handle->SetVerticalAnchorMinMax(FVector2D(value01, value01));
            }
            if (CheckFill())
            {
                Fill->SetVerticalAnchorMinMax(FVector2D(0, value01));
            }
        }
        break;
        case EUISliderDirectionType::TopToBottom:
        {
            if (CheckHandle())
            {
                float invValue01 = 1.0f - value01;
                Handle->SetVerticalAnchorMinMax(FVector2D(invValue01, invValue01));
            }
            if (CheckFill())
            {
                Fill->SetVerticalAnchorMinMax(FVector2D(1.0f - value01, 1));
            }
        }
        break;
        }
    }
}