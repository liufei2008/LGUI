// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UIGridLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "LTweenActor.h"
#include "LTweenBPLibrary.h"
#include "Core/Actor/LGUIManagerActor.h"

DECLARE_CYCLE_STAT(TEXT("UILayout GridRebuildLayout"), STAT_GridLayout, STATGROUP_LGUI);

void UUIGridLayout::SetPadding(FMargin value)
{
	if (Padding != value)
	{
		Padding = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetSpacing(FVector2D value)
{
	if (Spacing != value)
	{
		Spacing = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetHorizontalOrVertical(bool value)
{
	if (HorizontalOrVertical != value)
	{
		HorizontalOrVertical = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetDependOnSizeOrCount(bool value)
{
	if (DependOnSizeOrCount != value)
	{
		DependOnSizeOrCount = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetExpendChildSize(bool value)
{
	if (ExpendChildSize != value)
	{
		ExpendChildSize = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetCellSize(FVector2D value)
{
	if (CellSize != value)
	{
		CellSize = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetLineCount(int value)
{
	if (LineCount != value)
	{
		LineCount = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetWidthFitToChildren(bool value)
{
	if (WidthFitToChildren != value)
	{
		WidthFitToChildren = value;
		OnRebuildLayout();
	}
}
void UUIGridLayout::SetHeightFitToChildren(bool value)
{
	if (HeightFitToChildren != value)
	{
		HeightFitToChildren = value;
		OnRebuildLayout();
	}
}

void UUIGridLayout::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_GridLayout);
	if (!CheckRootUIComponent())return;
	if (!enable)return;
	FVector2D startPosition;
	startPosition.X = Padding.Left;
	startPosition.Y = -Padding.Top;//left top as start point
	FVector2D rectSize;
	rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
	rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

	float endPosX = startPosition.X + rectSize.X;
	float endPosY = startPosition.Y - rectSize.Y;

	const auto& uiChildrenList = GetAvailableChildren();
	int childrenCount = uiChildrenList.Num();
	float childWidth = CellSize.X, childHeight = CellSize.Y;
	ActuralRange.X = ActuralRange.Y = 0;
	if (DependOnSizeOrCount == false)//depend on count
	{
		if (HorizontalOrVertical)
		{
			childWidth = (rectSize.X - Spacing.X * (LineCount - 1)) / LineCount;
			if (ExpendChildSize)//expend height to fill
			{
				int verticalCount = FMath::CeilToInt((float)childrenCount / LineCount);
				childHeight = (rectSize.Y - Spacing.Y * (verticalCount - 1)) / verticalCount;
			}
		}
		else
		{
			childHeight = (rectSize.Y - Spacing.Y * (LineCount - 1)) / LineCount;
			if (ExpendChildSize)//expend width to fill
			{
				int horizontalCount = FMath::CeilToInt((float)childrenCount / LineCount);
				childWidth = (rectSize.X - Spacing.X * (horizontalCount - 1)) / horizontalCount;
			}
		}
	}
	if (HorizontalOrVertical)
	{
		ActuralRange.X = rectSize.X;
		ActuralRange.Y += childHeight;
	}
	else
	{
		ActuralRange.Y = rectSize.Y;
		ActuralRange.X += childWidth;
	}
	float posX = startPosition.X, posY = startPosition.Y;
	for (int i = 0; i < childrenCount; i ++)
	{
		auto uiItem = uiChildrenList[i].uiItem;
		uiItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Left);
		uiItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top);
		uiItem->SetWidth(childWidth);
		uiItem->SetHeight(childHeight);

		float anchorOffsetX, anchorOffsetY;
		if (HorizontalOrVertical)//use horizontal
		{
			if (posX - Spacing.X + childWidth > endPosX//if horizontal exceed range, then newline
				&& i != 0//first one cannot add newline
				)
			{
				posX = startPosition.X;
				posY -= childHeight + Spacing.Y;
				ActuralRange.Y += childHeight + Spacing.Y;
			}
			anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
			anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
			posX += childWidth + Spacing.X;
		}
		else//use vertical
		{
			if (posY + Spacing.Y - childHeight < endPosY//if vertical exceed range, then newline
				&& i != 0//first one cannot add newline
				)
			{
				posX += childWidth + Spacing.X;
				posY = startPosition.Y;
				ActuralRange.X += childWidth + Spacing.X;
			}
			anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
			anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
			posY -= childHeight + Spacing.Y;
		}

		EUILayoutChangePositionAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
		if (!ALGUIManagerActor::IsPlaying)
		{
			tempAnimationType = EUILayoutChangePositionAnimationType::Immediately;
		}
#endif
		switch (tempAnimationType)
		{
		default:
		case EUILayoutChangePositionAnimationType::Immediately:
		{
			uiItem->SetAnchorOffsetX(anchorOffsetX);
			uiItem->SetAnchorOffsetY(anchorOffsetY);
		}
		break;
		case EUILayoutChangePositionAnimationType::EaseAnimation:
		{
			auto tweener1 = ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(uiItem.Get(), &UUIItem::GetAnchorOffsetX), FLTweenFloatSetterFunction::CreateUObject(uiItem.Get(), &UUIItem::SetAnchorOffsetX), anchorOffsetX, AnimationDuration)->SetEase(LTweenEase::InOutSine);
			auto tweener2 = ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(uiItem.Get(), &UUIItem::GetAnchorOffsetY), FLTweenFloatSetterFunction::CreateUObject(uiItem.Get(), &UUIItem::SetAnchorOffsetY), anchorOffsetY, AnimationDuration)->SetEase(LTweenEase::InOutSine);
			TweenerArray.Add(tweener1);
			TweenerArray.Add(tweener2);
		}
		break;
		}
	}
	if (HorizontalOrVertical)
	{
		if (HeightFitToChildren)
		{
			if (DependOnSizeOrCount || (DependOnSizeOrCount == false && ExpendChildSize == false))
			{
				auto thisHeight = ActuralRange.Y + Padding.Top + Padding.Bottom;
				RootUIComp->SetHeight(thisHeight);
			}
		}
	}
	else
	{
		if (WidthFitToChildren)
		{
			if (DependOnSizeOrCount || (DependOnSizeOrCount == false && ExpendChildSize == false))
			{
				auto thisWidth = ActuralRange.X + Padding.Left + Padding.Right;
				RootUIComp->SetWidth(thisWidth);
			}
		}
	}
}
#if WITH_EDITOR
bool UUIGridLayout::CanControlChildAnchor()
{
	return true && enable;
}
bool UUIGridLayout::CanControlChildWidth()
{
	return true && enable;
}
bool UUIGridLayout::CanControlChildHeight()
{
	return true && enable;
}
bool UUIGridLayout::CanControlSelfHorizontalAnchor()
{
	return false;
}
bool UUIGridLayout::CanControlSelfVerticalAnchor()
{
	return false;
}
bool UUIGridLayout::CanControlSelfWidth()
{
	return (!GetHorizontalOrVertical() && GetWidthFitToChildren()) && enable;
}
bool UUIGridLayout::CanControlSelfHeight()
{
	return (GetHorizontalOrVertical() && GetHeightFitToChildren()) && enable;
}
#endif
