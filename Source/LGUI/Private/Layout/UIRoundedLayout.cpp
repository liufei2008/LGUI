// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UIRoundedLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"

void UUIRoundedLayout::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;
	if (!enable)return;

	const auto& uiChildrenList = GetAvailableChildren();
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
		uiItem->SetAnchoredPosition(FVector2D(x, y));
		if (bSetChildAngle)
		{
			uiItem->SetRelativeRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RadiansToDegrees(angle))));
		}
		angle += angleInterval;
	}
}
#if WITH_EDITOR
bool UUIRoundedLayout::CanControlChildAnchor()
{
	return true && enable;
}
bool UUIRoundedLayout::CanControlChildAnchorOffsetX()
{
	return true && enable;
}
bool UUIRoundedLayout::CanControlChildAnchorOffsetY()
{
	return true && enable;
}
bool UUIRoundedLayout::CanControlSelfAnchorOffsetX()
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfAnchorOffsetY()
{
	return false;
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
#endif
