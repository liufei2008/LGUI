// Copyright 2025-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexLayoutSelfFlexBox.h"
#include "LGUI.h"
#include "Core/Components/LexLayoutContainerFlexBox.h"
#include "Core/Components/LexVisual.h"

float FLexLayoutSize::Calculate(ULexWidget* Widget, bool IsVertical) const
{
    if (bEnable)
    {
        switch (Type)
        {
        case ELexLayoutSizeType::Auto:
            if (auto LayoutContainer = Widget->GetLayoutContainer())
            {
                auto LayoutPreferredSize = LayoutContainer->GetLayoutPreferredSize();
                return IsVertical ? LayoutPreferredSize.Y : LayoutPreferredSize.X;
            }
            if (auto Visual = Widget->GetVisual())
            {
                return IsVertical ? Visual->GetPreferredHeight() : Visual->GetPreferredWidth();
            }
            return 0;
        case ELexLayoutSizeType::Fixed:
            return FixedValue;
        case ELexLayoutSizeType::Percent:
            if (auto ParentWidget = Widget->GetParent())
            {
                if (IsVertical)
                {
                    float FinalSize = ParentWidget->GetHeight();
                    if (auto ParentLayoutContainer = Cast<ULexLayoutContainerFlexBox>(ParentWidget->GetLayoutContainer()))
                    {
                        auto& Padding = ParentLayoutContainer->GetPadding();
                        FinalSize -= Padding.Bottom + Padding.Top;
                    }
                    return PercentValue * FinalSize;
                }
                else
                {
                    float FinalSize = ParentWidget->GetWidth();
                    if (auto ParentLayoutContainer = Cast<ULexLayoutContainerFlexBox>(ParentWidget->GetLayoutContainer()))
                    {
                        auto& Padding = ParentLayoutContainer->GetPadding();
                        FinalSize -= Padding.Left + Padding.Right;
                    }
                    return PercentValue * FinalSize;
                }
            }
            return 0;//no valid parent, just return 0
        }
    }
    return IsVertical ? Widget->GetHeight() : Widget->GetWidth();
}

float FLexLayoutMinMaxSize::Calculate(ULexWidget* Widget, bool IsVertical,
    bool IsMinOrMax) const
{
    float CalculatedValue = IsMinOrMax ? -UE_MAX_FLT : UE_MAX_FLT;
    if (bEnable)
    {
        switch (Type)
        {
        case ELexLayoutMinMaxSizeType::Fixed:
            CalculatedValue = FixedValue;
            break;
        case ELexLayoutMinMaxSizeType::Percent:
            if (auto ParentWidget = Widget->GetParent())
            {
                if (IsVertical)
                {
                    return PercentValue * ParentWidget->GetHeight();
                }
                else
                {
                    return PercentValue * ParentWidget->GetWidth();
                }
            }
            else
            {
                return CalculatedValue;
            }
        }
    }
    return CalculatedValue;
}

FName ULexLayoutSelfFlexBox::GetPropertyName_PreferredWidth()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, PreferredWidth);
}
FName ULexLayoutSelfFlexBox::GetPropertyName_PreferredHeight()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, PreferredHeight);
}
FName ULexLayoutSelfFlexBox::GetPropertyName_MinWidth()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, MinWidth);
}
FName ULexLayoutSelfFlexBox::GetPropertyName_MinHeight()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, MinHeight);
}
FName ULexLayoutSelfFlexBox::GetPropertyName_MaxWidth()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, MaxWidth);
}
FName ULexLayoutSelfFlexBox::GetPropertyName_MaxHeight()
{
    return GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, MaxHeight);
}

#if WITH_EDITOR
void ULexLayoutSelfFlexBox::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
    Grow = FMath::Max(Grow, 0);
    Shrink = FMath::Max(Shrink, 0);
    RebuildSelfLayout();
}

bool ULexLayoutSelfFlexBox::CanEditChange(const FProperty* InProperty) const
{
    bool bCanEditChange = Super::CanEditChange(InProperty);
    if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ULexLayoutSelfFlexBox, Margin))
    {
        if (auto Widget = GetWidget())
        {
            if (auto ParentWidget = Widget->GetParent())
            {
                if (auto LayoutContainer = Cast<ULexLayoutContainerFlexBox>(ParentWidget->GetLayoutContainer()))
                {
                    bCanEditChange = true;
                }
                else
                {
                    bCanEditChange = false;
                }
            }
        }
    }
    return bCanEditChange;
}

void ULexLayoutSelfFlexBox::PostInitProperties()
{
    Super::PostInitProperties();
}
#endif

