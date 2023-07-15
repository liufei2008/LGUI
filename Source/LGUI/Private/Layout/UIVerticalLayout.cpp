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
            || (ExpandChildrenWidth && widthChanged)
            || (ExpandChildrenHeight && heightChanged)
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
void UUIVerticalLayout::SetExpandChildrenWidth(bool value)
{
    if (ExpandChildrenWidth != value)
    {
        ExpandChildrenWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetExpandChildrenHeight(bool value)
{
    if (ExpandChildrenHeight != value)
    {
        ExpandChildrenHeight = value;
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
	CancelAnimation();

    FVector2D startPosition;
    startPosition.X = Padding.Left;
    startPosition.Y = -Padding.Top; //left bottom as start point
    FVector2D rectSize;
    rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
    rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

    const auto& uiChildrenList = GetLayoutUIItemChildren();
    int childrenCount = uiChildrenList.Num();
    float childWidth = rectSize.X;
    childrenHeightList.Reset(childrenCount);
    if (ExpandChildrenHeight)
    {
        float sizeWithoutSpacing = rectSize.Y - Spacing * (childrenCount - 1);
        float autoSizeChildrenSize = sizeWithoutSpacing; //collect all children's size which is autosize or without layout element
        int autoSizeChildrenCount = 0;
        for (auto item : uiChildrenList)
        {
            float tempChildHeight = 0;
            if (item.layoutElement != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.layoutElement.Get());
                switch (layoutType)
                {
                case ELayoutElementType::AutoSize:
                    autoSizeChildrenCount++;
                    break;
                case ELayoutElementType::ConstantSize:
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetConstantSize(item.layoutElement.Get(), ELayoutElementSizeType::Vertical);
                    break;
                case ELayoutElementType::RatioSize:
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetRatioSize(item.layoutElement.Get(), ELayoutElementSizeType::Vertical) * sizeWithoutSpacing;
                    break;
                }
            }
            else
            {
                autoSizeChildrenCount++;
            }
            if (UseChildrenScaleOnHeight)
            {
                tempChildHeight *= item.uiItem->GetRelativeScale3D().Z;
            }
            autoSizeChildrenSize -= tempChildHeight;
            childrenHeightList.Add(tempChildHeight);
        }
        float autoSizeChildHeight = 0;
        if (UseChildrenScaleOnHeight)
        {
            float autoSizeChildrenScaleSum = 0;
            for (int i = 0; i < childrenCount; i++)
            {
                auto item = uiChildrenList[i];
                if (item.layoutElement != nullptr)
                {
                    auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.layoutElement.Get());
                    if (layoutType == ELayoutElementType::AutoSize)
                    {
                        autoSizeChildrenScaleSum += item.uiItem->GetRelativeScale3D().Z;
                    }
                }
                else
                {
                    autoSizeChildrenScaleSum += item.uiItem->GetRelativeScale3D().Z;
                }
            }
            autoSizeChildHeight = autoSizeChildrenSize / autoSizeChildrenScaleSum;
        }
        else
        {
            autoSizeChildHeight = autoSizeChildrenSize / autoSizeChildrenCount;
        }
        for (int i = 0; i < childrenCount; i++)
        {
            auto item = uiChildrenList[i];
            if (item.layoutElement != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.layoutElement.Get());
                if (layoutType == ELayoutElementType::AutoSize)
                {
                    childrenHeightList[i] = autoSizeChildHeight;
                }
            }
            else
            {
                childrenHeightList[i] = autoSizeChildHeight;
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
                auto uiItem = uiChildrenList[i].uiItem;
                auto itemHeight = uiItem->GetHeight();
                if (UseChildrenScaleOnHeight)
                {
                    itemHeight *= uiItem->GetRelativeScale3D().Z;
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
                auto uiItem = uiChildrenList[i].uiItem;
                auto itemHeight = uiItem->GetHeight();
                if (UseChildrenScaleOnHeight)
                {
                    itemHeight *= uiItem->GetRelativeScale3D().Z;
                }
                totalHeight += itemHeight;
            }
            totalHeight += Spacing * (childrenCount - 1);
            startPosition.Y -= (rectSize.Y - totalHeight);
        }
        break;
        }
    }

	EUILayoutChangePositionAnimationType tempAnimationType = AnimationType;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		tempAnimationType = EUILayoutChangePositionAnimationType::Immediately;
	}
#endif
    float posX = startPosition.X, posY = startPosition.Y;
    float tempActuralVerticalRange = 0;
    float tempVerticalMinSize = MAX_flt;
    float tempVerticalMaxSize = MIN_flt;
    for (int i = 0; i < childrenCount; i++)
    {
        auto uiItem = uiChildrenList[i].uiItem;
        if (ExpandChildrenWidth)
        {
            ApplyWidthWithAnimation(tempAnimationType, childWidth, uiItem.Get());
        }
        else
        {
            childWidth = uiItem->GetWidth();
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
        float childHeight;
        if (ExpandChildrenHeight)
        {
            childHeight = childrenHeightList[i];
            ApplyHeightWithAnimation(tempAnimationType, childHeight, uiItem.Get());
        }
        else
        {
            childHeight = uiItem->GetHeight();
        }
        if (UseChildrenScaleOnHeight)
        {
            childHeight *= uiItem->GetRelativeScale3D().Z;
        }

        float anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
        float anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
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

        posY -= childHeight + Spacing;
        tempActuralVerticalRange += childHeight;
        if (tempVerticalMinSize > childWidth)
        {
            tempVerticalMinSize = childWidth;
        }
        if (tempVerticalMaxSize < childWidth)
        {
            tempVerticalMaxSize = childWidth;
        }
    }
    if (childrenCount > 1)
    {
        tempActuralVerticalRange += Spacing * (childrenCount - 1);
    }
    ActuralRange = tempActuralVerticalRange;
    if (HeightFitToChildren && !ExpandChildrenHeight)
    {
        auto thisHeight = tempActuralVerticalRange + Padding.Top + Padding.Bottom;
        ApplyHeightWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
    }
    if (WidthFitToChildren && !ExpandChildrenWidth)
    {
        tempVerticalMinSize += Padding.Top + Padding.Bottom;
        tempVerticalMaxSize += Padding.Top + Padding.Bottom;
        auto thisHeight = FMath::Lerp(tempVerticalMinSize, tempVerticalMaxSize, WidthFitToChildrenFromMinToMax);
        ApplyWidthWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
    }

	if (tempAnimationType == EUILayoutChangePositionAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUIVerticalLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)
    {
        OutResult.bCanControlVerticalSizeDelta = (!GetExpandChildrenHeight() && GetHeightFitToChildren()) && this->GetEnable();
        return true;
    }
    else
    {
        if (InUIItem->GetAttachParent() != this->GetRootUIComponent())return false;
        UActorComponent* LayoutElementInterface = nullptr;
        bool bIgnoreLayout = false;
        GetLayoutElement(InUIItem->GetOwner(), LayoutElementInterface, bIgnoreLayout);
        if (bIgnoreLayout)
        {
            return true;
        }
        OutResult.bCanControlHorizontalAnchor = this->GetEnable();
        OutResult.bCanControlVerticalAnchor = this->GetEnable();
        OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlHorizontalSizeDelta = GetExpandChildrenWidth() && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = GetExpandChildrenHeight() && this->GetEnable();
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
