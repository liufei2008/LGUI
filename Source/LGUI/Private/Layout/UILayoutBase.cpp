// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UILayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"

void UUILayoutBase::Awake()
{
    Super::Awake();

    //recreate list at start
    RebuildChildrenList();

    OnRebuildLayout();
}

#if WITH_EDITOR
void UUILayoutBase::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    OnRebuildLayout();
    if (CheckRootUIComponent())
    {
        RootUIComp->EditorForceUpdateImmediately();
    }
}
#endif

void UUILayoutBase::OnRegister()
{
    Super::OnRegister();
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        RebuildChildrenList();
    }
#endif
}
void UUILayoutBase::OnUnregister()
{
    Super::OnUnregister();
}

void UUILayoutBase::RebuildChildrenList()
{
    if (CheckRootUIComponent())
    {
        availableChildrenArray.Reset();
        const auto &children = RootUIComp->GetAttachUIChildren();
        for (auto uiItem : children)
        {
            if (uiItem->IsUIActiveInHierarchy())
            {
                UUILayoutElement *layoutElement = GetLayoutElement(uiItem->GetOwner());
                if (layoutElement)
                {
                    if (layoutElement->GetIgnoreLayout())
                    {
                        continue;
                    }
                }
                FAvaliableChild child;
                child.uiItem = uiItem;
                child.layoutElement = layoutElement;
                availableChildrenArray.Add(child);
                OnAttachValidChild(uiItem);
            }
        }
        availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                                    {
                                        if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                                            return true;
                                        return false;
                                    });
    }
}

void UUILayoutBase::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    if (sizeChanged)
        OnRebuildLayout();
}
void UUILayoutBase::OnUIAttachmentChanged()
{
    Super::OnUIAttachmentChanged();
    OnRebuildLayout();
}
void UUILayoutBase::OnUIActiveInHierachy(bool activeOrInactive)
{
    Super::OnUIActiveInHierachy(activeOrInactive);
    OnRebuildLayout();
}
void UUILayoutBase::OnUIChildAcitveInHierarchy(UUIItem *InChild, bool InUIActive)
{
    Super::OnUIChildAcitveInHierarchy(InChild, InUIActive);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (InUIActive)
    {
        if (!availableChildrenArray.Find(childData, index))
        {
            UUILayoutElement *layoutElement = GetLayoutElement(InChild->GetOwner());
            if (layoutElement)
            {
                if (layoutElement->GetIgnoreLayout())
                {
                    return;
                }
            }
            childData.layoutElement = layoutElement;
            availableChildrenArray.Add(childData);

            availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                                        {
                                            if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                                                return true;
                                            return false;
                                        });
        }
    }
    else
    {
        if (availableChildrenArray.Find(childData, index))
        {
            availableChildrenArray.RemoveAt(index);
        }
    }

    OnRebuildLayout();
}
void UUILayoutBase::OnUIChildAttachmentChanged(UUIItem *InChild, bool attachOrDetach)
{
    Super::OnUIChildAttachmentChanged(InChild, attachOrDetach);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (attachOrDetach && InChild->IsUIActiveInHierarchy())
    {
        if (!availableChildrenArray.Find(childData, index))
        {
            UUILayoutElement *layoutElement = GetLayoutElement(InChild->GetOwner());
            if (layoutElement)
            {
                if (layoutElement->GetIgnoreLayout())
                {
                    return;
                }
            }
            childData.layoutElement = layoutElement;
            availableChildrenArray.Add(childData);
            //sort by hierarchy index
            availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                                        {
                                            if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                                                return true;
                                            return false;
                                        });
            OnAttachValidChild(InChild);
        }
    }
    else
    {
        if (availableChildrenArray.Find(childData, index))
        {
            availableChildrenArray.RemoveAt(index);
            OnDetachValidChild(InChild);
        }
    }

    OnRebuildLayout();
}
void UUILayoutBase::OnUIChildHierarchyIndexChanged(UUIItem *InChild)
{
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (availableChildrenArray.Find(childData, index))
    {
        availableChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                                    {
                                        if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                                            return true;
                                        return false;
                                    });
    }

    OnRebuildLayout();
}

FORCEINLINE UUILayoutElement *UUILayoutBase::GetLayoutElement(AActor *Target)
{
    return Target->FindComponentByClass<UUILayoutElement>();
}