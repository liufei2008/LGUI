// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UILayoutWithAnimation.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "LTweenManager.h"
#include "LTweenBPLibrary.h"
#include "Core/LGUISettings.h"


UUILayoutWithAnimation_CustomAnimation::UUILayoutWithAnimation_CustomAnimation()
{
	bCanExecuteBlueprintEvent = GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native);
}
void UUILayoutWithAnimation_CustomAnimation::BeginSetupAnimations()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveBeginSetupAnimations();
	}
}
void UUILayoutWithAnimation_CustomAnimation::ApplyAnchoredPositionAnimation(const FVector2D& Value, UUIItem* Target)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplyAnchoredPositionAnimation(Value, Target);
	}
}
void UUILayoutWithAnimation_CustomAnimation::ApplyRotatorAnimation(const FRotator& Value, UUIItem* Target)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplyRotatorAnimation(Value, Target);
	}
}
void UUILayoutWithAnimation_CustomAnimation::ApplyWidthAnimation(float Value, UUIItem* Target)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplyWidthAnimation(Value, Target);
	}
}
void UUILayoutWithAnimation_CustomAnimation::ApplyHeightAnimation(float Value, UUIItem* Target)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplyHeightAnimation(Value, Target);
	}
}
void UUILayoutWithAnimation_CustomAnimation::ApplySizeDeltaAnimation(const FVector2D& Value, UUIItem* Target)
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveApplySizeDeltaAnimation(Value, Target);
	}
}
void UUILayoutWithAnimation_CustomAnimation::EndSetupAnimations()
{
	if (bCanExecuteBlueprintEvent)
	{
		ReceiveEndSetupAnimations();
	}
}


void UUILayoutWithAnimation::BeginSetupAnimations()
{
	if (AnimationType == EUILayoutAnimationType::Custom && IsValid(CustomAnimation))
	{
		CustomAnimation->BeginSetupAnimations();
	}
	CancelAllAnimations(false);
}
void UUILayoutWithAnimation::CancelAllAnimations(bool callComplete)
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

void UUILayoutWithAnimation::EndSetupAnimations()
{
	if (AnimationType == EUILayoutAnimationType::Custom && IsValid(CustomAnimation))
	{
		CustomAnimation->EndSetupAnimations();
	}
	bIsAnimationPlaying = true;
	auto Tweener = ULTweenManager::VirtualTo(this, AnimationDuration)->OnComplete(FSimpleDelegate::CreateWeakLambda(this, [this] {
		bIsAnimationPlaying = false;
		if (bShouldRebuildLayoutAfterAnimation)
		{
			bShouldRebuildLayoutAfterAnimation = false;
			MarkNeedRebuildLayout();
		}
		}));
	if (Tweener)
	{
		bool bAffectByGamePause = false;
		bool bAffectByTimeDilation = false;
		if (this->GetRootUIComponent())
		{
			if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
			{
				bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
				bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
			}
			else
			{
				bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
				bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
			}
		}
		Tweener->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
		TweenerArray.Add(Tweener);
	}
}

