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
            || (ExpendChildrenWidth && widthChanged)
            || (ExpendChildrenHeight && heightChanged)
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
void UUIHorizontalLayout::SetExpendChildrenWidth(bool value)
{
    if (ExpendChildrenWidth != value)
    {
        ExpendChildrenWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIHorizontalLayout::SetExpendChildrenHeight(bool value)
{
    if (ExpendChildrenHeight != value)
    {
        ExpendChildrenHeight = value;
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
	CancelAnimation();

    FVector2D startPosition;
    startPosition.X = Padding.Left;
    startPosition.Y = -Padding.Top; //left bottom as start point
    FVector2D rectSize;
    rectSize.X = RootUIComp->GetWidth() - Padding.Left - Padding.Right;
    rectSize.Y = RootUIComp->GetHeight() - Padding.Top - Padding.Bottom;

    const auto& uiChildrenList = GetLayoutUIItemChildren();
    int childrenCount = uiChildrenList.Num();
    float childHeight = rectSize.Y;
    childrenWidthList.Reset(childrenCount);
    if (ExpendChildrenWidth)
    {
        float sizeWithoutSpacing = rectSize.X - Spacing * (childrenCount - 1);
        float autoSizeChildrenSize = sizeWithoutSpacing; //collect all children's size which is autosize or without layout element
        int autoSizeChildrenCount = 0;
        for (auto item : uiChildrenList)
        {
            float tempChildWidth = 0;
            if (item.layoutElement.IsValid())
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.layoutElement.Get());
                switch (layoutType)
                {
                case ELayoutElementType::AutoSize:
                    autoSizeChildrenCount++;
                    break;
                case ELayoutElementType::ConstantSize:
                    tempChildWidth = ILGUILayoutElementInterface::Execute_GetConstantSize(item.layoutElement.Get(), ELayoutElementSizeType::Horizontal);
                    break;
                case ELayoutElementType::RatioSize:
                    tempChildWidth = ILGUILayoutElementInterface::Execute_GetRatioSize(item.layoutElement.Get(), ELayoutElementSizeType::Horizontal) * sizeWithoutSpacing;
                    break;
                }
            }
            else
            {
                autoSizeChildrenCount++;
            }
            if (UseChildrenScaleOnWidth)
            {
                tempChildWidth *= item.uiItem->GetRelativeScale3D().Y;
            }
            autoSizeChildrenSize -= tempChildWidth;
            childrenWidthList.Add(tempChildWidth);
        }
        float autoSizeChildWidth = 0;
        if (UseChildrenScaleOnWidth)
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
                        autoSizeChildrenScaleSum += item.uiItem->GetRelativeScale3D().Y;
                    }
                }
                else
                {
                    autoSizeChildrenScaleSum += item.uiItem->GetRelativeScale3D().Y;
                }
            }
            autoSizeChildWidth = autoSizeChildrenSize / autoSizeChildrenScaleSum;
        }
        else
        {
            autoSizeChildWidth = autoSizeChildrenSize / autoSizeChildrenCount;
        }
        for (int i = 0; i < childrenCount; i++)
        {
            auto item = uiChildrenList[i];
            if (item.layoutElement != nullptr)
            {
                auto layoutType = ILGUILayoutElementInterface::Execute_GetLayoutType(item.layoutElement.Get());
                if (layoutType == ELayoutElementType::AutoSize)
                {
                    childrenWidthList[i] = autoSizeChildWidth;
                }
            }
            else
            {
                childrenWidthList[i] = autoSizeChildWidth;
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
                auto uiItem = uiChildrenList[i].uiItem;
                auto itemWidth = uiItem->GetWidth();
                if (UseChildrenScaleOnWidth)
                {
                    itemWidth *= uiItem->GetRelativeScale3D().Y;
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
                auto uiItem = uiChildrenList[i].uiItem;
                auto itemWidth = uiItem->GetWidth();
                if (UseChildrenScaleOnWidth)
                {
                    itemWidth *= uiItem->GetRelativeScale3D().Y;
                }
                totalWidth += itemWidth;
            }
            totalWidth += Spacing * (childrenCount - 1);
            startPosition.X += (rectSize.X - totalWidth);
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
    float tempActuralHorizontalRange = 0;
    float tempVerticalMinSize = MAX_flt;
    float tempVerticalMaxSize = MIN_flt;
    for (int i = 0; i < childrenCount; i++)
    {
        auto uiItem = uiChildrenList[i].uiItem;
        float childWidth;
        if (ExpendChildrenWidth)
        {
            childWidth = childrenWidthList[i];
            ApplyWidthWithAnimation(tempAnimationType, childWidth, uiItem.Get());
        }
        else
        {
            childWidth = uiItem->GetWidth();
        }
        if (UseChildrenScaleOnWidth)
        {
            childWidth *= uiItem->GetRelativeScale3D().Y;
        }

        if (ExpendChildrenHeight)
        {
            ApplyHeightWithAnimation(tempAnimationType, childHeight, uiItem.Get());
        }
        else
        {
            childHeight = uiItem->GetHeight();
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

        posX += childWidth + Spacing;
        tempActuralHorizontalRange += childWidth;
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
    if (WidthFitToChildren && !ExpendChildrenWidth)
    {
        auto thisWidth = tempActuralHorizontalRange + Padding.Left + Padding.Right;
        ApplyWidthWithAnimation(tempAnimationType, thisWidth, RootUIComp.Get());
    }
    if (HeightFitToChildren && !ExpendChildrenHeight)
    {
        tempVerticalMinSize += Padding.Bottom + Padding.Top;
        tempVerticalMaxSize += Padding.Bottom + Padding.Top;
        auto thisHeight = FMath::Lerp(tempVerticalMinSize, tempVerticalMaxSize, HeightFitToChildrenFromMinToMax);
        ApplyHeightWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
    }
	if (tempAnimationType == EUILayoutChangePositionAnimationType::EaseAnimation)
	{
		SetOnCompleteTween();
	}
}

bool UUIHorizontalLayout::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)
    {
        OutResult.bCanControlHorizontalSizeDelta = (!GetExpendChildrenWidth() && GetWidthFitToChildren()) && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = (!GetExpendChildrenHeight() && GetHeightFitToChildren()) && this->GetEnable();
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
        OutResult.bCanControlHorizontalSizeDelta = GetExpendChildrenWidth() && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = GetExpendChildrenHeight() && this->GetEnable();
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
