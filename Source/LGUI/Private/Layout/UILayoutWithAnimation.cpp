// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UILayoutWithAnimation.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "LTweenManager.h"
#include "LTweenBPLibrary.h"

void UUILayoutWithAnimation::CancelAnimation(bool callComplete)
{
	if (TweenerArray.Num() > 0)
	{
		for (auto item : TweenerArray)
		{
			if (IsValid(item))
			{
				ULTweenBPLibrary::KillIfIsTweening(this, item, callComplete);
			}
		}
		TweenerArray.Reset();
	}
	bIsAnimationPlaying = false;
}

void UUILayoutWithAnimation::SetOnCompleteTween()
{
	bIsAnimationPlaying = true;
	auto tweener = ULTweenManager::VirtualTo(this, AnimationDuration)->OnComplete(FSimpleDelegate::CreateWeakLambda(this, [this] {
		bIsAnimationPlaying = false;
		if (bShouldRebuildLayoutAfterAnimation)
		{
			bShouldRebuildLayoutAfterAnimation = false;
			MarkNeedRebuildLayout();
		}
		}));
	TweenerArray.Add(tweener);
}

void UUILayoutWithAnimation::ApplyAnchoredPositionWithAnimation(EUILayoutChangePositionAnimationType tempAnimationType, FVector2D offset, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutChangePositionAnimationType::Immediately:
	{
		ApplyUIItemAnchoredPosition(target, offset);
	}
	break;
	case EUILayoutChangePositionAnimationType::EaseAnimation:
	{
		if (target->GetAnchoredPosition() != offset)
		{
			auto tweener = ULTweenManager::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetAnchoredPosition), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetAnchoredPosition), offset, AnimationDuration)->SetEase(LTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyWidthWithAnimation(EUILayoutChangePositionAnimationType tempAnimationType, float width, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutChangePositionAnimationType::Immediately:
	{
		ApplyUIItemWidth(target, width);
	}
	break;
	case EUILayoutChangePositionAnimationType::EaseAnimation:
	{
		if (target->GetWidth() != width)
		{
			auto tweener = ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), width, AnimationDuration)->SetEase(LTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyHeightWithAnimation(EUILayoutChangePositionAnimationType tempAnimationType, float height, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutChangePositionAnimationType::Immediately:
	{
		ApplyUIItemHeight(target, height);
	}
	break;
	case EUILayoutChangePositionAnimationType::EaseAnimation:
	{
		if (target->GetHeight() != height)
		{
			auto tweener = ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), height, AnimationDuration)->SetEase(LTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::SetAnimationType(EUILayoutChangePositionAnimationType value)
{
	if (AnimationType != value)
	{
		AnimationType = value;
	}
}
void UUILayoutWithAnimation::SetAnimationDuration(float value)
{
	if (AnimationDuration != value)
	{
		AnimationDuration = value;
	}
}
