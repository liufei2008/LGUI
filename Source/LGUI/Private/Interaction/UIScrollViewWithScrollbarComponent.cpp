// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewWithScrollbarComponent.h"
#include "LGUI.h"
#include "Interaction/UIScrollbarComponent.h"
#include "LTweenManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/LGUIManagerActor.h"


UUIScrollViewWithScrollbarComponent::UUIScrollViewWithScrollbarComponent()
{
	bLayoutDirty = false;
}

void UUIScrollViewWithScrollbarComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		ULGUIEditorManagerObject::RegisterLGUILayout(this);
	}
	else
#endif
	{
		ALGUIManagerActor::RegisterLGUILayout(this);
	}
}
void UUIScrollViewWithScrollbarComponent::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		ULGUIEditorManagerObject::UnregisterLGUILayout(this);
	}
	else
#endif
	{
		ALGUIManagerActor::UnregisterLGUILayout(this);
	}
}

void UUIScrollViewWithScrollbarComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
	CheckScrollbarParameter();//Check and register scrollbar event
}

bool UUIScrollViewWithScrollbarComponent::OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)
{
	return Super::OnPointerDrag_Implementation(eventData);
}
bool UUIScrollViewWithScrollbarComponent::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	return Super::OnPointerScroll_Implementation(eventData);
}
void UUIScrollViewWithScrollbarComponent::UpdateProgress(bool InFireEvent)
{
	Super::UpdateProgress(InFireEvent);
	if (CheckScrollbarParameter())
	{
		if (bAllowHorizontalScroll && HorizontalScrollbar->GetUIItem()->GetIsUIActiveInHierarchy())
		{
			if (Progress.X > 1.0f)
			{
				HorizontalScrollbarComp->SetValueAndSize(1.0f, ContentParentUIItem->GetWidth() / (ContentUIItem->GetWidth() + (HorizontalRange.Y - HorizontalRange.X) * (Progress.X - 1.0f)), false);
			}
			else if (Progress.X < 0.0f)
			{
				HorizontalScrollbarComp->SetValueAndSize(0.0f, ContentParentUIItem->GetWidth() / (ContentUIItem->GetWidth() + (HorizontalRange.Y - HorizontalRange.X) * (0.0f - Progress.X)), false);
			}
			else
			{
				HorizontalScrollbarComp->SetValue(Progress.X, false);
			}
		}
		if (bAllowVerticalScroll && VerticalScrollbar->GetUIItem()->GetIsUIActiveInHierarchy())
		{
			if (Progress.Y > 1.0f)
			{
				VerticalScrollbarComp->SetValueAndSize(1.0f, ContentParentUIItem->GetHeight() / (ContentUIItem->GetHeight() + (VerticalRange.Y - VerticalRange.X) * (Progress.Y - 1.0f)), false);
			}
			else if (Progress.Y < 0.0f)
			{
				VerticalScrollbarComp->SetValueAndSize(0.0f, ContentParentUIItem->GetHeight() / (ContentUIItem->GetHeight() + (VerticalRange.Y - VerticalRange.X) * (0.0f - Progress.Y)), false);
			}
			else
			{
				VerticalScrollbarComp->SetValue(Progress.Y, false);
			}
		}
	}
}
bool UUIScrollViewWithScrollbarComponent::CheckScrollbarParameter()
{
	if (Horizontal)
	{
		if (HorizontalScrollbarComp.IsValid() && HorizontalScrollbar.IsValid())
		{
			return true;
		}
		if (!HorizontalScrollbar.IsValid())
		{
			return false;
		}
		if (!HorizontalScrollbarComp.IsValid())
		{
			HorizontalScrollbarComp = HorizontalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (HorizontalScrollbarComp.IsValid())
			{
				HorizontalScrollbarComp->RegisterSlideEvent(FLGUIFloatDelegate::CreateUObject(this, &UUIScrollViewWithScrollbarComponent::OnHorizontalScrollbar));
				return true;
			}
		}
	}

	if (Vertical)
	{
		if (VerticalScrollbarComp.IsValid() && VerticalScrollbar.IsValid())
		{
			return true;
		}
		if (!VerticalScrollbar.IsValid())
		{
			return false;
		}
		if (!VerticalScrollbarComp.IsValid())
		{
			VerticalScrollbarComp = VerticalScrollbar->FindComponentByClass<UUIScrollbarComponent>();
			if (VerticalScrollbarComp.IsValid())
			{
				VerticalScrollbarComp->RegisterSlideEvent(FLGUIFloatDelegate::CreateUObject(this, &UUIScrollViewWithScrollbarComponent::OnVerticalScrollbar));
				return true;
			}
		}
	}

	return false;
}
bool UUIScrollViewWithScrollbarComponent::CheckValidHit(USceneComponent* InHitComp)
{
	bool hitHorizontalScrollbar = HorizontalScrollbar.IsValid() && (InHitComp->IsAttachedTo(HorizontalScrollbar->GetUIItem()) || InHitComp == HorizontalScrollbar->GetUIItem());
	bool hitVerticalScrollbar = VerticalScrollbar.IsValid() && (InHitComp->IsAttachedTo(VerticalScrollbar->GetUIItem()) || InHitComp == VerticalScrollbar->GetUIItem());
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
				HorizontalScrollbarLayoutActionType = EScrollbarLayoutAction::NeedToHide;
				MarkLayoutDirty();
			}
		}
		else
		{
			if (HorizontalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				HorizontalScrollbarLayoutActionType = EScrollbarLayoutAction::NeedToShow;
				MarkLayoutDirty();
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
				VerticalScrollbarLayoutActionType = EScrollbarLayoutAction::NeedToHide;
				MarkLayoutDirty();
			}
		}
		else
		{
			if (VerticalScrollbarVisibility != EScrollViewScrollbarVisibility::Permanent)
			{
				VerticalScrollbarLayoutActionType = EScrollbarLayoutAction::NeedToShow;
				MarkLayoutDirty();
			}
		}
	}
}

