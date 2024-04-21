// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIGridLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

DECLARE_CYCLE_STAT(TEXT("UILayout GridRebuildLayout"), STAT_GridLayout, STATGROUP_LGUI);

void UUIGridLayout::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
	Super::OnUIChildDimensionsChanged(child, horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
}

void UUIGridLayout::SetPadding(FMargin value)
{
	if (Padding != value)
	{
		Padding = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetSpacing(FVector2D value)
{
	if (Spacing != value)
	{
		Spacing = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetAlign(ELGUILayoutAlignmentType value)
{
	if (Align != value)
	{
		Align = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetLastLineCanAlign(bool value)
{
	if (LastLineCanAlign != value)
	{
		LastLineCanAlign = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetHorizontalOrVertical(bool value)
{
	if (HorizontalOrVertical != value)
	{
		HorizontalOrVertical = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetDependOnSizeOrCount(bool value)
{
	if (DependOnSizeOrCount != value)
	{
		DependOnSizeOrCount = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetExpendChildSize(bool value)
{
	if (ExpendChildSize != value)
	{
		ExpendChildSize = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetCellSize(FVector2D value)
{
	if (CellSize != value)
	{
		CellSize = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetMaxItemCountInOneLine(int value)
{
	if (LineCount != value)
	{
		LineCount = FMath::Max(0, value);
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetWidthFitToChildren(bool value)
{
	if (WidthFitToChildren != value)
	{
		WidthFitToChildren = value;
		MarkNeedRebuildLayout();
	}
}
void UUIGridLayout::SetHeightFitToChildren(bool value)
{
	if (HeightFitToChildren != value)
	{
		HeightFitToChildren = value;
		MarkNeedRebuildLayout();
	}
}

void UUIGridLayout::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_GridLayout);
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAnimation();

	FVector2D startPosition;
	startPosition.X = Padding.Left;
	startPosition.Y = -Padding.Top;//left top as start point
	FVector2D rectSize;
	rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
	rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

	float endPosX = startPosition.X + rectSize.X;
	float endPosY = startPosition.Y - rectSize.Y;

	const auto& uiChildrenList = GetLayoutUIItemChildren();
	int childrenCount = uiChildrenList.Num();
	float childWidth = CellSize.X, childHeight = CellSize.Y;

	int maxItemCountInOneLine = FMath::Max(0, (int)LineCount);
	if (!DependOnSizeOrCount)//depend on count
	{
		if (HorizontalOrVertical)
		{
			childWidth = (rectSize.X - Spacing.X * (maxItemCountInOneLine - 1)) / maxItemCountInOneLine;
			if (ExpendChildSize)//expend height to fill
			{
				int verticalCount = FMath::CeilToInt((float)childrenCount / maxItemCountInOneLine);
				childHeight = (rectSize.Y - Spacing.Y * (verticalCount - 1)) / verticalCount;
			}
		}
		else
		{
			childHeight = (rectSize.Y - Spacing.Y * (maxItemCountInOneLine - 1)) / maxItemCountInOneLine;
			if (ExpendChildSize)//expend width to fill
			{
				int horizontalCount = FMath::CeilToInt((float)childrenCount / maxItemCountInOneLine);
				childWidth = (rectSize.X - Spacing.X * (horizontalCount - 1)) / horizontalCount;
			}
		}
	}

	EUILayoutAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutAnimationType::Immediately;
	}
#endif

	int lineCount = 1;
	int itemCountPerLine = 0;
	if (!DependOnSizeOrCount)//depend on count
	{
		itemCountPerLine = maxItemCountInOneLine;
		lineCount = childrenCount / itemCountPerLine + 1;
	}
	else
	{
		float posX = startPosition.X, posY = startPosition.Y;
		bool canCalculateItemCountPerLine = true;
		for (int i = 0; i < childrenCount; i++)
		{
			if (HorizontalOrVertical)//use horizontal
			{
				if ((posX - Spacing.X + childWidth) > endPosX//if horizontal exceed range, then newline
					)
				{
					posX = startPosition.X;
					lineCount++;
					canCalculateItemCountPerLine = false;
				}
				if (canCalculateItemCountPerLine)
				{
					itemCountPerLine++;
				}
				posX += childWidth + Spacing.X;
			}
			else//use vertical
			{
				if ((posY + Spacing.Y - childHeight) < endPosY//if vertical exceed range, then newline
					)
				{
					posY = startPosition.Y;
					lineCount++;
					canCalculateItemCountPerLine = false;
				}
				if (canCalculateItemCountPerLine)
				{
					itemCountPerLine++;
				}
				posY -= childHeight + Spacing.Y;
			}
		}
	}
	float posOffsetX = 0, posOffsetY = 0;
	float lastLinePosOffsetX = 0, lastLinePosOffsetY = 0;
	//if (DependOnSizeOrCount)//depend on size
	{
		if (HorizontalOrVertical)
		{
			switch (Align)
			{
			default:
			case ELGUILayoutAlignmentType::UpperLeft:
			case ELGUILayoutAlignmentType::MiddleLeft:
			case ELGUILayoutAlignmentType::LowerLeft:
				break;
			case ELGUILayoutAlignmentType::UpperCenter:
			case ELGUILayoutAlignmentType::MiddleCenter:
			case ELGUILayoutAlignmentType::LowerCenter:
			{
				posOffsetX = rectSize.X - (childWidth * itemCountPerLine + (Spacing.X * (itemCountPerLine - 1)));
				posOffsetX *= 0.5f;
				if (lineCount > 1 && LastLineCanAlign)
				{
					int lastLineItemCount = childrenCount % itemCountPerLine;
					if (lastLineItemCount == 0)
					{
						lastLinePosOffsetX = posOffsetX;
					}
					else
					{
						lastLinePosOffsetX = rectSize.X - (childWidth * lastLineItemCount + (Spacing.X * (lastLineItemCount - 1)));
						lastLinePosOffsetX *= 0.5f;
					}
				}
			}
				break;
			case ELGUILayoutAlignmentType::UpperRight:
			case ELGUILayoutAlignmentType::MiddleRight:
			case ELGUILayoutAlignmentType::LowerRight:
			{
				posOffsetX = rectSize.X - (childWidth * itemCountPerLine + (Spacing.X * (itemCountPerLine - 1)));
				if (lineCount > 1 && LastLineCanAlign)
				{
					int lastLineItemCount = childrenCount % itemCountPerLine;
					if (lastLineItemCount == 0)
					{
						lastLinePosOffsetX = posOffsetX;
					}
					else
					{
						lastLinePosOffsetX = rectSize.X - (childWidth * lastLineItemCount + (Spacing.X * (lastLineItemCount - 1)));
					}
				}
			}
				break;
			}
			switch (Align)
			{
			default:
			case ELGUILayoutAlignmentType::UpperLeft:
			case ELGUILayoutAlignmentType::UpperCenter:
			case ELGUILayoutAlignmentType::UpperRight:
				break;
			case ELGUILayoutAlignmentType::MiddleLeft:
			case ELGUILayoutAlignmentType::MiddleCenter:
			case ELGUILayoutAlignmentType::MiddleRight:
			{
				if (!HeightFitToChildren)
				{
					posOffsetY = rectSize.Y - (childHeight * lineCount + (Spacing.Y * (lineCount - 1)));
					posOffsetY *= 0.5f;
				}
			}
				break;
			case ELGUILayoutAlignmentType::LowerLeft:
			case ELGUILayoutAlignmentType::LowerCenter:
			case ELGUILayoutAlignmentType::LowerRight:
			{
				if (!HeightFitToChildren)
				{
					posOffsetY = rectSize.Y - (childHeight * lineCount + (Spacing.Y * (lineCount - 1)));
				}
			}
				break;
			}
		}
		else
		{
			switch (Align)
			{
			default:
			case ELGUILayoutAlignmentType::UpperLeft:
			case ELGUILayoutAlignmentType::MiddleLeft:
			case ELGUILayoutAlignmentType::LowerLeft:
				break;
			case ELGUILayoutAlignmentType::UpperCenter:
			case ELGUILayoutAlignmentType::MiddleCenter:
			case ELGUILayoutAlignmentType::LowerCenter:
			{
				if (!WidthFitToChildren)
				{
					posOffsetX = rectSize.X - (childWidth * lineCount + (Spacing.X * (lineCount - 1)));
					posOffsetX *= 0.5f;
				}
			}
			break;
			case ELGUILayoutAlignmentType::UpperRight:
			case ELGUILayoutAlignmentType::MiddleRight:
			case ELGUILayoutAlignmentType::LowerRight:
			{
				if (!WidthFitToChildren)
				{
					posOffsetX = rectSize.X - (childWidth * lineCount + (Spacing.X * (lineCount - 1)));
				}
			}
			break;
			}
			switch (Align)
			{
			default:
			case ELGUILayoutAlignmentType::UpperLeft:
			case ELGUILayoutAlignmentType::UpperCenter:
			case ELGUILayoutAlignmentType::UpperRight:
				break;
			case ELGUILayoutAlignmentType::MiddleLeft:
			case ELGUILayoutAlignmentType::MiddleCenter:
			case ELGUILayoutAlignmentType::MiddleRight:
			{
				posOffsetY = rectSize.Y - (childHeight * itemCountPerLine + (Spacing.Y * (itemCountPerLine - 1)));
				posOffsetY *= 0.5f;
				if (lineCount > 1 && LastLineCanAlign)
				{
					int lastLineItemCount = childrenCount % itemCountPerLine;
					if (lastLineItemCount == 0)
					{
						lastLinePosOffsetY = posOffsetY;
					}
					else
					{
						lastLinePosOffsetY = rectSize.Y - (childHeight * lastLineItemCount + (Spacing.Y * (lastLineItemCount - 1)));
						lastLinePosOffsetY *= 0.5f;
					}
				}
			}
			break;
			case ELGUILayoutAlignmentType::LowerLeft:
			case ELGUILayoutAlignmentType::LowerCenter:
			case ELGUILayoutAlignmentType::LowerRight:
			{
				posOffsetY = rectSize.Y - (childHeight * itemCountPerLine + (Spacing.Y * (itemCountPerLine - 1)));
				if (lineCount > 1 && LastLineCanAlign)
				{
					int lastLineItemCount = childrenCount % itemCountPerLine;
					if (lastLineItemCount == 0)
					{
						lastLinePosOffsetY = posOffsetY;
					}
					else
					{
						lastLinePosOffsetY = rectSize.Y - (childHeight * lastLineItemCount + (Spacing.Y * (lastLineItemCount - 1)));
					}
				}
			}
			break;
			}
		}
	}

	FVector2D tempActuralRange = FVector2D(0, 0);
	if (HorizontalOrVertical)
	{
		tempActuralRange.X = childWidth * itemCountPerLine + (Spacing.X * (itemCountPerLine - 1));
		tempActuralRange.Y = childHeight;
	}
	else
	{
		tempActuralRange.Y = childHeight * itemCountPerLine + (Spacing.Y * (itemCountPerLine - 1));
		tempActuralRange.X = childWidth;
	}

	float posX = startPosition.X + posOffsetX, posY = startPosition.Y - posOffsetY;
	int tempItemCountPerLine = 0;
	int tempLineIndex = 1;
	for (int i = 0; i < childrenCount; i++)
	{
		auto uiItem = uiChildrenList[i].ChildUIItem;
		float anchorOffsetX, anchorOffsetY;
		if (HorizontalOrVertical)//use horizontal
		{
			tempItemCountPerLine++;
			if (tempItemCountPerLine > itemCountPerLine//if horizontal exceed range, then newline
				)
			{
				tempLineIndex++;
				if (lineCount != 1//not first line
					&& tempLineIndex >= lineCount//is last line
					&& LastLineCanAlign)
				{
					posX = startPosition.X + lastLinePosOffsetX;
				}
				else
				{
					posX = startPosition.X + posOffsetX;
				}
				posY -= childHeight + Spacing.Y;
				tempActuralRange.Y += childHeight + Spacing.Y;
				tempItemCountPerLine = 1;
			}
			anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
			anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
			posX += childWidth + Spacing.X;
		}
		else//use vertical
		{
			tempItemCountPerLine++;
			if (tempItemCountPerLine > itemCountPerLine//if vertical exceed range, then newline
				)
			{
				tempLineIndex++;
				if (lineCount != 1 && tempLineIndex >= lineCount && LastLineCanAlign)
				{
					posY = startPosition.Y - lastLinePosOffsetY;
				}
				else
				{
					posY = startPosition.Y - posOffsetY;
				}
				posX += childWidth + Spacing.X;
				tempActuralRange.X += childWidth + Spacing.X;
				tempItemCountPerLine = 1;
			}
			anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
			anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
			posY -= childHeight + Spacing.Y;
		}

		auto AnchorMin = uiItem->GetAnchorMin();
		auto AnchorMax = uiItem->GetAnchorMax();
		if (AnchorMin.Y != AnchorMax.Y || AnchorMin.X != AnchorMax.X)//custom anchor not support
		{
			AnchorMin = FVector2D(0, 1);
			AnchorMax = FVector2D(0, 1);
			uiItem->SetHorizontalAndVerticalAnchorMinMax(AnchorMin, AnchorMax, true, true);
		}
		anchorOffsetX -= AnchorMin.X * RootUIComp->GetWidth();
		anchorOffsetY += (1 - AnchorMin.Y) * RootUIComp->GetHeight();
		ApplyAnchoredPositionWithAnimation(tempAnimationType, FVector2D(anchorOffsetX, anchorOffsetY), uiItem.Get());
		ApplyWidthWithAnimation(tempAnimationType, childWidth, uiItem.Get());
		ApplyHeightWithAnimation(tempAnimationType, childHeight, uiItem.Get());
	}

	if (HorizontalOrVertical)
	{
		if (HeightFitToChildren)
		{
			if (DependOnSizeOrCount || ExpendChildSize == false)
			{
				auto thisHeight = tempActuralRange.Y + Padding.Top + Padding.Bottom;
				ActuralRange = tempActuralRange;

				ApplyHeightWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
			}
		}
	}
	else
	{
		if (WidthFitToChildren)
		{
			if (DependOnSizeOrCount || ExpendChildSize == false)
			{
				auto thisWidth = tempActuralRange.X + Padding.Left + Padding.Right;
				ActuralRange = tempActuralRange;

				ApplyWidthWithAnimation(tempAnimationType, thisWidth, RootUIComp.Get());
			}
		}
	}

	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUIGridLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this->GetRootUIComponent() == InUIItem)
	{
		OutResult.bCanControlHorizontalSizeDelta = (!HorizontalOrVertical && (WidthFitToChildren && (DependOnSizeOrCount || ExpendChildSize == false))) && this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta = (HorizontalOrVertical && (HeightFitToChildren && (DependOnSizeOrCount || ExpendChildSize == false))) && this->GetEnable();
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
		OutResult.bCanControlHorizontalSizeDelta = this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta = this->GetEnable();
		return true;
	}
}
