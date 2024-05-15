// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIPanelLayout_FlexibleGrid.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

DECLARE_CYCLE_STAT(TEXT("UIPanelLayout FlexibleGrid RebuildLayout"), STAT_PanelLayout_FlexibleGrid, STATGROUP_LGUI);

UUIPanelLayout_FlexibleGrid::UUIPanelLayout_FlexibleGrid()
{
    Rows = Columns =
    {
        FUIPanelLayout_FlexibleGridSize(1.0f),
        FUIPanelLayout_FlexibleGridSize(1.0f),
    };
}

void UUIPanelLayout_FlexibleGrid::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    //skip UILayoutBase
    Super::Super::OnUIChildDimensionsChanged(child, horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (this->GetWorld() == nullptr)return;
    if (child->GetIsUIActiveInHierarchy())
    {
        MarkNeedRebuildLayout();
    }
}

void UUIPanelLayout_FlexibleGrid::OnRebuildLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_PanelLayout_FlexibleGrid);
    if (!CheckRootUIComponent())return;
    if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAllAnimations();

    EUILayoutAnimationType TempAnimationType = AnimationType;
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld())
    {
        TempAnimationType = EUILayoutAnimationType::Immediately;
    }
#endif

    FVector2D RectSize(RootUIComp->GetWidth(), RootUIComp->GetHeight());
    float ColumnTotalRatio = 0, RowTotalRatio = 0;
    float ColumnTotalConstantSize = 0, RowTotalConstantSize = 0;
    for (auto& Item : Columns)
    {
        if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
        {
            ColumnTotalRatio += Item.Value;
        }
        else
        {
            ColumnTotalConstantSize += Item.Value;
        }
    }
    for (auto& Item : Rows)
    {
        if (Item.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
        {
            RowTotalRatio += Item.Value;
        }
        else
        {
            RowTotalConstantSize += Item.Value;
        }
    }
    float InvColumnTotalRatio = 1.0f / ColumnTotalRatio;
    float InvRowTotalRatio = 1.0f / RowTotalRatio;
    float TotalFillWidth = RectSize.X - ColumnTotalConstantSize;
    float TotalFillHeight = RectSize.Y - RowTotalConstantSize;
    auto GetOffset = [=, this](int Column, int Row, float& OffsetX, float& OffsetY)
    {
        float OffsetXRatio = 0, OffsetYRatio = 0;
        float OffsetXConstant = 0, OffsetYConstant = 0;
        for (int i = 0; i < Column; i++)
        {
            auto& ColumnData = Columns[i];
            if (ColumnData.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
            {
                OffsetXRatio += ColumnData.Value;
            }
            else
            {
                OffsetXConstant += ColumnData.Value;
            }
        }
        for (int i = 0; i < Row; i++)
        {
            auto& RowData = Rows[i];
            if (RowData.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
            {
                OffsetYRatio += RowData.Value;
            }
            else
            {
                OffsetYConstant += RowData.Value;
            }
        }
        OffsetX = OffsetXRatio * InvColumnTotalRatio * TotalFillWidth + OffsetXConstant;
        OffsetY = -OffsetYRatio * InvRowTotalRatio * TotalFillHeight - OffsetYConstant;
    };
    auto& LayoutChildrenList = GetLayoutUIItemChildren();
    for (auto& LayoutChild : LayoutChildrenList)
    {
        if (auto Slot = Cast<UUIPanelLayout_FlexibleGrid_Slot>(LayoutChild.LayoutInterface.Get()))
        {
            if (Slot->GetIgnoreLayout())continue;
            auto& Padding = Slot->GetPadding();
            auto UIItem = LayoutChild.ChildUIItem.Get();

            float ColumnRatio = 0, ColumnConstant = 0;
            for (int i = Slot->GetColumn(), Count = FMath::Min(i + Slot->GetColumnSpan(), Columns.Num()); i < Count; i++)
            {
                auto& Column = Columns[i];
                if (Column.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
                {
                    ColumnRatio += Column.Value;
                }
                else
                {
                    ColumnConstant += Column.Value;
                }
            }
            float RowRatio = 0, RowConstant = 0;
            for (int i = Slot->GetRow(), Count = FMath::Min(i + Slot->GetRowSpan(), Rows.Num()); i < Count; i++)
            {
                auto& Row = Rows[i];
                if (Row.SizeType == EUIPanelLayout_FlexibleGridSizeRule::Ratio)
                {
                    RowRatio += Row.Value;
                }
                else
                {
                    RowConstant += Row.Value;
                }
            }

            float ItemAreaWidth = TotalFillWidth * ColumnRatio * InvColumnTotalRatio + ColumnConstant - (Padding.Left + Padding.Right);
            float ItemAreaHeight = TotalFillHeight * RowRatio * InvRowTotalRatio + RowConstant - (Padding.Top + Padding.Bottom);
            float PosX = 0, PosY = 0;
            GetOffset(Slot->GetColumn(), Slot->GetRow(), PosX, PosY);
            auto HAlign = Slot->GetHorizontalAlignment();
            auto VAlign = Slot->GetVerticalAlignment();
            float ItemWidth = Slot->GetDesiredSize().X;
            float ItemHeight = Slot->GetDesiredSize().Y;
            auto ItemOffsetX = 0.0f;
            auto ItemOffsetY = 0.0f;
            if (ItemWidth < ItemAreaWidth)
            {
                switch (HAlign)
                {
                case HAlign_Left:
                    ItemOffsetX = -(ItemAreaWidth - ItemWidth) * 0.5f;
                    break;
                case HAlign_Center:
                    break;
                case HAlign_Right:
                    ItemOffsetX = (ItemAreaWidth - ItemWidth) * 0.5f;
                    break;
                case HAlign_Fill:
                    ItemWidth = ItemAreaWidth;
                    break;
                }
            }
            else//ItemWidth should smaller than ItemAreaWidth
            {
                ItemWidth = ItemAreaWidth;
            }
            if (ItemHeight < ItemAreaHeight)
            {
                switch (VAlign)
                {
                case VAlign_Top:
                    ItemOffsetY = (ItemAreaHeight - ItemHeight) * 0.5f;
                    break;
                case VAlign_Center:
                    break;
                case VAlign_Bottom:
                    ItemOffsetY = -(ItemAreaHeight - ItemHeight) * 0.5f;
                    break;
                case VAlign_Fill:
                    ItemHeight = ItemAreaHeight;
                    break;
                }
            }
            else//ItemHeight should smaller than ItemAreaHeight
            {
                ItemHeight = ItemAreaHeight;
            }

            auto AnchorMin = UIItem->GetAnchorMin();
            auto AnchorMax = UIItem->GetAnchorMax();
            if (AnchorMin.X != AnchorMax.X)//custom anchor not support
            {
                UIItem->SetHorizontalAnchorMinMax(FVector2D(0, 0), true, true);
            }
            if (AnchorMin.Y != AnchorMax.Y)
            {
                UIItem->SetVerticalAnchorMinMax(FVector2D(1, 1), true, true);
            }
            float AnchorOffsetX = PosX + UIItem->GetPivot().X * ItemAreaWidth;
            float AnchorOffsetY = PosY - (1.0f - UIItem->GetPivot().Y) * ItemAreaHeight;
            //padding
            AnchorOffsetX += Padding.Left;
            AnchorOffsetY += -Padding.Top;
            //local offset
            AnchorOffsetX += ItemOffsetX;
            AnchorOffsetY += ItemOffsetY;
            //parent anchor
            AnchorOffsetX -= AnchorMin.X * RootUIComp->GetWidth();
            AnchorOffsetY += AnchorMin.Y * RootUIComp->GetHeight();
            ApplyAnchoredPositionWithAnimation(TempAnimationType, FVector2D(AnchorOffsetX, AnchorOffsetY), UIItem);
            ApplyWidthWithAnimation(TempAnimationType, ItemWidth, UIItem);
            ApplyHeightWithAnimation(TempAnimationType, ItemHeight, UIItem);
        }
    }
    
	if (TempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		EndSetupAnimations();
	}
}

bool UUIPanelLayout_FlexibleGrid::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)//self
    {
        OutResult.bCanControlHorizontalSizeDelta = false;
        OutResult.bCanControlVerticalSizeDelta = false;
        return true;
    }
    else if (this->GetRootUIComponent() == InUIItem->GetAttachParent())//child
    {
        UObject* LayoutElementInterface = nullptr;
        bool bIgnoreLayout = false;
        GetLayoutElement(InUIItem, LayoutElementInterface, bIgnoreLayout);
        if (bIgnoreLayout)
        {
            return true;
        }
        auto Slot = Cast<UUIPanelLayout_FlexibleGrid_Slot>(LayoutElementInterface);
        if (!Slot)return false;

        OutResult.bCanControlHorizontalAnchor = this->GetEnable();
        OutResult.bCanControlVerticalAnchor = this->GetEnable();
        OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlHorizontalSizeDelta = this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = this->GetEnable();
        return true;
    }
    return false;
}

UClass* UUIPanelLayout_FlexibleGrid::GetPanelLayoutSlotClass()const
{
    return UUIPanelLayout_FlexibleGrid_Slot::StaticClass();
}

#if WITH_EDITOR
FText UUIPanelLayout_FlexibleGrid::GetCategoryDisplayName()const
{
    return NSLOCTEXT("UIPanelLayout_FlexibleGrid", "CategoryDisplayName", "FlexibleGrid");
}
bool UUIPanelLayout_FlexibleGrid::CanMoveChildToCell(UUIItem* InChild, EMoveChildDirectionType InDirection)const
{
    return true;
}
void UUIPanelLayout_FlexibleGrid::MoveChildToCell(UUIItem* InChild, EMoveChildDirectionType InDirection)
{
    int MoveColumnValue = 0, MoveRowValue = 0;
    switch (InDirection)
    {
    case UUIPanelLayoutBase::EMoveChildDirectionType::Left:
        MoveColumnValue = -1;
        break;
    case UUIPanelLayoutBase::EMoveChildDirectionType::Right:
        MoveColumnValue = 1;
        break;
    case UUIPanelLayoutBase::EMoveChildDirectionType::Top:
        MoveRowValue = -1;
        break;
    case UUIPanelLayoutBase::EMoveChildDirectionType::Bottom:
        MoveRowValue = 1;
        break;
    }
    if (auto Slot = Cast<UUIPanelLayout_FlexibleGrid_Slot>(GetChildSlot(InChild)))
    {
        Slot->SetColumn(Slot->GetColumn() + MoveColumnValue);
        Slot->SetRow(Slot->GetRow() + MoveRowValue);
    }
}
#endif
void UUIPanelLayout_FlexibleGrid::SetColumns(const TArray<FUIPanelLayout_FlexibleGridSize>& Value)
{
    if (Columns != Value)
    {
        Columns = Value;
        //varify slot's column and row range
        {
            for (auto& KeyValue : MapChildToSlot)
            {
                if (auto Slot = Cast<UUIPanelLayout_FlexibleGrid_Slot>(KeyValue.Value))
                {
                    Slot->VerifyColumnAndRow(this);
                }
            }
        }
        MarkNeedRebuildLayout();
    }
}
void UUIPanelLayout_FlexibleGrid::SetRows(const TArray<FUIPanelLayout_FlexibleGridSize>& Value)
{
    if (Rows != Value)
    {
        Rows = Value;
        //varify slot's column and row range
        {
            for (auto& KeyValue : MapChildToSlot)
            {
                if (auto Slot = Cast<UUIPanelLayout_FlexibleGrid_Slot>(KeyValue.Value))
                {
                    Slot->VerifyColumnAndRow(this);
                }
            }
        }
        MarkNeedRebuildLayout();
    }
}

#if WITH_EDITOR
void UUIPanelLayout_FlexibleGrid_Slot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
    {
        VerifyColumnAndRow(Layout);
        Layout->MarkNeedRebuildLayout();
        Layout->MarkNeedRebuildChildrenList();
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::PostEditUndo()
{
    Super::PostEditUndo();
    if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
    {
        VerifyColumnAndRow(Layout);
        Layout->MarkNeedRebuildLayout();
        Layout->MarkNeedRebuildChildrenList();
    }
}
#endif
void UUIPanelLayout_FlexibleGrid_Slot::VerifyColumnAndRow(UUIPanelLayout_FlexibleGrid* Layout)
{
    Row = FMath::Clamp(Row, 0, Layout->GetRows().Num() - 1);
    RowSpan = FMath::Clamp(RowSpan, 1, Layout->GetRows().Num() - Row);
    Column = FMath::Clamp(Column, 0, Layout->GetColumns().Num() - 1);
    ColumnSpan = FMath::Clamp(ColumnSpan, 1, Layout->GetColumns().Num() - Column);
}
void UUIPanelLayout_FlexibleGrid_Slot::SetPadding(const FMargin& Value)
{
    if (Padding != Value)
    {
        Padding = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetColumn(int Value)
{
    if (Column != Value)
    {
        Column = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            VerifyColumnAndRow(Layout);
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetColumnSpan(int Value)
{
    if (ColumnSpan != Value)
    {
        ColumnSpan = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            VerifyColumnAndRow(Layout);
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetRow(int Value)
{
    if (Row != Value)
    {
        Row = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            VerifyColumnAndRow(Layout);
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetRowSpan(int Value)
{
    if (RowSpan != Value)
    {
        RowSpan = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            VerifyColumnAndRow(Layout);
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetHorizontalAlignment(EHorizontalAlignment Value)
{
    if (HorizontalAlignment != Value)
    {
        HorizontalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_FlexibleGrid_Slot::SetVerticalAlignment(EVerticalAlignment Value)
{
    if (VerticalAlignment != Value)
    {
        VerticalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_FlexibleGrid>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
