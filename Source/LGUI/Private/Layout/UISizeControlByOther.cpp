// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByOther.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/Actor/UIBaseActor.h"

void UUISizeControlByOther::SetControlWidth(bool value)
{
    if (ControlWidth != value)
    {
        ControlWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUISizeControlByOther::SetAdditionalWidth(float value)
{
    if (AdditionalWidth != value)
    {
        AdditionalWidth = value;
        MarkNeedRebuildLayout();
    }
}
void UUISizeControlByOther::SetControlHeight(bool value)
{
    if (ControlHeight != value)
    {
        ControlHeight = value;
        MarkNeedRebuildLayout();
    }
}
void UUISizeControlByOther::SetAdditionalHeight(float value)
{
    if (AdditionalHeight != value)
    {
        AdditionalHeight = value;
        MarkNeedRebuildLayout();
    }
}

void UUISizeControlByOtherHelper::Awake()
{
    Super::Awake();
    this->SetCanExecuteUpdate(false);
}
void UUISizeControlByOtherHelper::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (sizeChanged)
            TargetComp->MarkNeedRebuildLayout();
    }
}

void UUISizeControlByOther::Awake()
{
    CheckTargetUIItem();
    Super::Awake();
}

void UUISizeControlByOther::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    CheckTargetUIItem();
}
void UUISizeControlByOther::OnUIAttachmentChanged()
{
    Super::OnUIAttachmentChanged();
    CheckTargetUIItem();
}

#if WITH_EDITOR
void UUISizeControlByOther::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    if (auto Property = PropertyChangedEvent.Property)
    {
        auto propertyName = Property->GetFName();
        if (propertyName == TEXT("TargetActor"))
        {
            //delete old
            if (HelperComp.IsValid())
            {
                HelperComp->DestroyComponent();
            }
            //create new
            if (TargetActor.IsValid())
            {
                HelperComp = NewObject<UUISizeControlByOtherHelper>(TargetActor.Get());
                HelperComp->TargetComp = this;
                HelperComp->RegisterComponent();
            }
        }
    }
}
#endif

bool UUISizeControlByOther::CheckTargetUIItem()
{
    if (TargetUIItem.IsValid())
        return true;
    if (!TargetActor.IsValid())
        return false;
    if (!TargetUIItem.IsValid())
    {
        if (auto tempUIItem = Cast<UUIItem>(TargetActor->GetRootComponent()))
        {
            TargetUIItem = tempUIItem;
            if (HelperComp == nullptr)
            {
                HelperComp = NewObject<UUISizeControlByOtherHelper>(TargetActor.Get());
                HelperComp->TargetComp = this;
				HelperComp->RegisterComponent();
            }
            return true;
        }
    }
    return false;
}

void UUISizeControlByOther::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;
	if (!GetEnable())return;

    if (CheckTargetUIItem() && CheckRootUIComponent() && GetEnable())
    {
        if (ControlWidth)
        {
            ApplyUIItemWidth(RootUIComp.Get(), TargetUIItem->GetWidth() + AdditionalWidth);
        }
        if (ControlHeight)
        {
            ApplyUIItemHeight(RootUIComp.Get(), TargetUIItem->GetHeight() + AdditionalHeight);
        }
    }
}

bool UUISizeControlByOther::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
    if (this->GetRootUIComponent() == InUIItem)
    {
        OutResult.bCanControlHorizontalSizeDelta = GetControlWidth() && GetEnable();
        OutResult.bCanControlVerticalSizeDelta = GetControlHeight() && GetEnable();
        return true;
    }
    else
    {
        return false;
    }
}