FLexLayoutControlAnchorData ULexLayoutSelfFlexBox::GetLayoutControlAnchor(const ULexWidget* TargetWidget) const
{
    FLexLayoutControlAnchorData Result;
    auto ThisWidget = GetWidget();
    if (ThisWidget == TargetWidget)//self
    {
        if (PreferredWidth.bEnable)
        {
            Result.bCanControlHorizontalSize = true;
        }
        if (PreferredHeight.bEnable)
        {
            Result.bCanControlVerticalSize = true;
        }
    }
    return Result;
}

FVector2f ULexLayoutSelfFlexBox::GetLayoutPreferredSize()
{
    CalculateSize();
    return FVector2f(CalculatedPreferredWidth, CalculatedPreferredHeight);
}

void ULexLayoutSelfFlexBox::GetLayoutMinMax(FVector2f& OutMin, FVector2f& OutMax)
{
    OutMin.X = CalculatedMinWidth;
    OutMin.Y = CalculatedMinHeight;
    OutMax.X = CalculatedMaxWidth;
    OutMax.Y = CalculatedMaxHeight;
}

void ULexLayoutSelfFlexBox::RebuildSelfLayout()
{
    if (auto Widget = GetWidget())
    {
        if (!Widget->GetIgnoreLayout())
        {
            if (auto ParentWidget = Widget->GetParent())
            {
                if (ParentWidget->GetLayoutContainer())//if parent has layoutContainer, then mark it for late rebuild layout
                {
                    ULexWidget::MarkLayoutForRebuild(Widget);
                    return;
                }
            }
            if (auto LayoutContainer = Widget->GetLayoutContainer())
            {
                if (LayoutContainer->GetUseAnimation())//if use animation, then this size will be set by animation
                    return;
            }
        }
        bIsLayoutDirty = true;
        CalculateSize();//build layout and apply immediately
    }
}

void ULexLayoutSelfFlexBox::CalculateSize()
{
    if (!bIsLayoutDirty)return;
    bIsLayoutDirty = false;

    auto Widget = GetWidget();
    if (!Widget)return;

    {
        CalculatedPreferredWidth = PreferredWidth.Calculate(Widget, false);
        if (PreferredWidth.bEnable)
        {
            CalculatedMinWidth = MinWidth.Calculate(Widget, false, true);
        }
        else//if not enable width control then we also don't use minWidth, because layoutContainer could use minWidth to calculate and set the wrong width
        {
            CalculatedMinWidth = -UE_MAX_FLT;
        }
        CalculatedPreferredHeight = PreferredHeight.Calculate(Widget, true);
        if (PreferredHeight.bEnable)
        {
            CalculatedMinHeight = MinHeight.Calculate(Widget, true, true);
        }
        else//if not enable height control then we also don't use minHeight, because layoutContainer could use minHeight to calculate and set the wrong height
        {
            CalculatedMinHeight = -UE_MAX_FLT;
        }
        CalculatedMaxWidth = MaxWidth.Calculate(Widget, false, false);
        if (CalculatedMaxWidth < CalculatedMinWidth)
        {
            CalculatedMaxWidth = UE_MAX_FLT;
        }
        CalculatedMaxHeight = MaxHeight.Calculate(Widget, true, false);
        if (CalculatedMaxHeight < CalculatedMinHeight)
        {
            CalculatedMaxHeight = UE_MAX_FLT;
        }
        //clamp value
        CalculatedPreferredWidth = FMath::Clamp(CalculatedPreferredWidth, CalculatedMinWidth, CalculatedMaxWidth);
        CalculatedPreferredHeight = FMath::Clamp(CalculatedPreferredHeight, CalculatedMinHeight, CalculatedMaxHeight);
    }

    bool bShouldSetPreferredSize = true;
    if (auto ParentWidget = Widget->GetParent())
    {
        //if parent widget have FlexBoxContainer, then widget size should be set by it, because Grow/Shrink/Stretch is calculated by FlexBoxContainer
        if (Cast<ULexLayoutContainerFlexBox>(ParentWidget->GetLayoutContainer()))
        {
            bShouldSetPreferredSize = false;
        }
    }
    if (bShouldSetPreferredSize)
    {
        auto AnchorMin = Widget->GetAnchorMin();
        auto AnchorMax = Widget->GetAnchorMax();
        if (PreferredWidth.bEnable && AnchorMin.X != AnchorMax.X)//custom anchor not support
        {
            Widget->SetHorizontalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
        }
        if (PreferredHeight.bEnable && AnchorMin.Y != AnchorMax.Y)
        {
            Widget->SetVerticalAnchorMinMax(FVector2D(0.5, 0.5), true, true);
        }
        Widget->SetSize(FVector2D(CalculatedPreferredWidth, CalculatedPreferredHeight));

#if WITH_EDITOR
        if (PreferredWidth.Type == ELexLayoutSizeType::Auto)
        {
            PreferredWidth.AutoValue = CalculatedPreferredWidth;
        }
        if (PreferredHeight.Type == ELexLayoutSizeType::Auto)
        {
            PreferredHeight.AutoValue = CalculatedPreferredHeight;
        }
#endif
    }
}

