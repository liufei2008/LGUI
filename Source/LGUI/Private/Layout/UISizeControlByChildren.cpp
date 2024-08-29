// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByChildren.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif
DECLARE_CYCLE_STAT(TEXT("UILayout ChildSizeFitterRebuildLayout"), STAT_ChildSizeFitter, STATGROUP_LGUI);
void UUISizeControlByChildren::SetWidthFitToChildren(bool Value)
{
	if (bWidthFitToChildren != Value)
	{
		bWidthFitToChildren = Value;
		MarkNeedRebuildLayout();
	}
}
void UUISizeControlByChildren::SetAdditionalWidth(float Value)
{
	if (AdditionalWidth != Value)
	{
		AdditionalWidth = Value;
		MarkNeedRebuildLayout();
	}
}
void UUISizeControlByChildren::SetHeightFitToChildren(bool Value)
{
	if (bHeightFitToChildren != Value)
	{
		bHeightFitToChildren = Value;
		MarkNeedRebuildLayout();
	}
}
void UUISizeControlByChildren::SetAdditionalHeight(float Value)
{
	if (AdditionalHeight != Value)
	{
		AdditionalHeight = Value;
		MarkNeedRebuildLayout();
	}
}

void UUISizeControlByChildren::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_ChildSizeFitter);
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAllAnimations();

	EUILayoutAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutAnimationType::Immediately;
	}
#endif

	float MaxChildWidth = 0, MaxChildHeight = 0;
	const auto& uiChildrenList = GetLayoutUIItemChildren();
	int childrenCount = uiChildrenList.Num();
	for (int i = 0; i < childrenCount; i++)
	{
		auto& item = uiChildrenList[i];
		float tempChildWidth = 0;
		if (!item.ChildUIItem->GetAnchorData().IsHorizontalStretched())//can't fit to stretched child, because it will result in loop reference
		{
			if (item.ChildUIItem->GetWidth() > MaxChildWidth)
			{
				MaxChildWidth = item.ChildUIItem->GetWidth();
			}
		}
		if (!item.ChildUIItem->GetAnchorData().IsVerticalStretched())//can't fit to stretched child, because it will result in loop reference
		{
			if (item.ChildUIItem->GetHeight() > MaxChildHeight)
			{
				MaxChildHeight = item.ChildUIItem->GetHeight();
			}
		}
	}

	auto AnchorMin = RootUIComp->GetAnchorMin();
	auto AnchorMax = RootUIComp->GetAnchorMax();
	if (bWidthFitToChildren)
	{
		if (AnchorMin.X != AnchorMax.X)
		{
			RootUIComp->SetHorizontalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}
		ApplyWidthWithAnimation(tempAnimationType, MaxChildWidth + AdditionalWidth, RootUIComp.Get());
	}
	if (bHeightFitToChildren)
	{
		if (AnchorMin.Y != AnchorMax.Y)
		{
			RootUIComp->SetVerticalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}
		ApplyHeightWithAnimation(tempAnimationType, MaxChildHeight + AdditionalHeight, RootUIComp.Get());
	}
	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		EndSetupAnimations();
	}
}

bool UUISizeControlByChildren::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this->GetRootUIComponent() == InUIItem)
	{
		OutResult.bCanControlHorizontalAnchor = OutResult.bCanControlVerticalAnchor = false;
		OutResult.bCanControlHorizontalSizeDelta = bWidthFitToChildren && this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta = bHeightFitToChildren && this->GetEnable();
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
