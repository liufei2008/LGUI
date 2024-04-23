// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "UIPanelLayoutFlexibleGridComponentVisualizer.h"
#include "Layout/UIPanelLayout_FlexibleGrid.h"
#include "LGUIComponentVisualizerModule.h"
#include "LGUI.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIPanelLayoutFlexibleGridComponentVisualizer"


FUIPanelLayoutFlexibleGridComponentVisualizer::FUIPanelLayoutFlexibleGridComponentVisualizer()
	: FComponentVisualizer()
{
	SelectionState = NewObject<UUIPanelLayoutFlexibleGridVisualizerSelectionState>(GetTransientPackage(), TEXT("UIPanelLayoutFlexibleGrid_Visualizer_SelectionState"), RF_Transactional);
}
void FUIPanelLayoutFlexibleGridComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	auto Layout = Cast<UUIPanelLayout_FlexibleGrid>(Component);
	if (!Layout)return;
	auto UIItem = Layout->GetRootUIComponent();
	if (!UIItem)return;

	TargetComp = (UUIPanelLayout_FlexibleGrid*)Layout;
	if (TargetComp->GetWorld() != View->Family->Scene->GetWorld())return;

	auto& Columns = Layout->GetColumns();
	auto& Rows = Layout->GetRows();

	FVector2D rectSize;
	rectSize.X = UIItem->GetWidth();
	rectSize.Y = UIItem->GetHeight();

	float columnTotalRatio = 0, rowTotalRatio = 0;
	float columnTotalContstantSize = 0, rowTotalConstantSize = 0;
	for (auto& Item : Columns)
	{
		if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
		{
			columnTotalRatio += Item.Value;
		}
		else
		{
			columnTotalContstantSize += Item.Value;
		}
	}
	for (auto& Item : Rows)
	{
		if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
		{
			rowTotalRatio += Item.Value;
		}
		else
		{
			rowTotalConstantSize += Item.Value;
		}
	}
	float invColumnTotalRatio = 1.0f / columnTotalRatio;
	float invRowTotalRatio = 1.0f / rowTotalRatio;
	float thisFreeWidth = rectSize.X - columnTotalContstantSize;
	float thisFreeHeight = rectSize.Y - rowTotalConstantSize;

	//darw line at column's left
	float offsetX = 0, offsetY = 0;
	auto lineColor = FLinearColor(0, 1, 1, 1);
	auto handleSize = 10;
	for (int i = 0; i < Columns.Num(); i++)
	{
		auto& item = Columns[i];
		float itemWidth =
			item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Constant ?
			item.Value :
			item.Value * invColumnTotalRatio * thisFreeWidth
			;
		if (i != 0)
		{
			auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft() + offsetX, UIItem->GetLocalSpaceTop());
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

			PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridSpliterVisProxy(Layout, true, true, i));
			//draw click point
			PDI->DrawPoint(
				LineStart, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridSpliterVisProxy(Layout, true, false, i));
			PDI->DrawPoint(
				LineEnd, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
		}
		offsetX += itemWidth;
	}
	for (int i = 0; i < Rows.Num(); i++)
	{
		auto item = Rows[i];
		float itemHeight =
			item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Constant ?
			item.Value :
			item.Value * invRowTotalRatio * thisFreeHeight
			;
		if (i != 0)
		{
			auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceTop() - offsetY);
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
			PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridSpliterVisProxy(Layout, false, true, i));
			PDI->DrawPoint(
				LineStart, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
			PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridSpliterVisProxy(Layout, false, false, i));
			PDI->DrawPoint(
				LineEnd, lineColor, handleSize, SDPG_Foreground
			);
			PDI->SetHitProxy(NULL);
		}
		offsetY += itemHeight;
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
	PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridFrameLineVisProxy(Layout, HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Left));
	PDI->DrawLine(LeftTop, LeftBottom, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//top frame line
	PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridFrameLineVisProxy(Layout, HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Top));
	PDI->DrawLine(LeftTop, RightTop, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//right frame line
	PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridFrameLineVisProxy(Layout, HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Right));
	PDI->DrawLine(RightBottom, RightTop, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
	//bottom frame line
	PDI->SetHitProxy(new HUIPanelLayoutFlexibleGridFrameLineVisProxy(Layout, HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Bottom));
	PDI->DrawLine(RightBottom, LeftBottom, lineColor, SDPG_Foreground, FrameLineThickness);
	PDI->SetHitProxy(NULL);
}
bool FUIPanelLayoutFlexibleGridComponentVisualizer::VisProxyHandleClick(FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy, const FViewportClick& Click)
{
	if (!TargetComp.IsValid())return false;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return false;

	if (VisProxy->IsA(HUIPanelLayoutFlexibleGridSpliterVisProxy::StaticGetType()))
	{
		const HUIPanelLayoutFlexibleGridSpliterVisProxy* Proxy = (HUIPanelLayoutFlexibleGridSpliterVisProxy*)VisProxy;
		SelectionState->ProxyType = UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Spliter;
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
	else if (VisProxy->IsA(HUIPanelLayoutFlexibleGridFrameLineVisProxy::StaticGetType()))
	{
		const HUIPanelLayoutFlexibleGridFrameLineVisProxy* Proxy = (HUIPanelLayoutFlexibleGridFrameLineVisProxy*)VisProxy;
		SelectionState->ProxyType = UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Frame;
		SelectionState->FrameType = Proxy->FrameType;
		auto Plane = FPlane(UIItem->GetComponentLocation(), UIItem->GetForwardVector());
		SelectionState->CurrentClickPoint = FMath::RayPlaneIntersection(Click.GetOrigin(), Click.GetDirection(), Plane);
		return true;
	}
	return false;
}
bool FUIPanelLayoutFlexibleGridComponentVisualizer::HandleInputDelta(FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate, FRotator& DeltalRotate, FVector& DeltaScale)
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
				ColumnItem.Value -= LocalSpaceTranslate.Y;
				PrevColumnItem.Value += LocalSpaceTranslate.Y;
				if (ColumnItem.Value > 0 && PrevColumnItem.Value > 0)
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
				RowItem.Value += LocalSpaceTranslate.Z;
				PrevRowItem.Value -= LocalSpaceTranslate.Z;
				if (RowItem.Value > 0 && PrevRowItem.Value > 0)
				{
					TargetComp->SetRows(Rows);
					return true;
				}
			}
		}
	}
	return true;
}
bool FUIPanelLayoutFlexibleGridComponentVisualizer::GetWidgetLocation(const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!TargetComp.IsValid())return false;

	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return false;

	if (SelectionState->ProxyType == UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Frame)return false;
	if (SelectionState->SelectedSpliterIndex == -1)return false;

	auto& Columns = TargetComp->GetColumns();
	auto& Rows = TargetComp->GetRows();

	FVector2D rectSize;
	rectSize.X = UIItem->GetWidth();
	rectSize.Y = UIItem->GetHeight();

	float columnTotalRatio = 0, rowTotalRatio = 0;
	float columnTotalContstantSize = 0, rowTotalConstantSize = 0;
	for (auto& Item : Columns)
	{
		if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
		{
			columnTotalRatio += Item.Value;
		}
		else
		{
			columnTotalContstantSize += Item.Value;
		}
	}
	for (auto& Item : Rows)
	{
		if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
		{
			rowTotalRatio += Item.Value;
		}
		else
		{
			rowTotalConstantSize += Item.Value;
		}
	}
	float invColumnTotalRatio = 1.0f / columnTotalRatio;
	float invRowTotalRatio = 1.0f / rowTotalRatio;
	float thisFreeWidth = rectSize.X - columnTotalContstantSize;
	float thisFreeHeight = rectSize.Y - rowTotalConstantSize;

	float offsetX = 0, offsetY = 0;
	if (SelectionState->bHorizontalOrVertical)
	{
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto& item = Columns[i];
			float itemWidth =
				item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Constant ?
				item.Value :
				item.Value * invColumnTotalRatio * thisFreeWidth
				;
			if (i == SelectionState->SelectedSpliterIndex)
			{
				auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft() + offsetX, UIItem->GetLocalSpaceTop());
				auto LineEnd = FVector(0, LineStart.Y, UIItem->GetLocalSpaceBottom());
				LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
				LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
				OutLocation = SelectionState->bFirstOrSecond ? LineStart : LineEnd;
				return true;
			}
			offsetX += itemWidth;
		}
	}
	else
	{
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto item = Rows[i];
			float itemHeight =
				item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Constant ?
				item.Value :
				item.Value * invRowTotalRatio * thisFreeHeight
				;
			if (i == SelectionState->SelectedSpliterIndex)
			{
				auto LineStart = FVector(0, UIItem->GetLocalSpaceLeft(), UIItem->GetLocalSpaceTop() - offsetY);
				auto LineEnd = FVector(0, UIItem->GetLocalSpaceRight(), LineStart.Z);
				LineStart = UIItem->GetComponentTransform().TransformPosition(LineStart);
				LineEnd = UIItem->GetComponentTransform().TransformPosition(LineEnd);
				OutLocation = SelectionState->bFirstOrSecond ? LineStart : LineEnd;
				return true;
			}
			offsetY += itemHeight;
		}
	}
	return false;
}
TSharedPtr<SWidget> FUIPanelLayoutFlexibleGridComponentVisualizer::GenerateContextMenu() const
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.BeginSection("EditUIPanelLayoutFlexibleGrid", LOCTEXT("EditUIPanelLayoutFlexibleGrid", "UI FlexibleGridLayout"));
	{
		if (SelectionState->ProxyType == UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Spliter)
		{
			//@todo
			//MenuBuilder.AddMenuEntry(
			//	LOCTEXT("ChangeSpliterType", "Change Spliter Type"),
			//	LOCTEXT("ChangeSpliterType_Tooltip", "Change size type of this spliter"),
			//	FSlateIcon(),
			//	FUIAction(FExecuteAction::CreateSP((FUIPanelLayoutFlexibleGridComponentVisualizer*)this, &FUIPanelLayoutFlexibleGridComponentVisualizer::ChangeSpliterType))
			//);
			if (SelectionState->SelectedSpliterIndex != -1)
			{
				MenuBuilder.AddMenuEntry(
					LOCTEXT("DeleteSpliter", "Delete Spliter"),
					LOCTEXT("DeleteSpliter_Tooltip", "Delete this spliter"),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateSP((FUIPanelLayoutFlexibleGridComponentVisualizer*)this, &FUIPanelLayoutFlexibleGridComponentVisualizer::RemoveSpliter))
				);
			}
		}
		else if (SelectionState->ProxyType == UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Frame)
		{
			MenuBuilder.AddMenuEntry(
				LOCTEXT("AddSpliterHere", "Add Spliter Here"),
				LOCTEXT("AddSpliter_Tooltip", "Add a new spliter at click point"),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP((FUIPanelLayoutFlexibleGridComponentVisualizer*)this, &FUIPanelLayoutFlexibleGridComponentVisualizer::AddSpliter))
			);
		}
	}
	MenuBuilder.EndSection();

	TSharedPtr<SWidget> MenuWidget = MenuBuilder.MakeWidget();
	return MenuWidget;
}
void FUIPanelLayoutFlexibleGridComponentVisualizer::AddSpliter()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;

	GEditor->BeginTransaction(LOCTEXT("AddSpliter_Transaction", "UIPanelLayoutFlexibleGrid AddSpliter"));
	TargetComp->Modify();
	auto LocalClickPoint = UIItem->GetComponentTransform().InverseTransformPosition(SelectionState->CurrentClickPoint);
	switch (SelectionState->FrameType)
	{
	case HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Left:
	case HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Right:
	{
		LayoutChangeSizeToPixelRelevant(false);
		auto SplitPoint = LocalClickPoint.Z - UIItem->GetLocalSpaceTop();
		SplitPoint = -SplitPoint;
		auto Rows = TargetComp->GetRows();
		float CumulativeSplitSize = 0;
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto PrevCumulativeSplitSize = CumulativeSplitSize;
			CumulativeSplitSize += Rows[i].Value;
			if (SplitPoint < CumulativeSplitSize)
			{
				Rows[i].Value = CumulativeSplitSize - SplitPoint;
				Rows.Insert(FUIPanelLayout_FlexibleGridSize(SplitPoint - PrevCumulativeSplitSize), i);
				break;
			}
		}
		TargetComp->SetRows(Rows);
	}
	break;
	case HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Top:
	case HUIPanelLayoutFlexibleGridFrameLineVisProxy::EFrameType::Bottom:
	{
		LayoutChangeSizeToPixelRelevant(true);
		auto SplitPoint = LocalClickPoint.Y - UIItem->GetLocalSpaceLeft();
		auto Columns = TargetComp->GetColumns();
		float CumulativeSplitSize = 0;
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto PrevCumulativeSplitSize = CumulativeSplitSize;
			CumulativeSplitSize += Columns[i].Value;
			if (SplitPoint < CumulativeSplitSize)
			{
				Columns[i].Value = CumulativeSplitSize - SplitPoint;
				Columns.Insert(FUIPanelLayout_FlexibleGridSize(SplitPoint - PrevCumulativeSplitSize), i);
				break;
			}
		}
		TargetComp->SetColumns(Columns);
	}
	break;
	}
	GEditor->EndTransaction();
}
void FUIPanelLayoutFlexibleGridComponentVisualizer::RemoveSpliter()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;
	if (SelectionState->ProxyType == UUIPanelLayoutFlexibleGridVisualizerSelectionState::EHitProxyType::Frame)return;
	if (SelectionState->SelectedSpliterIndex == -1)return;

	GEditor->BeginTransaction(LOCTEXT("RemoveSpliter_Transaction", "UIPanelLayoutFlexibleGrid RemoveSpliter"));
	TargetComp->Modify();

	if (SelectionState->bHorizontalOrVertical)
	{
		LayoutChangeSizeToPixelRelevant(true);
		auto Columns = TargetComp->GetColumns();
		Columns[SelectionState->SelectedSpliterIndex - 1].Value += Columns[SelectionState->SelectedSpliterIndex].Value;
		Columns.RemoveAt(SelectionState->SelectedSpliterIndex);
		TargetComp->SetColumns(Columns);
	}
	else
	{
		LayoutChangeSizeToPixelRelevant(false);
		auto Rows = TargetComp->GetRows();
		Rows[SelectionState->SelectedSpliterIndex - 1].Value += Rows[SelectionState->SelectedSpliterIndex].Value;
		Rows.RemoveAt(SelectionState->SelectedSpliterIndex);
		TargetComp->SetRows(Rows);
	}
	SelectionState->SelectedSpliterIndex = -1;
	GEditor->EndTransaction();
}
void FUIPanelLayoutFlexibleGridComponentVisualizer::ChangeSpliterType()
{
	if (!TargetComp.IsValid())return;
	auto UIItem = TargetComp->GetRootUIComponent();
	if (!UIItem)return;


}
void FUIPanelLayoutFlexibleGridComponentVisualizer::LayoutChangeSizeToPixelRelevant(bool InHorizontalOrVertical)
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
			if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
			{
				columnTotalRatio += Item.Value;
			}
			else
			{
				columnTotalContstantSize += Item.Value;
			}
		}
		float invColumnTotalRatio = 1.0f / columnTotalRatio;
		float thisWidth = UIItem->GetWidth();
		float thisFreeWidth = thisWidth - columnTotalContstantSize;
		for (int i = 0; i < Columns.Num(); i++)
		{
			auto& Column = Columns[i];
			if (Column.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
			{
				Column.Value = thisFreeWidth * Column.Value * invColumnTotalRatio;
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
			if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
			{
				rowTotalRatio += Item.Value;
			}
			else
			{
				rowTotalConstantSize += Item.Value;
			}
		}
		float invRowTotalRatio = 1.0f / rowTotalRatio;
		float thisHeight = UIItem->GetHeight();
		float thisFreeHeight = thisHeight - rowTotalConstantSize;
		for (int i = 0; i < Rows.Num(); i++)
		{
			auto& Row = Rows[i];
			if (Row.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
			{
				Row.Value = thisFreeHeight * Row.Value * invRowTotalRatio;
			}
		}
		TargetComp->SetRows(Rows);
	}
}


IMPLEMENT_HIT_PROXY(HUIPanelLayoutFlexibleGridSpliterVisProxy, HComponentVisProxy);
HUIPanelLayoutFlexibleGridSpliterVisProxy::HUIPanelLayoutFlexibleGridSpliterVisProxy(const UUIPanelLayout_FlexibleGrid* InComponent, bool InHorizontalOrVertical, bool InFirstOrSecond, int32 InSelectingSpliterIndex
)
	: HComponentVisProxy(InComponent, HPP_Foreground)
{
	bHorizontalOrVertical = InHorizontalOrVertical;
	bFirstOrSecond = InFirstOrSecond;
	SpliterIndex = InSelectingSpliterIndex;
}

IMPLEMENT_HIT_PROXY(HUIPanelLayoutFlexibleGridFrameLineVisProxy, HComponentVisProxy);
HUIPanelLayoutFlexibleGridFrameLineVisProxy::HUIPanelLayoutFlexibleGridFrameLineVisProxy(const UUIPanelLayout_FlexibleGrid* InComponent, EFrameType InFrameType
)
	: HComponentVisProxy(InComponent, HPP_Foreground)
{
	FrameType = InFrameType;
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
