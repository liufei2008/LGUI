// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIFlexibleGridLayout.h"
#include "Layout/UIFlexibleGridLayoutElement.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

DECLARE_CYCLE_STAT(TEXT("UILayout FlexiableGridRebuildLayout"), STAT_FlexiableGridLayout, STATGROUP_LGUI);


UUIFlexibleGridLayout::UUIFlexibleGridLayout()
{
	Rows = Columns =
	{
		FUIFlexibleGridLayoutCellData(1.0f),
		FUIFlexibleGridLayoutCellData(1.0f),
	};
}

#if WITH_EDITOR
void UUIFlexibleGridLayout::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIFlexibleGridLayout::GetLayoutElement(UUIItem* InChild, UObject*& OutLayoutElement, bool& OutIgnoreLayout)const
{
	auto LayoutElement = InChild->GetOwner()->FindComponentByClass<UUIFlexibleGridLayoutElement>();
	if (LayoutElement == nullptr || !LayoutElement->GetEnable())
	{
		OutLayoutElement = nullptr;
		OutIgnoreLayout = true;
	}
	else
	{
		OutLayoutElement = LayoutElement;
		OutIgnoreLayout = LayoutElement->GetIgnoreLayout();
	}
}

void UUIFlexibleGridLayout::SetPadding(FMargin value)
{
	if (Padding != value)
	{
		Padding = value;
		MarkNeedRebuildLayout();
	}
}

void UUIFlexibleGridLayout::SetRows(const TArray<FUIFlexibleGridLayoutCellData>& value)
{
	if (Rows != value)
	{
		Rows = value;
		MarkNeedRebuildLayout();
	}
}
void UUIFlexibleGridLayout::SetColumns(const TArray<FUIFlexibleGridLayoutCellData>& value)
{
	if (Columns != value)
	{
		Columns = value;
		MarkNeedRebuildLayout();
	}
}
void UUIFlexibleGridLayout::SetSpacing(const FVector2D& value)
{
	if (Spacing != value)
	{
		Spacing = value;
		MarkNeedRebuildLayout();
	}
}

void UUIFlexibleGridLayout::OnRebuildLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_FlexiableGridLayout);
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

	EUILayoutAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutAnimationType::Immediately;
	}
#endif

	float columnTotalRatio = 0, rowTotalRatio = 0;
	float columnTotalContstantSize = 0, rowTotalConstantSize = 0;
	for (auto& Item : Columns)
	{
		if (Item.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
		{
			columnTotalRatio += Item.Size;
		}
		else
		{
			columnTotalContstantSize += Item.Size;
		}
	}
	for (auto& Item : Rows)
	{
		if (Item.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
		{
			rowTotalRatio += Item.Size;
		}
		else
		{
			rowTotalConstantSize += Item.Size;
		}
	}
	float invColumnTotalRatio = 1.0f / columnTotalRatio;
	float invRowTotalRatio = 1.0f / rowTotalRatio;
	float thisFreeWidth = rectSize.X - columnTotalContstantSize - Spacing.X * (Columns.Num() - 1);//width exclude constant size and all space
	float thisFreeHeight = rectSize.Y - rowTotalConstantSize - Spacing.Y * (Rows.Num() - 1);//height exclude constant size and all space
	auto GetOffset = [&](int columnIndex, int rowIndex)
	{
		float offsetXRatio = 0, offsetYRatio = 0;
		float offsetXConstant = 0, offsetYConstant = 0;
		for (int i = 0; i < columnIndex; i++)
		{
			auto& Column = Columns[i];
			if (Column.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				offsetXRatio += Column.Size;
			}
			else
			{
				offsetXConstant += Column.Size;
			}
		}
		for (int i = 0; i < rowIndex; i++)
		{
			auto& Row = Rows[i];
			if (Row.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				offsetYRatio += Row.Size;
			}
			else
			{
				offsetYConstant += Row.Size;
			}
		}
		return startPosition + FVector2D(
			offsetXRatio * invColumnTotalRatio * thisFreeWidth + offsetXConstant + columnIndex * Spacing.X
			, -offsetYRatio * invRowTotalRatio * thisFreeHeight - offsetYConstant - rowIndex * Spacing.Y
		);
	};
	const auto& LayoutElementList = GetLayoutUIItemChildren();
	for (auto& Item : LayoutElementList)
	{
		auto LayoutElement = (UUIFlexibleGridLayoutElement*)Item.LayoutInterface.Get();
		float columnRatio = 0, columnConstant = 0;
		for (int i = LayoutElement->GetColumnIndex(), count = FMath::Min(i + LayoutElement->GetColumnCount(), Columns.Num()); i < count; i++)
		{
			auto& Column = Columns[i];
			if (Column.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				columnRatio += Column.Size;
			}
			else
			{
				columnConstant += Column.Size;
			}
		}
		float rowRatio = 0, rowConstant = 0;
		for (int i = LayoutElement->GetRowIndex(), count = FMath::Min(i + LayoutElement->GetRowCount(), Rows.Num()); i < count; i++)
		{
			auto& Row = Rows[i];
			if (Row.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				rowRatio += Row.Size;
			}
			else
			{
				rowConstant += Row.Size;
			}
		}
		float columnSize = thisFreeWidth * columnRatio * invColumnTotalRatio + columnConstant + (LayoutElement->GetColumnCount() - 1) * Spacing.X;
		float rowSize = thisFreeHeight * rowRatio * invRowTotalRatio + rowConstant + (LayoutElement->GetRowCount() - 1) * Spacing.Y;
		auto uiItem = Item.ChildUIItem;
		float anchorOffsetX, anchorOffsetY;
		auto anchorOffset = GetOffset(FMath::Min(LayoutElement->GetColumnIndex(), Columns.Num() - 1), FMath::Min(LayoutElement->GetRowIndex(), Rows.Num() - 1));
		anchorOffsetX = columnSize * (uiItem->GetPivot().X) + anchorOffset.X;
		anchorOffsetY = -rowSize * (1.0f - uiItem->GetPivot().Y) + anchorOffset.Y;
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
		ApplyWidthWithAnimation(tempAnimationType, columnSize, uiItem.Get());
		ApplyHeightWithAnimation(tempAnimationType, rowSize, uiItem.Get());
	}
}

bool UUIFlexibleGridLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
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

		OutResult.bCanControlVerticalAnchor = this->GetEnable();
		OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta = this->GetEnable();

		OutResult.bCanControlHorizontalAnchor = this->GetEnable();
		OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
		OutResult.bCanControlHorizontalSizeDelta = this->GetEnable();
		return true;
	}
}
