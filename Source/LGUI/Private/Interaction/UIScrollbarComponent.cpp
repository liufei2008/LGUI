// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollbarComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"

UUIScrollbarComponent::UUIScrollbarComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UUIScrollbarComponent::Awake()
{
    Super::Awake();
}

void UUIScrollbarComponent::Start()
{
    Super::Start();
    ApplyValueToUI();
}

bool UUIScrollbarComponent::CheckHandle()
{
    if (Handle.IsValid() && HandleArea.IsValid())
        return true;
    if (!HandleActor.IsValid())
        return false;
    if (!HandleActor->GetAttachParentActor())
        return false;
    Handle = HandleActor->FindComponentByClass<UUIItem>();
    HandleArea = HandleActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
    if (Handle.IsValid() && HandleArea.IsValid())
        return true;
    return false;
}

#if WITH_EDITOR
void UUIScrollbarComponent::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    ApplyValueToUI();
    Handle = nullptr;
    HandleArea = nullptr;
    if (CheckHandle())
    {
        Handle->EditorForceUpdate();
    }
}
#endif

void UUIScrollbarComponent::OnUIActiveInHierachy(bool ativeOrInactive)
{
    Super::OnUIActiveInHierachy(ativeOrInactive);
    ApplyValueToUI();
}
void UUIScrollbarComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    ApplyValueToUI();
}