void ULexLayoutSelfFlexBox::MarkLayoutDirty()
{
    Super::MarkLayoutDirty();
    if (auto Widget = GetWidget())
    {
        if (!Widget->GetIgnoreLayout())
        {
            if (auto ParentWidget = Widget->GetParent())
            {
                if (ParentWidget->GetLayoutContainer())//if parent has layoutContainer, then LexWidget will do MarkLayoutForRebuild for it, so skip it here
                {
                    return;
                }
            }
            if (auto LayoutContainer = Widget->GetLayoutContainer())
            {
                if (LayoutContainer->GetUseAnimation())//if use animation, then this size will be set by animation
                    return;
            }
        }
        CalculateSize();//build layout and apply immediately
    }
}

float ULexLayoutSelfFlexBox::GetGrowForLayoutContainer(int Axis) const
{
    //If width not enable then it is danger to use Grow.
    //Because width is get from widget, after increase width by Grow the result will set to widget too, that will make width keep increasing.
    //Same problem for height.
    if (Axis == 0 && PreferredWidth.bEnable)
    {
        return Grow;
    }
    if (Axis == 1 && PreferredHeight.bEnable)
    {
        return Grow;
    }
    return 0;
}

float ULexLayoutSelfFlexBox::GetShrinkForLayoutContainer(int Axis) const
{
    //If width not enable then it is danger to use shrink.
    //Because width is get from widget, after decrease width by Shrink the result will set to widget too, that will make width keep decreasing.
    //Same problem for height.
    if (Axis == 0 && PreferredWidth.bEnable)
    {
        return Shrink;
    }
    if (Axis == 1 && PreferredHeight.bEnable)
    {
        return Shrink;
    }
    return 0;
}

ELexLayoutFlexBoxSecondaryAxisLineAlignment ULexLayoutSelfFlexBox::GetAlignmentForLayoutContainer(ELexLayoutFlexBoxSecondaryAxisLineAlignment DefaultAlignment) const
{
    if (SelfAlignment == ELexLayoutFlexBoxSecondaryAxisSelfAlignment::Auto)
    {
        return DefaultAlignment;
    }
    else
    {
        return static_cast<ELexLayoutFlexBoxSecondaryAxisLineAlignment>(static_cast<int>(SelfAlignment) - 1);
    }
}

bool ULexLayoutSelfFlexBox::GetSecondaryAxisSizeCanStretchByLayoutContainer(int SecondaryAxis) const
{
    if (SecondaryAxis == 0)
    {
        if (PreferredWidth.bEnable)
        {
            return true;
        }
    }
    else
    {
        if (PreferredHeight.bEnable)
        {
            return true;
        }
    }
    return false;
}

void ULexLayoutSelfFlexBox::SetFinalSizeByLayoutContainer(FVector2f Value)
{
    this->CalculatedFinalWidth = Value.X;
    this->CalculatedFinalHeight = Value.Y;

#if WITH_EDITOR
    if (PreferredWidth.Type == ELexLayoutSizeType::Auto)
    {
        PreferredWidth.AutoValue = Value.X;
    }
    if (PreferredHeight.Type == ELexLayoutSizeType::Auto)
    {
        PreferredHeight.AutoValue = Value.Y;
    }
#endif
}

void ULexLayoutSelfFlexBox::SetMinWidth(const FLexLayoutMinMaxSize& Value)
{
    if (MinWidth != Value)
    {
        MinWidth = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetMinHeight(const FLexLayoutMinMaxSize& Value)
{
    if (MinHeight != Value)
    {
        MinHeight = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetMaxWidth(const FLexLayoutMinMaxSize& Value)
{
    if (MaxWidth != Value)
    {
        MaxWidth = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetMaxHeight(const FLexLayoutMinMaxSize& Value)
{
    if (MaxHeight != Value)
    {
        MaxHeight = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetMargin(const FMargin& Value)
{
    if (Margin != Value)
    {
        Margin = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetPreferredWidth(const FLexLayoutSize& Value)
{
    if (PreferredWidth != Value)
    {
        PreferredWidth = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetPreferredHeight(const FLexLayoutSize& Value)
{
    if (PreferredHeight != Value)
    {
        PreferredHeight = Value;
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetGrow(float Value)
{
    if (Grow != Value)
    {
        Grow = FMath::Max(0, Value);
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetShrink(float Value)
{
    if (Shrink != Value)
    {
        Shrink = FMath::Max(0, Value);
        RebuildSelfLayout();
    }
}

void ULexLayoutSelfFlexBox::SetSelfAlignment(ELexLayoutFlexBoxSecondaryAxisSelfAlignment Value)
{
    if (SelfAlignment != Value)
    {
        SelfAlignment = Value;
        RebuildSelfLayout();
    }
}

