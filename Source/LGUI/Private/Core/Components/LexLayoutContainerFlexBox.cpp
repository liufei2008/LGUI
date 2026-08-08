// Copyright 2025-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexLayoutContainerFlexBox.h"
#include "Core/Components/LexLayoutSelfFlexBox.h"
#include "Core/Components/LexWidget.h"
#include "LGUI.h"
#include "Core/LexUIManager.h"

DECLARE_CYCLE_STAT(TEXT("LexLayoutContainer FlexBox"), STAT_LexLayoutContainerFlexBox, STATGROUP_LGUI);

void ULexLayoutContainerFlexBox::CalculateLayout()
{
    SCOPE_CYCLE_COUNTER(STAT_LexLayoutContainerFlexBox);

    if (!bIsLayoutDirty)return;
    bIsLayoutDirty = false;
    
#if LEXUI_LAYOUT_DEBUG
    if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
    {
        LexUIManager->IncreateLayoutCalculationCounter(FString::Printf(TEXT("%s_%d"), *this->GetPathDisplayName(GetWorld()), this));
    }
#endif
    
    DoCalculate(true);
}

void ULexLayoutContainerFlexBox::DoCalculate(bool bApplyResult)
{
    bool bIsVertical = Direction == ELexLayoutFlexBoxDirectionType::Vertical || Direction == ELexLayoutFlexBoxDirectionType::VerticalReverse;
    int PrimaryAxis = bIsVertical ? 1 : 0;
    int SecondaryAxis = bIsVertical ? 0 : 1;
    FVector2f ThisWidgetSize;

    struct FChildSizes
    {
        FVector2f Min, Max, Preferred, Grow, Shrink;
    };
    TMap<ULexWidget*, FChildSizes> MapWidgetToChildSizes;
    auto GetChildSizes = [&](ULexWidget* ChildWidget)
    {
        auto ChildSizePtr = MapWidgetToChildSizes.Find(ChildWidget);
        if (!ChildSizePtr)
        {
            FChildSizes Value;
            if (auto ChildLayoutSelf = Cast<ULexLayoutSelfFlexBox>(ChildWidget->GetLayoutSelf()))
            {
                auto MarginSize = ChildLayoutSelf->GetMargin().GetDesiredSize2f();
                Value.Preferred = ChildLayoutSelf->GetLayoutPreferredSize();
                ChildLayoutSelf->GetLayoutMinMax(Value.Min, Value.Max);
                Value.Preferred += MarginSize;
                Value.Min += MarginSize;
                Value.Max += MarginSize;
                if (PrimaryAxis == 0)
                {
                    Value.Grow = FVector2f(ChildLayoutSelf->GetGrowForLayoutContainer(0), 0);
                    Value.Shrink = FVector2f(ChildLayoutSelf->GetShrinkForLayoutContainer(0), 0);
                }
                else
                {
                    Value.Grow = FVector2f(0, ChildLayoutSelf->GetGrowForLayoutContainer(1));
                    Value.Shrink = FVector2f(0, ChildLayoutSelf->GetShrinkForLayoutContainer(1));
                }
            }
            else// child don't have LayoutSelf, then it's size will not be controlled by layout, then just use size delta
            {
                Value.Min = Value.Max = Value.Preferred = FVector2f(ChildWidget->GetSizeDelta());
                Value.Grow = Value.Shrink = FVector2f(0, 0);
            }
            ChildSizePtr = &MapWidgetToChildSizes.Add(ChildWidget, Value);
        }
        return ChildSizePtr;
    };
    auto SetChildPositionAndSize = [&](ULexWidget* ChildWidget, FVector2f Pos, FVector2f AreaSize, float SecondaryPreferred, bool ReverseX, bool ReverseY)
    {
        auto ChildLayoutSelf = Cast<ULexLayoutSelfFlexBox>(ChildWidget->GetLayoutSelf());

        auto ChildAlignment = ChildLayoutSelf ? ChildLayoutSelf->GetAlignmentForLayoutContainer(SecondaryLineAlignment) : SecondaryLineAlignment;
        if (ChildAlignment == ELexLayoutFlexBoxSecondaryAxisLineAlignment::Stretch)//stretch use full area size as widget size
        {
            if (ChildLayoutSelf && ChildLayoutSelf->GetSecondaryAxisSizeCanStretchByLayoutContainer(SecondaryAxis))
            {
                SecondaryPreferred = AreaSize[SecondaryAxis];
            }
        }
        float AlignmentOnAxis = 0;
        switch (ChildAlignment)
        {
        case ELexLayoutFlexBoxSecondaryAxisLineAlignment::Stretch://stretch
        case ELexLayoutFlexBoxSecondaryAxisLineAlignment::Start:
            AlignmentOnAxis = -0.5f;
            break;
        case ELexLayoutFlexBoxSecondaryAxisLineAlignment::Center:
            AlignmentOnAxis = 0.0f;
            break;
        case ELexLayoutFlexBoxSecondaryAxisLineAlignment::End:
            AlignmentOnAxis = 0.5f;
            break;
        }
        FVector2f OffsetInCell;
        OffsetInCell[SecondaryAxis] = (AreaSize[SecondaryAxis] - SecondaryPreferred) * AlignmentOnAxis;

        if (ReverseX)
        {
            Pos.X -= AreaSize.X;
        }
        if (ReverseY)
        {
            Pos.Y -= AreaSize.Y;
        }
        Pos[SecondaryAxis] += OffsetInCell[SecondaryAxis];

        float AnchoredPositionX = Pos[0] + AreaSize[0] * ChildWidget->GetPivot()[0];
        float AnchoredPositionY = -Pos[1] - AreaSize[1] * (1.0f - ChildWidget->GetPivot()[1]);
        auto AnchorMin = ChildWidget->GetAnchorMin();
        AnchoredPositionX += -AnchorMin.X * ThisWidgetSize.X;
        AnchoredPositionY += (1 - AnchorMin.Y) * ThisWidgetSize.Y;
        
        if (ChildLayoutSelf)
        {
            if (ChildAlignment != ELexLayoutFlexBoxSecondaryAxisLineAlignment::Stretch
                || !ChildLayoutSelf->GetSecondaryAxisSizeCanStretchByLayoutContainer(SecondaryAxis))
            {
                AreaSize[SecondaryAxis] = SecondaryPreferred;//not stretch mean we will not change it's secondary-axis size, so restore it
            }
            //calculate margin depend on final size
            auto& Margin = ChildLayoutSelf->GetMargin();
            AreaSize[0] -= Margin.Left + Margin.Right;
            AreaSize[1] -= Margin.Top + Margin.Bottom;
            AnchoredPositionX += FMath::Lerp(Margin.Left, -Margin.Right, ChildWidget->GetPivot()[0]);
            AnchoredPositionY += FMath::Lerp(Margin.Bottom, -Margin.Top, ChildWidget->GetPivot()[1]);
        }
        //now the AreaSize is the final size, apply to widget
        if (ChildLayoutSelf)
        {
            ChildLayoutSelf->SetFinalSizeByLayoutContainer(AreaSize);
            ChildWidget->SetAnchoredPositionAndSizeDelta(FVector2D(AnchoredPositionX, AnchoredPositionY), FVector2D(AreaSize));
        }
        else
        {
            ChildWidget->SetAnchoredPosition(FVector2D(AnchoredPositionX, AnchoredPositionY));
        }
    };

    bool bAllowWrap = Wrap == ELexLayoutFlexBoxWrapType::Wrap || Wrap == ELexLayoutFlexBoxWrapType::WrapReverse;
    
    auto ChildrenCount = Children.Num();
    //calculate lines
    LineDataArray.Reset();
    auto CurrentLine = FLineData();
    TotalPreferredSize = FVector2f(0, 0);
    
    auto Gap = FVector2f(WidthGap, HeightGap);
    auto Widget = GetWidget();
    auto ContainerSize = FVector2f(Widget->GetWidth(), Widget->GetHeight());
    ThisWidgetSize = ContainerSize;
    ContainerSize.Y -= Padding.Top + Padding.Bottom;
    ContainerSize.X -= Padding.Left + Padding.Right;

    for (int i = 0; i < ChildrenCount; i++)
    {
        auto Child = Children[i];
        auto ChildSizes = GetChildSizes(Child);
        CurrentLine.TotalMin[PrimaryAxis] += ChildSizes->Min[PrimaryAxis];
        CurrentLine.TotalMax[PrimaryAxis] += ChildSizes->Max[PrimaryAxis];
        CurrentLine.TotalPreferred[PrimaryAxis] += ChildSizes->Preferred[PrimaryAxis];
        CurrentLine.TotalGrow[PrimaryAxis] += ChildSizes->Grow[PrimaryAxis];
        CurrentLine.TotalShrink[PrimaryAxis] += ChildSizes->Shrink[PrimaryAxis];
        if (bAllowWrap)
        {
            if (CurrentLine.TotalPreferred[PrimaryAxis] > ContainerSize[PrimaryAxis])//larger than container size, wrap it
            {
                CurrentLine.TotalMin[PrimaryAxis] -= ChildSizes->Min[PrimaryAxis] + Gap[PrimaryAxis];
                CurrentLine.TotalMax[PrimaryAxis] -= ChildSizes->Max[PrimaryAxis] + Gap[PrimaryAxis];
                CurrentLine.TotalPreferred[PrimaryAxis] -= ChildSizes->Preferred[PrimaryAxis] + Gap[PrimaryAxis];
                CurrentLine.TotalGrow[PrimaryAxis] -= ChildSizes->Grow[PrimaryAxis];
                CurrentLine.TotalShrink[PrimaryAxis] -= ChildSizes->Shrink[PrimaryAxis];
                //finish current line and make new line
                LineDataArray.Add(CurrentLine);
                CurrentLine = FLineData();
                CurrentLine.TotalMin[PrimaryAxis] += ChildSizes->Min[PrimaryAxis];
                CurrentLine.TotalMax[PrimaryAxis] += ChildSizes->Max[PrimaryAxis];
                CurrentLine.TotalPreferred[PrimaryAxis] += ChildSizes->Preferred[PrimaryAxis];
                CurrentLine.TotalGrow[PrimaryAxis] += ChildSizes->Grow[PrimaryAxis];
                CurrentLine.TotalShrink[PrimaryAxis] += ChildSizes->Shrink[PrimaryAxis];
            }
        }
        CurrentLine.TotalMin[PrimaryAxis] += Gap[PrimaryAxis];
        CurrentLine.TotalMax[PrimaryAxis] += Gap[PrimaryAxis];
        CurrentLine.TotalPreferred[PrimaryAxis] += Gap[PrimaryAxis];
        
        //primary axis size: try not wrap content and get size
        TotalPreferredSize[PrimaryAxis] += ChildSizes->Preferred[PrimaryAxis] + Gap[PrimaryAxis];
        
        //secondary axis
        CurrentLine.TotalMin[SecondaryAxis] = FMath::Max(ChildSizes->Min[SecondaryAxis], CurrentLine.TotalMin[SecondaryAxis]);
        CurrentLine.TotalMax[SecondaryAxis] = FMath::Max(ChildSizes->Max[SecondaryAxis], CurrentLine.TotalMax[SecondaryAxis]);
        CurrentLine.TotalPreferred[SecondaryAxis] = FMath::Max(ChildSizes->Preferred[SecondaryAxis], CurrentLine.TotalPreferred[SecondaryAxis]);
        CurrentLine.TotalGrow[SecondaryAxis] = FMath::Max(ChildSizes->Grow[SecondaryAxis], CurrentLine.TotalGrow[SecondaryAxis]);
        CurrentLine.TotalShrink[SecondaryAxis] = FMath::Max(ChildSizes->Shrink[SecondaryAxis], CurrentLine.TotalShrink[SecondaryAxis]);
        
        CurrentLine.Children.Add(Child);
    }
    if (ChildrenCount > 0)
    {
        CurrentLine.TotalMin[PrimaryAxis] -= Gap[PrimaryAxis];
        CurrentLine.TotalPreferred[PrimaryAxis] -= Gap[PrimaryAxis];
        LineDataArray.Add(CurrentLine);

        TotalPreferredSize[PrimaryAxis] -= Gap[PrimaryAxis];
    }

    //secondary axis property
    float SecondaryTotalMin = 0, SecondaryTotalMax = 0, SecondaryTotalPreferred = 0, SecondaryTotalGrow = 0, SecondaryTotalShrink = 0;
    for (int LineIndex = 0; LineIndex < LineDataArray.Num(); LineIndex++)
    {
        auto& LineData = LineDataArray[LineIndex];
        SecondaryTotalMin += LineData.TotalMin[SecondaryAxis] + Gap[SecondaryAxis];
        SecondaryTotalMax += LineData.TotalMax[SecondaryAxis] + Gap[SecondaryAxis];
        SecondaryTotalPreferred += LineData.TotalPreferred[SecondaryAxis] + Gap[SecondaryAxis];
        SecondaryTotalGrow += LineData.TotalGrow[SecondaryAxis];
        SecondaryTotalShrink += LineData.TotalShrink[SecondaryAxis];
    }
    if (ChildrenCount > 0)
    {
        SecondaryTotalMin -= Gap[SecondaryAxis];
        SecondaryTotalMax -= Gap[SecondaryAxis];
        SecondaryTotalPreferred -= Gap[SecondaryAxis];
    }
    //secondary axis size: accumulate secondary sizes
    TotalPreferredSize[SecondaryAxis] += SecondaryTotalPreferred;
    TotalPreferredSize[PrimaryAxis] += (PrimaryAxis == 0 ? Padding.Left + Padding.Right : Padding.Top + Padding.Bottom);
    TotalPreferredSize[SecondaryAxis] += (SecondaryAxis == 0 ? Padding.Left + Padding.Right : Padding.Top + Padding.Bottom);
    if (!bApplyResult)return;

    bool bReverseHorizontal = Direction == ELexLayoutFlexBoxDirectionType::HorizontalReverse;
    bool bReverseVertical = Direction == ELexLayoutFlexBoxDirectionType::VerticalReverse;
    bool bReverse = bReverseHorizontal || bReverseVertical;
    
    float SecondarySurplusSpace = ContainerSize[SecondaryAxis] - SecondaryTotalPreferred;
    float SecondarySpaceGap = 0;//Space between two items beside gap value
    float SecondaryStretchedExtraSize = 0;

    FVector2f PosOffset(0,0);//default position use left top as origin
    PosOffset[SecondaryAxis] = SecondaryAxis == 0 ? Padding.Left : Padding.Top;
    if (SecondarySurplusSpace > 0)
    {
        switch (SecondaryAlignment)
        {
        case ELexLayoutFlexBoxSecondaryAxisAlignment::Start:break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::Center:
            PosOffset[SecondaryAxis] += SecondarySurplusSpace * 0.5f;
            break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::End:
            PosOffset[SecondaryAxis] += SecondarySurplusSpace;
            break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::SpaceBetween:
            //no offset needed
            SecondarySpaceGap = SecondarySurplusSpace / (LineDataArray.Num() - 1);
            break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::SpaceAround:
            //half space offset
            SecondarySpaceGap = SecondarySurplusSpace / LineDataArray.Num();
            PosOffset[SecondaryAxis] += SecondarySpaceGap * 0.5f;
            break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::SpaceEvenly:
            //space offset
            SecondarySpaceGap = SecondarySurplusSpace / (LineDataArray.Num() + 1);
            PosOffset[SecondaryAxis] += SecondarySpaceGap;
            break;
        case ELexLayoutFlexBoxSecondaryAxisAlignment::Stretch:
            SecondaryStretchedExtraSize = SecondarySurplusSpace / LineDataArray.Num();
            break;
        }
    }
    
    for (int LineIndex = 0; LineIndex < LineDataArray.Num(); LineIndex++)
    {
        auto Index = (Wrap == ELexLayoutFlexBoxWrapType::WrapReverse) ? (LineDataArray.Num() - 1 - LineIndex) : LineIndex;
        auto& LineData = LineDataArray[Index];
        if (bReverse)
        {
            PosOffset[PrimaryAxis] = ContainerSize[PrimaryAxis];
        }
        else
        {
            PosOffset[PrimaryAxis] = PrimaryAxis == 0 ? Padding.Left : Padding.Top;
        }
        float PrimaryGrowMultiplier = 0;
        float PrimarySurplusSpace = ContainerSize[PrimaryAxis] - LineData.TotalPreferred[PrimaryAxis];
        
        if (LineData.TotalGrow[PrimaryAxis] > 0)
        {
            PrimaryGrowMultiplier = PrimarySurplusSpace / LineData.TotalGrow[PrimaryAxis];
        }

        float PrimaryShrinkMultiplier = 0;
        float PrimaryDeficitSpace = -PrimarySurplusSpace;//container less than preferred
        if (PrimaryDeficitSpace > 0)
        {
            if (LineData.TotalShrink[PrimaryAxis] > 0)
            {
                PrimaryShrinkMultiplier = PrimaryDeficitSpace / LineData.TotalShrink[PrimaryAxis];
            }
        }

        //calculate alignment
        float PrimarySpaceGap = 0;//Space between two items beside gap value
        {
            float ActualSurplusSpace = 0;
            if (PrimarySurplusSpace > 0)//have extra spaces, may grow, can align
            {
                if (LineData.TotalGrow[PrimaryAxis] == 0)//no grow
                {
                    ActualSurplusSpace = PrimarySurplusSpace;
                }
                else
                {
                    //max value could limit grow size
                    ActualSurplusSpace = PrimarySurplusSpace - (LineData.TotalMax[PrimaryAxis] - LineData.TotalPreferred[PrimaryAxis]);
                }
            }
            float ActualDeficitSpace = 0;
            if (PrimaryDeficitSpace > 0)//have deficit spaces, may shrink, can align
            {
                if (LineData.TotalShrink[PrimaryAxis] == 0)//no shrink
                {
                    ActualDeficitSpace = PrimaryDeficitSpace;
                }
                else
                {
                    //min value could limit shrink size
                    ActualDeficitSpace = PrimaryDeficitSpace - (LineData.TotalPreferred[PrimaryAxis] - LineData.TotalMin[PrimaryAxis]);
                }
            }

            if (ActualSurplusSpace > 0)
            {
                switch (PrimaryAlignment)
                {
                case ELexLayoutFlexBoxPrimaryAxisAlignment::Start:break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::Center:
                    PosOffset[PrimaryAxis] += (bReverse ? -ActualSurplusSpace : ActualSurplusSpace) * 0.5f;
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::End:
                    PosOffset[PrimaryAxis] += (bReverse ? -ActualSurplusSpace : ActualSurplusSpace);
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::SpaceBetween:
                    //no offset needed
                    PrimarySpaceGap = ActualSurplusSpace / (LineData.Children.Num() - 1);
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::SpaceAround:
                    //half space offset
                    PrimarySpaceGap = ActualSurplusSpace / LineData.Children.Num();
                    PosOffset[PrimaryAxis] += (bReverse ? -PrimarySpaceGap : PrimarySpaceGap) * 0.5f;
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::SpaceEvenly:
                    //space offset
                    PrimarySpaceGap = ActualSurplusSpace / (LineData.Children.Num() + 1);
                    PosOffset[PrimaryAxis] += (bReverse ? -PrimarySpaceGap : PrimarySpaceGap);
                    break;
                }
            }
            if (ActualDeficitSpace > 0)
            {
                switch (PrimaryAlignment)
                {
                case ELexLayoutFlexBoxPrimaryAxisAlignment::Start:
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::Center:
                    PosOffset[PrimaryAxis] += (bReverse ? ActualDeficitSpace : -ActualDeficitSpace) * 0.5f;
                    break;
                case ELexLayoutFlexBoxPrimaryAxisAlignment::End:
                    PosOffset[PrimaryAxis] += (bReverse ? ActualDeficitSpace : -ActualDeficitSpace);
                    break;
                }
            }
        }

        auto CurrentLinePosOffset = PosOffset;
        float SecondaryAxisLineSize = LineData.TotalPreferred[SecondaryAxis];
        SecondaryAxisLineSize += SecondaryStretchedExtraSize;

        for (int ChildIndex = 0; ChildIndex < LineData.Children.Num(); ChildIndex++)
        {
            auto& Child = LineData.Children[ChildIndex];
            FVector2f AreaSize(0,0);
            auto ChildSizes = GetChildSizes(Child);
            //calculate primary size
            {
                float PrimarySize = ChildSizes->Preferred[PrimaryAxis];
                //handle shrink
                if (PrimaryShrinkMultiplier > 0)
                {
                    float ShrinkSize = ChildSizes->Shrink[PrimaryAxis] * PrimaryShrinkMultiplier;
                    float MaxShrinkSize = ChildSizes->Preferred[PrimaryAxis] - ChildSizes->Min[PrimaryAxis];
                    if (ShrinkSize >= MaxShrinkSize)
                    {
                        PrimarySize = ChildSizes->Min[PrimaryAxis];
                        //prepare data for next loop
                        LineData.TotalShrink[PrimaryAxis] -= ChildSizes->Shrink[PrimaryAxis];
                        PrimaryDeficitSpace -= MaxShrinkSize;
                        if (LineData.TotalShrink[PrimaryAxis] > 0)
                        {
                            PrimaryShrinkMultiplier = PrimaryDeficitSpace / LineData.TotalShrink[PrimaryAxis];
                        }
                    }
                    else
                    {
                        PrimarySize -= ShrinkSize;
                    }
                }
                //handle grow
                if (PrimaryGrowMultiplier > 0)
                {
                    float GrowSize = ChildSizes->Grow[PrimaryAxis] * PrimaryGrowMultiplier;
                    float MaxGrowSize = ChildSizes->Max[PrimaryAxis] - ChildSizes->Preferred[PrimaryAxis];
                    if (GrowSize >= MaxGrowSize)
                    {
                        PrimarySize = ChildSizes->Max[PrimaryAxis];
                        //prepare data for next loop
                        LineData.TotalGrow[PrimaryAxis] -= ChildSizes->Grow[PrimaryAxis];
                        PrimarySurplusSpace -= MaxGrowSize;
                        if (LineData.TotalGrow[PrimaryAxis] > 0)
                        {
                            PrimaryGrowMultiplier = PrimarySurplusSpace / LineData.TotalGrow[PrimaryAxis];
                        }
                    }
                    else
                    {
                        PrimarySize += GrowSize;
                    }
                }
                AreaSize[PrimaryAxis] = PrimarySize;
            }
            //calculate secondary size
            float SecondaryPreferredSize = 0;
            {
                AreaSize[SecondaryAxis] = SecondaryAxisLineSize;
                SecondaryPreferredSize = ChildSizes->Preferred[SecondaryAxis];
            }
            
            SetChildPositionAndSize(Child, CurrentLinePosOffset, AreaSize, SecondaryPreferredSize, bReverseHorizontal, bReverseVertical);
            if (bReverse)
            {
                CurrentLinePosOffset[PrimaryAxis] -= AreaSize[PrimaryAxis] + Gap[PrimaryAxis] + PrimarySpaceGap;
            }
            else
            {
                CurrentLinePosOffset[PrimaryAxis] += AreaSize[PrimaryAxis] + Gap[PrimaryAxis] + PrimarySpaceGap;
            }
        }
        PosOffset[PrimaryAxis] = CurrentLinePosOffset[PrimaryAxis];
        PosOffset[SecondaryAxis] += SecondaryAxisLineSize + Gap[SecondaryAxis] + SecondarySpaceGap;
    }
}

