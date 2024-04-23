// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIPanelLayout_HorizontalBox.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

DECLARE_CYCLE_STAT(TEXT("UIPanelLayout HorizontalBox RebuildLayout"), STAT_PanelLayout_Horizontal, STATGROUP_LGUI);

void UUIPanelLayout_HorizontalBox::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
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

void UUIPanelLayout_HorizontalBox::OnRebuildLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_PanelLayout_Horizontal);
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
    float TotalFillRatio = 0.0f;
    float TotalFillWidth = 0.0f;
    float TotalAutoWidth = 0.0f;
    float TotalPaddingWidth = 0.0f;
    float ChildHeightMin = MAX_FLT, ChildHeightMax = -MAX_FLT;
    auto& LayoutChildrenList = GetLayoutUIItemChildren();
    for (int i = 0; i < LayoutChildrenList.Num(); i++)
    {
        auto& LayoutChild = LayoutChildrenList[i];
        if (auto Slot = Cast<UUIPanelLayout_HorizontalBox_Slot>(LayoutChild.LayoutInterface.Get()))
        {
            if (Slot->GetIgnoreLayout())continue;
            auto& Size = Slot->GetSizeRule();
            if (Size.SizeRule == ESlateSizeRule::Fill)
            {
                TotalFillRatio += Size.Value;
                TotalFillWidth += Slot->GetDesiredSize().X;
            }
            else
            {
                TotalAutoWidth += Slot->GetDesiredSize().X;
            }
            auto& Padding = Slot->GetPadding();
            TotalPaddingWidth += Padding.Left + Padding.Right;

            if (bHeightFitToChildren)
            {
                auto HeightWithPadding = Padding.Top + Padding.Bottom + Slot->GetDesiredSize().Y;
                if (ChildHeightMin > HeightWithPadding)
                {
                    ChildHeightMin = HeightWithPadding;
                }
                if (ChildHeightMax < HeightWithPadding)
                {
                    ChildHeightMax = HeightWithPadding;
                }
            }
        }
    }
    if (bWidthFitToChildren)
    {
        RectSize.X = TotalFillWidth + TotalAutoWidth + TotalPaddingWidth;
        ApplyWidthWithAnimation(TempAnimationType, RectSize.X, RootUIComp.Get());
    }
    if (bHeightFitToChildren)
    {
        RectSize.Y = FMath::Lerp(ChildHeightMin, ChildHeightMax, HeightFitToChildrenFromMinToMax);
        ApplyHeightWithAnimation(TempAnimationType, RectSize.Y, RootUIComp.Get());
    }
    auto TotalFillSize = RectSize.X - TotalAutoWidth - TotalPaddingWidth;
    float Inv_TotalFillRatio = 1.0f / TotalFillRatio;
    float PosX = 0, PosY = 0;
    for (int i = 0; i < LayoutChildrenList.Num(); i++)
    {
        auto& LayoutChild = LayoutChildrenList[i];
        if (auto Slot = Cast<UUIPanelLayout_HorizontalBox_Slot>(LayoutChild.LayoutInterface.Get()))
        {
            if (Slot->GetIgnoreLayout())continue;
            auto& Padding = Slot->GetPadding();
            auto UIItem = LayoutChild.ChildUIItem.Get();
            auto& Size = Slot->GetSizeRule();
            float ItemAreaWidth = 0;
            float ItemAreaHeight = RectSize.Y - (Padding.Top + Padding.Bottom);
            if (Size.SizeRule == ESlateSizeRule::Fill)
            {
                ItemAreaWidth = TotalFillSize * Size.Value * Inv_TotalFillRatio;
            }
            else
            {
                ItemAreaWidth = Slot->GetDesiredSize().X;
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

            PosX += Padding.Left + Padding.Right;
            PosX += ItemAreaWidth;
        }
    }
    
	if (TempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		EndSetupAnimations();
	}
}

bool UUIPanelLayout_HorizontalBox::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
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
        auto Slot = Cast<UUIPanelLayout_HorizontalBox_Slot>(LayoutElementInterface);
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

UClass* UUIPanelLayout_HorizontalBox::GetPanelLayoutSlotClass()const
{
    return UUIPanelLayout_HorizontalBox_Slot::StaticClass();
}

#if WITH_EDITOR
FText UUIPanelLayout_HorizontalBox::GetCategoryDisplayName()const
{
    return NSLOCTEXT("UIPanelLayout_HorizontalBox", "CategoryDisplayName", "HorizontalBox");
}
#endif
void UUIPanelLayout_HorizontalBox::SetWidthFitToChildren(bool Value)
{
    if (bWidthFitToChildren != Value)
    {
        bWidthFitToChildren = Value;
        MarkNeedRebuildLayout();
    }
}
void UUIPanelLayout_HorizontalBox::SetHeightFitToChildren(bool Value)
{
    if (bHeightFitToChildren != Value)
    {
        bHeightFitToChildren = Value;
        MarkNeedRebuildLayout();
    }
}
void UUIPanelLayout_HorizontalBox::SetHeightFitToChildrenFromMinToMax(float Value)
{
    if (HeightFitToChildrenFromMinToMax != Value)
    {
        HeightFitToChildrenFromMinToMax = Value;
        MarkNeedRebuildLayout();
    }
}

void UUIPanelLayout_HorizontalBox_Slot::SetPadding(const FMargin& Value)
{
    if (Padding != Value)
    {
        Padding = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_HorizontalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_HorizontalBox_Slot::SetSizeRule(const FSlateChildSize& Value)
{
    if (SizeRule.SizeRule != Value.SizeRule || SizeRule.Value != Value.Value)
    {
        SizeRule = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_HorizontalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_HorizontalBox_Slot::SetHorizontalAlignment(const EHorizontalAlignment& Value)
{
    if (HorizontalAlignment != Value)
    {
        HorizontalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_HorizontalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
void UUIPanelLayout_HorizontalBox_Slot::SetVerticalAlignment(const EVerticalAlignment& Value)
{
    if (VerticalAlignment != Value)
    {
        VerticalAlignment = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayout_HorizontalBox>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}
