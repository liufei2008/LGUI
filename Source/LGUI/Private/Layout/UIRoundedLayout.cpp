// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UIRoundedLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

void UUIRoundedLayout::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;

	const auto& uiChildrenList = GetLayoutUIItemChildren();
	int childrenCount = uiChildrenList.Num();
	float angleInterval = (EndAngle - StartAngle) / childrenCount;
	angleInterval = FMath::DegreesToRadians(angleInterval);
	float angle = FMath::DegreesToRadians(StartAngle);
	float sin = 0, cos = 0, x = 0, y = 0;
	for (int i = 0; i < childrenCount; i ++)
	{
		auto uiItem = uiChildrenList[i].uiItem;
		uiItem->SetAnchorMin(FVector2D(0.5f, 0.5f));
		uiItem->SetAnchorMax(FVector2D(0.5f, 0.5f));
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * Radius;
		y = sin * Radius;
		ApplyUIItemAnchoredPosition(uiItem.Get(), FVector2D(x, y));
		if (bSetChildAngle)
		{
			uiItem->SetRelativeRotation(FQuat::MakeFromEuler(FVector(FMath::RadiansToDegrees(angle), 0, 0)));
		}
		angle += angleInterval;
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
		UActorComponent* layoutElement = nullptr;
		bool ignoreLayout = false;
		GetLayoutElement(InUIItem->GetOwner(), layoutElement, ignoreLayout);
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
