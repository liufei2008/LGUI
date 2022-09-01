// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
void UUIRecyclableScrollViewComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
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

void UUIRecyclableScrollViewComponent::GetUserFriendlyCacheCellList(TArray<FUIRecyclableScrollViewCellContainer>& OutResult)const
{
    OutResult.SetNumUninitialized(CacheCellList.Num());
    int IndexInSource = MinCellIndexInCacheCellList;
    for (int i = 0; i < CacheCellList.Num(); i++)
    {
        if (IndexInSource >= CacheCellList.Num())
        {
            IndexInSource -= CacheCellList.Num();
        }
        OutResult[i] = CacheCellList[IndexInSource];
        IndexInSource++;
    }
}

void UUIRecyclableScrollViewComponent::ClearAllCells()
{
    for (auto& Item : CacheCellList)
    {
        if (IsValid(Item.UIItem))
        {
            ULGUIBPLibrary::DestroyActorWithHierarchy(Item.UIItem->GetOwner());
        }
    }
    CacheCellList.Empty();

    DataItemCount = 0;
    MinCellIndexInCacheCellList = 0;
    MaxCellIndexInCacheCellList = 0;
    MinCellPosition = 0;
    MinCellDataIndex = 0;
}

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
void UUIRecyclableScrollViewComponent::SetPadding(const FMargin& value)
{
    if (Padding != value)
    {
        Padding = value;
        InitializeOnDataSource();
    }
}
void UUIRecyclableScrollViewComponent::SetSpace(const FVector2D& value)
{
    if (Space != value)
    {
        Space = value;
        InitializeOnDataSource();
    }
}

bool UUIRecyclableScrollViewComponent::GetCellItemByDataIndex(int Index, FUIRecyclableScrollViewCellContainer& OutResult)const
{
    auto MaxCellIndexInData = FMath::Min(Index + CacheCellList.Num() - 1, DataItemCount - 1);
    if (Index < MinCellDataIndex || Index > MaxCellIndexInData)
    {
        return false;
    }
    else
    {
        auto CellIndexOffset = Index - MinCellDataIndex;
        if (CellIndexOffset >= CacheCellList.Num())
        {
            return false;
        }
        auto CellIndex = MinCellIndexInCacheCellList + CellIndexOffset;
        if (CellIndex >= CacheCellList.Num())
        {
            CellIndex -= CacheCellList.Num();
        }
        if (CacheCellList.IsValidIndex(CellIndex))
        {
            OutResult = CacheCellList[CellIndex];
            return true;
        }
        else
        {
            UE_LOG(LGUI, Error, TEXT("[UUIRecyclableScrollViewComponent::GetCellItemByDataIndex] Wrong cell index. Index:%d, CellIndexOffset:%d, CellIndex:%d"), Index, CellIndexOffset, CellIndex);
            ensure(false);
            return false;
        }
    }
}