void UUILayoutWithAnimation::ApplyAnchoredPositionWithAnimation(EUILayoutAnimationType tempAnimationType, FVector2D Value, UUIItem* Target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemAnchoredPosition(Target, Value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (Target->GetAnchoredPosition() != Value)
		{
			auto Tweener = ULTweenManager::To(Target
				, FLTweenVector2DGetterFunction::CreateUObject(Target, &UUIItem::GetAnchoredPosition)
				, FLTweenVector2DSetterFunction::CreateUObject(Target, &UUIItem::SetAnchoredPosition)
				, Value, AnimationDuration);
			if (Tweener)
			{
				bool bAffectByGamePause = false;
				bool bAffectByTimeDilation = false;
				if (this->GetRootUIComponent())
				{
					if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
					}
					else
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
					}
				}
				Tweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
				TweenerArray.Add(Tweener);
			}
		}
	}
	break;
	case EUILayoutAnimationType::Custom:
	{
		if (IsValid(CustomAnimation))
		{
			CustomAnimation->ApplyAnchoredPositionAnimation(Value, Target);
		}
		else
		{
			ApplyUIItemAnchoredPosition(Target, Value);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyRotatorWithAnimation(EUILayoutAnimationType tempAnimationType, const FRotator& Value, UUIItem* Target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		Target->SetRelativeRotation(Value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (Target->GetRelativeRotation() != Value)
		{
			auto Tweener = ULTweenBPLibrary::LocalRotatorTo(Target, Value, false, AnimationDuration, 0, ELTweenEase::InOutSine);
			if (Tweener)
			{
				bool bAffectByGamePause = false;
				bool bAffectByTimeDilation = false;
				if (this->GetRootUIComponent())
				{
					if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
					}
					else
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
					}
				}
				Tweener->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
				TweenerArray.Add(Tweener);
			}
		}
	}
	break;
	case EUILayoutAnimationType::Custom:
	{
		if (IsValid(CustomAnimation))
		{
			CustomAnimation->ApplyRotatorAnimation(Value, Target);
		}
		else
		{
			Target->SetRelativeRotation(Value);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyWidthWithAnimation(EUILayoutAnimationType tempAnimationType, float Value, UUIItem* Target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemWidth(Target, Value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (Target->GetWidth() != Value)
		{
			auto Tweener = ULTweenManager::To(Target
				, FLTweenFloatGetterFunction::CreateUObject(Target, &UUIItem::GetWidth)
				, FLTweenFloatSetterFunction::CreateUObject(Target, &UUIItem::SetWidth)
				, Value, AnimationDuration);
			if (Tweener)
			{
				bool bAffectByGamePause = false;
				bool bAffectByTimeDilation = false;
				if (this->GetRootUIComponent())
				{
					if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
					}
					else
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
					}
				}
				Tweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
				TweenerArray.Add(Tweener);
			}
		}
	}
	break;
	case EUILayoutAnimationType::Custom:
	{
		if (IsValid(CustomAnimation))
		{
			CustomAnimation->ApplyWidthAnimation(Value, Target);
		}
		else
		{
			ApplyUIItemWidth(Target, Value);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplyHeightWithAnimation(EUILayoutAnimationType tempAnimationType, float Value, UUIItem* Target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemHeight(Target, Value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (Target->GetHeight() != Value)
		{
			auto Tweener = ULTweenManager::To(Target
				, FLTweenFloatGetterFunction::CreateUObject(Target, &UUIItem::GetHeight)
				, FLTweenFloatSetterFunction::CreateUObject(Target, &UUIItem::SetHeight)
				, Value, AnimationDuration);
			if (Tweener)
			{
				bool bAffectByGamePause = false;
				bool bAffectByTimeDilation = false;
				if (this->GetRootUIComponent())
				{
					if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
					}
					else
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
					}
				}
				Tweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
				TweenerArray.Add(Tweener);
			}
		}
	}
	break;
	case EUILayoutAnimationType::Custom:
	{
		if (IsValid(CustomAnimation))
		{
			CustomAnimation->ApplyHeightAnimation(Value, Target);
		}
		else
		{
			ApplyUIItemHeight(Target, Value);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::ApplySizeDeltaWithAnimation(EUILayoutAnimationType tempAnimationType, FVector2D Value, UUIItem* Target)
{
	switch (tempAnimationType)
	{
	default:
	case EUILayoutAnimationType::Immediately:
	{
		ApplyUIItemSizeDelta(Target, Value);
	}
	break;
	case EUILayoutAnimationType::EaseAnimation:
	{
		if (Target->GetSizeDelta() != Value)
		{
			auto Tweener = ULTweenManager::To(Target
				, FLTweenVector2DGetterFunction::CreateUObject(Target, &UUIItem::GetSizeDelta)
				, FLTweenVector2DSetterFunction::CreateUObject(Target, &UUIItem::SetSizeDelta)
				, Value, AnimationDuration);
			if (Tweener)
			{
				bool bAffectByGamePause = false;
				bool bAffectByTimeDilation = false;
				if (this->GetRootUIComponent())
				{
					if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByTimeDilation;
					}
					else
					{
						bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						bAffectByTimeDilation = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByTimeDilation;
					}
				}
				Tweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
				TweenerArray.Add(Tweener);
			}
		}
	}
	break;
	case EUILayoutAnimationType::Custom:
	{
		if (IsValid(CustomAnimation))
		{
			CustomAnimation->ApplySizeDeltaAnimation(Value, Target);
		}
		else
		{
			ApplyUIItemSizeDelta(Target, Value);
		}
	}
	break;
	}
}

void UUILayoutWithAnimation::SetAnimationType(EUILayoutAnimationType Value)
{
	if (AnimationType != Value)
	{
		AnimationType = Value;
	}
}
void UUILayoutWithAnimation::SetAnimationDuration(float Value)
{
	if (AnimationDuration != Value)
	{
		AnimationDuration = Value;
	}
}
void UUILayoutWithAnimation::SetCustomAnimation(UUILayoutWithAnimation_CustomAnimation* Value)
{
	if (CustomAnimation != Value)
	{
		CustomAnimation = Value;
	}
}
