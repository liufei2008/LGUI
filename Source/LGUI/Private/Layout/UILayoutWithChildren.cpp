// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Layout/UILayoutWithChildren.h"
#include "Layout/UILayoutElement.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

UUILayoutWithChildren::UUILayoutWithChildren()
{
	bNeedRebuildChildrenList = true;
	bNeedSortChildrenList = true;
}

void UUILayoutWithChildren::Awake()
{
    Super::Awake();

    MarkNeedRebuildChildrenList();
}

void UUILayoutWithChildren::MarkNeedRebuildChildrenList()
{
    bNeedRebuildChildrenList = true;
}

void UUILayoutWithChildren::EnsureChildValid()
{
    for (int i = 0; i < LayoutUIItemChildrenArray.Num(); i++)
    {
        if (!LayoutUIItemChildrenArray[i].uiItem.IsValid())
        {
            LayoutUIItemChildrenArray.RemoveAt(i);
            i--;
        }
    }
}

void UUILayoutWithChildren::RebuildChildrenList()const
{
    if (CheckRootUIComponent())
    {
        LayoutUIItemChildrenArray.Reset();
        const auto& children = RootUIComp->GetAttachUIChildren();
        for (auto uiItem : children)
        {
            if (!IsValid(uiItem))continue;
            if (uiItem->GetIsUIActiveInHierarchy())
            {
                if (uiItem->GetOwner()->GetRootComponent() != uiItem)continue;//only use root component
                UActorComponent* layoutElement = nullptr;
                bool ignoreLayout = false;
                GetLayoutElement(uiItem->GetOwner(), layoutElement, ignoreLayout);
                if (ignoreLayout)
                {
                    continue;
                }

                FAvaliableChild child;
                child.uiItem = uiItem;
                child.layoutElement = layoutElement;
                LayoutUIItemChildrenArray.Add(child);
            }
        }
        SortChildrenList();
    }
}

void UUILayoutWithChildren::SortChildrenList()const
{
    LayoutUIItemChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
        {
            if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                return true;
            return false;
        });
}

void UUILayoutWithChildren::GetLayoutElement(AActor* InActor, UActorComponent*& OutLayoutElement, bool& OutIgnoreLayout)const
{
    auto LayoutElement = InActor->FindComponentByClass<UUILayoutElement>();
    if (LayoutElement != nullptr && LayoutElement->GetEnable())
    {
        OutLayoutElement = LayoutElement;
        OutIgnoreLayout = ILGUILayoutElementInterface::Execute_GetLayoutType(OutLayoutElement) == ELayoutElementType::IgnoreLayout;
    }
}

const TArray<UUILayoutWithChildren::FAvaliableChild>& UUILayoutWithChildren::GetLayoutUIItemChildren()const
{
    if (bNeedRebuildChildrenList)
    {
        bNeedRebuildChildrenList = false;
        bNeedSortChildrenList = false;
        RebuildChildrenList();
    }
    else if (bNeedSortChildrenList)
    {
        bNeedSortChildrenList = false;
        SortChildrenList();
    }
    return LayoutUIItemChildrenArray;
}

void UUILayoutWithChildren::OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)
{
    Super::OnUIChildAcitveInHierarchy(InChild, InUIActive);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (InUIActive)
    {
        EnsureChildValid();
        if (!LayoutUIItemChildrenArray.Find(childData, index))
        {
            UActorComponent* layoutElement = nullptr;
            bool ignoreLayout = false;
            GetLayoutElement(InChild->GetOwner(), layoutElement, ignoreLayout);
            if (ignoreLayout)
            {
                return;
            }
            childData.layoutElement = layoutElement;
            LayoutUIItemChildrenArray.Add(childData);
            bNeedSortChildrenList = true;
        }
    }
    else
    {
        EnsureChildValid();
        if (LayoutUIItemChildrenArray.Find(childData, index))
        {
            LayoutUIItemChildrenArray.RemoveAt(index);
        }
    }

    MarkNeedRebuildLayout();
}
void UUILayoutWithChildren::OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)
{
    Super::OnUIChildAttachmentChanged(InChild, attachOrDetach);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (attachOrDetach && InChild->GetIsUIActiveInHierarchy())
    {
        EnsureChildValid();
        if (!LayoutUIItemChildrenArray.Find(childData, index))
        {
            UActorComponent* layoutElement = nullptr;
            bool ignoreLayout = false;
            GetLayoutElement(InChild->GetOwner(), layoutElement, ignoreLayout);
            if (ignoreLayout)
            {
                return;
            }
            childData.layoutElement = layoutElement;
            LayoutUIItemChildrenArray.Add(childData);
            bNeedSortChildrenList = true;
        }
    }
    else
    {
        if (LayoutUIItemChildrenArray.Find(childData, index))
        {
            LayoutUIItemChildrenArray.RemoveAt(index);
        }
    }

    MarkNeedRebuildLayout();
}
void UUILayoutWithChildren::OnUIChildHierarchyIndexChanged(UUIItem* InChild)
{
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    EnsureChildValid();
    if (LayoutUIItemChildrenArray.Find(childData, index))
    {
        bNeedSortChildrenList = true;
    }

    MarkNeedRebuildLayout();
}

void UUILayoutWithChildren::MarkRebuildLayout_Implementation()
{
    Super::MarkRebuildLayout_Implementation();
    MarkNeedRebuildChildrenList();
}
