// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByOther.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/Actor/UIBaseActor.h"

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
            TargetComp->OnRebuildLayout();
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
                TargetActor->AddInstanceComponent(HelperComp.Get());
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
				TargetActor->AddInstanceComponent(HelperComp.Get());
            }
            return true;
        }
    }
    return false;
}

void UUISizeControlByOther::OnRebuildLayout()
{
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
#if WITH_EDITOR
bool UUISizeControlByOther::CanControlChildAnchor()
{
    return false;
}
bool UUISizeControlByOther::CanControlChildWidth()
{
    return false;
}
bool UUISizeControlByOther::CanControlChildHeight()
{
    return false;
}
bool UUISizeControlByOther::CanControlSelfHorizontalAnchor()
{
    return GetControlWidth() && enable;
}
bool UUISizeControlByOther::CanControlSelfVerticalAnchor()
{
    return GetControlHeight() && enable;
}
bool UUISizeControlByOther::CanControlSelfWidth()
{
    return GetControlWidth() && enable;
}
bool UUISizeControlByOther::CanControlSelfHeight()
{
    return GetControlHeight() && enable;
}
#endif
void UUISizeControlByOther::SetControlWidth(bool value)
{
    if (ControlWidth != value)
    {
        ControlWidth = value;
        OnRebuildLayout();
    }
}
void UUISizeControlByOther::SetAdditionalWidth(float value)
{
    if (AdditionalWidth != value)
    {
        AdditionalWidth = value;
        OnRebuildLayout();
    }
}
void UUISizeControlByOther::SetControlHeight(bool value)
{
    if (ControlHeight != value)
    {
        ControlHeight = value;
        OnRebuildLayout();
    }
}
void UUISizeControlByOther::SetAdditionalHeight(float value)
{
    if (AdditionalHeight != value)
    {
        AdditionalHeight = value;
        OnRebuildLayout();
    }
}