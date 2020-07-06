// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIToggleComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleGroupComponent.h"
#include "LTweenActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UISprite.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/Actor/UIBaseActor.h"


void UUIToggleComponent::Awake()
{
	Super::Awake();
	CheckTarget();
	//check toggle group
	if (UIToggleGroupActor)
	{
		GroupComp = UIToggleGroupActor->FindComponentByClass<UUIToggleGroupComponent>();
	}
}

void UUIToggleComponent::Start()
{
	Super::Start();
	if (IsValid(GroupComp) && IsOn)
	{
		GroupComp->SetSelection(this);//if default is selected, set to group
	}
	ApplyToggleState(true);
}

bool UUIToggleComponent::CheckTarget()
{
	if (ToggleActor)return true;
	return false;
}

void UUIToggleComponent::SetState(bool newState, bool fireEvent)
{
	if (IsOn != newState)
	{
		if (GroupComp)
		{
			if (GroupComp->GetAllowNoneSelected() == false && GroupComp->GetSelectedItem() == this && newState == false)//not allow none select
			{
				return;
			}
		}

		IsOn = newState;
		if (fireEvent)
		{
			if (OnToggleCPP.IsBound())OnToggleCPP.Broadcast(IsOn);
			OnToggle.FireEvent(IsOn);
		}

		ApplyToggleState(false);

		if (GroupComp)
		{
			if (IsOn)
			{
				GroupComp->SelectItem(this);
			}
		}
	}
}
void UUIToggleComponent::ApplyToggleState(bool immediateSet)
{
	if (!CheckTarget())return;
	if (ToggleTransition == UIToggleTransitionType::Fade)
	{
		if (auto uiItem = ToggleActor->GetUIItem())
		{
			if (ALTweenActor::IsTweening(ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				uiItem->SetAlpha(IsOn ? OnAlpha : OffAlpha);
			}
			else
			{
				ToggleTransitionTweener = ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(uiItem, &UUIItem::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(uiItem, &UUIItem::SetAlpha), IsOn ? OnAlpha : OffAlpha, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
	}
	else if (ToggleTransition == UIToggleTransitionType::ColorTint)
	{
		if (auto uiItem = ToggleActor->GetUIItem())
		{
			if (ALTweenActor::IsTweening(ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				uiItem->SetColor(IsOn ? OnColor : OffColor);
			}
			else
			{
				ToggleTransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(uiItem, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(uiItem, &UUIItem::SetColor), IsOn ? OnColor : OffColor, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
	}
	else if (ToggleTransition == UIToggleTransitionType::TransitionComponent)
	{
		if (ToggleTransitionComp == nullptr)
		{
			ToggleTransitionComp = ToggleActor->FindComponentByClass<UUISelectableTransitionComponent>();
		}
		if (ToggleTransitionComp)
		{
			ToggleTransitionComp->OnStartCustomTransition(IsOn ? OnTransitionName : OffTransitionName, immediateSet);
		}
	}
}

bool UUIToggleComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	SetState(!IsOn);
	return AllowEventBubbleUp;
}

void UUIToggleComponent::RegisterToggleEvent(const FLGUIBoolDelegate& InDelegate)
{
	OnToggleCPP.Add(InDelegate);
}
void UUIToggleComponent::UnregisterToggleEvent(const FLGUIBoolDelegate& InDelegate)
{
	OnToggleCPP.Remove(InDelegate.GetHandle());
}

FLGUIDelegateHandleWrapper UUIToggleComponent::RegisterToggleEvent(const FLGUIToggleDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnToggleCPP.AddLambda([InDelegate](bool InIsOn) {
		if (InDelegate.IsBound())InDelegate.Execute(InIsOn);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIToggleComponent::UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnToggleCPP.Remove(InDelegateHandle.DelegateHandle);
}