// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIHorizontalLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

DECLARE_CYCLE_STAT(TEXT("UILayout HorizontalRebuildLayout"), STAT_HorizontalLayout, STATGROUP_LGUI);

void UUIHorizontalLayout::OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
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

void UUIHorizontalLayout::SetPadding(FMargin value)
{
    if (Padding != value)
    {
        Padding = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetSpacing(float value)
{
    if (Spacing != value)
    {
        Spacing = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetAlign(ELGUILayoutAlignmentType value)
{
    if (Align != value)
    {
        Align = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetExpandChildWidthArea(bool value)
{
    if (ExpandChildWidthArea != value)
    {
        ExpandChildWidthArea = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetExpandChildHeightArea(bool value)
{
    if (ExpandChildHeightArea != value)
    {
        ExpandChildHeightArea = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetWidthFitToChildren(bool value)
{
    if (WidthFitToChildren != value)
    {
        WidthFitToChildren = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetHeightFitToChildren(bool value)
{
    if (HeightFitToChildren != value)
    {
        HeightFitToChildren = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetHeightFitToChildrenFromMinToMax(float value)
{
    if (HeightFitToChildrenFromMinToMax != value)
    {
        HeightFitToChildrenFromMinToMax = value;
        MarkNeedRebuildChildrenList();
    }
}
void UUIHorizontalLayout::SetUseChildScaleOnWidth(bool value)
{
    if (UseChildScaleOnWidth != value)
    {
        UseChildScaleOnWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetControlChildWidth(bool value)
{
    if (ControlChildWidth != value)
    {
        ControlChildWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetControlChildHeight(bool value)
{
    if (ControlChildHeight != value)
    {
        ControlChildHeight = value;
        MarkNeedRebuildLayout();
    }
}


void UUIHorizontalLayout::OnRebuildLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_HorizontalLayout);
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
    float childHeight = rectSize.Y;
    childrenWidthList.SetNumUninitialized(childrenCount);
    if (ExpandChildWidthArea)
    {
        float sizeWithoutSpacing = rectSize.X - Spacing * (childrenCount - 1);
        float autoSizeChildrenTotalWidth = sizeWithoutSpacing; //collect all children's size which is autosize or without layout element
        int autoSizeChildrenCount = 0;
        for (int i = 0; i < childrenCount; i++)
        {
            auto& item = uiChildrenList[i];
            float tempChildWidth = 0;
            if (item.LayoutInterface.IsValid())
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.LayoutInterface.Get());
                switch (layoutType)
                {
                case ELayoutElementType::AutoSize:
                    autoSizeChildrenCount++;
                    break;
                case ELayoutElementType::ConstantSize:
                    tempChildWidth = ILGUILayoutElementInterface::Execute_GetConstantSize(item.LayoutInterface.Get(), ELayoutElementSizeType::Horizontal);
                    break;
                case ELayoutElementType::RatioSize:
                    tempChildWidth = ILGUILayoutElementInterface::Execute_GetRatioSize(item.LayoutInterface.Get(), ELayoutElementSizeType::Horizontal) * sizeWithoutSpacing;
                    break;
                }
            }
            else
            {
                autoSizeChildrenCount++;
            }
            if (UseChildScaleOnWidth)
            {
                tempChildWidth *= item.ChildUIItem->GetRelativeScale3D().Y;
            }
            autoSizeChildrenTotalWidth -= tempChildWidth;
            childrenWidthList[i].AreaWidth = tempChildWidth;
            childrenWidthList[i].Width = tempChildWidth;
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
                autoSizeChildScaleSum += UseChildScaleOnWidth ? item.ChildUIItem->GetRelativeScale3D().Y : 1.0f;
            }
        }

        if (ControlChildWidth)
        {
            float childAverangeWidth = autoSizeChildrenTotalWidth / autoSizeChildScaleSum;
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
                    auto childWidth = childAverangeWidth;
                    childrenWidthList[i].Width = childWidth;
                    if (UseChildScaleOnWidth)
                    {
                        childWidth *= item.ChildUIItem->GetRelativeScale3D().Y;
                    }
                    childrenWidthList[i].AreaWidth = childWidth;
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
                    auto childWidth = item.ChildUIItem->GetWidth();
                    if (UseChildScaleOnWidth)
                    {
                        childWidth *= item.ChildUIItem->GetRelativeScale3D().Y;
                    }
                    autoSizeChildSizeSum += childWidth;
                }
            }

            auto extraChildWidthTotal = autoSizeChildrenTotalWidth - autoSizeChildSizeSum;
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
                    auto childWidth = item.ChildUIItem->GetWidth();
                    float extraChildWidth;
                    if (UseChildScaleOnWidth)
                    {
                        childWidth *= item.ChildUIItem->GetRelativeScale3D().Y;
                        extraChildWidth = extraChildWidthTotal * item.ChildUIItem->GetRelativeScale3D().Y / autoSizeChildScaleSum;
                    }
                    else
                    {
                        extraChildWidth = extraChildWidthTotal / autoSizeChildrenCount;
                    }
                    childrenWidthList[i].AreaWidth = childWidth + extraChildWidth;
                    childrenWidthList[i].Width = childWidth;
                }
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
                auto& ChildUIItem = uiChildrenList[i].ChildUIItem;
                auto itemWidth = ChildUIItem->GetWidth();
                if (UseChildScaleOnWidth)
                {
                    itemWidth *= ChildUIItem->GetRelativeScale3D().Y;
                }
                totalWidth += itemWidth;
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
                auto& ChildUIItem = uiChildrenList[i].ChildUIItem;
                auto itemWidth = ChildUIItem->GetWidth();
                if (UseChildScaleOnWidth)
                {
                    itemWidth *= ChildUIItem->GetRelativeScale3D().Y;
                }
                totalWidth += itemWidth;
            }
            totalWidth += Spacing * (childrenCount - 1);
            startPosition.X += (rectSize.X - totalWidth);
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
                auto childWidth = item.ChildUIItem->GetWidth();
                childrenWidthList[i].Width = childWidth;
                if (UseChildScaleOnWidth)
                {
                    childWidth *= item.ChildUIItem->GetRelativeScale3D().Y;
                }
                childrenWidthList[i].AreaWidth = childWidth;
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
    float tempActuralHorizontalRange = 0;
    float tempVerticalMinSize = MAX_flt;
    float tempVerticalMaxSize = MIN_flt;
    //we need to calculate this's size first, because we need to apply this's size then child's size for less layout calculation
    for (int i = 0; i < childrenCount; i++)
    {
        auto ChildUIItem = uiChildrenList[i].ChildUIItem;
        float childWidthArea = childrenWidthList[i].AreaWidth;
        if (!ExpandChildHeightArea)
        {
            childHeight = ChildUIItem->GetHeight();
        }

        tempActuralHorizontalRange += childWidthArea;
        if (tempVerticalMinSize > childHeight)
        {
            tempVerticalMinSize = childHeight;
        }
        if (tempVerticalMaxSize < childHeight)
        {
            tempVerticalMaxSize = childHeight;
        }
    }

    if (childrenCount > 1)
    {
        tempActuralHorizontalRange += Spacing * (childrenCount - 1);
    }
    ActuralRange = tempActuralHorizontalRange;
    float RootUIItemWidth = RootUIComp->GetWidth();
    float RootUIItemHeight = RootUIComp->GetHeight();
    if (WidthFitToChildren && !ExpandChildWidthArea)
    {
        RootUIItemWidth = tempActuralHorizontalRange + Padding.Left + Padding.Right;
        ApplyWidthWithAnimation(tempAnimationType, RootUIItemWidth, RootUIComp.Get());
    }
    if (HeightFitToChildren && !ExpandChildHeightArea)
    {
        tempVerticalMinSize += Padding.Bottom + Padding.Top;
        tempVerticalMaxSize += Padding.Bottom + Padding.Top;
        RootUIItemHeight = FMath::Lerp(tempVerticalMinSize, tempVerticalMaxSize, HeightFitToChildrenFromMinToMax);
        ApplyHeightWithAnimation(tempAnimationType, RootUIItemHeight, RootUIComp.Get());
    }

    for (int i = 0; i < childrenCount; i++)
    {
        auto ChildUIItem = uiChildrenList[i].ChildUIItem;
        float childWidthArea = childrenWidthList[i].AreaWidth;
        float childWidth = childrenWidthList[i].Width;
        if (ExpandChildWidthArea)
        {
            if (ControlChildWidth)
            {
                ApplyWidthWithAnimation(tempAnimationType, childWidth, ChildUIItem.Get());
            }
        }

        if (ExpandChildHeightArea)
        {
            if (ControlChildHeight)
            {
                ApplyHeightWithAnimation(tempAnimationType, childHeight, ChildUIItem.Get());
            }
        }
        else
        {
            childHeight = ChildUIItem->GetHeight();
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

        auto AnchorMin = ChildUIItem->GetAnchorMin();
        auto AnchorMax = ChildUIItem->GetAnchorMax();
        if (AnchorMin.X != AnchorMax.X)//custom anchor not support
        {
            ChildUIItem->SetHorizontalAnchorMinMax(FVector2D(0, 0), true, true);
        }
        if (ExpandChildHeightArea || ControlChildHeight)
        {
            if (AnchorMin.Y != AnchorMax.Y)
            {
                ChildUIItem->SetVerticalAnchorMinMax(FVector2D(1, 1), true, true);
            }
        }
        float anchorOffsetX = posX + ChildUIItem->GetPivot().X * childWidthArea;
        float anchorOffsetY = posY - (1.0f - ChildUIItem->GetPivot().Y) * childHeight;
        if (ExpandChildWidthArea)
        {
            switch (Align)
            {
            case ELGUILayoutAlignmentType::UpperLeft:
            case ELGUILayoutAlignmentType::MiddleLeft:
            case ELGUILayoutAlignmentType::LowerLeft:
            {
                anchorOffsetX += (childWidthArea - childWidth) * (0.0f - ChildUIItem->GetPivot().X);
            }
            break;
            case ELGUILayoutAlignmentType::UpperCenter:
            case ELGUILayoutAlignmentType::MiddleCenter:
            case ELGUILayoutAlignmentType::LowerCenter:
            {
                anchorOffsetX += (childWidthArea - childWidth) * (0.5f - ChildUIItem->GetPivot().X);
            }
            break;
            case ELGUILayoutAlignmentType::UpperRight:
            case ELGUILayoutAlignmentType::MiddleRight:
            case ELGUILayoutAlignmentType::LowerRight:
            {
                anchorOffsetX += (childWidthArea - childWidth) * (1.0f - ChildUIItem->GetPivot().X);
            }
            break;
            }
        }
        if (ControlChildHeight)
        {
            anchorOffsetY += (1 - AnchorMin.Y) * RootUIItemHeight;
        }
        else
        {
            anchorOffsetY = ChildUIItem->GetAnchoredPosition().Y;
        }
        anchorOffsetX -= AnchorMin.X * RootUIItemWidth;
        ApplyAnchoredPositionWithAnimation(tempAnimationType, FVector2D(anchorOffsetX, anchorOffsetY), ChildUIItem.Get());

        posX += childWidthArea + Spacing;
    }

	if (tempAnimationType == EUILayoutAnimationType::EaseAnimation)
	{
		EndSetupAnimations();
	}
}

bool UUIHorizontalLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
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
        OutResult.bCanControlHorizontalAnchor = this->GetEnable();
        OutResult.bCanControlVerticalAnchor = this->GetEnable() && (ExpandChildHeightArea || ControlChildHeight);
        OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable() && (ExpandChildHeightArea || ControlChildHeight);
        OutResult.bCanControlHorizontalSizeDelta = ExpandChildWidthArea && ControlChildWidth && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = ExpandChildHeightArea && ControlChildHeight && this->GetEnable();
        if (auto LayoutElementComp = Cast<UUILayoutElement>(LayoutElementInterface))
        {
            if (LayoutElementComp->GetConstantSizeType() == EUILayoutElement_ConstantSizeType::UseUIItemSize)
            {
                OutResult.bCanControlHorizontalSizeDelta = false;
            }
        }
        return true;
    }
}
