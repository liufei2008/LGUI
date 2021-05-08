﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UISliderComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"

void UUISliderComponent::Awake()
{
    Super::Awake();
}

void UUISliderComponent::Start()
{
    Super::Start();
    ApplyValueToUI();
}

bool UUISliderComponent::CheckFill()
{
    if (Fill.IsValid() && FillArea.IsValid())
        return true;
    if (!FillActor.IsValid())
        return false;
    Fill = FillActor->GetUIItem();
    if (IsValid(FillActor->GetAttachParentActor()))
    {
        FillArea = FillActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
    }
    if (Fill.IsValid() && FillArea.IsValid())
        return true;
    return false;
}
bool UUISliderComponent::CheckHandle()
{
    if (Handle.IsValid() && HandleArea.IsValid())
        return true;
    if (!HandleActor.IsValid())
        return false;
    Handle = HandleActor->GetUIItem();
    if (IsValid(HandleActor->GetAttachParentActor()))
    {
        HandleArea = HandleActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
    }
    if (Handle.IsValid() && HandleArea.IsValid())
        return true;
    return false;
}

#if WITH_EDITOR
void UUISliderComponent::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (WholeNumbers)
    {
        Value = FMath::FloorToFloat(Value);
    }
    ApplyValueToUI();

    Handle = nullptr; //force refind
    HandleArea = nullptr;
    if (CheckHandle())
    {
        Handle->EditorForceUpdateImmediately();
    }
    Fill = nullptr;
    FillArea = nullptr;
    if (CheckFill())
    {
        Fill->EditorForceUpdateImmediately();
    }
    Value = FMath::Clamp(Value, MinValue, MaxValue);
}
#endif

void UUISliderComponent::OnUIActiveInHierachy(bool ativeOrInactive)
{
    Super::OnUIActiveInHierachy(ativeOrInactive);
    ApplyValueToUI();
}
void UUISliderComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    ApplyValueToUI();
}
void UUISliderComponent::SetValue(float InValue, bool FireEvent)
{
    InValue = FMath::Clamp(InValue, MinValue, MaxValue);
    if (Value != InValue)
    {
        Value = InValue;
        ApplyValueToUI();
        if (FireEvent)
        {
            if (OnValueChangeCPP.IsBound())
                OnValueChangeCPP.Broadcast(Value);
            OnValueChange.FireEvent(Value);
        }
    }
}
void UUISliderComponent::SetMinValue(float InMinValue, bool KeepRelativeValue, bool FireEvent)
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
        ApplyValueToUI();
		if (FireEvent)
		{
			if (OnValueChangeCPP.IsBound())
				OnValueChangeCPP.Broadcast(Value);
			OnValueChange.FireEvent(Value);
		}
    }
}
void UUISliderComponent::SetMaxValue(float InMaxValue, bool KeepRelativeValue, bool FireEvent)
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
		ApplyValueToUI();
		if (FireEvent)
		{
			if (OnValueChangeCPP.IsBound())
				OnValueChangeCPP.Broadcast(Value);
			OnValueChange.FireEvent(Value);
		}
	}
}