void UUIScrollViewWithScrollbarComponent::OnUIChildHierarchyIndexChanged(UUIItem* child)
{
	Super::OnUIChildHierarchyIndexChanged(child);
	MarkLayoutDirty();
}
void UUIScrollViewWithScrollbarComponent::OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach)
{
	Super::OnUIChildAttachmentChanged(child, attachOrDetach);
	MarkLayoutDirty();
}

bool UUIScrollViewWithScrollbarComponent::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (Viewport.IsValid())
	{
		if (InUIItem == Viewport->GetUIItem())
		{
			if (HorizontalScrollbar.IsValid())
			{
				if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
				{
					OutResult.bCanControlVerticalAnchor = true;
					OutResult.bCanControlVerticalAnchoredPosition = true;
					OutResult.bCanControlVerticalSizeDelta = true;
					return true;
				}
			}
			if (VerticalScrollbar.IsValid())
			{
				if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
				{
					OutResult.bCanControlHorizontalAnchor = true;
					OutResult.bCanControlHorizontalAnchoredPosition = true;
					OutResult.bCanControlHorizontalSizeDelta = true;
					return true;
				}
			}
		}
	}
	if (HorizontalScrollbar.IsValid())
	{
		if (InUIItem == HorizontalScrollbar->GetUIItem())
		{
			if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
			{
				OutResult.bCanControlVerticalAnchor = true;
				OutResult.bCanControlVerticalAnchoredPosition = true;
				return true;
			}
		}
	}
	if (VerticalScrollbar.IsValid())
	{
		if (InUIItem == VerticalScrollbar->GetUIItem())
		{
			if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
			{
				OutResult.bCanControlHorizontalAnchor = true;
				OutResult.bCanControlHorizontalAnchoredPosition = true;
				return true;
			}
		}
	}
	return false;
}

