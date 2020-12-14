// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewWithScrollbarComponent.h"
#include "LGUI.h"
#include "Interaction/UIScrollbarComponent.h"
#include "LTweenActor.h"
#include "Core/Actor/UIBaseActor.h"

void UUIScrollViewWithScrollbarComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
	CheckScrollbarParameter();//Check and register scrollbar event
}


bool UUIScrollViewWithScrollbarComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerDrag_Implementation(eventData);
	ValueIsSetFromHorizontalScrollbar = false;
	ValueIsSetFromVerticalScrollbar = false;
	return AllowEventBubbleUp;
}
bool UUIScrollViewWithScrollbarComponent::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	Super::OnPointerScroll_Implementation(eventData);
	ValueIsSetFromHorizontalScrollbar = false;
	ValueIsSetFromVerticalScrollbar = false;
	return AllowEventBubbleUp;
}
void UUIScrollViewWithScrollbarComponent::UpdateProgress(bool InFireEvent)
{
	Super::UpdateProgress(InFireEvent);
	if (CheckScrollbarParameter())
	{
		auto& parentWidget = ContentParentUIItem->GetWidget();
		auto& contentWidget = ContentUIItem->GetWidget();
		auto& contentLocation = ContentUIItem->RelativeLocation;
		if (AllowHorizontalScroll && !ValueIsSetFromHorizontalScrollbar && HorizontalScrollbar->GetUIItem()->IsUIActiveInHierarchy())
		{
			if (Progress.X > 1.0f)
			{
				HorizontalScrollbarComp->SetValueAndSize(1.0f, parentWidget.width / (contentWidget.width + (HorizontalRange.Y - HorizontalRange.X) * (Progress.X - 1.0f)), false);
			}
			else if (Progress.X < 0.0f)
			{
				HorizontalScrollbarComp->SetValueAndSize(0.0f, parentWidget.width / (contentWidget.width + (HorizontalRange.Y - HorizontalRange.X) * (0.0f - Progress.X)), false);
			}
			else
			{
				HorizontalScrollbarComp->SetValue(Progress.X, false);
			}
			ValueIsSetFromHorizontalScrollbar = false;
		}
		if (AllowVerticalScroll && !ValueIsSetFromVerticalScrollbar && VerticalScrollbar->GetUIItem()->IsUIActiveInHierarchy())
		{
			if (Progress.Y > 1.0f)
			{
				VerticalScrollbarComp->SetValueAndSize(1.0f, parentWidget.height / (contentWidget.height + (VerticalRange.Y - VerticalRange.X) * (Progress.Y - 1.0f)), false);
			}
			else if (Progress.Y < 0.0f)
			{
				VerticalScrollbarComp->SetValueAndSize(0.0f, parentWidget.height / (contentWidget.height + (VerticalRange.Y - VerticalRange.X) * (0.0f - Progress.Y)), false);
			}
			else
			{
				VerticalScrollbarComp->SetValue(Progress.Y, false);
			}
			ValueIsSetFromVerticalScrollbar = false;
		}
	}
}
bool UUIScrollViewWithScrollbarComponent::CheckScrollbarParameter()
{
	if (Horizontal)
	{
		if (!IsValid(HorizontalScrollbarComp))
		{
			if (!IsValid(HorizontalScrollbar))return false;
			HorizontalScrollbarComp = HorizontalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (IsValid(HorizontalScrollbarComp))
			{
				HorizontalScrollbarComp->RegisterSlideEvent(FLGUIFloatDelegate::CreateUObject(this, &UUIScrollViewWithScrollbarComponent::OnHorizontalScrollbar));
			}
			else
			{
				return false;
			}
		}
	}
	if (Vertical)
	{
		if (!IsValid(VerticalScrollbarComp))
		{
			if (!IsValid(VerticalScrollbar))return false;
			VerticalScrollbarComp = VerticalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (IsValid(VerticalScrollbarComp))
			{
				VerticalScrollbarComp->RegisterSlideEvent(FLGUIFloatDelegate::CreateUObject(this, &UUIScrollViewWithScrollbarComponent::OnVerticalScrollbar));
			}
			else
			{
				return false;
			}
		}
	}
	return true;
}
bool UUIScrollViewWithScrollbarComponent::CheckValidHit(USceneComponent* InHitComp)
{
	bool hitHorizontalScrollbar = IsValid(HorizontalScrollbar) && (InHitComp->IsAttachedTo(HorizontalScrollbar->GetUIItem()) || InHitComp == HorizontalScrollbar->GetUIItem());
	bool hitVerticalScrollbar = IsValid(VerticalScrollbar) && (InHitComp->IsAttachedTo(VerticalScrollbar->GetUIItem()) || InHitComp == VerticalScrollbar->GetUIItem());
	return Super::CheckValidHit(InHitComp)
		&& !hitHorizontalScrollbar && !hitVerticalScrollbar;//make sure hit component is not scrollbar
}
void UUIScrollViewWithScrollbarComponent::CalculateHorizontalRange()
{
	Super::CalculateHorizontalRange();
	if (CheckScrollbarParameter())
	{
		auto parentWidth = ContentParentUIItem->GetWidth();
		auto contentWidth = ContentUIItem->GetWidth();
		if (parentWidth >= contentWidth)
		{
			if (HorizontalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (IsValid(HorizontalScrollbar))
				{
					HorizontalScrollbar->GetUIItem()->SetUIActive(false);
					if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && IsValid(Viewport))
					{
						Viewport->GetUIItem()->SetStretchBottom(0);
					}
				}
			}
		}
		else
		{
			if (HorizontalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (IsValid(HorizontalScrollbar))
				{
					HorizontalScrollbar->GetUIItem()->SetUIActive(true);
					HorizontalScrollbarComp->SetValueAndSize(Progress.X, parentWidth / contentWidth, false);
					if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && IsValid(Viewport))
					{
						Viewport->GetUIItem()->SetStretchBottom(HorizontalScrollbar->GetUIItem()->GetHeight());
					}
				}
			}
		}
	}
}
void UUIScrollViewWithScrollbarComponent::CalculateVerticalRange()
{
	Super::CalculateVerticalRange();
	if (CheckScrollbarParameter())
	{
		auto parentHeight = ContentParentUIItem->GetHeight();
		auto contentHeight = ContentUIItem->GetHeight();
		if (parentHeight >= contentHeight)
		{
			if (VerticalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (IsValid(VerticalScrollbar))
				{
					VerticalScrollbar->GetUIItem()->SetUIActive(false);
					if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && IsValid(Viewport))
					{
						Viewport->GetUIItem()->SetStretchRight(0);
					}
				}
			}
		}
		else
		{
			if (VerticalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (IsValid(VerticalScrollbar))
				{
					VerticalScrollbar->GetUIItem()->SetUIActive(true);
					VerticalScrollbarComp->SetValueAndSize(Progress.Y, parentHeight / contentHeight, false);
					if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && IsValid(Viewport))
					{
						Viewport->GetUIItem()->SetStretchRight(VerticalScrollbar->GetUIItem()->GetWidth());
					}
				}
			}
		}
	}
}
void UUIScrollViewWithScrollbarComponent::OnHorizontalScrollbar(float InScrollValue)
{
	if (!IsValid(ContentUIItem))return;
	CanUpdateAfterDrag = false;
	ValueIsSetFromHorizontalScrollbar = true;
	AllowHorizontalScroll = true;

	Position.X = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, InScrollValue);
	ContentUIItem->SetUIRelativeLocation(Position);
	UpdateProgress();
}
void UUIScrollViewWithScrollbarComponent::OnVerticalScrollbar(float InScrollValue)
{
	if (!IsValid(ContentUIItem))return;
	CanUpdateAfterDrag = false;
	ValueIsSetFromVerticalScrollbar = true;
	AllowVerticalScroll = true;

	Position.Y = FMath::Lerp(VerticalRange.X, VerticalRange.Y, InScrollValue);
	ContentUIItem->SetUIRelativeLocation(Position);
	UpdateProgress();
}
void UUIScrollViewWithScrollbarComponent::SetHorizontalScrollbarVisibility(EScrollViewScrollbarVisibility value)
{
	if (HorizontalScrollbarVisibility != value)
	{
		HorizontalScrollbarVisibility = value;
		CalculateHorizontalRange();
	}
}
void UUIScrollViewWithScrollbarComponent::SetVerticalScrollbarVisibility(EScrollViewScrollbarVisibility value)
{
	if (VerticalScrollbarVisibility != value)
	{
		VerticalScrollbarVisibility = value;
		CalculateVerticalRange();
	}
}