// Copyright 2019 LexLiu. All Rights Reserved.

#include "Layout/UIHorizontalLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"

DECLARE_CYCLE_STAT(TEXT("UILayout HorizontalRebuildLayout"), STAT_HorizontalLayout, STATGROUP_LGUI);

void UUIHorizontalLayout::OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (sizeChanged && child->IsUIActiveInHierarchy())
	{
		if (!ExpendChildrenWidth)
		{
			OnRebuildLayout();
		}
	}
}

UUIHorizontalLayout::UUIHorizontalLayout()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIHorizontalLayout::BeginPlay()
{
	Super::BeginPlay();
}
void UUIHorizontalLayout::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UUIHorizontalLayout::SetPadding(FMargin value)
{
	if (Padding != value)
	{
		Padding = value;
		OnRebuildLayout();
	}
}
void UUIHorizontalLayout::SetSpacing(float value)
{
	if (Spacing != value)
	{
		Spacing = value;
		OnRebuildLayout();
	}
}
void UUIHorizontalLayout::SetAlign(ELGUILayoutAlignmentType value)
{
	if (Align != value)
	{
		Align = value;
		OnRebuildLayout();
	}
}
void UUIHorizontalLayout::SetExpendChildrenWidth(bool value)
{
	if (ExpendChildrenWidth != value)
	{
		ExpendChildrenWidth = value;
		OnRebuildLayout();
	}
}
void UUIHorizontalLayout::SetExpendChildrenHeight(bool value)
{
	if (ExpendChildrenHeight != value)
	{
		ExpendChildrenHeight = value;
		OnRebuildLayout();
	}
}
void UUIHorizontalLayout::SetWidthFitToChildren(bool value)
{
	if (WidthFitToChildren != value)
	{
		WidthFitToChildren = value;
		OnRebuildLayout();
	}
}

