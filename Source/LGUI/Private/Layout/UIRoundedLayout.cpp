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

bool UUIRoundedLayout::CanControlChildAnchor_Implementation()const
{
	return this->GetEnable();
}
bool UUIRoundedLayout::CanControlChildHorizontalAnchoredPosition_Implementation()const
{
	return this->GetEnable();
}
bool UUIRoundedLayout::CanControlChildVerticalAnchoredPosition_Implementation()const
{
	return this->GetEnable();
}
bool UUIRoundedLayout::CanControlChildWidth_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlChildHeight_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlChildAnchorLeft_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlChildAnchorRight_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlChildAnchorBottom_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlChildAnchorTop_Implementation()const
{
	return false;
}

bool UUIRoundedLayout::CanControlSelfAnchor_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfHorizontalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfVerticalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfWidth_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfHeight_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfAnchorLeft_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfAnchorRight_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfAnchorBottom_Implementation()const
{
	return false;
}
bool UUIRoundedLayout::CanControlSelfAnchorTop_Implementation()const
{
	return false;
}
