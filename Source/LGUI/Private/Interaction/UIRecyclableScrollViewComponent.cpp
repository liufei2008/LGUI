﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIRecyclableScrollViewComponent.h"
#include "LGUI.h"
#include "LTweenActor.h"
#include "Core/Actor/UIBaseActor.h"
#include "LGUIBPLibrary.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

void UUIRecyclableScrollViewComponent::Awake()
{
    Super::Awake();
}

void UUIRecyclableScrollViewComponent::Start()
{
    Super::Start();
    InitializeOnDataSource();
}

void UUIRecyclableScrollViewComponent::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
}

void UUIRecyclableScrollViewComponent::OnDestroy()
{
    Super::OnDestroy();
    if (OnScrollEventDelegateHandle.IsValid())
    {
        this->UnregisterScrollEvent(OnScrollEventDelegateHandle);
    }
}

#if WITH_EDITOR
void UUIRecyclableScrollViewComponent::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (auto Property = PropertyChangedEvent.MemberProperty)
    {
        auto PropertyName = Property->GetFName();
        if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, Horizontal))
        {
            Vertical = !Horizontal;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, Vertical))
        {
            Horizontal = !Vertical;
        }
    }
}
#endif

void UUIRecyclableScrollViewComponent::SetDataSource(TScriptInterface<IUIRecyclableScrollViewDataSource> InDataSource)
{
    if (InDataSource == nullptr)
    {
        DataSource = nullptr;
        InitializeOnDataSource();
        return;
    }
    if (DataSource != InDataSource.GetObject())
    {
        DataSource = InDataSource.GetObject();
        InitializeOnDataSource();
    }
}
void UUIRecyclableScrollViewComponent::SetRows(int value)
{
    value = FMath::Max(1, value);
    if (Rows != value)
    {
        Rows = value;
        if (Horizontal)
        {
            InitializeOnDataSource();
        }
    }
}
void UUIRecyclableScrollViewComponent::SetColumns(int value)
{
    value = FMath::Max(1, value);
    if (Columns != value)
    {
        Columns = value;
        if (Vertical)
        {
            InitializeOnDataSource();
        }
    }
}

