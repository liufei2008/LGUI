// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UILayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Core/LGUIManager.h"
#include "Utils/LGUIUtils.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

UUILayoutBase::UUILayoutBase()
{
    bNeedRebuildLayout = true;
}
void UUILayoutBase::Awake()
{
    Super::Awake();

    MarkNeedRebuildLayout();

    this->SetCanExecuteUpdate(false);
}

void UUILayoutBase::OnEnable()
{
    Super::OnEnable();
}
void UUILayoutBase::OnDisable()
{
    Super::OnDisable();
}

#if WITH_EDITOR
void UUILayoutBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    MarkNeedRebuildLayout();
    if (CheckRootUIComponent())
    {
        RootUIComp->EditorForceUpdate();
    }
}
void UUILayoutBase::PostEditUndo()
{
    Super::PostEditUndo();
}
#endif

void UUILayoutBase::OnRegister()
{
    Super::OnRegister();
    ULGUIManagerWorldSubsystem::RegisterLGUILayout(this);
}
void UUILayoutBase::OnUnregister()
{
    Super::OnUnregister();
    ULGUIManagerWorldSubsystem::UnregisterLGUILayout(this);
}

void UUILayoutBase::OnUpdateLayout_Implementation()
{
    if (bNeedRebuildLayout && GetIsActiveAndEnable())
    {
        bNeedRebuildLayout = false;
        OnRebuildLayout();
    }
}



void UUILayoutBase::MarkNeedRebuildLayout()
{
    bNeedRebuildLayout = true; 
    ULGUIManagerWorldSubsystem::MarkUpdateLayout(this->GetWorld());
}

void UUILayoutBase::OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    Super::OnUIDimensionsChanged(horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (horizontalPositionChanged || verticalPositionChanged || widthChanged || heightChanged)
    {
        MarkNeedRebuildLayout();
    }
}
void UUILayoutBase::OnUIAttachmentChanged()
{
    Super::OnUIAttachmentChanged();
    MarkNeedRebuildLayout();
}
void UUILayoutBase::OnUIActiveInHierachy(bool activeOrInactive)
{
    Super::OnUIActiveInHierachy(activeOrInactive);
    MarkNeedRebuildLayout();
}

void UUILayoutBase::ApplyUIItemWidth(UUIItem* InUIItem, const float& InWidth)
{
#if WITH_EDITOR
    auto bIsValueDirty = InUIItem->GetWidth() != InWidth;
#endif
    InUIItem->SetWidth(InWidth);
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld() && bIsValueDirty)
    {
        LGUIUtils::NotifyPropertyChanged(InUIItem, UUIItem::GetAnchorDataPropertyName());
    }
#endif
}
void UUILayoutBase::ApplyUIItemHeight(UUIItem* InUIItem, const float& InHeight)
{
#if WITH_EDITOR
    auto bIsValueDirty = InUIItem->GetHeight() != InHeight;
#endif
    InUIItem->SetHeight(InHeight);
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld() && bIsValueDirty)
    {
        LGUIUtils::NotifyPropertyChanged(InUIItem, UUIItem::GetAnchorDataPropertyName());
    }
#endif
}
void UUILayoutBase::ApplyUIItemAnchoredPosition(UUIItem* InUIItem, const FVector2D& InAnchoredPosition)
{
#if WITH_EDITOR
    auto bIsValueDirty = InUIItem->GetAnchoredPosition() != InAnchoredPosition;
#endif
    InUIItem->SetAnchoredPosition(InAnchoredPosition);
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld() && bIsValueDirty)
    {
        LGUIUtils::NotifyPropertyChanged(InUIItem, UUIItem::GetAnchorDataPropertyName());
    }
#endif
}
void UUILayoutBase::ApplyUIItemSizeDelta(UUIItem* InUIItem, const FVector2D& InSizedDelta)
{
#if WITH_EDITOR
    auto bIsValueDirty = InUIItem->GetSizeDelta() != InSizedDelta;
#endif
    InUIItem->SetSizeDelta(InSizedDelta);
#if WITH_EDITOR
    if (!this->GetWorld()->IsGameWorld() && bIsValueDirty)
    {
        LGUIUtils::NotifyPropertyChanged(InUIItem, UUIItem::GetAnchorDataPropertyName());
    }
#endif
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif

