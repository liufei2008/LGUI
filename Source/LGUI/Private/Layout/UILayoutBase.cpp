﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UILayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Core/Actor/LGUIManagerActor.h"

void UUILayoutBase::Awake()
{
    Super::Awake();

    //recreate list at start
    RebuildChildrenList();

    MarkNeedRebuildLayout();

    this->SetCanExecuteUpdate(false);
}

void UUILayoutBase::OnEnable()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        RebuildChildrenList();
        ULGUIEditorManagerObject::RegisterLGUILayout(this);
    }
    else
#endif
    {
        ALGUIManagerActor::RegisterLGUILayout(this);
    }
}
void UUILayoutBase::OnDisable()
{
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        ULGUIEditorManagerObject::UnregisterLGUILayout(this);
    }
    else
#endif
    {
        ALGUIManagerActor::UnregisterLGUILayout(this);
    }
}

#if WITH_EDITOR
void UUILayoutBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    MarkNeedRebuildLayout();
    if (CheckRootUIComponent())
    {
        RootUIComp->EditorForceUpdateImmediately();
    }
}
#endif

void UUILayoutBase::OnRegister()
{
    Super::OnRegister();
}
void UUILayoutBase::OnUnregister()
{
    Super::OnUnregister();
}

void UUILayoutBase::OnUpdateLayout_Implementation()
{
    if (bNeedRebuildLayout)
    {
        bNeedRebuildLayout = false;
        OnRebuildLayout();
    }
}

void UUILayoutBase::RebuildChildrenList()
{
    if (CheckRootUIComponent())
    {
        availableChildrenArray.Reset();
        const auto& children = RootUIComp->GetAttachUIChildren();
        for (auto uiItem : children)
        {
            if (uiItem->GetIsUIActiveInHierarchy())
            {
                if (uiItem->GetOwner()->GetRootComponent() != uiItem)continue;//only use root component
                UUILayoutElement* layoutElement = GetLayoutElement(uiItem->GetOwner());
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
    MarkNeedRebuildLayout();
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
void UUILayoutBase::OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)
{
    Super::OnUIChildAcitveInHierarchy(InChild, InUIActive);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (InUIActive)
    {
        if (!availableChildrenArray.Find(childData, index))
        {
            UUILayoutElement* layoutElement = GetLayoutElement(InChild->GetOwner());
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

    MarkNeedRebuildLayout();
}
void UUILayoutBase::OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)
{
    Super::OnUIChildAttachmentChanged(InChild, attachOrDetach);
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    if (attachOrDetach && InChild->GetIsUIActiveInHierarchy())
    {
        if (!availableChildrenArray.Find(childData, index))
        {
            UUILayoutElement* layoutElement = GetLayoutElement(InChild->GetOwner());
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
        }
    }
    else
    {
        if (availableChildrenArray.Find(childData, index))
        {
            availableChildrenArray.RemoveAt(index);
        }
    }

    MarkNeedRebuildLayout();
}
void UUILayoutBase::OnUIChildHierarchyIndexChanged(UUIItem* InChild)
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

    MarkNeedRebuildLayout();
}

FORCEINLINE UUILayoutElement* UUILayoutBase::GetLayoutElement(AActor* Target)
{
    return Target->FindComponentByClass<UUILayoutElement>();
}