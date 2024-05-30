// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIRecyclableScrollViewComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "LGUIBPLibrary.h"
#include "LTweenManager.h"
#include "Core/LGUISettings.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
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
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, OnlyOneDirection))
        {
            OnlyOneDirection = true;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, bInfiniteLoop))
        {
            RestrictRectArea = !bInfiniteLoop;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, Rows))
        {
            if (Horizontal)
            {
                if (Rows != 1)
                {
                    bInfiniteLoop = false;
                }
            }
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, Columns))
        {
            if (Vertical)
            {
                if (Columns != 1)
                {
                    bInfiniteLoop = false;
                }
            }
        }
    }
}
bool UUIRecyclableScrollViewComponent::CanEditChange(const FProperty* InProperty)const
{
    if (Super::CanEditChange(InProperty))
    {
        auto PropertyName = InProperty->GetFName();
        if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, CellTemplate))
        {
            return CellTemplateType == EUIRecyclableScrollViewCellTemplateType::Actor;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, CellTemplatePrefab))
        {
            return CellTemplateType == EUIRecyclableScrollViewCellTemplateType::Prefab;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, OnlyOneDirection))
        {
            return false;
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, bInfiniteLoop))
        {
            if (Horizontal)
            {
                return Rows == 1;
            }
            if (Vertical)
            {
                return Columns == 1;
            }
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRecyclableScrollViewComponent, RestrictRectArea))
        {
            return !bInfiniteLoop;
        }
        return true;
    }
    return false;
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
    auto InDataSourceObject = InDataSource.GetObject();
    if (!IsValid(InDataSourceObject))
    {
        DataSource = nullptr;
        InitializeOnDataSource();
        return;
    }
    if (DataSource != InDataSourceObject)
    {
        DataSource = InDataSourceObject;
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
void UUIRecyclableScrollViewComponent::SetInfiniteLoop(bool value)
{
    if ((Horizontal && Rows != 1) || (Vertical && Columns != 1))
    {
        UE_LOG(LGUI, Error, TEXT("[%s] InfiniteLoop only work when Rows and Columns equals 1"), ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }
    if (value != bInfiniteLoop)
    {
        bInfiniteLoop = value;
        RestrictRectArea = false;
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
    auto ValidMinCellDataIndex = GetValidCellDataIndex(MinCellDataIndex);
    if (Index < ValidMinCellDataIndex || Index > MaxCellIndexInData)
    {
        return false;
    }
    else
    {
        auto CellIndexOffset = Index - ValidMinCellDataIndex;
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
            UE_LOG(LGUI, Error, TEXT("[%s] Wrong cell index. Index:%d, CellIndexOffset:%d, CellIndex:%d"), ANSI_TO_TCHAR(__FUNCTION__), Index, CellIndexOffset, CellIndex);
            ensure(false);
            return false;
        }
    }
}

void UUIRecyclableScrollViewComponent::ScrollToByDataIndex(int InDataIndex, bool InEaseAnimation, float InAnimationDuration)
{
    if (Horizontal == Vertical)return;
    if (CacheCellList.Num() == 0)return;
    if (DataItemCount == 0)return;
    if (InDataIndex < 0 || InDataIndex >= DataItemCount)
    {
        UE_LOG(LGUI, Warning, TEXT("[%s] Invalid InDataIndex:%d in range [0, %d]"), ANSI_TO_TCHAR(__FUNCTION__), InDataIndex, DataItemCount);
        return;
    }

    auto ValidMinCellDataIndex = GetValidCellDataIndex(MinCellDataIndex);
    if (Horizontal)
    {
        float CellWidth = WorkingCellTemplateSize.X;
        float StartPos = MinCellPosition + CellWidth * 0.5f;//start cell center horizontal position
        float TargetContentPos = StartPos;
        if (InDataIndex == ValidMinCellDataIndex)
        {

        }
        else if (InDataIndex > ValidMinCellDataIndex)//data index bigger than current minimal cell
        {
            for (int StartIndex = ValidMinCellDataIndex + 1; StartIndex <= InDataIndex; StartIndex += Rows)
            {
                TargetContentPos += CellWidth + Space.X;
            }
        }
        else if (InDataIndex < ValidMinCellDataIndex)
        {
            for (int StartIndex = ValidMinCellDataIndex - 1; StartIndex >= InDataIndex; StartIndex -= Rows)
            {
                TargetContentPos -= CellWidth + Space.X;
            }
        }

        TargetContentPos = FMath::Clamp(-TargetContentPos, HorizontalRange.X, HorizontalRange.Y);
        if (InEaseAnimation)
        {
            auto tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this
                , [this] {
                    auto ContentLocation = ContentUIItem->GetRelativeLocation();
                    return ContentLocation.Y;
                })
                , FLTweenFloatSetterFunction::CreateWeakLambda(this, [this](float value) {
                    this->SetScrollValue(FVector2D(value, 0));
                    }), TargetContentPos, InAnimationDuration);
            if (tweener)
            {
                bool bAffectByGamePause = false;
                if (this->GetRootUIComponent())
                {
                    if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
                    {
                        bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
                    }
                    else
                    {
                        bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
                    }
                }
                tweener->SetAffectByGamePause(bAffectByGamePause);
            }
        }
        else
        {
            SetScrollValue(FVector2D(TargetContentPos, 0));
        }
    }
    else if (Vertical)
    {
        float CellHeight = WorkingCellTemplateSize.Y;
        float StartPos = MinCellPosition - CellHeight * 0.5f;//start cell center vertical position
        float TargetContentPos = StartPos;
        if (InDataIndex == ValidMinCellDataIndex)
        {

        }
        else if (InDataIndex > ValidMinCellDataIndex)
        {
            for (int StartIndex = ValidMinCellDataIndex + 1; StartIndex <= InDataIndex; StartIndex += Columns)
            {
                TargetContentPos -= CellHeight + Space.Y;
            }
        }
        else if (InDataIndex < ValidMinCellDataIndex)
        {
            for (int StartIndex = ValidMinCellDataIndex - 1; StartIndex >= InDataIndex; StartIndex -= Columns)
            {
                TargetContentPos += CellHeight + Space.X;
            }
        }

        TargetContentPos = FMath::Clamp(-TargetContentPos, VerticalRange.X, VerticalRange.Y);
        if (InEaseAnimation)
        {
            auto tweener = ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateWeakLambda(this
                , [this] {
                    auto ContentLocation = ContentUIItem->GetRelativeLocation();
                    return ContentLocation.Z;
                })
                , FLTweenFloatSetterFunction::CreateWeakLambda(this, [this](float value) {
                    this->SetScrollValue(FVector2D(0, value));
                    }), TargetContentPos, InAnimationDuration);
            if (tweener)
            {
                bool bAffectByGamePause = false;
                if (this->GetRootUIComponent())
                {
                    if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
                    {
                        bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
                    }
                    else
                    {
                        bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
                    }
                }
                tweener->SetAffectByGamePause(bAffectByGamePause);
            }
        }
        else
        {
            SetScrollValue(FVector2D(0, TargetContentPos));
        }
    }
}