void UUIScrollViewWithScrollbarComponent::MarkLayoutDirty()
{
	bLayoutDirty = true;
	if (auto World = this->GetWorld())
	{
#if WITH_EDITOR
		if (!World->IsGameWorld())
		{

		}
		else
#endif
		{
			ALGUIManagerActor::MarkUpdateLayout(World);
		}
	}
}

void UUIScrollViewWithScrollbarComponent::OnUpdateLayout_Implementation()
{
	if (bLayoutDirty)
	{
		if (!Viewport.IsValid())return;
		if (!CheckParameters())return;
		if (!CheckScrollbarParameter())return;

		bLayoutDirty = false;

		auto ViewportUIItem = Viewport->GetUIItem();
		if (ViewportUIItem->GetAttachParent() != this->GetRootUIComponent())
		{
			ViewportUIItem->AttachToComponent(this->GetRootUIComponent(), FAttachmentTransformRules::KeepWorldTransform);
		}

		if (VerticalScrollbar.IsValid())
		{
			auto VerticalScrollbarUIItem = VerticalScrollbar->GetUIItem();
			if (VerticalScrollbarUIItem->GetAttachParent() != this->GetRootUIComponent())
			{
				VerticalScrollbarUIItem->AttachToComponent(this->GetRootUIComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
			auto parentHeight = ContentParentUIItem->GetHeight();
			auto contentHeight = ContentUIItem->GetHeight();
			switch (VerticalScrollbarLayoutActionType)
			{
			case UUIScrollViewWithScrollbarComponent::EScrollbarLayoutAction::NeedToShow:
			{
				VerticalScrollbarUIItem->SetIsUIActive(true);
			}
			break;
			case UUIScrollViewWithScrollbarComponent::EScrollbarLayoutAction::NeedToHide:
			{
				VerticalScrollbarUIItem->SetIsUIActive(false);
			}
			break;
			}
			if (VerticalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
			{
				if (VerticalScrollbarUIItem->GetIsUIActiveInHierarchy())
				{
					if (VerticalScrollbarUIItem->GetFlattenHierarchyIndex() > ViewportUIItem->GetFlattenHierarchyIndex())
					{
						ViewportUIItem->SetAnchorRight(VerticalScrollbarUIItem->GetWidth());
						ViewportUIItem->SetAnchorLeft(0);

						VerticalScrollbarUIItem->SetHorizontalAnchorMinMax(FVector2D(1, 1), true);
						float AnchorOffset = (VerticalScrollbarUIItem->GetPivot().X - 1.0f) * VerticalScrollbarUIItem->GetWidth();
						VerticalScrollbarUIItem->SetHorizontalAnchoredPosition(AnchorOffset);
					}
					else
					{
						ViewportUIItem->SetAnchorLeft(VerticalScrollbarUIItem->GetWidth());
						ViewportUIItem->SetAnchorRight(0);

						VerticalScrollbarUIItem->SetHorizontalAnchorMinMax(FVector2D(0, 0), true);
						float AnchorOffset = VerticalScrollbarUIItem->GetPivot().X * VerticalScrollbarUIItem->GetWidth();
						VerticalScrollbarUIItem->SetHorizontalAnchoredPosition(AnchorOffset);
					}
				}
				else
				{
					ViewportUIItem->SetAnchorLeft(0);
					ViewportUIItem->SetAnchorRight(0);
				}
			}
			if (VerticalScrollbarComp.IsValid())
			{
				VerticalScrollbarComp->SetValueAndSize(Progress.Y, parentHeight / contentHeight, false);
			}
			VerticalScrollbarLayoutActionType = EScrollbarLayoutAction::None;
		}

		if (HorizontalScrollbar.IsValid())
		{
			auto HorizontalScrollbarUIItem = HorizontalScrollbar->GetUIItem();
			if (HorizontalScrollbarUIItem->GetAttachParent() != this->GetRootUIComponent())
			{
				HorizontalScrollbarUIItem->AttachToComponent(this->GetRootUIComponent(), FAttachmentTransformRules::KeepWorldTransform);
			}
			auto parentWidth = ContentParentUIItem->GetWidth();
			auto contentWidth = ContentUIItem->GetWidth();
			switch (HorizontalScrollbarLayoutActionType)
			{
			case UUIScrollViewWithScrollbarComponent::EScrollbarLayoutAction::NeedToShow:
			{
				HorizontalScrollbarUIItem->SetIsUIActive(true);
			}
			break;
			case UUIScrollViewWithScrollbarComponent::EScrollbarLayoutAction::NeedToHide:
			{
				HorizontalScrollbar->GetUIItem()->SetIsUIActive(false);
			}
			break;
			}
			if (HorizontalScrollbarVisibility == EScrollViewScrollbarVisibility::AutoHideAndExpandViewport)
			{
				if (HorizontalScrollbarUIItem->GetIsUIActiveInHierarchy())
				{
					if (HorizontalScrollbarUIItem->GetFlattenHierarchyIndex() > ViewportUIItem->GetFlattenHierarchyIndex())
					{
						ViewportUIItem->SetAnchorBottom(HorizontalScrollbarUIItem->GetHeight());
						ViewportUIItem->SetAnchorTop(0);

						HorizontalScrollbarUIItem->SetVerticalAnchorMinMax(FVector2D(0, 0), true);
						float AnchorOffset = HorizontalScrollbarUIItem->GetPivot().Y * HorizontalScrollbarUIItem->GetHeight();
						HorizontalScrollbarUIItem->SetVerticalAnchoredPosition(AnchorOffset);
					}
					else
					{
						ViewportUIItem->SetAnchorTop(HorizontalScrollbarUIItem->GetHeight());
						ViewportUIItem->SetAnchorBottom(0);

						HorizontalScrollbarUIItem->SetVerticalAnchorMinMax(FVector2D(1, 1), true);
						float AnchorOffset = (HorizontalScrollbarUIItem->GetPivot().Y - 1.0f) * HorizontalScrollbarUIItem->GetHeight();
						HorizontalScrollbarUIItem->SetVerticalAnchoredPosition(AnchorOffset);
					}
				}
				else
				{
					ViewportUIItem->SetAnchorTop(0);
					ViewportUIItem->SetAnchorBottom(0);
				}
			}
			if (HorizontalScrollbarComp.IsValid())
			{
				HorizontalScrollbarComp->SetValueAndSize(Progress.X, parentWidth / contentWidth, false);
			}
			HorizontalScrollbarLayoutActionType = EScrollbarLayoutAction::None;
		}
	}
}

void UUIScrollViewWithScrollbarComponent::OnHorizontalScrollbar(float InScrollValue)
{
	if (!ContentUIItem.IsValid())return;
	bCanUpdateAfterDrag = false;
	bAllowHorizontalScroll = true;

	InScrollValue = FMath::Clamp(InScrollValue, 0.0f, 1.0f);
	auto Position = ContentUIItem->GetRelativeLocation();
	Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, InScrollValue);
	ContentUIItem->SetRelativeLocation(Position);
	Super::UpdateProgress();//use parent's function, skip the set scrollbar code
}
void UUIScrollViewWithScrollbarComponent::OnVerticalScrollbar(float InScrollValue)
{
	if (!ContentUIItem.IsValid())return;
	bCanUpdateAfterDrag = false;
	bAllowVerticalScroll = true;

	InScrollValue = FMath::Clamp(InScrollValue, 0.0f, 1.0f);
	auto Position = ContentUIItem->GetRelativeLocation();
	Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, InScrollValue);
	ContentUIItem->SetRelativeLocation(Position);
	Super::UpdateProgress();//use parent's function, skip the set scrollbar code
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