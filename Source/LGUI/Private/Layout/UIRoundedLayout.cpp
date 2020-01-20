// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Layout/UIRoundedLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"


UUIRoundedLayout::UUIRoundedLayout()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIRoundedLayout::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;

	const auto& uiChildrenList = GetAvailableChildren();
	int childrenCount = uiChildrenList.Num();
	float angleInterval = (EndAngle - StartAngle) / childrenCount;
	angleInterval = FMath::DegreesToRadians(angleInterval);
	float angle = FMath::DegreesToRadians(StartAngle);
	float sin = 0, cos = 0, x = 0, y = 0;
	for (int i = 0; i < childrenCount; i ++)
	{
		auto uiItem = uiChildrenList[i].uiItem;
		uiItem->SetAnchorHAlign(UIAnchorHorizontalAlign::None);
		uiItem->SetAnchorVAlign(UIAnchorVerticalAlign::None);
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * Radius;
		y = sin * Radius;
		auto pos = uiItem->GetRelativeLocation();
		pos.X = x;
		pos.Y = y;
		if (bSetChildAngle)
		{
			uiItem->SetRelativeLocationAndRotation(pos, FQuat::MakeFromEuler(FVector(0, 0, FMath::RadiansToDegrees(angle))));
		}
		else
		{
			uiItem->SetRelativeLocation(pos);
		}
		angle += angleInterval;
	}
}

bool UUIRoundedLayout::CanControlChildAnchor()
{
	return true;
}
bool UUIRoundedLayout::CanControlChildWidth()
{
	return false;
}
bool UUIRoundedLayout::CanControlChildHeight()
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfHorizontalAnchor()
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfVerticalAnchor()
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfWidth()
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfHeight()
{
	return false;
}