FDelegateHandle UUISliderComponent::RegisterSlideEvent(const FLGUIFloatDelegate &InDelegate)
{
    return OnValueChangeCPP.Add(InDelegate);
}
FDelegateHandle UUISliderComponent::RegisterSlideEvent(const TFunction<void(float)> &InFunction)
{
    return OnValueChangeCPP.AddLambda(InFunction);
}
void UUISliderComponent::UnregisterSlideEvent(const FDelegateHandle &InHandle)
{
    OnValueChangeCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUISliderComponent::RegisterSlideEvent(const FLGUISliderDynamicDelegate &InDelegate)
{
    auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](float InValue) {
        if (InDelegate.IsBound())
            InDelegate.Execute(InValue);
    });
    return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUISliderComponent::UnregisterSlideEvent(const FLGUIDelegateHandleWrapper &InDelegateHandle)
{
    OnValueChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

bool UUISliderComponent::OnPointerDown_Implementation(ULGUIPointerEventData *eventData)
{
    Super::OnPointerDown_Implementation(eventData);
    if (eventData->inputType == ELGUIPointerInputType::Pointer)
    {
        CalculateInputValue(eventData);
    }
    return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerUp_Implementation(ULGUIPointerEventData *eventData)
{
    Super::OnPointerUp_Implementation(eventData);
    return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData *eventData)
{
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerDrag_Implementation(ULGUIPointerEventData *eventData)
{
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData *eventData)
{
    CalculateInputValue(eventData);
    return AllowEventBubbleUp;
}
bool UUISliderComponent::OnNavigate(ELGUINavigationDirection InDirection)
{
    float valueIntervalMultiply = 0.0f;
    if (
        (DirectionType == UISliderDirectionType::LeftToRight && InDirection == ELGUINavigationDirection::Left) || (DirectionType == UISliderDirectionType::RightToLeft && InDirection == ELGUINavigationDirection::Right) || (DirectionType == UISliderDirectionType::BottomToTop && InDirection == ELGUINavigationDirection::Down) || (DirectionType == UISliderDirectionType::TopToBottom && InDirection == ELGUINavigationDirection::Up))
    {
        valueIntervalMultiply = -0.1f;
    }
    else if (
        (DirectionType == UISliderDirectionType::LeftToRight && InDirection == ELGUINavigationDirection::Right) || (DirectionType == UISliderDirectionType::RightToLeft && InDirection == ELGUINavigationDirection::Left) || (DirectionType == UISliderDirectionType::BottomToTop && InDirection == ELGUINavigationDirection::Up) || (DirectionType == UISliderDirectionType::TopToBottom && InDirection == ELGUINavigationDirection::Down))
    {
        valueIntervalMultiply = 0.1f;
    }
    if (valueIntervalMultiply == 0.0f)
    {
        return true;
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

void UUISliderComponent::CalculateInputValue(ULGUIPointerEventData *eventData)
{
    UUIItem *mainUIItem = nullptr;
    UUIItem *areaUIItem = nullptr;
    if (CheckHandle())
    {
        mainUIItem = Handle.Get();
        areaUIItem = HandleArea.Get();
    }
    else
    {
        if (CheckFill())
        {
            mainUIItem = Fill.Get();
            areaUIItem = FillArea.Get();
        }
    }
    if (mainUIItem != nullptr && areaUIItem != nullptr)
    {
        //calculate value to 0-1 range
        auto localPointerPosition = areaUIItem->GetComponentTransform().InverseTransformPosition(eventData->GetWorldPointInPlane());
        auto widget = areaUIItem->GetWidget();
        float MinPosition = 0;
        float value01 = 0;
        switch (DirectionType)
        {
        case UISliderDirectionType::LeftToRight:
        {
            MinPosition = -widget.pivot.X * widget.width;
            value01 = (localPointerPosition.X - MinPosition) / widget.width;
        }
        break;
        case UISliderDirectionType::RightToLeft:
        {
            MinPosition = -widget.pivot.X * widget.width;
            value01 = 1.0f - (localPointerPosition.X - MinPosition) / widget.width;
        }
        break;
        case UISliderDirectionType::BottomToTop:
        {
            MinPosition = -widget.pivot.Y * widget.height;
            value01 = (localPointerPosition.Y - MinPosition) / widget.height;
        }
        break;
        case UISliderDirectionType::TopToBottom:
        {
            MinPosition = -widget.pivot.Y * widget.height;
            value01 = 1.0f - (localPointerPosition.Y - MinPosition) / widget.height;
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
void UUISliderComponent::ApplyValueToUI()
{
    float value01 = (Value - MinValue) / (MaxValue - MinValue);
    value01 = FMath::Clamp(value01, 0.0f, 1.0f);

    if (CheckHandle() || CheckFill())
    {
        switch (DirectionType)
        {
        case UISliderDirectionType::LeftToRight:
        {
            if (CheckHandle())
            {
                if (Handle->GetAnchorHAlign() != UIAnchorHorizontalAlign::Stretch)
                {
                    Handle->SetAnchorHAlign(UIAnchorHorizontalAlign::Left);
                }
                auto anchorOffsetX = value01 * HandleArea->GetWidth();
                Handle->SetAnchorOffsetX(anchorOffsetX);
            }
            if (CheckFill())
            {
                float stretch = (1.0f - value01) * FillArea->GetWidth();
                Fill->SetHorizontalStretch(FVector2D(0, stretch));
            }
        }
        break;
        case UISliderDirectionType::RightToLeft:
        {
            if (CheckHandle())
            {
                if (Handle->GetAnchorHAlign() != UIAnchorHorizontalAlign::Stretch)
                {
                    Handle->SetAnchorHAlign(UIAnchorHorizontalAlign::Right);
                }
                auto anchorOffsetX = -value01 * HandleArea->GetWidth();
                Handle->SetAnchorOffsetX(anchorOffsetX);
            }
            if (CheckFill())
            {
                float stretch = (1.0f - value01) * FillArea->GetWidth();
                Fill->SetHorizontalStretch(FVector2D(stretch, 0));
            }
        }
        break;
        case UISliderDirectionType::BottomToTop:
        {
            if (CheckHandle())
            {
                if (Handle->GetAnchorVAlign() != UIAnchorVerticalAlign::Stretch)
                {
                    Handle->SetAnchorVAlign(UIAnchorVerticalAlign::Bottom);
                }
                auto anchorOffsetY = value01 * HandleArea->GetHeight();
                Handle->SetAnchorOffsetY(anchorOffsetY);
            }
            if (CheckFill())
            {
                float stretch = (1.0f - value01) * FillArea->GetHeight();
                Fill->SetVerticalStretch(FVector2D(0, stretch));
            }
        }
        break;
        case UISliderDirectionType::TopToBottom:
        {
            if (CheckHandle())
            {
                if (Handle->GetAnchorVAlign() != UIAnchorVerticalAlign::Stretch)
                {
                    Handle->SetAnchorVAlign(UIAnchorVerticalAlign::Top);
                }
                auto anchorOffsetY = -value01 * HandleArea->GetHeight();
                Handle->SetAnchorOffsetY(anchorOffsetY);
            }
            if (CheckFill())
            {
                float stretch = (1.0f - value01) * FillArea->GetHeight();
                Fill->SetVerticalStretch(FVector2D(stretch, 0));
            }
        }
        break;
        }
    }
}