void UUIHorizontalLayout::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_HorizontalLayout);
	if (!CheckRootUIComponent())return;
	FVector2D startPosition;
	startPosition.X = Padding.Left;
	startPosition.Y = -Padding.Top;//left bottom as start point
	FVector2D rectSize;
	rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
	rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

	const auto& uiChildrenList = GetAvailableChildren();
	int childrenCount = uiChildrenList.Num();
	float childHeight = rectSize.Y;
	childrenWidthList.Reset(childrenCount);
	ActuralRange = 0;
	if (ExpendChildrenWidth)
	{
		float sizeWithoutSpacing = rectSize.X - Spacing * (childrenCount - 1);
		float autoSizeChildrenSize = sizeWithoutSpacing;//collect all children's size which is autosize or without layout element
		int autoSizeChildrenCount = 0;
		for (auto item : uiChildrenList)
		{
			float tempChildWidth = 0;
			if (item.layoutElement != nullptr)
			{
				auto layoutType = item.layoutElement->GetLayoutType();
				switch (layoutType)
				{
				case ELayoutElementType::AutoSize:
					autoSizeChildrenCount++;
					break;
				case ELayoutElementType::ConstantSize:
					tempChildWidth = item.layoutElement->GetConstantSize();
					break;
				case ELayoutElementType::RatioSize:
					tempChildWidth = item.layoutElement->GetRatioSize() * sizeWithoutSpacing;
					break;
				}
			}
			else
			{
				autoSizeChildrenCount++;
			}
			autoSizeChildrenSize -= tempChildWidth;
			childrenWidthList.Add(tempChildWidth);
		}
		float autoSizeChildWidth = autoSizeChildrenSize / autoSizeChildrenCount;
		for (int i = 0; i < childrenCount; i++)
		{
			auto item = uiChildrenList[i];
			if (item.layoutElement != nullptr)
			{
				auto layoutType = item.layoutElement->GetLayoutType();
				if (layoutType == ELayoutElementType::AutoSize)
				{
					childrenWidthList[i] = autoSizeChildWidth;
				}
			}
			else
			{
				childrenWidthList[i] = autoSizeChildWidth;
			}
		}
	}
	else
	{
		switch (Align)
		{
		case ELGUILayoutAlignmentType::UpperLeft:
		case ELGUILayoutAlignmentType::MiddleLeft:
		case ELGUILayoutAlignmentType::LowerLeft:
		{
			
		}
		break;
		case ELGUILayoutAlignmentType::UpperCenter:
		case ELGUILayoutAlignmentType::MiddleCenter:
		case ELGUILayoutAlignmentType::LowerCenter:
		{
			float totalWidth = 0;
			for (int i = 0; i < childrenCount; i++)
			{
				auto uiItem = uiChildrenList[i].uiItem;
				totalWidth += uiItem->GetWidth();
			}
			totalWidth += Spacing * (childrenCount - 1);
			startPosition.X += (rectSize.X - totalWidth) * 0.5f;
		}
		break;
		case ELGUILayoutAlignmentType::UpperRight:
		case ELGUILayoutAlignmentType::MiddleRight:
		case ELGUILayoutAlignmentType::LowerRight:
		{
			float totalWidth = 0;
			for (int i = 0; i < childrenCount; i++)
			{
				auto uiItem = uiChildrenList[i].uiItem;
				totalWidth += uiItem->GetWidth();
			}
			totalWidth += Spacing * (childrenCount - 1);
			startPosition.X += (rectSize.X - totalWidth);
		}
		break;
		}
	}
	float posX = startPosition.X, posY = startPosition.Y;
	for (int i = 0; i < childrenCount; i ++)
	{
		auto uiItem = uiChildrenList[i].uiItem;
		uiItem->SetAnchorHAlign(UIAnchorHorizontalAlign::Left);
		uiItem->SetAnchorVAlign(UIAnchorVerticalAlign::Top);
		float childWidth;
		if (ExpendChildrenWidth)
		{
			childWidth = childrenWidthList[i];
			uiItem->SetWidth(childWidth);
		}
		else
		{
			childWidth = uiItem->GetWidth();
		}
		uiItem->SetAnchorOffsetX(posX + uiItem->GetPivot().X * childWidth);

		if (ExpendChildrenHeight)
		{
			uiItem->SetHeight(childHeight);
		}
		else
		{
			childHeight = uiItem->GetHeight();
			switch (Align)
			{
			default:
			case ELGUILayoutAlignmentType::UpperLeft:
			case ELGUILayoutAlignmentType::UpperCenter:
			case ELGUILayoutAlignmentType::UpperRight:
			{
				posY = startPosition.Y;
			}
			break;
			case ELGUILayoutAlignmentType::MiddleLeft:
			case ELGUILayoutAlignmentType::MiddleCenter:
			case ELGUILayoutAlignmentType::MiddleRight:
			{
				posY = startPosition.Y - (rectSize.Y - childHeight) * 0.5f;
			}
			break;
			case ELGUILayoutAlignmentType::LowerLeft:
			case ELGUILayoutAlignmentType::LowerCenter:
			case ELGUILayoutAlignmentType::LowerRight:
			{
				posY = startPosition.Y - rectSize.Y + childHeight;
			}
			break;
			}
		}
		uiItem->SetAnchorOffsetY(posY - (1.0f - uiItem->GetPivot().Y) * childHeight);
		posX += childWidth + Spacing;
		ActuralRange += childWidth;
	}
	if (childrenCount > 1)
	{
		ActuralRange += Spacing * (childrenCount - 1);
	}
	if (WidthFitToChildren)
	{
		auto thisWidth = ActuralRange + Padding.Left + Padding.Right;
		RootUIComp->SetWidth(thisWidth);
	}
}

bool UUIHorizontalLayout::CanControlChildAnchor()
{
	return true;
}
bool UUIHorizontalLayout::CanControlChildWidth()
{
	return GetExpendChildrenWidth();
}
bool UUIHorizontalLayout::CanControlChildHeight()
{
	return GetExpendChildrenHeight();
}
bool UUIHorizontalLayout::CanControlSelfHorizontalAnchor()
{ 
	return false; 
}
bool UUIHorizontalLayout::CanControlSelfVerticalAnchor()
{ 
	return false; 
}
bool UUIHorizontalLayout::CanControlSelfWidth()
{
	return !GetExpendChildrenWidth() && GetWidthFitToChildren();
}
bool UUIHorizontalLayout::CanControlSelfHeight()
{ 
	return false;
}
