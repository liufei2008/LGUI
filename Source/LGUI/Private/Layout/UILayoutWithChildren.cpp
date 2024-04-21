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
        if (!LayoutUIItemChildrenArray[i].ChildUIItem.IsValid())
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
        const auto& UIChildren = RootUIComp->GetAttachUIChildren();
        for (auto UIChild : UIChildren)
        {
            if (!IsValid(UIChild))continue;
            if (UIChild->GetIsUIActiveInHierarchy())
            {
                if (UIChild->GetOwner()->GetRootComponent() != UIChild)continue;//only use root component
                UObject* layoutElement = nullptr;
                bool ignoreLayout = false;
                GetLayoutElement(UIChild, layoutElement, ignoreLayout);
                if (ignoreLayout)
                {
                    continue;
                }

                FLayoutChild child;
                child.ChildUIItem = UIChild;
                child.LayoutInterface = layoutElement;
                LayoutUIItemChildrenArray.Add(child);
            }
        }
        SortChildrenList();
    }
}

void UUILayoutWithChildren::SortChildrenList()const
{
    LayoutUIItemChildrenArray.Sort([](FLayoutChild A, FLayoutChild B) //sort children by HierarchyIndex
        {
            if (A.ChildUIItem->GetHierarchyIndex() < B.ChildUIItem->GetHierarchyIndex())
                return true;
            return false;
        });
}

void UUILayoutWithChildren::GetLayoutElement(UUIItem* InChild, UObject*& OutLayoutElement, bool& OutIgnoreLayout)const
{
    auto LayoutElement = InChild->GetOwner()->FindComponentByClass<UUILayoutElement>();
    if (LayoutElement != nullptr && LayoutElement->GetEnable())
    {
        OutLayoutElement = LayoutElement;
        OutIgnoreLayout = ILGUILayoutElementInterface::Execute_GetLayoutType(OutLayoutElement) == ELayoutElementType::IgnoreLayout;
    }
}

const TArray<UUILayoutWithChildren::FLayoutChild>& UUILayoutWithChildren::GetLayoutUIItemChildren()const
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
    FLayoutChild childData;
    childData.ChildUIItem = InChild;
    if (InUIActive)
    {
        EnsureChildValid();
        if (!LayoutUIItemChildrenArray.Find(childData, index))
        {
            UObject* layoutElement = nullptr;
            bool ignoreLayout = false;
            GetLayoutElement(InChild, layoutElement, ignoreLayout);
            if (ignoreLayout)
            {
                return;
            }
            childData.LayoutInterface = layoutElement;
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
    FLayoutChild childData;
    childData.ChildUIItem = InChild;
    if (attachOrDetach && InChild->GetIsUIActiveInHierarchy())
    {
        EnsureChildValid();
        if (!LayoutUIItemChildrenArray.Find(childData, index))
        {
            UObject* layoutElement = nullptr;
            bool ignoreLayout = false;
            GetLayoutElement(InChild, layoutElement, ignoreLayout);
            if (ignoreLayout)
            {
                return;
            }
            childData.LayoutInterface = layoutElement;
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
    FLayoutChild childData;
    childData.ChildUIItem = InChild;
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
