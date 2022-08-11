// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UIVerticalLayout.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Layout/ILGUILayoutElementInterface.h"

DECLARE_CYCLE_STAT(TEXT("UILayout VerticalRebuildLayout"), STAT_VerticalLayout, STATGROUP_LGUI);

void UUIVerticalLayout::OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
    Super::OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
    if (child->GetIsUIActiveInHierarchy())
    {
        if (this->GetWorld() == nullptr)return;
        if (!ExpendChildrenHeight)
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
void UUIVerticalLayout::SetExpendChildrenWidth(bool value)
{
    if (ExpendChildrenWidth != value)
    {
        ExpendChildrenWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUIVerticalLayout::SetExpendChildrenHeight(bool value)
{
    if (ExpendChildrenHeight != value)
    {
        ExpendChildrenHeight = value;
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
    float tempActuralRange = 0;
    if (ExpendChildrenHeight)
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
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetConstantSize(item.layoutElement.Get());
                    break;
                case ELayoutElementType::RatioSize:
                    tempChildHeight = ILGUILayoutElementInterface::Execute_GetRatioSize(item.layoutElement.Get()) * sizeWithoutSpacing;
                    break;
                }
            }
            else
            {
                autoSizeChildrenCount++;
            }
            autoSizeChildrenSize -= tempChildHeight;
            childrenHeightList.Add(tempChildHeight);
        }
        float autoSizeChildHeight = autoSizeChildrenSize / autoSizeChildrenCount;
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
                totalHeight += uiItem->GetHeight();
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
                totalHeight += uiItem->GetHeight();
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
    for (int i = 0; i < childrenCount; i++)
    {
        auto uiItem = uiChildrenList[i].uiItem;
        uiItem->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0, 1), FVector2D(0, 1), true, true);
        if (ExpendChildrenWidth)
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
        if (ExpendChildrenHeight)
        {
            childHeight = childrenHeightList[i];
            ApplyHeightWithAnimation(tempAnimationType, childHeight, uiItem.Get());
        }
        else
        {
            childHeight = uiItem->GetHeight();
        }

        float anchorOffsetX = posX + uiItem->GetPivot().X * childWidth;
        float anchorOffsetY = posY - (1.0f - uiItem->GetPivot().Y) * childHeight;
        ApplyAnchoredPositionWithAnimation(tempAnimationType, FVector2D(anchorOffsetX, anchorOffsetY), uiItem.Get());

        posY -= childHeight + Spacing;
        tempActuralRange += childHeight;
    }
    if (childrenCount > 1)
    {
        tempActuralRange += Spacing * (childrenCount - 1);
    }
    if (HeightFitToChildren && (ExpendChildrenHeight == false))
    {
        auto thisHeight = tempActuralRange + Padding.Top + Padding.Bottom;
        ActuralRange = tempActuralRange;
        ApplyHeightWithAnimation(tempAnimationType, thisHeight, RootUIComp.Get());
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
        OutResult.bCanControlVerticalSizeDelta = (!GetExpendChildrenHeight() && GetHeightFitToChildren()) && this->GetEnable();
        return true;
    }
    else
    {
        if (InUIItem->GetAttachParent() != this->GetRootUIComponent())return false;
        UActorComponent* layoutElement = nullptr;
        bool ignoreLayout = false;
        GetLayoutElement(InUIItem->GetOwner(), layoutElement, ignoreLayout);
        if (ignoreLayout)
        {
            return true;
        }
        OutResult.bCanControlHorizontalAnchor = this->GetEnable();
        OutResult.bCanControlVerticalAnchor = this->GetEnable();
        OutResult.bCanControlHorizontalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlVerticalAnchoredPosition = this->GetEnable();
        OutResult.bCanControlHorizontalSizeDelta = GetExpendChildrenWidth() && this->GetEnable();
        OutResult.bCanControlVerticalSizeDelta = GetExpendChildrenHeight() && this->GetEnable();
        return true;
    }
}