void UUIScrollbarComponent::SetValue(float InValue, bool FireEvent)
{
    if (Value != InValue)
    {
        InValue = FMath::Clamp(InValue, 0.0f, 1.0f);
        Value = InValue;
        ApplyValueToUI();
        if (FireEvent)
        {
            OnValueChangeCPP.Broadcast(Value);
            OnValueChange.FireEvent(Value);
        }
    }
}
void UUIScrollbarComponent::SetSize(float InSize)
{
    if (Size != InSize)
    {
        InSize = FMath::Clamp(InSize, 0.0f, 1.0f);
        Size = InSize;
        ApplyValueToUI();
    }
}
void UUIScrollbarComponent::SetValueAndSize(float InValue, float InSize, bool FireEvent)
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
        ApplyValueToUI();
        if (FireEvent)
        {
            OnValueChangeCPP.Broadcast(Value);
            OnValueChange.FireEvent(Value);
        }
    }
}
FDelegateHandle UUIScrollbarComponent::RegisterSlideEvent(const FLGUIFloatDelegate &InDelegate)
{
    return OnValueChangeCPP.Add(InDelegate);
}
FDelegateHandle UUIScrollbarComponent::RegisterSlideEvent(const TFunction<void(float)> &InFunction)
{
    return OnValueChangeCPP.AddLambda(InFunction);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FDelegateHandle &InHandle)
{
    OnValueChangeCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUIScrollbarComponent::RegisterSlideEvent(const FLGUIScrollbarDynamicDelegate &InDelegate)
{
    auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](float InValue) {
        InDelegate.ExecuteIfBound(InValue);
    });
    return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FLGUIDelegateHandleWrapper &InDelegateHandle)
{
    OnValueChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

bool UUIScrollbarComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
    Super::OnPointerDown_Implementation(eventData);
    if (eventData->inputType == ELGUIPointerInputType::Pointer)
    {
        if (CheckHandle())
        {
            if (eventData->enterComponent != Handle)
            {
                const auto& pointerInHandleAreaSpace = HandleArea->GetComponentTransform().InverseTransformPosition(eventData->worldPoint);
                float value01 = Value;
                switch (DirectionType)
                {
                case UIScrollbarDirectionType::LeftToRight:
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
                case UIScrollbarDirectionType::RightToLeft:
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
                case UIScrollbarDirectionType::BottomToTop:
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
                case UIScrollbarDirectionType::TopToBottom:
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
bool UUIScrollbarComponent::OnPointerUp_Implementation(ULGUIPointerEventData *eventData)
{
    Super::OnPointerUp_Implementation(eventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData *eventData)
{
    PressValue = Value;
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerDrag_Implementation(ULGUIPointerEventData *eventData)
{
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData *eventData)
{
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnNavigate_Implementation(ELGUINavigationDirection direction, TScriptInterface<ILGUINavigationInterface>& result)
{
    float valueIntervalMultiply = 0.0f;
    if (
        (DirectionType == UIScrollbarDirectionType::LeftToRight && direction == ELGUINavigationDirection::Left) || (DirectionType == UIScrollbarDirectionType::RightToLeft && direction == ELGUINavigationDirection::Right) || (DirectionType == UIScrollbarDirectionType::BottomToTop && direction == ELGUINavigationDirection::Down) || (DirectionType == UIScrollbarDirectionType::TopToBottom && direction == ELGUINavigationDirection::Up))
    {
        valueIntervalMultiply = -0.1f;
    }
    else if (
        (DirectionType == UIScrollbarDirectionType::LeftToRight && direction == ELGUINavigationDirection::Right) || (DirectionType == UIScrollbarDirectionType::RightToLeft && direction == ELGUINavigationDirection::Left) || (DirectionType == UIScrollbarDirectionType::BottomToTop && direction == ELGUINavigationDirection::Up) || (DirectionType == UIScrollbarDirectionType::TopToBottom && direction == ELGUINavigationDirection::Down))
    {
        valueIntervalMultiply = 0.1f;
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

void UUIScrollbarComponent::CalculateInputValue(ULGUIPointerEventData *eventData)
{
    if (CheckHandle())
    {
        auto localCumulativeMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(eventData->GetWorldPointInPlane() - eventData->pressWorldPoint);
        localCumulativeMoveDelta.X = 0;
        float slideAreaSize = 0;
        float handleSize = 0;
        float value01 = Value;
        switch (DirectionType)
        {
        case UIScrollbarDirectionType::LeftToRight:
        {
            handleSize = HandleArea->GetWidth() * Size;
            slideAreaSize = HandleArea->GetWidth() - handleSize;
            value01 = PressValue + localCumulativeMoveDelta.Y / slideAreaSize;
        }
        break;
        case UIScrollbarDirectionType::RightToLeft:
        {
            handleSize = HandleArea->GetWidth() * Size;
            slideAreaSize = HandleArea->GetWidth() - handleSize;
            value01 = PressValue - localCumulativeMoveDelta.Y / slideAreaSize;
        }
        break;
        case UIScrollbarDirectionType::BottomToTop:
        {
            handleSize = HandleArea->GetHeight() * Size;
            slideAreaSize = HandleArea->GetHeight() - handleSize;
            value01 = PressValue + localCumulativeMoveDelta.Z / slideAreaSize;
        }
        break;
        case UIScrollbarDirectionType::TopToBottom:
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
void UUIScrollbarComponent::ApplyValueToUI()
{
    if (CheckHandle())
    {
        float value01 = Value;
        switch (DirectionType)
        {
        case UIScrollbarDirectionType::LeftToRight:
        {
            auto HorizontalMinMax = FVector2D((1.0f - Size) * value01, FMath::Lerp(Size, 1.0f, value01));
            Handle->SetHorizontalAnchorMinMax(HorizontalMinMax);
        }
        break;
        case UIScrollbarDirectionType::RightToLeft:
        {
            auto HorizontalMinMax = FVector2D((1.0f - Size) * (1.0f - value01), FMath::Lerp(1.0f, Size, value01));
            Handle->SetHorizontalAnchorMinMax(HorizontalMinMax);
        }
        break;
        case UIScrollbarDirectionType::BottomToTop:
        {
            auto VerticalMinMax = FVector2D((1.0f - Size) * value01, FMath::Lerp(Size, 1.0f, value01));
            Handle->SetVerticalAnchorMinMax(VerticalMinMax);
        }
        break;
        case UIScrollbarDirectionType::TopToBottom:
        {
            auto VerticalMinMax = FVector2D((1.0f - Size) * (1.0f - value01), FMath::Lerp(1.0f, Size, value01));
            Handle->SetVerticalAnchorMinMax(VerticalMinMax);
        }
        break;
        }
    }
}