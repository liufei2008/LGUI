// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexLayoutContainerGrid.h"
#include "Core/Components/LexLayoutSelfGrid.h"
#include "LGUI.h"

DECLARE_CYCLE_STAT(TEXT("LexLayoutContainer Grid"), STAT_LexLayoutContainerGrid, STATGROUP_LGUI);

ULexLayoutContainerGrid::ULexLayoutContainerGrid()
{
	Rows = Columns =
	{
		FLexLayoutGridSize(ELexLayoutGridSizeType::Ratio, 1.0f),
		FLexLayoutGridSize(ELexLayoutGridSizeType::Ratio, 1.0f),
	};
}

#if WITH_EDITOR
void ULexLayoutContainerGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ULexLayoutContainerGrid::SetPadding(FMargin Value)
{
	if (Padding != Value)
	{
		Padding = Value;
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}

void ULexLayoutContainerGrid::SetRows(const TArray<FLexLayoutGridSize>& Value)
{
	if (Rows != Value)
	{
		Rows = Value;
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}
void ULexLayoutContainerGrid::SetColumns(const TArray<FLexLayoutGridSize>& Value)
{
	if (Columns != Value)
	{
		Columns = Value;
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}
void ULexLayoutContainerGrid::SetSpacing(const FVector2D& Value)
{
	if (Spacing != Value)
	{
		Spacing = Value;
		ULexWidget::MarkLayoutForRebuild(GetWidget());
	}
}

void ULexLayoutContainerGrid::CalculateLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_LexLayoutContainerGrid);
	auto Widget = GetWidget();
	if (!Widget)return;
	FVector2D StartPosition;
	StartPosition.X = Padding.Left;
	StartPosition.Y = -Padding.Top;//left top as start point
	FVector2D RectSize;
	RectSize.X = Widget->GetWidth() - Padding.Left - Padding.Right;
	RectSize.Y = Widget->GetHeight() - Padding.Top - Padding.Bottom;

	float ColumnTotalRatio = 0, RowTotalRatio = 0;
	float ColumnTotalConstantSize = 0, RowTotalConstantSize = 0;
	for (auto& Item : Columns)
	{
		if (Item.Type == ELexLayoutGridSizeType::Ratio)
		{
			ColumnTotalRatio += Item.RatioValue;
		}
		else
		{
			ColumnTotalConstantSize += Item.FixedValue;
		}
	}
	for (auto& Item : Rows)
	{
		if (Item.Type == ELexLayoutGridSizeType::Ratio)
		{
			RowTotalRatio += Item.RatioValue;
		}
		else
		{
			RowTotalConstantSize += Item.FixedValue;
		}
	}
	float InvColumnTotalRatio = 1.0f / ColumnTotalRatio;
	float InvRowTotalRatio = 1.0f / RowTotalRatio;
	float ThisFreeWidth = RectSize.X - ColumnTotalConstantSize - Spacing.X * (Columns.Num() - 1);//width exclude constant size and all space
	float ThisFreeHeight = RectSize.Y - RowTotalConstantSize - Spacing.Y * (Rows.Num() - 1);//height exclude constant size and all space
	auto GetOffset = [&](int ColumnIndex, int RowIndex)
	{
		float OffsetXRatio = 0, OffsetYRatio = 0;
		float OffsetXConstant = 0, OffsetYConstant = 0;
		for (int i = 0; i < ColumnIndex; i++)
		{
			auto& Column = Columns[i];
			if (Column.Type == ELexLayoutGridSizeType::Ratio)
			{
				OffsetXRatio += Column.RatioValue;
			}
			else
			{
				OffsetXConstant += Column.FixedValue;
			}
		}
		for (int i = 0; i < RowIndex; i++)
		{
			auto& Row = Rows[i];
			if (Row.Type == ELexLayoutGridSizeType::Ratio)
			{
				OffsetYRatio += Row.RatioValue;
			}
			else
			{
				OffsetYConstant += Row.FixedValue;
			}
		}
		return StartPosition + FVector2D(
			(ColumnTotalRatio == 0 ? ThisFreeWidth : OffsetXRatio * InvColumnTotalRatio * ThisFreeWidth) + OffsetXConstant + ColumnIndex * Spacing.X
			, -(RowTotalRatio == 0 ? ThisFreeHeight : OffsetYRatio * InvRowTotalRatio * ThisFreeHeight) - OffsetYConstant - RowIndex * Spacing.Y
		);
	};
	TDoubleLinkedList<ULexWidget*> NotLocatedWidgetList;
	TArray<int> AlreadyFilledRows;
	TArray<int> AlreadyFilledColumns;
	//firstly locate widgets with layout self data, then locate widgets without layout self data in left grid cell
	for (auto& ChildWidget : Widget->GetChildren())
	{
		if (!ChildWidget->GetWidgetActiveInHierarchy())continue;
		if (ChildWidget->GetIgnoreLayout())continue;

		auto AnchorMin = ChildWidget->GetAnchorMin();
		auto AnchorMax = ChildWidget->GetAnchorMax();
		if (AnchorMin.X != AnchorMax.X)//custom anchor not support
		{
			ChildWidget->SetHorizontalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}
		if (AnchorMin.Y != AnchorMax.Y)
		{
			ChildWidget->SetVerticalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
		}

		auto ChildLayoutSelf = Cast<ULexLayoutSelfGrid>(ChildWidget->GetLayoutSelf());
		if (!ChildLayoutSelf)
		{
			NotLocatedWidgetList.AddTail(ChildWidget);
			continue;
		}
		
		int ColumnIndex = ChildLayoutSelf->GetColumnIndex();
		int ColumnCount = ChildLayoutSelf->GetColumnCount();
		int RowIndex = ChildLayoutSelf->GetRowIndex();
		int RowCount = ChildLayoutSelf->GetRowCount();
		float ColumnRatio = 0, ColumnConstant = 0;
		for (int i = ColumnIndex, Count = FMath::Min(i + ColumnCount, Columns.Num()); i < Count; i++)
		{
			AlreadyFilledColumns.AddUnique(i);
			auto& Column = Columns[i];
			if (Column.Type == ELexLayoutGridSizeType::Ratio)
			{
				ColumnRatio += Column.RatioValue;
			}
			else
			{
				ColumnConstant += Column.FixedValue;
			}
		}
		float RowRatio = 0, RowConstant = 0;
		for (int i = RowIndex, Count = FMath::Min(i + RowCount, Rows.Num()); i < Count; i++)
		{
			AlreadyFilledRows.AddUnique(i);
			auto& Row = Rows[i];
			if (Row.Type == ELexLayoutGridSizeType::Ratio)
			{
				RowRatio += Row.RatioValue;
			}
			else
			{
				RowConstant += Row.FixedValue;
			}
		}
		float ColumnSize = (ColumnTotalRatio == 0 ? ThisFreeWidth : ThisFreeWidth * ColumnRatio * InvColumnTotalRatio) + ColumnConstant + (ColumnCount - 1) * Spacing.X;
		float RowSize = (RowTotalRatio == 0 ? ThisFreeHeight : ThisFreeHeight * RowRatio * InvRowTotalRatio) + RowConstant + (RowCount - 1) * Spacing.Y;
		auto AnchorOffset = GetOffset(FMath::Min(ColumnIndex, Columns.Num() - 1), FMath::Min(RowIndex, Rows.Num() - 1));
		float AnchorOffsetX = ColumnSize * (ChildWidget->GetPivot().X) + AnchorOffset.X;
		float AnchorOffsetY = -RowSize * (1.0f - ChildWidget->GetPivot().Y) + AnchorOffset.Y;
		AnchorMin = ChildWidget->GetAnchorMin();
		AnchorOffsetX -= AnchorMin.X * Widget->GetWidth();
		AnchorOffsetY += (1 - AnchorMin.Y) * Widget->GetHeight();
		ChildWidget->SetAnchoredPosition(FVector2D(AnchorOffsetX, AnchorOffsetY));

		if (ChildLayoutSelf)
		{
			ChildLayoutSelf->SetSizeByLayoutContainer(FVector2f(ColumnSize, RowSize));
		}
	}
	//locate widgets without layout self data in left grid cell
	if (NotLocatedWidgetList.Num() > 0)
	{
		for (int ColumnIndex = 0; ColumnIndex < Columns.Num(); ColumnIndex++)
		{
			for (int RowIndex = 0; RowIndex < Rows.Num(); RowIndex++)
			{
				if (!AlreadyFilledColumns.Contains(ColumnIndex) && !AlreadyFilledRows.Contains(RowIndex))
				{
					auto ChildWidget = NotLocatedWidgetList.GetHead()->GetValue();
					NotLocatedWidgetList.RemoveNode(NotLocatedWidgetList.GetHead());

					float ColumnRatio = 0, ColumnConstant = 0;
					auto& Column = Columns[ColumnIndex];
					if (Column.Type == ELexLayoutGridSizeType::Ratio)
					{
						ColumnRatio = Column.RatioValue;
					}
					else
					{
						ColumnConstant = Column.FixedValue;
					}
					float RowRatio = 0, RowConstant = 0;
					auto& Row = Rows[RowIndex];
					if (Row.Type == ELexLayoutGridSizeType::Ratio)
					{
						RowRatio = Row.RatioValue;
					}
					else
					{
						RowConstant = Row.FixedValue;
					}
					float ColumnSize = (ColumnTotalRatio == 0 ? ThisFreeWidth : ThisFreeWidth * ColumnRatio * InvColumnTotalRatio) + ColumnConstant;
					float RowSize = (RowTotalRatio == 0 ? ThisFreeHeight : ThisFreeHeight * RowRatio * InvRowTotalRatio) + RowConstant;
					auto AnchorOffset = GetOffset(FMath::Min(ColumnIndex, Columns.Num() - 1), FMath::Min(RowIndex, Rows.Num() - 1));
					float AnchorOffsetX = ColumnSize * (ChildWidget->GetPivot().X) + AnchorOffset.X;
					float AnchorOffsetY = -RowSize * (1.0f - ChildWidget->GetPivot().Y) + AnchorOffset.Y;
					auto AnchorMin = ChildWidget->GetAnchorMin();
					AnchorOffsetX -= AnchorMin.X * Widget->GetWidth();
					AnchorOffsetY += (1 - AnchorMin.Y) * Widget->GetHeight();
					ChildWidget->SetAnchoredPosition(FVector2D(AnchorOffsetX, AnchorOffsetY));
					
					if (NotLocatedWidgetList.Num() <= 0)//break the loop if all widgets are located
					{
						ColumnIndex = Columns.Num();
						RowIndex = Rows.Num();
					}
				}
			}
		}
	}
}

FLexLayoutControlAnchorData ULexLayoutContainerGrid::GetLayoutControlAnchor(const ULexWidget* TargetWidget)const
{
	FLexLayoutControlAnchorData Result;
	auto ThisWidget = GetWidget();
	if (ThisWidget == TargetWidget)//self
	{
        
	}
	else if (ThisWidget->GetChildren().Contains(TargetWidget))//child
	{
		auto bIgnoreLayout = TargetWidget->GetIgnoreLayout();
		if (!bIgnoreLayout)
		{
			Result.bCanControlHorizontalPosition = true;
			Result.bCanControlVerticalPosition = true;
		}
	}
	return Result;
}
