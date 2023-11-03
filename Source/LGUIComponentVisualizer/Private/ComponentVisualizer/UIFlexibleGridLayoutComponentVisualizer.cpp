// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "ComponentVisualizer/UIFlexibleGridLayoutComponentVisualizer.h"
#include "Layout/UIFlexibleGridLayout.h"
#include "LGUIComponentVisualizerModule.h"
#include "LGUI.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIFlexibleGridLayoutComponentVisualizer"


FUIFlexibleGridLayoutComponentVisualizer::FUIFlexibleGridLayoutComponentVisualizer()
	: FComponentVisualizer()
{
	SelectionState = NewObject<UUIFlexibleGridLayoutVisualizerSelectionState>(GetTransientPackage(), TEXT("UIFlexibleGridLayout_Visualizer_SelectionState"), RF_Transactional);
}
void FUIFlexibleGridLayoutComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	auto Layout = Cast<UUIFlexibleGridLayout>(Component);
	if (!Layout)return;
	auto UIItem = Layout->GetRootUIComponent();
	if (!UIItem)return;

	TargetComp = (UUIFlexibleGridLayout*)Layout;
	if (TargetComp->GetWorld() != View->Family->Scene->GetWorld())return;

	auto& Columns = Layout->GetColumns();
	auto& Rows = Layout->GetRows();

	FVector2D rectSize;
	rectSize.X = UIItem->GetWidth() - Layout->GetPadding().Left - Layout->GetPadding().Right;
	rectSize.Y = UIItem->GetHeight() - Layout->GetPadding().Top - Layout->GetPadding().Bottom;

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
	float thisFreeWidth = rectSize.X - columnTotalContstantSize - Layout->GetSpacing().X * (Columns.Num() - 1);//width exclude constant size and all space
	float thisFreeHeight = rectSize.Y - rowTotalConstantSize - Layout->GetSpacing().Y * (Rows.Num() - 1);//height exclude constant size and all space

	//darw line at column's left
	float offsetX = Layout->GetPadding().Left, offsetY = Layout->GetPadding().Bottom;
	float halfSpaceX = Layout->GetSpacing().X * 0.5f;
	float halfSpaceY = Layout->GetSpacing().Y * 0.5f;
	auto lineColor = FLinearColor(0, 1, 1, 1);
	auto handleSize = 10;
	for (int i = 0; i < Columns.Num(); i++)
	{
		auto& item = Columns[i];
		float itemWidth =
			item.SizeType == EUIFlexibleGridLayoutCellSizeType::Constant ?
			item.Size :
			item.Size * invColumnTotalRatio * thisFreeWidth
			;
		if (i != 0)
		{
			auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft() + offsetX - halfSpaceX, UIItem->GetLocalSpaceTop());
			auto LineEnd = FVector(0, LineStart.Y, UIItem->GetLocalSpaceBottom());
			LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
			LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
			//draw split line
			PDI->DrawLine(
				LineStart,
				LineEnd,
				FLinearColor(0, 1, 1, 1),
				SDPG_Foreground
			);

			PDI->SetHitProxy(new HUIFlexibleGridLayoutSpliterVisProxy(Layout, true, true, i));
			//draw click point
			PDI->DrawPoint(
				LineStart, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HUIFlexibleGridLayoutSpliterVisProxy(Layout, true, false, i));
			PDI->DrawPoint(
				LineEnd, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
		}
		offsetX += itemWidth + Layout->GetSpacing().X;
	}
	for (int i = 0; i < Rows.Num(); i++)
	{
		auto item = Rows[i];
		float itemHeight =
			item.SizeType == EUIFlexibleGridLayoutCellSizeType::Constant ?
			item.Size :
			item.Size * invRowTotalRatio * thisFreeHeight
			;
		if (i != 0)
		{
			auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceTop() - offsetY + halfSpaceY);
			auto LineEnd = FVector(0, UIItem->GetLocalSpaceRight(), LineStart.Z);
			LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
			LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
			//draw split line
			PDI->DrawLine(
				LineStart,
				LineEnd,
				FLinearColor(0, 1, 1, 0.1),
				SDPG_Foreground
			);
			//draw click point
			PDI->SetHitProxy(new HUIFlexibleGridLayoutSpliterVisProxy(Layout, false, true, i));
			PDI->DrawPoint(
				LineStart, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HUIFlexibleGridLayoutSpliterVisProxy(Layout, false, false, i));
			PDI->DrawPoint(
				LineEnd, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
		}
		offsetY += itemHeight + Layout->GetSpacing().Y;
	}

	lineColor = FLinearColor(0, 1, 0, 1);
	const float FrameLineThickness = 0.5f;
	auto LeftTop = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceTop());
	auto RightTop = FVector(0, UIItem->GetLocalSpaceRight(), UIItem->GetLocalSpaceTop());
	auto LeftBottom = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceBottom());
	auto RightBottom = FVector(0, UIItem->GetLocalSpaceRight(), UIItem->GetLocalSpaceBottom());
	LeftTop = UIItem->GetComponentTransform().TransformPosition(LeftTop);
	RightTop = UIItem->GetComponentTransform().TransformPosition(RightTop);
	LeftBottom = UIItem->GetComponentTransform().TransformPosition(LeftBottom);
	RightBottom = UIItem->GetComponentTransform().TransformPosition(RightBottom);
	//left frame line
	PDI->SetHitProxy(new HUIFlexibleGridLayoutFrameLineVisProxy(Layout, HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Left));
	PDI->DrawLine(LeftTop, LeftBottom, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//top frame line
	PDI->SetHitProxy(new HUIFlexibleGridLayoutFrameLineVisProxy(Layout, HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Top));
	PDI->DrawLine(LeftTop, RightTop, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//right frame line
	PDI->SetHitProxy(new HUIFlexibleGridLayoutFrameLineVisProxy(Layout, HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Right));
	PDI->DrawLine(RightBottom, RightTop, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//bottom frame line
	PDI->SetHitProxy(new HUIFlexibleGridLayoutFrameLineVisProxy(Layout, HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Bottom));
	PDI->DrawLine(RightBottom, LeftBottom, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
}
bool FUIFlexibleGridLayoutComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (!TargetComp.IsValid())return false;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return false;

	if (VisProxy->IsA(HUIFlexibleGridLayoutSpliterVisProxy::StaticGetType()))
	{
		const HUIFlexibleGridLayoutSpliterVisProxy* Proxy = (HUIFlexibleGridLayoutSpliterVisProxy*)VisProxy;
		SelectionState->ProxyType = UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Spliter;
		SelectionState->SelectedSpliterIndex = Proxy->SpliterIndex;
		SelectionState->bHorizontalOrVertical = Proxy->bHorizontalOrVertical;
		SelectionState->bFirstOrSecond = Proxy->bFirstOrSecond;
		if (SelectionState->bHorizontalOrVertical)
		{
			if (!TargetComp->GetColumns().IsValidIndex(SelectionState->SelectedSpliterIndex))
			{
				SelectionState->SelectedSpliterIndex = -1;
			}
		}
		else
		{
			if (!TargetComp->GetRows().IsValidIndex(SelectionState->SelectedSpliterIndex))
			{
				SelectionState->SelectedSpliterIndex = -1;
			}
		}
		return true;
	}
	else if (VisProxy->IsA(HUIFlexibleGridLayoutFrameLineVisProxy::StaticGetType()))
	{
		const HUIFlexibleGridLayoutFrameLineVisProxy* Proxy = (HUIFlexibleGridLayoutFrameLineVisProxy*)VisProxy;
		SelectionState->ProxyType = UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Frame;
		SelectionState->FrameType = Proxy->FrameType;
		auto Plane = FPlane(UIItem->GetComponentLocation(), UIItem->GetForwardVector());
		SelectionState->CurrentClickPoint = FMath::RayPlaneIntersection(Click.GetOrigin(), Click.GetDirection(), Plane);
		return true;
	}
	return false;
}
bool FUIFlexibleGridLayoutComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltalRotate, FVector& DeltaScale)
{
	if (!DeltaTranslate.IsZero())
	{
		if (!TargetComp.IsValid())return false;

		auto UIItem = TargetComp->GetRootUIComponent();
		if (!UIItem)return true;
		if (SelectionState->SelectedSpliterIndex == -1)return true;

		TargetComp->Modify();

		auto LocalSpaceTranslate = UIItem->GetComponentTransform().InverseTransformVector(DeltaTranslate);
		if (SelectionState->bHorizontalOrVertical)
		{
			if (LocalSpaceTranslate.Y != 0)
			{
				LayoutChangeSizeToPixelRelevant(true);
				auto Columns = TargetComp->GetColumns();
				auto& ColumnItem = Columns[SelectionState->SelectedSpliterIndex];
				auto& PrevColumnItem = Columns[SelectionState->SelectedSpliterIndex - 1];
				ColumnItem.Size -= LocalSpaceTranslate.Y;
				PrevColumnItem.Size += LocalSpaceTranslate.Y;
				if (ColumnItem.Size > 0 && PrevColumnItem.Size > 0)
				{
					TargetComp->SetColumns(Columns);
					return true;
				}
			}
		}
		else
		{
			if (LocalSpaceTranslate.Z != 0)
			{
				LayoutChangeSizeToPixelRelevant(false);
				auto Rows = TargetComp->GetRows();
				auto& RowItem = Rows[SelectionState->SelectedSpliterIndex];
				auto& PrevRowItem = Rows[SelectionState->SelectedSpliterIndex - 1];
				RowItem.Size += LocalSpaceTranslate.Z;
				PrevRowItem.Size -= LocalSpaceTranslate.Z;
				if (RowItem.Size > 0 && PrevRowItem.Size > 0)
				{
					TargetComp->SetRows(Rows);
					return true;
				}
			}
		}
	}
	return true;
}
bool FUIFlexibleGridLayoutComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!TargetComp.IsValid())return false;

	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return false;

	if (SelectionState->ProxyType == UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Frame)return false;
	if (SelectionState->SelectedSpliterIndex == -1)return false;

	auto& Columns = TargetComp->GetColumns();
	auto& Rows = TargetComp->GetRows();

	FVector2D rectSize;
	rectSize.X = UIItem->GetWidth() - TargetComp->GetPadding().Left - TargetComp->GetPadding().Right;
	rectSize.Y = UIItem->GetHeight() - TargetComp->GetPadding().Top - TargetComp->GetPadding().Bottom;

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
	float thisFreeWidth = rectSize.X - columnTotalContstantSize - TargetComp->GetSpacing().X * (Columns.Num() - 1);//width exclude constant size and all space
	float thisFreeHeight = rectSize.Y - rowTotalConstantSize - TargetComp->GetSpacing().Y * (Rows.Num() - 1);//height exclude constant size and all space

	float offsetX = 0, offsetY = 0;
	float halfSpaceX = TargetComp->GetSpacing().X * 0.5f;
	float halfSpaceY = TargetComp->GetSpacing().Y * 0.5f;
	if (SelectionState->bHorizontalOrVertical)
	{
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto& item = Columns[i];
			float itemWidth =
				item.SizeType == EUIFlexibleGridLayoutCellSizeType::Constant ?
				item.Size :
				item.Size * invColumnTotalRatio * thisFreeWidth
				;
			if (i == SelectionState->SelectedSpliterIndex)
			{
				auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft() + offsetX - halfSpaceX, UIItem->GetLocalSpaceTop());
				auto LineEnd = FVector(0, LineStart.Y, UIItem->GetLocalSpaceBottom());
				LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
				LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
				OutLocation = SelectionState->bFirstOrSecond ? LineStart : LineEnd;
				return true;
			}
			offsetX += itemWidth + TargetComp->GetSpacing().X;
		}
	}
	else
	{
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto item = Rows[i];
			float itemHeight =
				item.SizeType == EUIFlexibleGridLayoutCellSizeType::Constant ?
				item.Size :
				item.Size * invRowTotalRatio * thisFreeHeight
				;
			if (i == SelectionState->SelectedSpliterIndex)
			{
				auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceTop() - offsetY + halfSpaceY);
				auto LineEnd = FVector(0, UIItem->GetLocalSpaceRight(), LineStart.Z);
				LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
				LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
				OutLocation = SelectionState->bFirstOrSecond ? LineStart : LineEnd;
				return true;
			}
			offsetY += itemHeight + TargetComp->GetSpacing().Y;
		}
	}
	return false;
}
TSharedPtr<SWidget> FUIFlexibleGridLayoutComponentVisualizer::GenerateContextMenu() const
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("EditUIFlexibleGridLayout", LOCTEXT("EditUIFlexibleGridLayout", "UI FlexibleGridLayout"));
	{
		if (SelectionState->ProxyType == UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Spliter)
		{
			//@todo
			//MenuBuilder.AddMenuEntry(
			//	LOCTEXT("ChangeSpliterType", "Change Spliter Type"),
			//	LOCTEXT("ChangeSpliterType_Tooltip", "Change size type of this spliter"),
			//	FSlateIcon(),
			//	FUIAction(FExecuteAction::CreateSP((FUIFlexibleGridLayoutComponentVisualizer*)this, &FUIFlexibleGridLayoutComponentVisualizer::ChangeSpliterType))
			//);
			if (SelectionState->SelectedSpliterIndex != -1)
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("DeleteSpliter", "Delete Spliter"),
					LOCTEXT("DeleteSpliter_Tooltip", "Delete this spliter"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP((FUIFlexibleGridLayoutComponentVisualizer*)this, &FUIFlexibleGridLayoutComponentVisualizer::RemoveSpliter))
				);
			}
		}
		else if (SelectionState->ProxyType == UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Frame)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("AddSpliterHere", "Add Spliter Here"),
				LOCTEXT("AddSpliter_Tooltip", "Add a new spliter at click point"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP((FUIFlexibleGridLayoutComponentVisualizer*)this, &FUIFlexibleGridLayoutComponentVisualizer::AddSpliter))
			);
		}
	}
	MenuBuilder.EndSection();

	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	return MenuWidget;
}
void FUIFlexibleGridLayoutComponentVisualizer::AddSpliter()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;

	GEditor->BeginTransaction(LOCTEXT("AddSpliter_Transaction", "UIFlexibleGridLayout AddSpliter"));
	TargetComp->Modify();
	auto LocalClickPoint = UIItem->GetComponentTransform().InverseTransformPosition(SelectionState->CurrentClickPoint);
	switch (SelectionState->FrameType)
	{
	case HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Left:
	case HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Right:
	{
		LayoutChangeSizeToPixelRelevant(false);
		auto SplitPoint = LocalClickPoint.Z - UIItem->GetLocalSpaceTop();
		SplitPoint = -SplitPoint;
		auto Rows = TargetComp->GetRows();
		auto Space = TargetComp->GetSpacing().Y;
		float CumulativeSplitSize = 0;
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto PrevCumulativeSplitSize = CumulativeSplitSize;
			CumulativeSplitSize += Rows[i].Size;
			if (SplitPoint < CumulativeSplitSize)
			{
				Rows[i].Size = CumulativeSplitSize - SplitPoint - Space * 0.5f;
				Rows.Insert(FUIFlexibleGridLayoutCellData(SplitPoint - PrevCumulativeSplitSize - Space * 0.5f), i);
				break;
			}
			SplitPoint -= Space;
		}
		TargetComp->SetRows(Rows);
	}
	break;
	case HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Top:
	case HUIFlexibleGridLayoutFrameLineVisProxy::EFrameType::Bottom:
	{
		LayoutChangeSizeToPixelRelevant(true);
		auto SplitPoint = LocalClickPoint.Y - UIItem->GetLocalSpaceLeft();
		auto Columns = TargetComp->GetColumns();
		auto Space = TargetComp->GetSpacing().X;
		float CumulativeSplitSize = 0;
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto PrevCumulativeSplitSize = CumulativeSplitSize;
			CumulativeSplitSize += Columns[i].Size;
			if (SplitPoint < CumulativeSplitSize)
			{
				Columns[i].Size = CumulativeSplitSize - SplitPoint - Space * 0.5f;
				Columns.Insert(FUIFlexibleGridLayoutCellData(SplitPoint - PrevCumulativeSplitSize - Space * 0.5f), i);
				break;
			}
			SplitPoint -= Space;
		}
		TargetComp->SetColumns(Columns);
	}
	break;
	}
	GEditor->EndTransaction();
}
void FUIFlexibleGridLayoutComponentVisualizer::RemoveSpliter()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;
	if (SelectionState->ProxyType == UUIFlexibleGridLayoutVisualizerSelectionState::EHitProxyType::Frame)return;
	if (SelectionState->SelectedSpliterIndex == -1)return;

	GEditor->BeginTransaction(LOCTEXT("RemoveSpliter_Transaction", "UIFlexibleGridLayout RemoveSpliter"));
	TargetComp->Modify();

	if (SelectionState->bHorizontalOrVertical)
	{
		LayoutChangeSizeToPixelRelevant(true);
		auto Columns = TargetComp->GetColumns();
		Columns[SelectionState->SelectedSpliterIndex - 1].Size += Columns[SelectionState->SelectedSpliterIndex].Size + TargetComp->GetSpacing().X;
		Columns.RemoveAt(SelectionState->SelectedSpliterIndex);
		TargetComp->SetColumns(Columns);
	}
	else
	{
		LayoutChangeSizeToPixelRelevant(false);
		auto Rows = TargetComp->GetRows();
		Rows[SelectionState->SelectedSpliterIndex - 1].Size += Rows[SelectionState->SelectedSpliterIndex].Size + TargetComp->GetSpacing().Y;
		Rows.RemoveAt(SelectionState->SelectedSpliterIndex);
		TargetComp->SetRows(Rows);
	}
	SelectionState->SelectedSpliterIndex = -1;
	GEditor->EndTransaction();
}
void FUIFlexibleGridLayoutComponentVisualizer::ChangeSpliterType()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;


}
void FUIFlexibleGridLayoutComponentVisualizer::LayoutChangeSizeToPixelRelevant(bool InHorizontalOrVertical)
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;

	if (InHorizontalOrVertical)
	{
		auto Columns = TargetComp->GetColumns();
		float columnTotalRatio = 0, columnTotalContstantSize = 0;
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
		float invColumnTotalRatio = 1.0f / columnTotalRatio;
		float thisWidth = UIItem->GetWidth() - TargetComp->GetPadding().Left - TargetComp->GetPadding().Right;
		float thisFreeWidth = thisWidth - columnTotalContstantSize - TargetComp->GetSpacing().X * (Columns.Num() - 1);//width exclude constant size and all space
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto& Column = Columns[i];
			if (Column.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				Column.Size = thisFreeWidth * Column.Size * invColumnTotalRatio;
			}
		}
		TargetComp->SetColumns(Columns);
	}
	else
	{
		auto Rows = TargetComp->GetRows();
		float rowTotalRatio = 0, rowTotalConstantSize = 0;
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
		float invRowTotalRatio = 1.0f / rowTotalRatio;
		float thisHeight = UIItem->GetHeight() - TargetComp->GetPadding().Top - TargetComp->GetPadding().Bottom;
		float thisFreeHeight = thisHeight - rowTotalConstantSize - TargetComp->GetSpacing().Y * (Rows.Num() - 1);//height exclude constant size and all space
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto& Row = Rows[i];
			if (Row.SizeType == EUIFlexibleGridLayoutCellSizeType::Ratio)
			{
				Row.Size = thisFreeHeight * Row.Size * invRowTotalRatio;
			}
		}
		TargetComp->SetRows(Rows);
	}
}


IMPLEMENT_HIT_PROXY(HUIFlexibleGridLayoutSpliterVisProxy, HComponentVisProxy);
HUIFlexibleGridLayoutSpliterVisProxy::HUIFlexibleGridLayoutSpliterVisProxy(const UUIFlexibleGridLayout* InComponent, bool InHorizontalOrVertical, bool InFirstOrSecond, int32 InSelectingSpliterIndex
)
	: HComponentVisProxy(InComponent, HPP_Foreground)
{
	bHorizontalOrVertical = InHorizontalOrVertical;
	bFirstOrSecond = InFirstOrSecond;
	SpliterIndex = InSelectingSpliterIndex;
}

IMPLEMENT_HIT_PROXY(HUIFlexibleGridLayoutFrameLineVisProxy, HComponentVisProxy);
HUIFlexibleGridLayoutFrameLineVisProxy::HUIFlexibleGridLayoutFrameLineVisProxy(const UUIFlexibleGridLayout* InComponent, EFrameType InFrameType
)
	: HComponentVisProxy(InComponent, HPP_Foreground)
{
	FrameType = InFrameType;
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
