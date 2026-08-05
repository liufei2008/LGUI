// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewWithScrollbar.h"

#include "Core/Components/LexWidget.h"
#include "Interaction/UIScrollbar.h"


UUIScrollViewWithScrollbar::UUIScrollViewWithScrollbar()
{
	
}

#if WITH_EDITOR
void UUIScrollViewWithScrollbar::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	if (PropertyAboutToChange)
	{
		auto PropertyName = PropertyAboutToChange->GetName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbar))
		{
			if (HorizontalScrollbar.IsValid() && HorizontalScrollbarDelegateHandle.IsValid())
			{
				HorizontalScrollbar->GetOnValueChangedEvent().Remove(HorizontalScrollbarDelegateHandle);
				HorizontalScrollbarDelegateHandle.Reset();
			}
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbar))
		{
			if (VerticalScrollbar.IsValid() && VerticalScrollbarDelegateHandle.IsValid())
			{
				VerticalScrollbar->GetOnValueChangedEvent().Remove(VerticalScrollbarDelegateHandle);
				VerticalScrollbarDelegateHandle.Reset();
			}
		}
	}
}
void UUIScrollViewWithScrollbar::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		auto PropertyName = PropertyChangedEvent.Property->GetName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, HorizontalScrollbar))
		{
			HorizontalScrollbarWidget.Reset();
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIScrollViewWithScrollbar, VerticalScrollbar))
		{
			VerticalScrollbarWidget.Reset();
		}
	}
}
#endif

void UUIScrollViewWithScrollbar::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
	Super::OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
	CheckScrollbarParameter();//Check and register scrollbar event
}

bool UUIScrollViewWithScrollbar::OnPointerDrag_Implementation(ULexPointerEventData* EventData)
{
	return Super::OnPointerDrag_Implementation(EventData);
}
bool UUIScrollViewWithScrollbar::OnPointerScroll_Implementation(ULexPointerEventData* EventData)
{
	return Super::OnPointerScroll_Implementation(EventData);
}
void UUIScrollViewWithScrollbar::UpdateProgress(bool InFireEvent)
{
	Super::UpdateProgress(InFireEvent);
	if (CheckScrollbarParameter())
	{
		if (bAllowHorizontalScroll && HorizontalScrollbarWidget->GetWidgetActiveInHierarchy())
		{
			if (Progress.X > 1.0f)
			{
				HorizontalScrollbar->SetValueAndSize(1.0f, ContentParent->GetWidth() / (Content->GetWidth() + (HorizontalRange.Y - HorizontalRange.X) * (Progress.X - 1.0f)), false);
			}
			else if (Progress.X < 0.0f)
			{
				HorizontalScrollbar->SetValueAndSize(0.0f, ContentParent->GetWidth() / (Content->GetWidth() + (HorizontalRange.Y - HorizontalRange.X) * (0.0f - Progress.X)), false);
			}
			else
			{
				HorizontalScrollbar->SetValueWithoutNotify(Progress.X);
			}
		}
		if (bAllowVerticalScroll && VerticalScrollbarWidget->GetWidgetActiveInHierarchy())
		{
			if (Progress.Y > 1.0f)
			{
				VerticalScrollbar->SetValueAndSize(1.0f, ContentParent->GetHeight() / (Content->GetHeight() + (VerticalRange.Y - VerticalRange.X) * (Progress.Y - 1.0f)), false);
			}
			else if (Progress.Y < 0.0f)
			{
				VerticalScrollbar->SetValueAndSize(0.0f, ContentParent->GetHeight() / (Content->GetHeight() + (VerticalRange.Y - VerticalRange.X) * (0.0f - Progress.Y)), false);
			}
			else
			{
				VerticalScrollbar->SetValueWithoutNotify(Progress.Y);
			}
		}
	}
}
bool UUIScrollViewWithScrollbar::CheckScrollbarParameter()
{
	bool bHorizontalValid = false;
	bool bVerticalValid = false;
	if (Horizontal)
	{
		if (HorizontalScrollbarWidget.IsValid())
		{
			bHorizontalValid = true;
		}
		else
		{
			if (HorizontalScrollbar.IsValid())
			{
#if WITH_EDITOR
				if (GetWorld() && GetWorld()->IsGameWorld())//register event only in game mode
#endif
				{
					HorizontalScrollbarDelegateHandle = HorizontalScrollbar->GetOnValueChangedEvent().AddUObject(this, &UUIScrollViewWithScrollbar::OnHorizontalScrollbar);
				}
				HorizontalScrollbarWidget = HorizontalScrollbar->GetWidget();
				bHorizontalValid = true;
			}
		}
	}

	if (Vertical)
	{
		if (VerticalScrollbarWidget.IsValid())
		{
			bVerticalValid = true;
		}
		else
		{
			if (VerticalScrollbar.IsValid())
			{
#if WITH_EDITOR
				if (GetWorld() && GetWorld()->IsGameWorld())//register event only in game mode
#endif
				{
					VerticalScrollbarDelegateHandle = VerticalScrollbar->GetOnValueChangedEvent().AddUObject(this, &UUIScrollViewWithScrollbar::OnVerticalScrollbar);
				}
				VerticalScrollbarWidget = VerticalScrollbar->GetWidget();
				bVerticalValid = true;
			}
		}
	}

	if (Horizontal && Vertical)
	{
		if (bHorizontalValid && bVerticalValid)
		{
			return true;
		}
	}
	else
	{
		if (Horizontal)
		{
			return bHorizontalValid;
		}
		if (Vertical)
		{
			return bVerticalValid;
		}
	}

	return false;
}
bool UUIScrollViewWithScrollbar::CheckValidHit(ULexWidget* InHitComp)
{
	bool bHitHorizontalScrollbar = HorizontalScrollbarWidget.IsValid() && (InHitComp->IsChildOf(HorizontalScrollbarWidget.Get()) || InHitComp == HorizontalScrollbarWidget);
	bool bHitVerticalScrollbar = VerticalScrollbarWidget.IsValid() && (InHitComp->IsChildOf(VerticalScrollbarWidget.Get()) || InHitComp == VerticalScrollbarWidget);
	return Super::CheckValidHit(InHitComp)
		&& !bHitHorizontalScrollbar && !bHitVerticalScrollbar;//make sure hit component is not scrollbar
}
void UUIScrollViewWithScrollbar::CalculateHorizontalRange()
{
	Super::CalculateHorizontalRange();
	if (CheckScrollbarParameter())
	{
		auto ParentWidth = ContentParent->GetWidth();
		auto ContentWidth = Content->GetWidth();
		if (HorizontalScrollbarVisibility != ELexUIScrollViewScrollbarVisibility::None)
		{
			bool ShouldScrollbarActive = true;
			if (ParentWidth >= ContentWidth)
			{
				if (HorizontalScrollbarVisibility == ELexUIScrollViewScrollbarVisibility::Permanent)
				{
					ShouldScrollbarActive = true;
				}
				else
				{
					ShouldScrollbarActive = false;
				}
			}
			else
			{
				if (HorizontalScrollbarVisibility == ELexUIScrollViewScrollbarVisibility::Permanent)
				{
					ShouldScrollbarActive = true;
				}
				else
				{
					ShouldScrollbarActive = true;
				}
			}
			if (HorizontalScrollbarWidget.IsValid())
			{
				HorizontalScrollbarWidget->SetWidgetActive(ShouldScrollbarActive);
			}
		}
		if (HorizontalScrollbar.IsValid())
		{
			HorizontalScrollbar->SetValueAndSize(Progress.X, ParentWidth / ContentWidth, false);
		}
	}
}
void UUIScrollViewWithScrollbar::CalculateVerticalRange()
{
	Super::CalculateVerticalRange();
	if (CheckScrollbarParameter())
	{
		auto ParentHeight = ContentParent->GetHeight();
		auto ContentHeight = Content->GetHeight();
		if (VerticalScrollbarVisibility != ELexUIScrollViewScrollbarVisibility::None)
		{
			bool ShouldScrollbarActive = true;
			if (ParentHeight >= ContentHeight)
			{
				if (VerticalScrollbarVisibility == ELexUIScrollViewScrollbarVisibility::Permanent)
				{
					ShouldScrollbarActive = true;
				}
				else
				{
					ShouldScrollbarActive = false;
				}
			}
			else
			{
				if (VerticalScrollbarVisibility == ELexUIScrollViewScrollbarVisibility::Permanent)
				{
					ShouldScrollbarActive = true;
				}
				else
				{
					ShouldScrollbarActive = true;
				}
			}
			if (VerticalScrollbarWidget.IsValid())
			{
				VerticalScrollbarWidget->SetWidgetActive(ShouldScrollbarActive);
			}
		}
		if (VerticalScrollbar.IsValid())
		{
			VerticalScrollbar->SetValueAndSize(Progress.Y, ParentHeight / ContentHeight, false);
		}
	}
}

