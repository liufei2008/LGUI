// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	if (!enable)return;

    if (CheckTargetUIItem() && CheckRootUIComponent() && enable)
    {
        if (ControlWidth)
        {
            RootUIComp->SetWidth(TargetUIItem->GetWidth() + AdditionalWidth);
        }
        if (ControlHeight)
        {
            RootUIComp->SetHeight(TargetUIItem->GetHeight() + AdditionalHeight);
        }
    }
}

bool UUISizeControlByOther::CanControlChildAnchor_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildHorizontalAnchoredPosition_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildVerticalAnchoredPosition_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildWidth_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildHeight_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildAnchorLeft_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildAnchorRight_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildAnchorBottom_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlChildAnchorTop_Implementation()const
{
    return false;
}

bool UUISizeControlByOther::CanControlSelfAnchor_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfHorizontalAnchoredPosition_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfVerticalAnchoredPosition_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfWidth_Implementation()const
{
    return GetControlWidth() && enable;
}
bool UUISizeControlByOther::CanControlSelfHeight_Implementation()const
{
    return GetControlHeight() && enable;
}
bool UUISizeControlByOther::CanControlSelfAnchorLeft_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfAnchorRight_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfAnchorBottom_Implementation()const
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfAnchorTop_Implementation()const
{
    return false;
}
