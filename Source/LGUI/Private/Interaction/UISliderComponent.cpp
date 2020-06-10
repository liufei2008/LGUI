// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UISliderComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"


void UUISliderComponent::Awake()
{
	Super::Awake();
	ApplyValueToUI();
}

bool UUISliderComponent::CheckFill()
{
	if (Fill != nullptr && FillArea != nullptr)return true;
	if (!IsValid(FillActor))return false;
	Fill = FillActor->GetUIItem();
	if (IsValid(FillActor->GetAttachParentActor()))
	{
		FillArea = FillActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
	}
	if (Fill != nullptr && FillArea != nullptr)return true;
	return false;
}
bool UUISliderComponent::CheckHandle()
{
	if (Handle != nullptr && HandleArea != nullptr)return true;
	if (!IsValid(HandleActor))return false;
	Handle = HandleActor->GetUIItem();
	if (IsValid(HandleActor->GetAttachParentActor()))
	{
		HandleArea = HandleActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
	}
	if (Handle != nullptr && HandleArea != nullptr)return true;
	return false;
}

#if WITH_EDITOR
void UUISliderComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (WholeNumbers)
	{
		Value = FMath::FloorToFloat(Value);
	}
	ApplyValueToUI();

	Handle = nullptr;//force refind
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
	ApplyValueToUI();
}
void UUISliderComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	ApplyValueToUI();
}
void UUISliderComponent::SetValue(float InValue, bool FireEvent)
{
	if (Value != InValue)
	{
		Value = InValue;
		ApplyValueToUI();
		if (FireEvent)
		{
			if (OnValueChangeCPP.IsBound())OnValueChangeCPP.Broadcast(Value);
			OnValueChange.FireEvent(Value);
		}
	}
}

void UUISliderComponent::RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate)
{
	OnValueChangeCPP.Add(InDelegate);
}
void UUISliderComponent::UnregisterSlideEvent(const FLGUIFloatDelegate& InDelegate)
{
	OnValueChangeCPP.Remove(InDelegate.GetHandle());
}

FLGUIDelegateHandleWrapper UUISliderComponent::RegisterSlideEvent(const FLGUISliderDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](float Value) {
		if (InDelegate.IsBound())InDelegate.Execute(Value);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUISliderComponent::UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnValueChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

bool UUISliderComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	Super::OnPointerDown_Implementation(eventData);
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	Super::OnPointerUp_Implementation(eventData);
	return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUISliderComponent::OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}

void UUISliderComponent::CalculateInputValue(const FLGUIPointerEventData& eventData)
{
	UUIItem* mainUIItem = nullptr;
	UUIItem* areaUIItem = nullptr;
	if (CheckHandle())
	{
		mainUIItem = Handle;
		areaUIItem = HandleArea;
	}
	else
	{
		if (CheckFill())
		{
			mainUIItem = Fill;
			areaUIItem = FillArea;
		}
	}
	if (mainUIItem != nullptr && areaUIItem != nullptr)
	{
		//calculate value to 0-1 range
		auto localPointerPosition = areaUIItem->GetComponentTransform().InverseTransformPosition(eventData.GetWorldPointInPlane());
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