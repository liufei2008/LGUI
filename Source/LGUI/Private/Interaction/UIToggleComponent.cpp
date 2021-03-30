// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	if (UIToggleGroupActor.IsValid())
	{
		GroupComp = UIToggleGroupActor->FindComponentByClass<UUIToggleGroupComponent>();
	}
}

void UUIToggleComponent::Start()
{
	Super::Start();
	if (GroupComp.IsValid() && IsOn)
	{
		GroupComp->SetSelection(this);//if default is selected, set to group
	}
	ApplyToggleState(true);
}

bool UUIToggleComponent::CheckTarget()
{
	if (ToggleActor.IsValid())return true;
	return false;
}

void UUIToggleComponent::SetState(bool newState, bool fireEvent)
{
	if (IsOn != newState)
	{
		if (GroupComp.IsValid())
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

		if (GroupComp.IsValid())
		{
			if (IsOn)
			{
				GroupComp->SetSelection(this);
			}
			else
			{
				if (GroupComp->GetSelectedItem() == this)
				{
					GroupComp->ClearSelection();
				}
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
			if (ALTweenActor::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				uiItem->SetAlpha(IsOn ? OnAlpha : OffAlpha);
			}
			else
			{
				ToggleTransitionTweener = ALTweenActor::To(this, FLTweenFloatGetterFunction::CreateUObject(uiItem, &UUIItem::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(uiItem, &UUIItem::SetAlpha), IsOn ? OnAlpha : OffAlpha, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
	}
	else if (ToggleTransition == UIToggleTransitionType::ColorTint)
	{
		if (auto uiItem = ToggleActor->GetUIItem())
		{
			if (ALTweenActor::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				uiItem->SetColor(IsOn ? OnColor : OffColor);
			}
			else
			{
				ToggleTransitionTweener = ALTweenActor::To(this, FLTweenColorGetterFunction::CreateUObject(uiItem, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(uiItem, &UUIItem::SetColor), IsOn ? OnColor : OffColor, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
	}
	else if (ToggleTransition == UIToggleTransitionType::TransitionComponent)
	{
		if (!ToggleTransitionComp.IsValid())
		{
			ToggleTransitionComp = ToggleActor->FindComponentByClass<UUISelectableTransitionComponent>();
		}
		if (ToggleTransitionComp.IsValid())
		{
			ToggleTransitionComp->OnStartCustomTransition(IsOn ? OnTransitionName : OffTransitionName, immediateSet);
		}
	}
}

void UUIToggleComponent::SetToggleGroup(UUIToggleGroupComponent* InGroupComp)
{
	if (GroupComp != InGroupComp)
	{
		GroupComp = InGroupComp;
	}
}

bool UUIToggleComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	SetState(!IsOn);
	return AllowEventBubbleUp;
}

FDelegateHandle UUIToggleComponent::RegisterToggleEvent(const FLGUIBoolDelegate& InDelegate)
{
	return OnToggleCPP.Add(InDelegate);
}
FDelegateHandle UUIToggleComponent::RegisterToggleEvent(const TFunction<void(bool)>& InFunction)
{
	return OnToggleCPP.AddLambda(InFunction);
}
void UUIToggleComponent::UnregisterToggleEvent(const FDelegateHandle& InHandle)
{
	OnToggleCPP.Remove(InHandle);
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