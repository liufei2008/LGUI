// Copyright 2019-Present LexLiu. All Rights Reserved.

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

void UUILayoutWithAnimation::ApplyAnchoredPositionWithAnimation(EUILayoutAnimationType tempAnimationType, FVector2D offset, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemAnchoredPosition(target, offset);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (target->GetAnchoredPosition() != offset)
		{
			auto tweener = ULTweenManager::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetAnchoredPosition), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetAnchoredPosition), offset, AnimationDuration)->SetEase(ELTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyRotatorWithAnimation(EUILayoutAnimationType tempAnimationType, const FRotator& value, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		target->SetRelativeRotation(value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (target->GetRelativeRotation() != value)
		{
			auto tweener = ULTweenBPLibrary::LocalRotatorTo(target, value, false, AnimationDuration, 0, ELTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyWidthWithAnimation(EUILayoutAnimationType tempAnimationType, float width, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemWidth(target, width);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (target->GetWidth() != width)
		{
			auto tweener = ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetWidth), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetWidth), width, AnimationDuration)->SetEase(ELTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyHeightWithAnimation(EUILayoutAnimationType tempAnimationType, float height, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemHeight(target, height);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (target->GetHeight() != height)
		{
			auto tweener = ULTweenManager::To(target, FLTweenFloatGetterFunction::CreateUObject(target, &UUIItem::GetHeight), FLTweenFloatSetterFunction::CreateUObject(target, &UUIItem::SetHeight), height, AnimationDuration)->SetEase(ELTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplySizeDeltaWithAnimation(EUILayoutAnimationType tempAnimationType, FVector2D sizeDelta, UUIItem* target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemSizeDelta(target, sizeDelta);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (target->GetSizeDelta() != sizeDelta)
		{
			auto tweener = ULTweenManager::To(target, FLTweenVector2DGetterFunction::CreateUObject(target, &UUIItem::GetSizeDelta), FLTweenVector2DSetterFunction::CreateUObject(target, &UUIItem::SetSizeDelta), sizeDelta, AnimationDuration)->SetEase(ELTweenEase::InOutSine);
			TweenerArray.Add(tweener);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::SetAnimationType(EUILayoutAnimationType value)
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