void UUIRecyclableScrollViewComponent::InitializeOnDataSource()
{
    if (DataSource == nullptr)return;
    if (!IsValid(DataSource))return;
    if (!CellTemplate.IsValid())return;

    auto GetComponentByInterface = [](AActor* InActor, UClass* InInterfaceClass) {
        for (UActorComponent* Component : InActor->GetComponents())
        {
            if (Component && Component->GetClass()->ImplementsInterface(InInterfaceClass))
            {
                return Component;
            }
        }
        return (UActorComponent*)nullptr;
    };
    if (GetComponentByInterface(CellTemplate.Get(), UUIRecyclableScrollViewCell::StaticClass()) == nullptr)
    {
        return;
    }
    if (!CheckParameters())return;
    if (Horizontal == Vertical)return;
    DataItemCount = IUIRecyclableScrollViewDataSource::Execute_GetItemCount(DataSource);
    if (DataItemCount <= 0)return;

    if (OnScrollEventDelegateHandle.IsValid())
    {
        this->UnregisterScrollEvent(OnScrollEventDelegateHandle);
    }

    int VisibleColumnOrRowCount = 0;
    int VisibleCellCount = 0;
    if (Horizontal)
    {
        RangeArea.X = ContentParentUIItem->GetLocalSpaceLeft();
        RangeArea.Y = ContentParentUIItem->GetLocalSpaceRight();
        VisibleColumnOrRowCount = FMath::CeilToInt((RangeArea.Y - RangeArea.X) / CellTemplate->GetUIItem()->GetWidth());
        VisibleColumnOrRowCount += 1;
        VisibleCellCount = VisibleColumnOrRowCount * Rows;
        VisibleCellCount = FMath::Min(VisibleCellCount, DataItemCount);
        float ContentSize = FMath::CeilToInt((float)DataItemCount / Rows) * CellTemplate->GetUIItem()->GetWidth();
        ContentUIItem->SetWidth(ContentSize);
        CellTemplate->GetUIItem()->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), true, true);
        CellTemplate->GetUIItem()->SetPivot(FVector2D(0.0f, 1.0f));
    }
    else
    {
        RangeArea.X = ContentParentUIItem->GetLocalSpaceBottom();
        RangeArea.Y = ContentParentUIItem->GetLocalSpaceTop();
        VisibleColumnOrRowCount = FMath::CeilToInt((RangeArea.Y - RangeArea.X) / CellTemplate->GetUIItem()->GetHeight());
        VisibleColumnOrRowCount += 1;
        VisibleCellCount = VisibleColumnOrRowCount * Columns;
        VisibleCellCount = FMath::Min(VisibleCellCount, DataItemCount);
        float ContentSize = FMath::CeilToInt((float)DataItemCount / Columns) * CellTemplate->GetUIItem()->GetHeight();
        ContentUIItem->SetHeight(ContentSize);
        CellTemplate->GetUIItem()->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), true, true);
        CellTemplate->GetUIItem()->SetPivot(FVector2D(0.0f, 1.0f));
    }

    CellTemplate->GetUIItem()->SetIsUIActive(true);
    float CellWidth;
    float CellHeight;
    if (Horizontal)
    {
        CellWidth = CellTemplate->GetUIItem()->GetWidth();
        CellHeight = ContentUIItem->GetHeight() / Rows;
    }
    else
    {
        CellWidth = ContentUIItem->GetWidth() / Columns;
        CellHeight = CellTemplate->GetUIItem()->GetHeight();
    }

    //create more cells
    while (CacheCellList.Num() < VisibleCellCount)
    {
        auto CopiedCell = ULGUIBPLibrary::DuplicateActorT(CellTemplate.Get(), ContentUIItem.Get());
        auto CellInterfaceClass = UUIRecyclableScrollViewCell::StaticClass();
        auto CellInterfaceComponent = GetComponentByInterface(CopiedCell, CellInterfaceClass);
        FUIRecyclableScrollViewCellContainer CellContainer;
        CellContainer.UIItem = CopiedCell->GetUIItem();
        CellContainer.CellComponent = CellInterfaceComponent;
        check(CellInterfaceComponent != nullptr);
        IUIRecyclableScrollViewDataSource::Execute_InitOnCreate(DataSource, CellInterfaceComponent);
        CacheCellList.Add(CellContainer);
    }
    CellTemplate->GetUIItem()->SetIsUIActive(false);
    //delete extra cells
    while (CacheCellList.Num() > VisibleCellCount)
    {
        int LastIndex = CacheCellList.Num() - 1;
        auto& Item = CacheCellList[LastIndex];
        ULGUIBPLibrary::DestroyActorWithHierarchy(Item.UIItem->GetOwner());
        CacheCellList.RemoveAt(LastIndex);
    }
    MinCellIndexInCacheCellList = 0;
    MaxCellIndexInCacheCellList = (VisibleColumnOrRowCount - 1) * (Horizontal ? Rows : Columns);
    //set cell position and size and data
    float PosX = 0, PosY = 0;
    int RowOrColumnIndex = 0;
    for (int i = 0; i < CacheCellList.Num(); i++)
    {
        auto& CellItem = CacheCellList[i];
        IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, i);
        if (Horizontal)
        {
            CellItem.UIItem->SetHeight(CellHeight);
            CellItem.UIItem->SetAnchoredPosition(FVector2D(PosX, PosY));
            RowOrColumnIndex++;
            if (RowOrColumnIndex >= Rows)
            {
                PosX += CellWidth;
                PosY = 0;
                RowOrColumnIndex = 0;
            }
            else
            {
                PosY -= CellHeight;
            }
        }
        else
        {
            CellItem.UIItem->SetWidth(CellWidth);
            CellItem.UIItem->SetAnchoredPosition(FVector2D(PosX, PosY));
            RowOrColumnIndex++;
            if (RowOrColumnIndex >= Columns)
            {
                PosY -= CellHeight;
                PosX = 0;
                RowOrColumnIndex = 0;
            }
            else
            {
                PosX += CellWidth;
            }
        }
    }

    auto PrevProgress = this->Progress;
    if (Horizontal)
    {
        this->SetScrollProgress(FVector2D(1.0f, PrevProgress.Y));
    }
    else
    {
        this->SetScrollProgress(FVector2D(PrevProgress.X, 0.0f));
    }
    MinCellPosition = 0;
    MinCellIndexInData = 0;

    PrevContentPosition = FVector2D(ContentUIItem->GetRelativeLocation().Y, ContentUIItem->GetRelativeLocation().Z);
    OnScrollEventDelegateHandle = this->RegisterScrollEvent(FLGUIVector2Delegate::CreateUObject(this, &UUIRecyclableScrollViewComponent::OnScrollCallback));
    //this->SetScrollProgress(PrevProgress);
}
void UUIRecyclableScrollViewComponent::OnScrollCallback(FVector2D value)
{
    if (Horizontal == Vertical)return;
    if (CacheCellList.Num() == 0)return;
    if (DataItemCount == 0)return;

    const auto ContentPosition = FVector2D(ContentUIItem->GetRelativeLocation().Y, ContentUIItem->GetRelativeLocation().Z);
    if (Horizontal)
    {
        float CellWidth = CellTemplate->GetUIItem()->GetWidth();
        auto PointToScrollViewSpaceOffset = ContentUIItem->GetRelativeLocation().Y;
        if (ContentPosition.X > PrevContentPosition.X)//scroll from left to right
        {
            while (MinCellIndexInData > 0)
            {
                int CellDataIndex = MinCellIndexInData - Rows;//flip data
                auto& RightTopCellItem = CacheCellList[MaxCellIndexInCacheCellList];
                auto CellLeftPointInScrollViewSpace = RightTopCellItem.UIItem->GetLocalSpaceLeft() + RightTopCellItem.UIItem->GetRelativeLocation().Y + PointToScrollViewSpaceOffset;
                if (CellLeftPointInScrollViewSpace > RangeArea.Y)//left item out of range
                {
                    for (int i = 0; i < Rows; i++)
                    {
                        auto& CellItem = CacheCellList[MaxCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.X = MinCellPosition - CellWidth;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellIndexInData--;
                        //set data
                        if (CellDataIndex + i < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, CellDataIndex + i);
                        }
                        else
                        {
                            CellItem.UIItem->SetIsUIActive(false);
                        }
                    }
                    //decrease index
                    DecreaseMinMaxCellIndexInCacheCellList(Rows);
                    //left cell position
                    MinCellPosition -= CellWidth;
                }
                else
                {
                    break;
                }
            }
        }
        else if (ContentPosition.X < PrevContentPosition.X)//scroll from right to left
        {
            int RightCellIndexInData = MinCellIndexInData + CacheCellList.Num() - 1;
            while (RightCellIndexInData + 1 < DataItemCount)//check if right cell reach end data
            {
                auto& LeftTopCellItem = CacheCellList[MinCellIndexInCacheCellList];
                auto CellRightPointInScrollViewSpace = LeftTopCellItem.UIItem->GetLocalSpaceRight() + LeftTopCellItem.UIItem->GetRelativeLocation().Y + PointToScrollViewSpaceOffset;
                if (CellRightPointInScrollViewSpace < RangeArea.X)//left item out of range
                {
                    for (int i = 0; i < Rows; i++)
                    {
                        auto& CellItem = CacheCellList[MinCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.X = MinCellPosition + CellWidth * (CacheCellList.Num() / Rows);
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellIndexInData++;
                        RightCellIndexInData = MinCellIndexInData + CacheCellList.Num() - 1;
                        //set data
                        if (RightCellIndexInData < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, RightCellIndexInData);
                        }
                        else
                        {
                            CellItem.UIItem->SetIsUIActive(false);
                        }
                    }
                    //increase index
                    IncreaseMinMaxCellIndexInCacheCellList(Rows);
                    //left cell position
                    MinCellPosition += CellWidth;
                }
                else
                {
                    break;
                }
            }
        }
    }
    else
    {
        float CellHeight = CellTemplate->GetUIItem()->GetHeight();
        auto PointToScrollViewSpaceOffset = ContentUIItem->GetRelativeLocation().Z;
        if (ContentPosition.Y < PrevContentPosition.Y)//scroll from top to bottom
        {
            while (MinCellIndexInData > 0)
            {
                int CellDataIndex = MinCellIndexInData - Columns;//flip data
                auto& BottomLeftCellItem = CacheCellList[MaxCellIndexInCacheCellList];
                auto CellTopPointInScrollViewSpace = BottomLeftCellItem.UIItem->GetLocalSpaceTop() + BottomLeftCellItem.UIItem->GetRelativeLocation().Z + PointToScrollViewSpaceOffset;
                if (CellTopPointInScrollViewSpace < RangeArea.X)//bottom item out of range
                {
                    //move bottom to top
                    for (int i = 0; i < Columns; i++)
                    {
                        auto& CellItem = CacheCellList[MaxCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.Y = MinCellPosition + CellHeight;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellIndexInData--;
                        //set data
                        if (CellDataIndex + i < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, CellDataIndex + i);
                        }
                        else
                        {
                            CellItem.UIItem->SetIsUIActive(false);
                        }
                    }
                    //decrease index
                    DecreaseMinMaxCellIndexInCacheCellList(Columns);
                    //top cell position
                    MinCellPosition += CellHeight;
                }
                else//none out of range, no need recycle anything
                {
                    break;
                }
            }
        }
        else//scroll from bottom to top
        {
            int BottomCellIndexInData = MinCellIndexInData + CacheCellList.Num() - 1;
            while (BottomCellIndexInData + 1 < DataItemCount)//check if bottom cell reach end data
            {
                auto& TopLeftCellItem = CacheCellList[MinCellIndexInCacheCellList];
                auto CellBottomPointInScrollViewSpace = TopLeftCellItem.UIItem->GetLocalSpaceBottom() + TopLeftCellItem.UIItem->GetRelativeLocation().Z + PointToScrollViewSpaceOffset;
                if (CellBottomPointInScrollViewSpace > RangeArea.Y)//top item out of range
                {
                    for (int i = 0; i < Columns; i++)
                    {
                        auto& CellItem = CacheCellList[MinCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.Y = MinCellPosition - CellHeight * (CacheCellList.Num() / Columns);
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellIndexInData++;
                        BottomCellIndexInData = MinCellIndexInData + CacheCellList.Num() - 1;
                        //set data
                        if (BottomCellIndexInData < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, BottomCellIndexInData);
                        }
                        else
                        {
                            CellItem.UIItem->SetIsUIActive(false);
                        }
                    }
                    //increase index
                    IncreaseMinMaxCellIndexInCacheCellList(Columns);
                    //top cell position
                    MinCellPosition -= CellHeight;
                }
                else//none out of range, no need recycle anything
                {
                    break;
                }
            }
        }
    }
    PrevContentPosition = ContentPosition;
}

void UUIRecyclableScrollViewComponent::IncreaseMinMaxCellIndexInCacheCellList(int Count)
{
    MinCellIndexInCacheCellList += Count;
    MaxCellIndexInCacheCellList += Count;
    if (MinCellIndexInCacheCellList >= CacheCellList.Num())
    {
        MinCellIndexInCacheCellList = 0;
    }
    if (MaxCellIndexInCacheCellList >= CacheCellList.Num())
    {
        MaxCellIndexInCacheCellList = 0;
    }
}
void UUIRecyclableScrollViewComponent::DecreaseMinMaxCellIndexInCacheCellList(int Count)
{
    MinCellIndexInCacheCellList -= Count;
    MaxCellIndexInCacheCellList -= Count;
    if (MinCellIndexInCacheCellList < 0)
    {
        MinCellIndexInCacheCellList = CacheCellList.Num() - Count;
    }
    if (MaxCellIndexInCacheCellList < 0)
    {
        MaxCellIndexInCacheCellList = CacheCellList.Num() - Count;
    }
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif