// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIPanelLayout_VerticalBox.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

DECLARE_CYCLE_STAT(TEXT("UIPanelLayout VerticalBox RebuildLayout"), STAT_PanelLayout_Vertical, STATGROUP_LGUI);

void UUIPanelLayout_VerticalBox::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    //skip UILayoutBase
    Super::Super::OnUIChildDimensionsChanged(child, horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (this->GetWorld() == nullptr)return;
    if (child->GetIsUIActiveInHierarchy())
    {
        if (horizontalPositionChanged
            || verticalPositionChanged
            || widthChanged
            || heightChanged
            )
        {
            MarkNeedRebuildLayout();
        }
    }
}

void UUIPanelLayout_VerticalBox::OnRebuildLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_PanelLayout_Vertical);
    if (!CheckRootUIComponent())return;
    if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAnimation();

    EUILayoutAnimationType TempAnimationType = AnimationType;
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld())
    {
        TempAnimationType = EUILayoutAnimationType::Immediately;
    }
#endif

    FVector2D RectSize(RootUIComp->GetWidth(), RootUIComp->GetHeight());
    float TotalFillRatio = 0.0f;
    float TotalFillHeight = 0.0f;
    float TotalAutoHeight = 0.0f;
    float TotalPaddingHeight = 0.0f;
    float ChildWidthMin = MAX_FLT, ChildWidthMax = -MAX_FLT;
    auto& LayoutChildrenList = GetLayoutUIItemChildren();
    for (int i = 0; i < LayoutChildrenList.Num(); i++)
    {
        auto& LayoutChild = LayoutChildrenList[i];
        if (auto Slot = Cast<UUIPanelLayout_VerticalBox_Slot>(LayoutChild.LayoutInterface.Get()))
        {
            if (Slot->GetIgnoreLayout())continue;
            auto& Size = Slot->GetSizeRule();
            if (Size.SizeRule == ESlateSizeRule::Fill)
            {
                TotalFillRatio += Size.Value;
                TotalFillHeight += Slot->GetDesiredSize().Y;
            }
            else
            {
                TotalAutoHeight += Slot->GetDesiredSize().Y;
            }
            auto& Padding = Slot->GetPadding();
            TotalPaddingHeight += Padding.Top + Padding.Bottom;

            if (bWidthFitToChildren)
            {
                auto WidthWithPadding = Padding.Left + Padding.Right + Slot->GetDesiredSize().X;
                if (ChildWidthMin > WidthWithPadding)
                {
                    ChildWidthMin = WidthWithPadding;
                }
                if (ChildWidthMax < WidthWithPadding)
                {
                    ChildWidthMax = WidthWithPadding;
                }
            }
        }
    }
    if (bWidthFitToChildren)
    {
        RectSize.X = FMath::Lerp(ChildWidthMin, ChildWidthMax, WidthFitToChildrenFromMinToMax);
        RootUIComp->SetWidth(RectSize.X);
    }
    if (bHeightFitToChildren)
    {
        RectSize.Y = TotalFillHeight + TotalAutoHeight + TotalPaddingHeight;
        RootUIComp->SetHeight(RectSize.Y);
    }
    auto TotalFillSize = RectSize.Y - TotalAutoHeight - TotalPaddingHeight;
    float Inv_TotalFillRatio = 1.0f / TotalFillRatio;
    float PosX = 0, PosY = 0;
    for (int i = 0; i < LayoutChildrenList.Num(); i++)
    {
        auto& LayoutChild = LayoutChildrenList[i];
        if (auto Slot = Cast<UUIPanelLayout_VerticalBox_Slot>(LayoutChild.LayoutInterface.Get()))
        {
            if (Slot->GetIgnoreLayout())continue;
            auto& Padding = Slot->GetPadding();
            auto UIItem = LayoutChild.ChildUIItem.Get();
            auto& Size = Slot->GetSizeRule();
            float ItemAreaHeight = 0;
            float ItemAreaWidth = RectSize.X - (Padding.Left + Padding.Right);
            if (Size.SizeRule == ESlateSizeRule::Fill)
            {
                ItemAreaHeight = TotalFillSize * Size.Value * Inv_TotalFillRatio;
            }
            else
            {
                ItemAreaHeight = Slot->GetDesiredSize().Y;
            }
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

            PosY -= Padding.Top + Padding.Bottom;
            PosY -= ItemAreaHeight;
        }
    }
    
	if (TempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUIPanelLayout_VerticalBox::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)//self
    {
        OutResult.bCanControlHorizontalSizeDelta = bWidthFitToChildren && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = bHeightFitToChildren && this->GetEnable();
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
        auto Slot = Cast<UUIPanelLayout_VerticalBox_Slot>(LayoutElementInterface);
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

UClass* UUIPanelLayout_VerticalBox::GetPanelLayoutSlotClass()const
{
    return UUIPanelLayout_VerticalBox_Slot::StaticClass();
}

#if WITH_EDITOR
FText UUIPanelLayout_VerticalBox::GetCategoryDisplayName()const
{
    return NSLOCTEXT("UIPanelLayout_VerticalBox", "CategoryDisplayName", "VerticalBox");
}
#endif
void UUIPanelLayout_VerticalBox::SetWidthFitToChildren(bool Value)
{
    if (bWidthFitToChildren != Value)
    {
        bWidthFitToChildren = Value;
        MarkNeedRebuildLayout();
    }
}
void UUIPanelLayout_VerticalBox::SetHeightFitToChildren(bool Value)
{
    if (bHeightFitToChildren != Value)
    {
        bHeightFitToChildren = Value;
        MarkNeedRebuildLayout();
    }
}
void UUIPanelLayout_VerticalBox::SetWidthFitToChildrenFromMinToMax(float Value)
{
    if (WidthFitToChildrenFromMinToMax != Value)
    {
        WidthFitToChildrenFromMinToMax = Value;
        MarkNeedRebuildLayout();
    }
}

void UUIPanelLayout_VerticalBox_Slot::SetPadding(const FMargin& Value)
{
    if (Padding != Value)
    {
        Padding = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_VerticalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_VerticalBox_Slot::SetSizeRule(const FSlateChildSize& Value)
{
    if (SizeRule.SizeRule != Value.SizeRule || SizeRule.Value != Value.Value)
    {
        SizeRule = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_VerticalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_VerticalBox_Slot::SetHorizontalAlignment(const EHorizontalAlignment& Value)
{
    if (HorizontalAlignment != Value)
    {
        HorizontalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_VerticalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_VerticalBox_Slot::SetVerticalAlignment(const EVerticalAlignment& Value)
{
    if (VerticalAlignment != Value)
    {
        VerticalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_VerticalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
