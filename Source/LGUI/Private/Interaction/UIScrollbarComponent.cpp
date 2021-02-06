// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollbarComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"

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
FDelegateHandle UUIScrollbarComponent::RegisterSlideEvent(const FLGUIFloatDelegate& InDelegate)
{
	return OnValueChangeCPP.Add(InDelegate);
}
FDelegateHandle UUIScrollbarComponent::RegisterSlideEvent(const TFunction<void(float)>& InFunction)
{
	return OnValueChangeCPP.AddLambda(InFunction);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FDelegateHandle& InHandle)
{
	OnValueChangeCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUIScrollbarComponent::RegisterSlideEvent(const FLGUIScrollbarDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnValueChangeCPP.AddLambda([InDelegate](float InValue) {
		if (InDelegate.IsBound())InDelegate.Execute(InValue);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIScrollbarComponent::UnregisterSlideEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
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
	}
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerUp_Implementation(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)
{
	PressValue = Value;
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)
{
	CalculateInputValue(eventData);
	return AllowEventBubbleUp;
}
bool UUIScrollbarComponent::OnNavigate(ELGUINavigationDirection InDirection)
{
	float valueIntervalMultiply = 0.0f;
	if (
		(DirectionType == UIScrollbarDirectionType::LeftToRight && InDirection == ELGUINavigationDirection::Left)
		|| (DirectionType == UIScrollbarDirectionType::RightToLeft && InDirection == ELGUINavigationDirection::Right)
		|| (DirectionType == UIScrollbarDirectionType::BottomToTop && InDirection == ELGUINavigationDirection::Down)
		|| (DirectionType == UIScrollbarDirectionType::TopToBottom && InDirection == ELGUINavigationDirection::Up)
		)
	{
		valueIntervalMultiply = -0.1f;
	}
	else if (
		(DirectionType == UIScrollbarDirectionType::LeftToRight && InDirection == ELGUINavigationDirection::Right)
		|| (DirectionType == UIScrollbarDirectionType::RightToLeft && InDirection == ELGUINavigationDirection::Left)
		|| (DirectionType == UIScrollbarDirectionType::BottomToTop && InDirection == ELGUINavigationDirection::Up)
		|| (DirectionType == UIScrollbarDirectionType::TopToBottom && InDirection == ELGUINavigationDirection::Down)
		)
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
		tempValue += valueIntervalMultiply;
		tempValue = FMath::Clamp(tempValue, 0.0f, 1.0f);
		SetValue(tempValue);
		return false;
	}
}

void UUIScrollbarComponent::CalculateInputValue(ULGUIPointerEventData* eventData)
{
	if (CheckHandle())
	{
		auto localCumulativeMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(eventData->GetWorldPointInPlane() - eventData->pressWorldPoint);
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