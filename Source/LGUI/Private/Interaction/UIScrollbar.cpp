// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIScrollbar.h"

#include "LGUI.h"
#include "Core/Components/LexWidget.h"

UUIScrollbar::UUIScrollbar()
{
}

void UUIScrollbar::Awake()
{
    Super::Awake();
}

void UUIScrollbar::Start()
{
    Super::Start();
    ApplyValueToVisual();
}

bool UUIScrollbar::CheckHandle()
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
void UUIScrollbar::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    HandleArea = nullptr;//force re-check
    ApplyValueToVisual();
}
#endif

void UUIScrollbar::OnEnable()
{
    Super::OnEnable();
    ApplyValueToVisual();
}
void UUIScrollbar::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
    Super::OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
    ApplyValueToVisual();
}

void UUIScrollbar::SetValue(float InValue, bool FireEvent)
{
    if (Value != InValue)
    {
        InValue = FMath::Clamp(InValue, 0.0f, 1.0f);
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

void UUIScrollbar::SetValue(float InValue)
{
    SetValue(InValue, true);
}

void UUIScrollbar::SetValueWithoutNotify(float InValue)
{
    SetValue(InValue, false);
}

void UUIScrollbar::SetSize(float InSize)
{
    if (Size != InSize)
    {
        InSize = FMath::Clamp(InSize, 0.0f, 1.0f);
        Size = InSize;
        ApplyValueToVisual();
    }
}
void UUIScrollbar::SetValueAndSize(float InValue, float InSize, bool FireEvent)
{
    bool somethingChanged = false;
    if (Value != InValue)
    {
        InValue = FMath::Clamp(InValue, 0.0f, 1.0f);
        Value = InValue;
        somethingChanged = true;
    }
    if (Size != InSize)
    {
        InSize = FMath::Clamp(InSize, 0.0f, 1.0f);
        Size = InSize;
        somethingChanged = true;
    }
    if (somethingChanged)
    {
        ApplyValueToVisual();
        if (FireEvent)
        {
            OnValueChangedCPP.Broadcast(Value);
            OnValueChangedED.FireEvent((double)Value);
        }
    }
}
void UUIScrollbar::SetNavigationChangeInterval(float InValue)
{
    NavigationChangeInterval = InValue;
}

bool UUIScrollbar::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
    Super::OnPointerDown_Implementation(EventData);
    if (EventData->InputType == ELexUIPointerInputType::Pointer)
    {
        if (CheckHandle())
        {
            if (EventData->EnterWidget != Handle)
            {
                const auto& pointerInHandleAreaSpace = HandleArea->GetWorldTransform().InverseTransformPosition(EventData->WorldPoint);
                float value01 = Value;
                switch (DirectionType)
                {
                case EUIScrollbarDirectionType::LeftToRight:
                {
                    float validSpace = HandleArea->GetWidth() * (1.0f - Size);
                    float valueDiff01;
                    if (pointerInHandleAreaSpace.Y > Handle->GetRelativeLocation().Y)
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceRight() + Handle->GetRelativeLocation().Y)) / validSpace;
                    }
                    else
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceLeft() + Handle->GetRelativeLocation().Y)) / validSpace;
                    }
                    value01 += valueDiff01;
                }
                break;
                case EUIScrollbarDirectionType::RightToLeft:
                {
                    float validSpace = HandleArea->GetWidth() * (1.0f - Size);
                    float valueDiff01;
                    if (pointerInHandleAreaSpace.Y > Handle->GetRelativeLocation().Y)
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceRight() + Handle->GetRelativeLocation().Y)) / validSpace;
                    }
                    else
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceLeft() + Handle->GetRelativeLocation().Y)) / validSpace;
                    }
                    value01 -= valueDiff01;
                }
                break;
                case EUIScrollbarDirectionType::BottomToTop:
                {
                    float validSpace = HandleArea->GetHeight() * (1.0f - Size);
                    float valueDiff01;
                    if (pointerInHandleAreaSpace.Z > Handle->GetRelativeLocation().Z)
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Z - (Handle->GetLocalSpaceTop() + Handle->GetRelativeLocation().Z)) / validSpace;
                    }
                    else
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Z - (Handle->GetLocalSpaceBottom() + Handle->GetRelativeLocation().Z)) / validSpace;
                    }
                    value01 += valueDiff01;
                }
                break;
                case EUIScrollbarDirectionType::TopToBottom:
                {
                    float validSpace = HandleArea->GetHeight() * (1.0f - Size);
                    float valueDiff01;
                    if (pointerInHandleAreaSpace.Z > Handle->GetRelativeLocation().Z)
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Z - (Handle->GetLocalSpaceTop() + Handle->GetRelativeLocation().Z)) / validSpace;
                    }
                    else
                    {
                        valueDiff01 = (pointerInHandleAreaSpace.Z - (Handle->GetLocalSpaceBottom() + Handle->GetRelativeLocation().Z)) / validSpace;
                    }
                    value01 -= valueDiff01;
                }
                break;
                }
                value01 = FMath::Clamp(value01, 0.0f, 1.0f);
                SetValue(value01, true);
            }
        }
    }
    return AllowEventBubbleUp;
}
bool UUIScrollbar::OnPointerUp_Implementation(ULexPointerEventData *EventData)
{
    Super::OnPointerUp_Implementation(EventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbar::OnPointerBeginDrag_Implementation(ULexPointerEventData *EventData)
{
    PressValue = Value;
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbar::OnPointerDrag_Implementation(ULexPointerEventData *EventData)
{
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbar::OnPointerEndDrag_Implementation(ULexPointerEventData *EventData)
{
    CalculateInputValue(EventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbar::OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)
{
    float valueIntervalMultiply = 0.0f;
    if (
        (DirectionType == EUIScrollbarDirectionType::LeftToRight && direction == ELexUINavigationDirection::Left) || (DirectionType == EUIScrollbarDirectionType::RightToLeft && direction == ELexUINavigationDirection::Right) || (DirectionType == EUIScrollbarDirectionType::BottomToTop && direction == ELexUINavigationDirection::Down) || (DirectionType == EUIScrollbarDirectionType::TopToBottom && direction == ELexUINavigationDirection::Up))
    {
        valueIntervalMultiply = -NavigationChangeInterval;
    }
    else if (
        (DirectionType == EUIScrollbarDirectionType::LeftToRight && direction == ELexUINavigationDirection::Right) || (DirectionType == EUIScrollbarDirectionType::RightToLeft && direction == ELexUINavigationDirection::Left) || (DirectionType == EUIScrollbarDirectionType::BottomToTop && direction == ELexUINavigationDirection::Up) || (DirectionType == EUIScrollbarDirectionType::TopToBottom && direction == ELexUINavigationDirection::Down))
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
        tempValue += valueIntervalMultiply;
        tempValue = FMath::Clamp(tempValue, 0.0f, 1.0f);
        SetValue(tempValue);
        return false;
    }
}

void UUIScrollbar::CalculateInputValue(ULexPointerEventData *EventData)
{
    if (CheckHandle())
    {
        auto localCumulativeMoveDelta = EventData->PressWorldToLocalTransform.TransformVector(EventData->GetWorldPointInPlane() - EventData->PressWorldPoint);
        localCumulativeMoveDelta.X = 0;
        float slideAreaSize = 0;
        float handleSize = 0;
        float value01 = Value;
        switch (DirectionType)
        {
        case EUIScrollbarDirectionType::LeftToRight:
        {
            handleSize = HandleArea->GetWidth() * Size;
            slideAreaSize = HandleArea->GetWidth() - handleSize;
            value01 = PressValue + localCumulativeMoveDelta.Y / slideAreaSize;
        }
        break;
        case EUIScrollbarDirectionType::RightToLeft:
        {
            handleSize = HandleArea->GetWidth() * Size;
            slideAreaSize = HandleArea->GetWidth() - handleSize;
            value01 = PressValue - localCumulativeMoveDelta.Y / slideAreaSize;
        }
        break;
        case EUIScrollbarDirectionType::BottomToTop:
        {
            handleSize = HandleArea->GetHeight() * Size;
            slideAreaSize = HandleArea->GetHeight() - handleSize;
            value01 = PressValue + localCumulativeMoveDelta.Z / slideAreaSize;
        }
        break;
        case EUIScrollbarDirectionType::TopToBottom:
        {
            handleSize = HandleArea->GetHeight() * Size;
            slideAreaSize = HandleArea->GetHeight() - handleSize;
            value01 = PressValue - localCumulativeMoveDelta.Z / slideAreaSize;
        }
        break;
        }
        value01 = FMath::Clamp(value01, 0.0f, 1.0f);
        SetValue(value01, true);
    }
}
void UUIScrollbar::ApplyValueToVisual()
{
    if (CheckHandle())
    {
        float value01 = Value;
        switch (DirectionType)
        {
        case EUIScrollbarDirectionType::LeftToRight:
        {
            auto HorizontalMinMax = FVector2D((1.0f - Size) * value01, FMath::Lerp(Size, 1.0f, value01));
            Handle->SetHorizontalAnchorMinMax(HorizontalMinMax);
        }
        break;
        case EUIScrollbarDirectionType::RightToLeft:
        {
            auto HorizontalMinMax = FVector2D((1.0f - Size) * (1.0f - value01), FMath::Lerp(1.0f, Size, value01));
            Handle->SetHorizontalAnchorMinMax(HorizontalMinMax);
        }
        break;
        case EUIScrollbarDirectionType::BottomToTop:
        {
            auto VerticalMinMax = FVector2D((1.0f - Size) * value01, FMath::Lerp(Size, 1.0f, value01));
            Handle->SetVerticalAnchorMinMax(VerticalMinMax);
        }
        break;
        case EUIScrollbarDirectionType::TopToBottom:
        {
            auto VerticalMinMax = FVector2D((1.0f - Size) * (1.0f - value01), FMath::Lerp(1.0f, Size, value01));
            Handle->SetVerticalAnchorMinMax(VerticalMinMax);
        }
        break;
        }
    }
}