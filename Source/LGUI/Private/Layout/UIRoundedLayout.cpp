// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIRoundedLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

void UUIRoundedLayout::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;
	CancelAnimation();

	EUILayoutAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutAnimationType::Immediately;
	}
#endif

	const auto& uiChildrenList = GetLayoutUIItemChildren();
	int childrenCount = uiChildrenList.Num();
	float angleInterval = (EndAngle - StartAngle) / childrenCount;
	angleInterval = FMath::DegreesToRadians(angleInterval);
	float angle = FMath::DegreesToRadians(StartAngle);
	float sin = 0, cos = 0, x = 0, y = 0;
	for (int i = 0; i < childrenCount; i ++)
	{
		auto uiItem = uiChildrenList[i].ChildUIItem;
		uiItem->SetAnchorMin(FVector2D(0.5f, 0.5f));
		uiItem->SetAnchorMax(FVector2D(0.5f, 0.5f));
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * Radius;
		y = sin * Radius;
		ApplyAnchoredPositionWithAnimation(tempAnimationType, FVector2D(x, y), uiItem.Get());
		if (bSetChildAngle)
		{
			auto angleInDegree = FMath::RadiansToDegrees(angle);
			angleInDegree = -angleInDegree;
			auto Rotator = uiItem->GetRelativeRotation();
			if (Rotator.Roll != angleInDegree)
			{
				Rotator.Roll = angleInDegree;
				ApplyRotatorWithAnimation(tempAnimationType, Rotator, uiItem.Get());
			}
		}
		angle += angleInterval;
	}

	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUIRoundedLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this->GetRootUIComponent() == InUIItem)
	{
		return true;
	}
	else
	{
		if (InUIItem->GetAttachParent() != this->GetRootUIComponent())return false;
		UObject* layoutElement = nullptr;
		bool ignoreLayout = false;
		GetLayoutElement(InUIItem, layoutElement, ignoreLayout);
		if (ignoreLayout)
		{
			return true;
		}
		OutResult.bCanControlHorizontalAnchor = this->GetEnable();
		OutResult.bCanControlVerticalAnchor = this->GetEnable();
		OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
		OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
		return true;
	}
}
