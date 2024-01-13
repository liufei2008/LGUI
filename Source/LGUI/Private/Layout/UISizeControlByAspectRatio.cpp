// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByAspectRatio.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
DECLARE_CYCLE_STAT(TEXT("UILayout SizeControlByAspectRatioRebuildLayout"), STAT_SizeControlByAspectRatio, STATGROUP_LGUI);
void UUISizeControlByAspectRatio::SetControlMode(EUISizeControlByAspectRatioMode value)
{
	if (ControlMode != value)
	{
		ControlMode = value;
		MarkNeedRebuildLayout();
	}
}
void UUISizeControlByAspectRatio::SetAspectRatio(float value)
{
	if (AspectRatio != value)
	{
		AspectRatio = value;
		MarkNeedRebuildLayout();
	}
}

void UUISizeControlByAspectRatio::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_SizeControlByAspectRatio);
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAnimation();

	EUILayoutAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutAnimationType::Immediately;
	}
#endif
	switch (ControlMode)
	{
	case EUISizeControlByAspectRatioMode::HeightControlWidth:
	{
		ApplyWidthWithAnimation(tempAnimationType, RootUIComp->GetHeight() * AspectRatio, RootUIComp.Get());
	}
	break;
	case EUISizeControlByAspectRatioMode::WidthControlHeight:
	{
		ApplyHeightWithAnimation(tempAnimationType, RootUIComp->GetWidth() / AspectRatio, RootUIComp.Get());
	}
	break;
	case EUISizeControlByAspectRatioMode::FitInParent:
	{
		if (auto parent = RootUIComp->GetParentUIItem())
		{
			RootUIComp->SetAnchorMin(FVector2D(0, 0));
			RootUIComp->SetAnchorMax(FVector2D(1, 1));

			auto parentWidth = parent->GetWidth();
			auto parentHeight = parent->GetHeight();
			auto parentAspectRatio = parentWidth / parentHeight;
			if (parentAspectRatio > AspectRatio)
			{
				auto SizeDelta = RootUIComp->GetSizeDelta();
				SizeDelta.X = -(parentWidth - parentHeight * AspectRatio);
				SizeDelta.Y = 0;
				ApplySizeDeltaWithAnimation(tempAnimationType, SizeDelta, RootUIComp.Get());
			}
			else
			{
				auto SizeDelta = RootUIComp->GetSizeDelta();
				SizeDelta.Y = -(parentHeight - parentWidth / AspectRatio);
				SizeDelta.X = 0;
				ApplySizeDeltaWithAnimation(tempAnimationType, SizeDelta, RootUIComp.Get());
			}
		}
	}
	break;
	case EUISizeControlByAspectRatioMode::EnvelopeParent:
	{
		if (auto parent = RootUIComp->GetParentUIItem())
		{
			RootUIComp->SetAnchorMin(FVector2D(0, 0));
			RootUIComp->SetAnchorMax(FVector2D(1, 1));

			auto parentWidth = parent->GetWidth();
			auto parentHeight = parent->GetHeight();
			auto parentAspectRatio = parentWidth / parentHeight;
			if (parentAspectRatio > AspectRatio)
			{
				auto SizeDelta = RootUIComp->GetSizeDelta();
				SizeDelta.Y = -(parentHeight - parentWidth / AspectRatio);
				SizeDelta.X = 0;
				ApplySizeDeltaWithAnimation(tempAnimationType, SizeDelta, RootUIComp.Get());
			}
			else
			{
				auto SizeDelta = RootUIComp->GetSizeDelta();
				SizeDelta.X = -(parentWidth - parentHeight * AspectRatio);
				SizeDelta.Y = 0;
				ApplySizeDeltaWithAnimation(tempAnimationType, SizeDelta, RootUIComp.Get());
			}
		}
	}
	break;
	}
	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUISizeControlByAspectRatio::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this->GetRootUIComponent() == InUIItem)
	{
		OutResult.bCanControlHorizontalAnchor = OutResult.bCanControlVerticalAnchor =
			(ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		OutResult.bCanControlHorizontalSizeDelta =
			(ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth
				|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta =
			(ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight
				|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		return true;
	}
	else
	{
		return false;
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