void UUIRecyclableScrollViewComponent::InitializeOnDataSource()
{
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
        float RangeSize = RangeArea.Y - RangeArea.X - (Padding.Left + Padding.Right);
        float AllVisibleCellWidth = 0;
        float CellWidth = CellTemplate->GetUIItem()->GetWidth();
        while (true)
        {
            VisibleColumnOrRowCount++;
            AllVisibleCellWidth += CellWidth;
            if (AllVisibleCellWidth >= RangeSize)
            {
                break;
            }
            AllVisibleCellWidth += Space.X;
        }
        VisibleColumnOrRowCount += 1;
        VisibleCellCount = VisibleColumnOrRowCount * Rows;
        VisibleCellCount = FMath::Min(VisibleCellCount, DataItemCount);
        int HorizontalCellCount = FMath::CeilToInt((float)DataItemCount / Rows);
        float ContentSize = HorizontalCellCount * CellWidth + (HorizontalCellCount - 1) * Space.X + Padding.Left + Padding.Right;
        ContentUIItem->SetWidth(ContentSize);
        CellTemplate->GetUIItem()->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), true, true);
    }
    else
    {
        RangeArea.X = ContentParentUIItem->GetLocalSpaceBottom();
        RangeArea.Y = ContentParentUIItem->GetLocalSpaceTop();
        float RangeSize = RangeArea.Y - RangeArea.X - (Padding.Bottom + Padding.Top);
        float AllVisibleCellHeight = 0;
        float CellHeight = CellTemplate->GetUIItem()->GetHeight();
        while (true)
        {
            VisibleColumnOrRowCount++;
            AllVisibleCellHeight += CellHeight;
            if (AllVisibleCellHeight > RangeSize)
            {
                break;
            }
            AllVisibleCellHeight += Space.Y;
        }
        VisibleColumnOrRowCount += 1;
        VisibleCellCount = VisibleColumnOrRowCount * Columns;
        VisibleCellCount = FMath::Min(VisibleCellCount, DataItemCount);
        int VerticalCellCount = FMath::CeilToInt((float)DataItemCount / Columns);
        float ContentSize = VerticalCellCount * CellHeight + (VerticalCellCount - 1) * Space.Y + Padding.Bottom + Padding.Top;
        ContentUIItem->SetHeight(ContentSize);
        CellTemplate->GetUIItem()->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), true, true);
    }

    CellTemplate->GetUIItem()->SetIsUIActive(true);
    float CellWidth;
    float CellHeight;
    if (Horizontal)
    {
        CellWidth = CellTemplate->GetUIItem()->GetWidth();
        CellHeight = (ContentUIItem->GetHeight() 
            - (Padding.Top + Padding.Bottom)//padding
            - (Rows - 1) * Space.Y//space
            ) / Rows;
    }
    else
    {
        CellWidth = (ContentUIItem->GetWidth()
            - (Padding.Left + Padding.Right)//padding
            - (Columns - 1) * Space.X//space
            ) / Columns;
        CellHeight = CellTemplate->GetUIItem()->GetHeight();
    }

    //create more cells
    FLGUIDuplicateDataContainer DuplicateData;
    if (CacheCellList.Num() < VisibleCellCount)
    {
        ULGUIBPLibrary::PrepareDuplicateData(CellTemplate.Get(), DuplicateData);
    }
    while (CacheCellList.Num() < VisibleCellCount)
    {
        auto CopiedCell = (AUIBaseActor*)ULGUIBPLibrary::DuplicateActorWithPreparedData(DuplicateData, ContentUIItem.Get());
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

    IUIRecyclableScrollViewDataSource::Execute_BeforeSetCell(DataSource);
    //set cell position and size and data
    float PosX = Padding.Left, PosY = -Padding.Top;
    int RowOrColumnIndex = 0;
    for (int i = 0; i < CacheCellList.Num(); i++)
    {
        auto& CellItem = CacheCellList[i];
        IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, i);
        if (Horizontal)
        {
            CellItem.UIItem->SetHeight(CellHeight);
            auto AnchoredPosition = FVector2D(
                PosX + CellItem.UIItem->GetPivot().X * CellWidth
                , PosY - (1.0f - CellItem.UIItem->GetPivot().Y) * CellHeight);
            CellItem.UIItem->SetAnchoredPosition(AnchoredPosition);
            RowOrColumnIndex++;
            if (RowOrColumnIndex >= Rows)
            {
                PosX += CellWidth + Space.X;
                PosY = -Padding.Top;
                RowOrColumnIndex = 0;
            }
            else
            {
                PosY -= CellHeight + Space.Y;
            }
        }
        else
        {
            CellItem.UIItem->SetWidth(CellWidth);
            auto AnchoredPosition = FVector2D(
                PosX + CellItem.UIItem->GetPivot().X * CellWidth
                , PosY - (1.0f - CellItem.UIItem->GetPivot().Y) * CellHeight);
            CellItem.UIItem->SetAnchoredPosition(AnchoredPosition);
            RowOrColumnIndex++;
            if (RowOrColumnIndex >= Columns)
            {
                PosY -= CellHeight + Space.Y;
                PosX = Padding.Left;
                RowOrColumnIndex = 0;
            }
            else
            {
                PosX += CellWidth + Space.X;
            }
        }
    }
    IUIRecyclableScrollViewDataSource::Execute_AfterSetCell(DataSource);

    auto PrevProgress = this->Progress;
    if (Horizontal)
    {
        this->SetScrollProgress(FVector2D(1.0f, PrevProgress.Y));
        MinCellPosition = Padding.Left;
    }
    else
    {
        this->SetScrollProgress(FVector2D(PrevProgress.X, 0.0f));
        MinCellPosition = -Padding.Top;
    }
    MinCellDataIndex = 0;

    PrevContentPosition = FVector2D(ContentUIItem->GetRelativeLocation().Y, ContentUIItem->GetRelativeLocation().Z);
    OnScrollEventDelegateHandle = this->RegisterScrollEvent(FLGUIVector2Delegate::CreateUObject(this, &UUIRecyclableScrollViewComponent::OnScrollCallback));
    //this->SetScrollProgress(PrevProgress);
}
void UUIRecyclableScrollViewComponent::OnScrollCallback(FVector2D value)
{
    if (Horizontal == Vertical)return;
    if (CacheCellList.Num() == 0)return;
    if (DataItemCount == 0)return;

    IUIRecyclableScrollViewDataSource::Execute_BeforeSetCell(DataSource);
    const auto ContentPosition = FVector2D(ContentUIItem->GetRelativeLocation().Y, ContentUIItem->GetRelativeLocation().Z);
    if (Horizontal)
    {
        float CellWidth = CellTemplate->GetUIItem()->GetWidth();
        auto PointToScrollViewSpaceOffset = ContentUIItem->GetRelativeLocation().Y;
        if (ContentPosition.X > PrevContentPosition.X)//scroll from left to right
        {
            while (MinCellDataIndex > 0)
            {
                int CellDataIndex = MinCellDataIndex - Rows;//flip data
                auto& RightTopCellItem = CacheCellList[MaxCellIndexInCacheCellList];
                auto CellLeftPointInScrollViewSpace = RightTopCellItem.UIItem->GetLocalSpaceLeft() + RightTopCellItem.UIItem->GetRelativeLocation().Y + PointToScrollViewSpaceOffset;
                if (CellLeftPointInScrollViewSpace > RangeArea.Y)//right item out of range
                {
                    for (int i = 0; i < Rows; i++)
                    {
                        auto& CellItem = CacheCellList[MaxCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.X = MinCellPosition - (CellWidth + Space.X);
                        Pos.X = Pos.X + CellItem.UIItem->GetPivot().X * CellWidth;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellDataIndex--;
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
                    MinCellPosition -= CellWidth + Space.X;
                }
                else
                {
                    break;
                }
            }
        }
        else if (ContentPosition.X < PrevContentPosition.X)//scroll from right to left
        {
            int RightCellIndexInData = MinCellDataIndex + CacheCellList.Num() - 1;
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
                        Pos.X = MinCellPosition + (CellWidth + Space.X) * (CacheCellList.Num() / Rows);
                        Pos.X = Pos.X + CellItem.UIItem->GetPivot().X * CellWidth;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellDataIndex++;
                        RightCellIndexInData = MinCellDataIndex + CacheCellList.Num() - 1;
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
                    MinCellPosition += CellWidth + Space.X;
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
            while (MinCellDataIndex > 0)
            {
                int CellDataIndex = MinCellDataIndex - Columns;//flip data
                auto& BottomLeftCellItem = CacheCellList[MaxCellIndexInCacheCellList];
                auto CellTopPointInScrollViewSpace = BottomLeftCellItem.UIItem->GetLocalSpaceTop() + BottomLeftCellItem.UIItem->GetRelativeLocation().Z + PointToScrollViewSpaceOffset;
                if (CellTopPointInScrollViewSpace < RangeArea.X)//bottom item out of range
                {
                    //move bottom to top
                    for (int i = 0; i < Columns; i++)
                    {
                        auto& CellItem = CacheCellList[MaxCellIndexInCacheCellList + i];
                        auto Pos = CellItem.UIItem->GetAnchoredPosition();
                        Pos.Y = MinCellPosition + (CellHeight + Space.Y);
                        Pos.Y = Pos.Y - (1.0f - CellItem.UIItem->GetPivot().Y) * CellHeight;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellDataIndex--;
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
                    MinCellPosition += CellHeight + Space.Y;
                }
                else//none out of range, no need recycle anything
                {
                    break;
                }
            }
        }
        else if (ContentPosition.Y > PrevContentPosition.Y)//scroll from bottom to top
        {
            int BottomCellIndexInData = MinCellDataIndex + CacheCellList.Num() - 1;
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
                        Pos.Y = MinCellPosition - (CellHeight + Space.Y) * (CacheCellList.Num() / Columns);
                        Pos.Y = Pos.Y - (1.0f - CellItem.UIItem->GetPivot().Y) * CellHeight;
                        CellItem.UIItem->SetAnchoredPosition(Pos);
                        //data index
                        MinCellDataIndex++;
                        BottomCellIndexInData = MinCellDataIndex + CacheCellList.Num() - 1;
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
                    MinCellPosition -= CellHeight + Space.Y;
                }
                else//none out of range, no need recycle anything
                {
                    break;
                }
            }
        }
    }
    IUIRecyclableScrollViewDataSource::Execute_AfterSetCell(DataSource);
    PrevContentPosition = ContentPosition;
}

void UUIRecyclableScrollViewComponent::UpdateCellData()
{
    if (!IsValid(DataSource))return;

    IUIRecyclableScrollViewDataSource::Execute_BeforeSetCell(DataSource);
    auto CellDataIndex = MinCellDataIndex;
    FUIRecyclableScrollViewCellContainer CellContainer;
    for (int i = 0; i < CacheCellList.Num(); i++)
    {
        GetCellItemByDataIndex(CellDataIndex, CellContainer);
        IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellContainer.CellComponent, CellDataIndex);
        CellDataIndex++;
        if (CellDataIndex >= DataItemCount)
        {
            break;
        }
    }
    IUIRecyclableScrollViewDataSource::Execute_AfterSetCell(DataSource);
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
