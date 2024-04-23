// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIVerticalLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

DECLARE_CYCLE_STAT(TEXT("UILayout VerticalRebuildLayout"), STAT_VerticalLayout, STATGROUP_LGUI);

void UUIVerticalLayout::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    //skip UILayoutBase
    Super::Super::OnUIChildDimensionsChanged(child, horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (this->GetWorld() == nullptr)return;
    if (child->GetIsUIActiveInHierarchy())
    {
        if (horizontalPositionChanged || verticalPositionChanged
            || (ExpandChildWidthArea && widthChanged)
            || (ExpandChildHeightArea && heightChanged)
            )
        {
            MarkNeedRebuildLayout();
        }
    }
}

void UUIVerticalLayout::SetPadding(FMargin value)
{
    if (Padding != value)
    {
        Padding = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetSpacing(float value)
{
    if (Spacing != value)
    {
        Spacing = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetAlign(ELGUILayoutAlignmentType value)
{
    if (Align != value)
    {
        Align = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetExpandChildWidthArea(bool value)
{
    if (ExpandChildWidthArea != value)
    {
        ExpandChildWidthArea = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetExpandChildHeightArea(bool value)
{
    if (ExpandChildHeightArea != value)
    {
        ExpandChildHeightArea = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetHeightFitToChildren(bool value)
{
    if (HeightFitToChildren != value)
    {
        HeightFitToChildren = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetWidthFitToChildren(bool value)
{
    if (WidthFitToChildren != value)
    {
        WidthFitToChildren = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetWidthFitToChildrenFromMinToMax(float value)
{
    if (WidthFitToChildrenFromMinToMax != value)
    {
        WidthFitToChildrenFromMinToMax = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetUseChildScaleOnHeight(bool value)
{
    if (UseChildScaleOnHeight != value)
    {
        UseChildScaleOnHeight = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetControlChildWidth(bool value)
{
    if (ControlChildWidth != value)
    {
        ControlChildWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetControlChildHeight(bool value)
{
    if (ControlChildHeight != value)
    {
        ControlChildHeight = value;
        MarkNeedRebuildLayout();
    }
}


void UUIVerticalLayout::OnRebuildLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_VerticalLayout);
    if (!CheckRootUIComponent())return;
    if (!GetEnable())return;
	if (bIsAnimationPlaying)
	{
		bShouldRebuildLayoutAfterAnimation = true;
		return;
	}
	CancelAllAnimations();

    FVector2D startPosition;
    startPosition.X = Padding.Left;
    startPosition.Y = -Padding.Top; //left bottom as start point
    FVector2D rectSize;
    rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
    rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

    const auto& uiChildrenList = GetLayoutUIItemChildren();
    int childrenCount = uiChildrenList.Num();
    float childWidth = rectSize.X;
    childrenHeightList.SetNumUninitialized(childrenCount);
    if (ExpandChildHeightArea)
    {
        float sizeWithoutSpacing = rectSize.Y - Spacing * (childrenCount - 1);
        float autoSizeChildrenTotalHeight = sizeWithoutSpacing; //collect all children's size which is autosize or without layout element
        int autoSizeChildrenCount = 0;
        for (int i = 0; i < childrenCount; i++)
        {
            auto& item = uiChildrenList[i];
            float tempChildHeight = 0;
            if (item.LayoutInterface != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                switch (layoutType)
                {
                case ELayoutElementType::AutoSize:
                    autoSizeChildrenCount++;
                    break;
                case ELayoutElementType::ConstantSize:
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetConstantSize(item.LayoutInterface.Get(), ELayoutElementSizeType::Vertical);
                    break;
                case ELayoutElementType::RatioSize:
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetRatioSize(item.LayoutInterface.Get(), ELayoutElementSizeType::Vertical) * sizeWithoutSpacing;
                    break;
                }
            }
            else
            {
                autoSizeChildrenCount++;
            }
            if (UseChildScaleOnHeight)
            {
                tempChildHeight *= item.ChildUIItem->GetRelativeScale3D().Z;
            }
            autoSizeChildrenTotalHeight -= tempChildHeight;
            childrenHeightList[i].AreaHeight = tempChildHeight;
            childrenHeightList[i].Height = tempChildHeight;
        }
        
        float autoSizeChildScaleSum = 0;
        for (int i = 0; i < childrenCount; i++)
        {
            auto& item = uiChildrenList[i];
            bool bCanCollectSize = false;
            if (item.LayoutInterface != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                bCanCollectSize = layoutType == ELayoutElementType::AutoSize;
            }
            else
            {
                bCanCollectSize = true;
            }
            if (bCanCollectSize)
            {
                autoSizeChildScaleSum += UseChildScaleOnHeight ? item.ChildUIItem->GetRelativeScale3D().Z : 1.0f;
            }
        }

        if (ControlChildHeight)
        {
            float childAverangeHeight = autoSizeChildrenTotalHeight / autoSizeChildScaleSum;
            for (int i = 0; i < childrenCount; i++)
            {
                auto& item = uiChildrenList[i];
                bool bCanSetSize = false;
                if (item.LayoutInterface != nullptr)
                {
                    auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                    bCanSetSize = layoutType == ELayoutElementType::AutoSize;
                }
                else
                {
                    bCanSetSize = true;
                }
                if (bCanSetSize)
                {
                    auto childHeight = childAverangeHeight;
                    childrenHeightList[i].Height = childHeight;
                    if (UseChildScaleOnHeight)
                    {
                        childHeight *= item.ChildUIItem->GetRelativeScale3D().Z;
                    }
                    childrenHeightList[i].AreaHeight = childHeight;
                }
            }
        }
        else
        {
            float autoSizeChildSizeSum = 0;
            for (int i = 0; i < childrenCount; i++)
            {
                auto& item = uiChildrenList[i];
                bool bCanCollectSize = false;
                if (item.LayoutInterface != nullptr)
                {
                    auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                    bCanCollectSize = layoutType == ELayoutElementType::AutoSize;
                }
                else
                {
                    bCanCollectSize = true;
                }
                if (bCanCollectSize)
                {
                    auto childHeight = item.ChildUIItem->GetHeight();
                    if (UseChildScaleOnHeight)
                    {
                        childHeight *= item.ChildUIItem->GetRelativeScale3D().Z;
                    }
                    autoSizeChildSizeSum += childHeight;
                }
            }

            auto extraChildHeightTotal = autoSizeChildrenTotalHeight - autoSizeChildSizeSum;
            for (int i = 0; i < childrenCount; i++)
            {
                auto& item = uiChildrenList[i];
                bool bCanCollectSize = false;
                if (item.LayoutInterface != nullptr)
                {
                    auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                    bCanCollectSize = layoutType == ELayoutElementType::AutoSize;
                }
                else
                {
                    bCanCollectSize = true;
                }
                if (bCanCollectSize)
                {
                    auto childHeight = item.ChildUIItem->GetHeight();
                    float extraChildHeight;
                    if (UseChildScaleOnHeight)
                    {
                        childHeight *= item.ChildUIItem->GetRelativeScale3D().Z;
                        extraChildHeight = extraChildHeightTotal * item.ChildUIItem->GetRelativeScale3D().Z / autoSizeChildScaleSum;
                    }
                    else
                    {
                        extraChildHeight = extraChildHeightTotal / autoSizeChildrenCount;
                    }
                    childrenHeightList[i].AreaHeight = childHeight + extraChildHeight;
                    childrenHeightList[i].Height = childHeight;
                }
            }
        }
    }
    else
    {
        switch (Align)
        {
        case ELGUILayoutAlignmentType::UpperLeft:
        case ELGUILayoutAlignmentType::UpperCenter:
        case ELGUILayoutAlignmentType::UpperRight:
        {
        }
        break;
        case ELGUILayoutAlignmentType::MiddleLeft:
        case ELGUILayoutAlignmentType::MiddleCenter:
        case ELGUILayoutAlignmentType::MiddleRight:
        {
            float totalHeight = 0;
            for (int i = 0; i < childrenCount; i++)
            {
                auto& ChildUIItem = uiChildrenList[i].ChildUIItem;
                auto itemHeight = ChildUIItem->GetHeight();
                if (UseChildScaleOnHeight)
                {
                    itemHeight *= ChildUIItem->GetRelativeScale3D().Z;
                }
                totalHeight += itemHeight;
            }
            totalHeight += Spacing * (childrenCount - 1);
            startPosition.Y -= (rectSize.Y - totalHeight) * 0.5f;
        }
        break;
        case ELGUILayoutAlignmentType::LowerLeft:
        case ELGUILayoutAlignmentType::LowerCenter:
        case ELGUILayoutAlignmentType::LowerRight:
        {
            float totalHeight = 0;
            for (int i = 0; i < childrenCount; i++)
            {
                auto& ChildUIItem = uiChildrenList[i].ChildUIItem;
                auto itemHeight = ChildUIItem->GetHeight();
                if (UseChildScaleOnHeight)
                {
                    itemHeight *= ChildUIItem->GetRelativeScale3D().Z;
                }
                totalHeight += itemHeight;
            }
            totalHeight += Spacing * (childrenCount - 1);
            startPosition.Y -= (rectSize.Y - totalHeight);
        }
        break;
        }

        for (int i = 0; i < childrenCount; i++)
        {
            auto& item = uiChildrenList[i];
            bool bCanCollectSize = false;
            if (item.LayoutInterface != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                bCanCollectSize = layoutType == ELayoutElementType::AutoSize;
            }
            else
            {
                bCanCollectSize = true;
            }
            if (bCanCollectSize)
            {
                auto childHeight = item.ChildUIItem->GetHeight();
                childrenHeightList[i].Height = childHeight;
                if (UseChildScaleOnHeight)
                {
                    childHeight *= item.ChildUIItem->GetRelativeScale3D().Z;
                }
                childrenHeightList[i].AreaHeight = childHeight;
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
    float posX = startPosition.X, posY = startPosition.Y;
    float tempActuralVerticalRange = 0;
    float tempHorizontalMinSize = MAX_flt;
    float tempHorizontalMaxSize = MIN_flt;
    for (int i = 0; i < childrenCount; i++)
    {
        auto ChildUIItem = uiChildrenList[i].ChildUIItem;
        if (ExpandChildWidthArea)
        {
            if (ControlChildWidth)
            {
                ApplyWidthWithAnimation(tempAnimationType, childWidth, ChildUIItem.Get());
            }
        }
        else
        {
            childWidth = ChildUIItem->GetWidth();
            switch (Align)
            {
            default:
            case ELGUILayoutAlignmentType::UpperLeft:
            case ELGUILayoutAlignmentType::MiddleLeft:
            case ELGUILayoutAlignmentType::LowerLeft:
            {
                posX = startPosition.X;
            }
            break;
            case ELGUILayoutAlignmentType::UpperCenter:
            case ELGUILayoutAlignmentType::MiddleCenter:
            case ELGUILayoutAlignmentType::LowerCenter:
            {
                posX = startPosition.X + (rectSize.X - childWidth) * 0.5f;
            }
            break;
            case ELGUILayoutAlignmentType::UpperRight:
            case ELGUILayoutAlignmentType::MiddleRight:
            case ELGUILayoutAlignmentType::LowerRight:
            {
                posX = startPosition.X + rectSize.X - childWidth;
            }
            break;
            }
        }
        float childHeightArea = childrenHeightList[i].AreaHeight;
        float childHeight = childrenHeightList[i].Height;
        if (ExpandChildHeightArea)
        {
            if (ControlChildHeight)
            {
                ApplyHeightWithAnimation(tempAnimationType, childHeight, ChildUIItem.Get());
            }
        }

        auto AnchorMin = ChildUIItem->GetAnchorMin();
        auto AnchorMax = ChildUIItem->GetAnchorMax();
        if (AnchorMin.Y != AnchorMax.Y)//custom anchor not support
        {
            ChildUIItem->SetVerticalAnchorMinMax(FVector2D(1, 1), true, true);
        }
        if (ExpandChildWidthArea || ControlChildWidth)
        {
            if (AnchorMin.X != AnchorMax.X)
            {
                ChildUIItem->SetHorizontalAnchorMinMax(FVector2D(0, 0), true, true);
            }
        }
        float anchorOffsetX = posX + ChildUIItem->GetPivot().X * childWidth;
        float anchorOffsetY = posY - (1.0f - ChildUIItem->GetPivot().Y) * childHeightArea;
        if (ExpandChildHeightArea)
        {
            switch (Align)
            {
            case ELGUILayoutAlignmentType::UpperLeft:
            case ELGUILayoutAlignmentType::UpperCenter:
            case ELGUILayoutAlignmentType::UpperRight:
            {
                anchorOffsetY += (childHeightArea - childHeight) * (1.0f - ChildUIItem->GetPivot().Y);
            }
            break;
            case ELGUILayoutAlignmentType::MiddleLeft:
            case ELGUILayoutAlignmentType::MiddleCenter:
            case ELGUILayoutAlignmentType::MiddleRight:
            {
                anchorOffsetY += (childHeightArea - childHeight) * (0.5f - ChildUIItem->GetPivot().Y);
            }
            break;
            case ELGUILayoutAlignmentType::LowerLeft:
            case ELGUILayoutAlignmentType::LowerCenter:
            case ELGUILayoutAlignmentType::LowerRight:
            {
                anchorOffsetY += (childHeightArea - childHeight) * (0.0f - ChildUIItem->GetPivot().Y);
            }
            break;
            }
        }
        if (ControlChildWidth)
        {
            anchorOffsetX -= AnchorMin.X * RootUIComp->GetWidth();
        }
        else
        {
            anchorOffsetX = ChildUIItem->GetAnchoredPosition().X;
        }
        anchorOffsetY += (1 - AnchorMin.Y) * RootUIComp->GetHeight();
        ApplyAnchoredPositionWithAnimation(tempAnimationType, FVector2D(anchorOffsetX, anchorOffsetY), ChildUIItem.Get());

        posY -= childHeightArea + Spacing;
        tempActuralVerticalRange += childHeightArea;
        if (tempHorizontalMinSize > childWidth)
        {
            tempHorizontalMinSize = childWidth;
        }
        if (tempHorizontalMaxSize < childWidth)
        {
            tempHorizontalMaxSize = childWidth;
        }
    }
    if (childrenCount > 1)
    {
        tempActuralVerticalRange += Spacing * (childrenCount - 1);
    }
    ActuralRange = tempActuralVerticalRange;
    if (HeightFitToChildren && !ExpandChildHeightArea)
    {
        auto thisHeight = tempActuralVerticalRange + Padding.Top + Padding.Bottom;
        ApplyHeightWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
    }
    if (WidthFitToChildren && !ExpandChildWidthArea)
    {
        tempHorizontalMinSize += Padding.Left + Padding.Right;
        tempHorizontalMaxSize += Padding.Left + Padding.Right;
        auto thisWidth = FMath::Lerp(tempHorizontalMinSize, tempHorizontalMaxSize, WidthFitToChildrenFromMinToMax);
        ApplyWidthWithAnimation(tempAnimationType, thisWidth, RootUIComp.Get());
    }

	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		EndSetupAnimations();
	}
}

bool UUIVerticalLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)
    {
        OutResult.bCanControlHorizontalSizeDelta = (!ExpandChildWidthArea && WidthFitToChildren) && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = (!ExpandChildHeightArea && HeightFitToChildren) && this->GetEnable();
        return true;
    }
    else
    {
        if (InUIItem->GetAttachParent() != this->GetRootUIComponent())return false;
        UObject* LayoutElementInterface = nullptr;
        bool bIgnoreLayout = false;
        GetLayoutElement(InUIItem, LayoutElementInterface, bIgnoreLayout);
        if (bIgnoreLayout)
        {
            return true;
        }
        OutResult.bCanControlHorizontalAnchor = this->GetEnable() && (ExpandChildWidthArea || ControlChildWidth);
        OutResult.bCanControlVerticalAnchor = this->GetEnable();
        OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable() && (ExpandChildWidthArea || ControlChildWidth);
        OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlHorizontalSizeDelta = ExpandChildWidthArea && ControlChildWidth && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = ExpandChildHeightArea && ControlChildHeight && this->GetEnable();
        if (auto LayoutElementComp = Cast<UUILayoutElement>(LayoutElementInterface))
        {
            if (LayoutElementComp->GetConstantSizeType() == EUILayoutElement_ConstantSizeType::UseUIItemSize)
            {
                OutResult.bCanControlVerticalSizeDelta = false;
            }
        }
        return true;
    }
}
