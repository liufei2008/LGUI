// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollbarComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"

UUIScrollbarComponent::UUIScrollbarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIScrollbarComponent::BeginPlay()
{
	Super::BeginPlay();
	ApplyValueToUI();
}

bool UUIScrollbarComponent::CheckHandle()
{
	if (Handle != nullptr && HandleArea != nullptr)return true;
	if (HandleActor == nullptr)return false;
	Handle = HandleActor->FindComponentByClass<UUIItem>();
	HandleArea = HandleActor->GetAttachParentActor()->FindComponentByClass<UUIItem>();
	if (Handle != nullptr && HandleArea != nullptr)return true;
	return false;
}

#if WITH_EDITOR
void UUIScrollbarComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ApplyValueToUI();
	Handle = nullptr;
	HandleArea = nullptr;
	if (CheckHandle())
	{
		Handle->EditorForceUpdateImmediately();
	}
}
#endif

void UUIScrollbarComponent::OnUIActiveInHierachy(bool ativeOrInactive)
{
	ApplyValueToUI();
}
void UUIScrollbarComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
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
			if (OnValueChangeCPP.IsBound())OnValueChangeCPP.Broadcast(Value);
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
			if (OnValueChangeCPP.IsBound())OnValueChangeCPP.Broadcast(Value);
			OnValueChange.FireEvent(Value);
		}
	}
}
void UUIScrollbarComponent::RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate)
{
	OnValueChangeCPP.Add(InDelegate);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FLGUIFloatDelegate& InDelegate)
{
	OnValueChangeCPP.Remove(InDelegate.GetHandle());
}

FLGUIDelegateHandleWrapper UUIScrollbarComponent::RegisterSlideEvent(const FLGUIScrollbarDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](float Value) {
		if (InDelegate.IsBound())InDelegate.Execute(Value);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnValueChangeCPP.Remove(InDelegateHandle.DelegateHandle);
}

bool UUIScrollbarComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	Super::OnPointerDown_Implementation(eventData);
	if (CheckHandle())
	{
		if (eventData.currentComponent != Handle)
		{
			const auto& pointerInHandleAreaSpace = HandleArea->GetComponentTransform().InverseTransformPosition(eventData.worldPoint);
			float value01 = Value;
			switch (DirectionType)
			{
			case UIScrollbarDirectionType::LeftToRight:
			{
				float validSpace = HandleArea->GetWidth() * (1.0f - Size);
				float valueDiff01;
				if (pointerInHandleAreaSpace.X > Handle->GetRelativeLocation().X)
				{
					valueDiff01 = (pointerInHandleAreaSpace.X - (Handle->GetLocalSpaceRight() + Handle->GetRelativeLocation().X)) / validSpace;
				}
				else
				{
					valueDiff01 = (pointerInHandleAreaSpace.X - (Handle->GetLocalSpaceLeft() + Handle->GetRelativeLocation().X)) / validSpace;
				}
				value01 += valueDiff01;
			}
			break;
			case UIScrollbarDirectionType::RightToLeft:
			{
				float validSpace = HandleArea->GetWidth() * (1.0f - Size);
				float valueDiff01;
				if (pointerInHandleAreaSpace.X > Handle->GetRelativeLocation().X)
				{
					valueDiff01 = (pointerInHandleAreaSpace.X - (Handle->GetLocalSpaceRight() + Handle->GetRelativeLocation().X)) / validSpace;
				}
				else
				{
					valueDiff01 = (pointerInHandleAreaSpace.X - (Handle->GetLocalSpaceLeft() + Handle->GetRelativeLocation().X)) / validSpace;
				}
				value01 -= valueDiff01;
			}
			break;
			case UIScrollbarDirectionType::BottomToTop:
			{
				float validSpace = HandleArea->GetHeight() * (1.0f - Size);
				float valueDiff01;
				if (pointerInHandleAreaSpace.Y > Handle->GetRelativeLocation().Y)
				{
					valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceTop() + Handle->GetRelativeLocation().Y)) / validSpace;
				}
				else
				{
					valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceBottom() + Handle->GetRelativeLocation().Y)) / validSpace;
				}
				value01 += valueDiff01;
			}
			break;
			case UIScrollbarDirectionType::TopToBottom:
			{
				float validSpace = HandleArea->GetHeight() * (1.0f - Size);
				float valueDiff01;
				if (pointerInHandleAreaSpace.Y > Handle->GetRelativeLocation().Y)
				{
					valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceTop() + Handle->GetRelativeLocation().Y)) / validSpace;
				}
				else
				{
					valueDiff01 = (pointerInHandleAreaSpace.Y - (Handle->GetLocalSpaceBottom() + Handle->GetRelativeLocation().Y)) / validSpace;
				}
				value01 -= valueDiff01;
			}
			break;
			}
			value01 = FMath::Clamp(value01, 0.0f, 1.0f);
			SetValue(value01, true);
		}
	}
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	Super::OnPointerUp_Implementation(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	PressValue = Value;
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}

void UUIScrollbarComponent::CalculateInputValue(const FLGUIPointerEventData& eventData)
{
	if (CheckHandle())
	{
		auto localCumulativeMoveDelta = eventData.pressWorldToLocalTransform.TransformVector(eventData.GetWorldPointInPlane() - eventData.pressWorldPoint);
		localCumulativeMoveDelta.Z = 0;
		float slideAreaSize = 0;
		float handleSize = 0;
		float value01 = Value;
		switch (DirectionType)
		{
		case UIScrollbarDirectionType::LeftToRight:
		{
			handleSize = HandleArea->GetWidth() * Size;
			slideAreaSize = HandleArea->GetWidth() - handleSize;
			value01 = PressValue + localCumulativeMoveDelta.X / slideAreaSize;
		}
		break;
		case UIScrollbarDirectionType::RightToLeft:
		{
			handleSize = HandleArea->GetWidth() * Size;
			slideAreaSize = HandleArea->GetWidth() - handleSize;
			value01 = PressValue - localCumulativeMoveDelta.X / slideAreaSize;
		}
		break;
		case UIScrollbarDirectionType::BottomToTop:
		{
			handleSize = HandleArea->GetHeight() * Size;
			slideAreaSize = HandleArea->GetHeight() - handleSize;
			value01 = PressValue + localCumulativeMoveDelta.Y / slideAreaSize;
		}
		break;
		case UIScrollbarDirectionType::TopToBottom:
		{
			handleSize = HandleArea->GetHeight() * Size;
			slideAreaSize = HandleArea->GetHeight() - handleSize;
			value01 = PressValue - localCumulativeMoveDelta.Y / slideAreaSize;
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
			float validSpace = HandleArea->GetWidth() * (1.0f - Size);
			Handle->SetHorizontalStretch(FVector2D(validSpace * value01, validSpace * (1.0f - value01)));
		}
		break;
		case UIScrollbarDirectionType::RightToLeft:
		{
			float validSpace = HandleArea->GetWidth() * (1.0f - Size);
			Handle->SetHorizontalStretch(FVector2D(validSpace * (1.0f - value01), validSpace * value01));
		}
		break;
		case UIScrollbarDirectionType::BottomToTop:
		{
			float validSpace = HandleArea->GetHeight() * (1.0f - Size);
			Handle->SetVerticalStretch(FVector2D(validSpace * value01, validSpace * (1.0f - value01)));
		}
		break;
		case UIScrollbarDirectionType::TopToBottom:
		{
			float validSpace = HandleArea->GetHeight() * (1.0f - Size);
			Handle->SetVerticalStretch(FVector2D(validSpace * (1.0f - value01), validSpace * value01));
		}
		break;
		}
	}
}