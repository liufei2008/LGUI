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
		auto contentLocation = ContentUIItem->GetRelativeLocation();
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
		if (HorizontalScrollbarComp == nullptr)
		{
			if (HorizontalScrollbar == nullptr)return false;
			HorizontalScrollbarComp = HorizontalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (HorizontalScrollbarComp != nullptr)
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
		if (VerticalScrollbarComp == nullptr)
		{
			if (VerticalScrollbar == nullptr)return false;
			VerticalScrollbarComp = VerticalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (VerticalScrollbarComp != nullptr)
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
	bool hitHorizontalScrollbar = HorizontalScrollbar != nullptr && (InHitComp->IsAttachedTo(HorizontalScrollbar->GetUIItem()) || InHitComp == HorizontalScrollbar->GetUIItem());
	bool hitVerticalScrollbar = VerticalScrollbar != nullptr && (InHitComp->IsAttachedTo(VerticalScrollbar->GetUIItem()) || InHitComp == VerticalScrollbar->GetUIItem());
	return Super::CheckValidHit(InHitComp)
		&& !hitHorizontalScrollbar && !hitVerticalScrollbar;//make sure hit component is not scrollbar
}
void UUIScrollViewWithScrollbarComponent::CalculateHorizontalRange()
{
	Super::CalculateHorizontalRange();
	if (CheckScrollbarParameter())
	{
		auto& parentWidget = ContentParentUIItem->GetWidget();
		auto& contentWidget = ContentUIItem->GetWidget();
		if (parentWidget.width > contentWidget.width)
		{
			if (HorizontalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (HorizontalScrollbar != nullptr)
				{
					HorizontalScrollbar->GetUIItem()->SetUIActive(false);
					if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && Viewport != nullptr)
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
				if (HorizontalScrollbar != nullptr)
				{
					HorizontalScrollbar->GetUIItem()->SetUIActive(true);
					HorizontalScrollbarComp->SetValueAndSize(Progress.X, parentWidget.width / contentWidget.width, false);
					if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && Viewport != nullptr)
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
		auto& parentWidget = ContentParentUIItem->GetWidget();
		auto& contentWidget = ContentUIItem->GetWidget();
		if (parentWidget.height > contentWidget.height)
		{
			if (VerticalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				if (VerticalScrollbar != nullptr)
				{
					VerticalScrollbar->GetUIItem()->SetUIActive(false);
					if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && Viewport != nullptr)
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
				if (VerticalScrollbar != nullptr)
				{
					VerticalScrollbar->GetUIItem()->SetUIActive(true);
					VerticalScrollbarComp->SetValueAndSize(Progress.Y, parentWidget.height / contentWidget.height, false);
					if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport && Viewport != nullptr)
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
	if (ContentUIItem == nullptr)return;
	CanUpdateAfterDrag = false;
	ValueIsSetFromHorizontalScrollbar = true;
	AllowHorizontalScroll = true;

	Position.X = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, InScrollValue);
	ContentUIItem->SetUIRelativeLocation(Position);
	UpdateProgress();
}
void UUIScrollViewWithScrollbarComponent::OnVerticalScrollbar(float InScrollValue)
{
	if (ContentUIItem == nullptr)return;
	CanUpdateAfterDrag = false;
	ValueIsSetFromVerticalScrollbar = true;
	AllowVerticalScroll = true;

	Position.Y = FMath::Lerp(VerticalRange.X, VerticalRange.Y, InScrollValue);
	ContentUIItem->SetUIRelativeLocation(Position);
	UpdateProgress();
}