void ULexLayoutContainerFlexBox::CalculatePreferredSize()
{
    RefreshChildren();
    DoCalculate(false);
}

FLexLayoutControlAnchorData ULexLayoutContainerFlexBox::GetLayoutControlAnchor(const ULexWidget* TargetWidget)const
{
    FLexLayoutControlAnchorData Result;
    auto ThisWidget = GetWidget();
    if (ThisWidget == TargetWidget)//self
    {
        
    }
    else if (ThisWidget->GetChildren().Contains(TargetWidget))//child
    {
        bool bIgnoreLayout = TargetWidget->GetIgnoreLayout();
        if (!bIgnoreLayout)
        {
            Result.bCanControlHorizontalPosition = true;
            Result.bCanControlVerticalPosition = true;
        }
    }
    return Result;
}

#if WITH_EDITOR
void ULexLayoutContainerFlexBox::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

FVector2f ULexLayoutContainerFlexBox::GetLayoutPreferredSize()
{
    CalculatePreferredSize();
    return this->TotalPreferredSize;
}

void ULexLayoutContainerFlexBox::SetDirection(ELexLayoutFlexBoxDirectionType Value)
{
    if (Direction != Value)
    {
        Direction = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetWrap(ELexLayoutFlexBoxWrapType Value)
{
    if (Wrap != Value)
    {
        Wrap = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetPrimaryAlignment(ELexLayoutFlexBoxPrimaryAxisAlignment Value)
{
    if (PrimaryAlignment != Value)
    {
        PrimaryAlignment = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetSecondaryAlignment(ELexLayoutFlexBoxSecondaryAxisAlignment Value)
{
    if (SecondaryAlignment != Value)
    {
        SecondaryAlignment = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetSecondaryLineAlignment(ELexLayoutFlexBoxSecondaryAxisLineAlignment Value)
{
    if (SecondaryLineAlignment != Value)
    {
        SecondaryLineAlignment = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetWidthGap(float Value)
{
    if (WidthGap != Value)
    {
        WidthGap = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetHeightGap(float Value)
{
    if (HeightGap != Value)
    {
        HeightGap = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}

void ULexLayoutContainerFlexBox::SetPadding(const FMargin& Value)
{
    if (Padding != Value)
    {
        Padding = Value;
        ULexWidget::MarkLayoutForRebuild(GetWidget());
    }
}


