// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UICanvasGroup.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "LTweenManager.h"

UUICanvasGroup::UUICanvasGroup()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUICanvasGroup::BeginPlay()
{
	Super::BeginPlay();
}

void UUICanvasGroup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
#if WITH_EDITOR
void UUICanvasGroup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propName = Property->GetFName();
		if (propName == GET_MEMBER_NAME_CHECKED(UUICanvasGroup, bInteractable) || propName == GET_MEMBER_NAME_CHECKED(UUICanvasGroup, bIgnoreParentGroup))
		{
			CheckInteractableStateChange();
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUICanvasGroup, Alpha))
		{
			OnAlphaChange();
		}
	}
}
#endif

void UUICanvasGroup::OnRegister()
{
	Super::OnRegister();
	if (CheckUIItem())
	{
		UIItem->RegisterCanvasGroup(this);
		UIHierarchyChangeDelegateHandle = UIItem->RegisterUIHierarchyChanged(FSimpleDelegate::CreateUObject(this, &UUICanvasGroup::OnUIHierarchyChanged));
	}
}
void UUICanvasGroup::OnUnregister()
{
	Super::OnUnregister();
	if (UIItem.IsValid())
	{
		UIItem->UnregisterCanvasGroup();
		UIItem->UnregisterUIHierarchyChanged(UIHierarchyChangeDelegateHandle);
	}
}

void UUICanvasGroup::SetParentCanvasGroup(UUICanvasGroup* InParentCanvasGroup)
{
	if (ParentUICanvasGroup != InParentCanvasGroup)
	{
		if (ParentUICanvasGroup.IsValid())
		{
			ParentUICanvasGroup->UnregisterInteractableStateChange(this->ParentInteractableStateChangeDelegateHandle);
			ParentUICanvasGroup->UnregisterAlphaChange(this->ParentAlphaChangeDelegteHandle);
		}
		ParentUICanvasGroup = InParentCanvasGroup;
		if (ParentUICanvasGroup.IsValid())
		{
			this->ParentInteractableStateChangeDelegateHandle = ParentUICanvasGroup->RegisterInteractableStateChange(FSimpleDelegate::CreateUObject(this, &UUICanvasGroup::OnParentInteractableStateChange));
			this->ParentAlphaChangeDelegteHandle = ParentUICanvasGroup->RegisterAlphaChange(FSimpleDelegate::CreateUObject(this, &UUICanvasGroup::OnAlphaChange));
		}
		OnParentInteractableStateChange();
		OnAlphaChange();
	}
}

void UUICanvasGroup::OnUIHierarchyChanged()
{
	UUICanvasGroup* NewParentUICanvasGroup = nullptr;
	if (this->IsRegistered())//find new CanvasGroup only for OnRegister
	{
		NewParentUICanvasGroup = UUIItem::GetComponentInParentUI<UUICanvasGroup>(this->GetOwner()->GetAttachParentActor(), true);
	}
	SetParentCanvasGroup(NewParentUICanvasGroup);
}

bool UUICanvasGroup::CheckUIItem()
{
	if (UIItem.IsValid())
		return true;
	else
		UIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	if (UIItem.IsValid())
		return true;
	return false;
}

void UUICanvasGroup::OnParentInteractableStateChange()
{
	CheckInteractableStateChange();
}

void UUICanvasGroup::OnAlphaChange()
{
	bIsAlphaDirty = true;
	if (AlphaChangeDelegate.IsBound())
	{
		AlphaChangeDelegate.Broadcast();
	}
}

FDelegateHandle UUICanvasGroup::RegisterInteractableStateChange(const FSimpleDelegate& InCallback)
{
	return InteractableStateChangeDelegate.Add(InCallback);
}
void UUICanvasGroup::UnregisterInteractableStateChange(const FDelegateHandle& InHandle)
{
	InteractableStateChangeDelegate.Remove(InHandle);
}

FDelegateHandle UUICanvasGroup::RegisterAlphaChange(const FSimpleDelegate& InCallback)
{
	return AlphaChangeDelegate.Add(InCallback);
}
void UUICanvasGroup::UnregisterAlphaChange(const FDelegateHandle& InHandle)
{
	AlphaChangeDelegate.Remove(InHandle);
}

float UUICanvasGroup::GetFinalAlpha() const
{
	if (bIsAlphaDirty)
	{
		if (ParentUICanvasGroup.IsValid())
		{
			CacheFinalAlpha = ParentUICanvasGroup->GetFinalAlpha() * this->Alpha;
		}
		else
		{
			CacheFinalAlpha = this->Alpha;
		}
		bIsAlphaDirty = false;
	}
	return CacheFinalAlpha;
}

bool UUICanvasGroup::GetFinalInteractable() const
{
	if (ParentUICanvasGroup.IsValid())
	{
		if (bIgnoreParentGroup)
		{
			return bInteractable;
		}
		else
		{
			return bInteractable && ParentUICanvasGroup->GetFinalInteractable();
		}
	}
	else
	{
		return bInteractable;
	}
}

const UUICanvasGroup* UUICanvasGroup::GetRestrictNavigationAreaCanvasGroup() const
{
	if (ParentUICanvasGroup.IsValid())
	{
		if (bIgnoreParentGroup)
		{
			if (bRestrictNavigationArea)
			{
				return this;
			}
			return nullptr;
		}
		else
		{
			if (bRestrictNavigationArea)
			{
				return this;
			}
			else
			{
				return ParentUICanvasGroup->GetRestrictNavigationAreaCanvasGroup();
			}
		}
	}
	else
	{
		if (bRestrictNavigationArea)
		{
			return this;
		}
		return nullptr;
	}
}

void UUICanvasGroup::SetAlpha(float value)
{
	if (value != Alpha)
	{
		Alpha = value;
		OnAlphaChange();
	}
}

void UUICanvasGroup::CheckInteractableStateChange()
{
	bool NewIsInteractable = GetFinalInteractable();
	if (bPrevIsInteractable != NewIsInteractable)
	{
		bPrevIsInteractable = NewIsInteractable;
		if (InteractableStateChangeDelegate.IsBound())
		{
			InteractableStateChangeDelegate.Broadcast();
		}
	}
}

void UUICanvasGroup::SetInteractable(bool value)
{
	if (value != bInteractable)
	{
		bInteractable = value;
		CheckInteractableStateChange();
	}
}
void UUICanvasGroup::SetIgnoreParentGroup(bool value)
{
	if (value != bIgnoreParentGroup)
	{
		bIgnoreParentGroup = value;
		CheckInteractableStateChange();
	}
}

ULTweener* UUICanvasGroup::AlphaTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUICanvasGroup::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &UUICanvasGroup::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUICanvasGroup::AlphaFrom(float startValue, float duration, float delay, ELTweenEase ease)
{
	auto endValue = this->GetAlpha();
	this->SetAlpha(startValue);
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUICanvasGroup::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(this, &UUICanvasGroup::SetAlpha), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