void UUIRecyclableScrollViewComponent::SetCellTemplate(AUIBaseActor* value)
{
    if (CellTemplate != value)
    {
        CellTemplate = value;
    }
}

void UUIRecyclableScrollViewComponent::SetCellTemplatePrefab(class ULGUIPrefab* value)
{
    if (CellTemplatePrefab != value)
    {
        CellTemplatePrefab = value;
        if (WorkingCellTemplateType == EUIRecyclableScrollViewCellTemplateType::Prefab)//if WorkingCellTemplate is created by prefab, then we need to destroy it so a new one will be created from new prefab
        {
            ULGUIBPLibrary::DestroyActorWithHierarchy(WorkingCellTemplate.Get());
        }
    }
}

void UUIRecyclableScrollViewComponent::InitializeOnDataSource()
{
    if (!IsValid(DataSource))return;
    if (!CheckParameters())return;
    if (Horizontal == Vertical)return;
    DataItemCount = IUIRecyclableScrollViewDataSource::Execute_GetItemCount(DataSource);

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

    switch (CellTemplateType)
    {
    default:
    case EUIRecyclableScrollViewCellTemplateType::Actor:
    {
        if (!IsValid(CellTemplate))return;
        WorkingCellTemplate = CellTemplate;
        if (GetComponentByInterface(WorkingCellTemplate.Get(), UUIRecyclableScrollViewCell::StaticClass()) == nullptr)
        {
            UE_LOG(LGUI, Error, TEXT("[%s] CellTemplate's root actor must have a ActorComponent which implement UIRecyclableScrollViewCell interface!"), ANSI_TO_TCHAR(__FUNCTION__));
            return;
        }
        WorkingCellTemplateType = EUIRecyclableScrollViewCellTemplateType::Actor;
    }
        break;
    case EUIRecyclableScrollViewCellTemplateType::Prefab:
    {
        if (!IsValid(CellTemplatePrefab))return;
        if (WorkingCellTemplateType != EUIRecyclableScrollViewCellTemplateType::Prefab || !WorkingCellTemplate.IsValid())//WorkingCellTemplate is already created by prefab
        {
            auto CellTemplateInstance = CellTemplatePrefab->LoadPrefab(this->GetWorld(), ContentUIItem.Get());
            WorkingCellTemplate = Cast<AUIBaseActor>(CellTemplateInstance);
        }
        if (!WorkingCellTemplate.IsValid())
        {
            ULGUIBPLibrary::DestroyActorWithHierarchy(WorkingCellTemplate.Get());
            UE_LOG(LGUI, Error, TEXT("[%s] CellTemplatePrefab's root actor must be a UI actor!"), ANSI_TO_TCHAR(__FUNCTION__));
            return;
        }
        if (GetComponentByInterface(WorkingCellTemplate.Get(), UUIRecyclableScrollViewCell::StaticClass()) == nullptr)
        {
            ULGUIBPLibrary::DestroyActorWithHierarchy(WorkingCellTemplate.Get());
            UE_LOG(LGUI, Error, TEXT("[%s] CellTemplatePrefab's root actor must have a ActorComponent which implement UIRecyclableScrollViewCell interface!"), ANSI_TO_TCHAR(__FUNCTION__));
            return;
        }
        WorkingCellTemplateType = EUIRecyclableScrollViewCellTemplateType::Prefab;
    }
        break;
    }
    WorkingCellTemplateSize.X = WorkingCellTemplate->GetUIItem()->GetWidth();
    WorkingCellTemplateSize.Y = WorkingCellTemplate->GetUIItem()->GetHeight();


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
        float CellWidth = WorkingCellTemplateSize.X;
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
    }
    else
    {
        RangeArea.X = ContentParentUIItem->GetLocalSpaceBottom();
        RangeArea.Y = ContentParentUIItem->GetLocalSpaceTop();
        float RangeSize = RangeArea.Y - RangeArea.X - (Padding.Bottom + Padding.Top);
        float AllVisibleCellHeight = 0;
        float CellHeight = WorkingCellTemplateSize.Y;
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
    }
    WorkingCellTemplate->GetUIItem()->SetHorizontalAndVerticalAnchorMinMax(FVector2D(0.0f, 1.0f), FVector2D(0.0f, 1.0f), true, true);

    WorkingCellTemplate->GetUIItem()->SetIsUIActive(true);
    float CellWidth;
    float CellHeight;
    if (Horizontal)
    {
        CellWidth = WorkingCellTemplateSize.X;
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
        CellHeight = WorkingCellTemplateSize.Y;
    }

    //create more cells
    FLGUIDuplicateDataContainer DuplicateData;
    if (CacheCellList.Num() < VisibleCellCount)
    {
        ULGUIBPLibrary::PrepareDuplicateData(WorkingCellTemplate.Get(), DuplicateData);
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
    WorkingCellTemplate->GetUIItem()->SetIsUIActive(false);
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
    auto PosX = Padding.Left, PosY = -Padding.Top;
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
        auto CellWidth = WorkingCellTemplateSize.X;
        auto PointToScrollViewSpaceOffset = ContentUIItem->GetRelativeLocation().Y;
        if (ContentPosition.X > PrevContentPosition.X)//scroll from left to right
        {
            while (MinCellDataIndex > 0 || (bInfiniteLoop && Rows == 1))
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
                        CellDataIndex = GetValidCellDataIndex(CellDataIndex + i);
                        if (CellDataIndex < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, CellDataIndex);
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
            int RightCellIndexInData = GetValidCellDataIndex(MinCellDataIndex) + CacheCellList.Num() - 1;
            while (RightCellIndexInData + 1 < DataItemCount || (bInfiniteLoop && Columns == 1))//check if right cell reach end data
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
                        RightCellIndexInData = GetValidCellDataIndex(RightCellIndexInData);
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
        auto CellHeight = WorkingCellTemplateSize.Y;
        auto PointToScrollViewSpaceOffset = ContentUIItem->GetRelativeLocation().Z;
        if (ContentPosition.Y < PrevContentPosition.Y)//scroll from top to bottom
        {
            while (MinCellDataIndex > 0 || (bInfiniteLoop && Columns == 1))
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
                        CellDataIndex = GetValidCellDataIndex(CellDataIndex + i);
                        if (CellDataIndex < DataItemCount)
                        {
                            CellItem.UIItem->SetIsUIActive(true);
                            IUIRecyclableScrollViewDataSource::Execute_SetCell(DataSource, CellItem.CellComponent, CellDataIndex);
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
            int BottomCellIndexInData = GetValidCellDataIndex(MinCellDataIndex) + CacheCellList.Num() - 1;
            while (BottomCellIndexInData + 1 < DataItemCount || (bInfiniteLoop && Columns == 1))//check if bottom cell reach end data
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
                        BottomCellIndexInData = GetValidCellDataIndex(BottomCellIndexInData);
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

void UUIRecyclableScrollViewComponent::ApplyContentPositionWithProgress()
{
    Super::ApplyContentPositionWithProgress();
    OnScrollCallback(FVector2D::ZeroVector);
}

void UUIRecyclableScrollViewComponent::UpdateCellData()
{
    if (!IsValid(DataSource))return;

    IUIRecyclableScrollViewDataSource::Execute_BeforeSetCell(DataSource);
    auto CellDataIndex = GetValidCellDataIndex(MinCellDataIndex);
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

// Infinite loop could use out-of-range index, so use this to get a valid index
int UUIRecyclableScrollViewComponent::GetValidCellDataIndex(int InMinCellDataIndex)const
{
    auto TempMinCellDataIndex = InMinCellDataIndex;
    while (TempMinCellDataIndex < 0)
    {
        TempMinCellDataIndex += DataItemCount;
    }
    while (TempMinCellDataIndex >= DataItemCount)
    {
        TempMinCellDataIndex -= DataItemCount;
    }
    return TempMinCellDataIndex;
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
UE_ENABLE_OPTIMIZATION
#endif
