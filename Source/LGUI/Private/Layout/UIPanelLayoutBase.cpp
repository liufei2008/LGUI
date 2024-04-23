// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UIPanelLayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

UUIPanelLayoutBase::UUIPanelLayoutBase()
{
}
void UUIPanelLayoutBase::GetLayoutElement(UUIItem* InChild, UObject*& OutLayoutElement, bool& OutIgnoreLayout)const
{
    UUIPanelLayoutSlotBase* LayoutElement = nullptr;
    if (auto LayoutElementPtr = MapChildToSlot.Find(InChild))
    {
        LayoutElement = *LayoutElementPtr;
    }
    else
    {
        LayoutElement = NewObject<UUIPanelLayoutSlotBase>(const_cast<UUIPanelLayoutBase*>(this), GetPanelLayoutSlotClass(), NAME_None, RF_Public | RF_Transactional);
        LayoutElement->SetDesiredSize(FVector2D(InChild->GetWidth(), InChild->GetHeight()));
        MapChildToSlot.Add(InChild, LayoutElement);
    }
    OutLayoutElement = LayoutElement;
    OutIgnoreLayout = LayoutElement->GetIgnoreLayout();
}
void UUIPanelLayoutBase::RebuildChildrenList()const
{
    Super::RebuildChildrenList();
    const_cast<UUIPanelLayoutBase*>(this)->CleanMapChildToSlot();
}
void UUIPanelLayoutBase::OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)
{
    Super::OnUIChildAcitveInHierarchy(InChild, InUIActive);
}
void UUIPanelLayoutBase::OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)
{
    Super::OnUIChildAttachmentChanged(InChild, attachOrDetach);
}
void UUIPanelLayoutBase::OnUIChildHierarchyIndexChanged(UUIItem* InChild)
{
    Super::OnUIChildHierarchyIndexChanged(InChild);
}
void UUIPanelLayoutBase::CleanMapChildToSlot()
{
    //check map if child not exist
    TSet<UUIItem*> KeysToRemove;
    const auto& UIChildren = RootUIComp->GetAttachUIChildren();
    for (auto& KeyValue : MapChildToSlot)
    {
        auto FoundIndex = UIChildren.IndexOfByKey(KeyValue.Key);
        if (FoundIndex == INDEX_NONE)
        {
            KeysToRemove.Add(KeyValue.Key);
        }
    }
    for (auto& Key : KeysToRemove)
    {
        MapChildToSlot.Remove(Key);
    }
}

void UUIPanelLayoutBase::Awake()
{
    Super::Awake();
}
TObjectPtr<UUIPanelLayoutSlotBase> UUIPanelLayoutBase::GetChildSlot(UUIItem* InChild)
{
    if (auto LayoutElementPtr = MapChildToSlot.Find(InChild))
    {
        return *LayoutElementPtr;
    }
    return nullptr;
}
#if WITH_EDITOR
FText UUIPanelLayoutBase::GetCategoryDisplayName()const
{
    return FText::FromString(this->GetClass()->GetName());
}
#endif

#if WITH_EDITOR
void UUIPanelLayoutSlotBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (auto Layout = GetTypedOuter<UUIPanelLayoutBase>())
    {
        Layout->MarkNeedRebuildLayout();
        Layout->MarkNeedRebuildChildrenList();
    }
}
void UUIPanelLayoutSlotBase::PostEditUndo()
{
    Super::PostEditUndo();
    if (auto Layout = GetTypedOuter<UUIPanelLayoutBase>())
    {
        Layout->MarkNeedRebuildLayout();
        Layout->MarkNeedRebuildChildrenList();
    }
}
#endif
void UUIPanelLayoutSlotBase::SetIgnoreLayout(bool Value)
{
    if (bIgnoreLayout != Value)
    {
        bIgnoreLayout = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayoutBase>())
        {
            Layout->MarkNeedRebuildLayout();
            Layout->MarkNeedRebuildChildrenList();
        }
    }
}
void UUIPanelLayoutSlotBase::SetDesiredSize(const FVector2D& Value)
{
    if (DesiredSize != Value)
    {
        DesiredSize = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayoutBase>())
        {
            Layout->MarkNeedRebuildLayout();
        }
    }
}

void UUIPanelLayoutWithOverrideOrder::SortChildrenList()const
{
    LayoutUIItemChildrenArray.Sort([](FLayoutChild A, FLayoutChild B) //sort children by HierarchyIndex
        {
            auto ASlot = Cast<UUIPanelLayoutSlotWithOverrideOrder>(A.LayoutInterface);
            auto BSlot = Cast<UUIPanelLayoutSlotWithOverrideOrder>(B.LayoutInterface);
            if (ASlot && BSlot)
            {
                if (ASlot->GetOverrideLayoutOrder() < BSlot->GetOverrideLayoutOrder())
                {
                    return true;
                }
                else if (ASlot->GetOverrideLayoutOrder() > BSlot->GetOverrideLayoutOrder())
                {
                    return false;
                }
                //else use HierarchyIndex
            }
            else
            {
                if (ASlot && ASlot->GetOverrideLayoutOrder() != 0)
                {
                    return ASlot->GetOverrideLayoutOrder() < 0;
                }
                if (BSlot && BSlot->GetOverrideLayoutOrder() != 0)
                {
                    return BSlot->GetOverrideLayoutOrder() < 0;
                }
            }
            if (A.ChildUIItem->GetHierarchyIndex() < B.ChildUIItem->GetHierarchyIndex())
                return true;
            return false;
        });
}
void UUIPanelLayoutSlotWithOverrideOrder::SetOverrideLayoutOrder(int Value)
{
    if (OverrideLayoutOrder != Value)
    {
        OverrideLayoutOrder = Value;
        if (auto Layout = GetTypedOuter<UUIPanelLayoutBase>())
        {
            Layout->MarkNeedRebuildLayout();
            Layout->MarkNeedSortChildrenList();
        }
    }
}