void UUIScrollViewWithScrollbar::OnHorizontalScrollbar(float InScrollValue)
{
	if (!Content.IsValid())return;
	bCanUpdateAfterDrag = false;
	bAllowHorizontalScroll = true;

	InScrollValue = FMath::Clamp(InScrollValue, 0.0f, 1.0f);
	auto Position = Content->GetRelativeLocation();
	Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, 1.0f - InScrollValue);
	Content->SetRelativeLocation(Position);
	Super::UpdateProgress();//use parent's function, skip the set scrollbar code
}
void UUIScrollViewWithScrollbar::OnVerticalScrollbar(float InScrollValue)
{
	if (!Content.IsValid())return;
	bCanUpdateAfterDrag = false;
	bAllowVerticalScroll = true;

	InScrollValue = FMath::Clamp(InScrollValue, 0.0f, 1.0f);
	auto Position = Content->GetRelativeLocation();
	Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, InScrollValue);
	Content->SetRelativeLocation(Position);
	Super::UpdateProgress();//use parent's function, skip the set scrollbar code
}
void UUIScrollViewWithScrollbar::SetHorizontalScrollbarVisibility(ELexUIScrollViewScrollbarVisibility value)
{
	if (HorizontalScrollbarVisibility != value)
	{
		HorizontalScrollbarVisibility = value;
		CalculateHorizontalRange();
	}
}
void UUIScrollViewWithScrollbar::SetVerticalScrollbarVisibility(ELexUIScrollViewScrollbarVisibility value)
{
	if (VerticalScrollbarVisibility != value)
	{
		VerticalScrollbarVisibility = value;
		CalculateVerticalRange();
	}
}