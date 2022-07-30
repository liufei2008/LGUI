// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UILayoutBase.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Layout/UILayoutElement.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Utils/LGUIUtils.h"

void UUILayoutBase::Awake()
{
    Super::Awake();

    MarkNeedRebuildChildrenList();
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
#if WITH_EDITOR
    if (!GetWorld()->IsGameWorld())
    {
        MarkNeedRebuildChildrenList();
        ULGUIEditorManagerObject::RegisterLGUILayout(this);
    }
    else
#endif
    {
        ALGUIManagerActor::RegisterLGUILayout(this);
    }
}
void UUILayoutBase::OnUnregister()
{
    Super::OnUnregister();
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

void UUILayoutBase::OnUpdateLayout_Implementation()
{
    if (bNeedRebuildLayout && GetIsActiveAndEnable())
    {
        bNeedRebuildLayout = false;
        OnRebuildLayout();
    }
}

void UUILayoutBase::EnsureChildValid()
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

void UUILayoutBase::RebuildChildrenList()const
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
        LayoutUIItemChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
            {
                if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                    return true;
                return false;
            });
    }
}

void UUILayoutBase::GetLayoutElement(AActor* InActor, UActorComponent*& OutLayoutElement, bool& OutIgnoreLayout)const
{
    OutLayoutElement = InActor->FindComponentByClass<UUILayoutElement>();
    if (OutLayoutElement)
    {
        OutIgnoreLayout = ILGUILayoutElementInterface::Execute_GetIgnoreLayout(OutLayoutElement);
    }
}

const TArray<UUILayoutBase::FAvaliableChild>& UUILayoutBase::GetLayoutUIItemChildren()const
{
    if(bNeedRebuildChildrenList)
    {
        bNeedRebuildChildrenList = false;
        RebuildChildrenList();
    }
    return LayoutUIItemChildrenArray;
}

void UUILayoutBase::MarkNeedRebuildChildrenList()
{
    bNeedRebuildChildrenList = true;
}
void UUILayoutBase::MarkNeedRebuildLayout()
{
    bNeedRebuildLayout = true; 
    if (auto World = this->GetWorld())
    {
#if WITH_EDITOR
        if (!World->IsGameWorld())
        {

        }
        else
#endif
        {
            ALGUIManagerActor::MarkUpdateLayout(World);
        }
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

            LayoutUIItemChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                {
                    if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                        return true;
                    return false;
                });
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
void UUILayoutBase::OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)
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
            //sort by hierarchy index
            LayoutUIItemChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
                {
                    if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                        return true;
                    return false;
                });
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
void UUILayoutBase::OnUIChildHierarchyIndexChanged(UUIItem* InChild)
{
    int32 index;
    FAvaliableChild childData;
    childData.uiItem = InChild;
    EnsureChildValid();
    if (LayoutUIItemChildrenArray.Find(childData, index))
    {
        LayoutUIItemChildrenArray.Sort([](FAvaliableChild A, FAvaliableChild B) //sort children by HierarchyIndex
            {
                if (A.uiItem->GetHierarchyIndex() < B.uiItem->GetHierarchyIndex())
                    return true;
                return false;
            });
    }

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
