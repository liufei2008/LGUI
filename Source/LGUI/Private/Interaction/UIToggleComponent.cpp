// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Interaction/UIToggleComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleGroupComponent.h"
#include "LTweenActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UISprite.h"
#include "Interaction/UISelectableTransitionComponent.h"


UUIToggleComponent::UUIToggleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIToggleComponent::BeginPlay()
{
	Super::BeginPlay();
	TargetComp = nullptr;
	CheckTarget();
	if (GroupComp)
	{
		if (IsOn)
		{
			GroupComp->SetSelection(this);//if default is selected, set to group
		}
	}
}

bool UUIToggleComponent::CheckTarget()
{
	if (TargetComp != nullptr)return true;
	bool result = false;
	if (!ToggleActor) ToggleActor = Cast<AUIBaseActor>(GetOwner());
	if (ToggleActor != nullptr)
	{
		if (ToggleTransition == UIToggleTransitionType::Fade)
		{
			if (auto uiItem = ToggleActor->FindComponentByClass<UUIItem>())
			{
				TargetComp = uiItem;
				uiItem->SetAlpha(IsOn ? OnAlpha : OffAlpha);
				result = true;
			}
		}
		else if (ToggleTransition == UIToggleTransitionType::ColorTint)
		{
			if (auto uiItem = ToggleActor->FindComponentByClass<UUIItem>())
			{
				TargetComp = uiItem;
				uiItem->SetColor(IsOn ? OnColor : OffColor);
				result = true;
			}
		}
	}
	if (UIToggleGroupActor)
	{
		GroupComp = UIToggleGroupActor->FindComponentByClass<UUIToggleGroupComponent>();
	}
	return result;
}

#if WITH_EDITOR
void UUIToggleComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

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

		if (ALTweenActor::IsTweening(ToggleTransitionTweener))ToggleTransitionTweener->Kill();
		switch (ToggleTransition)
		{
		case UIToggleTransitionType::Fade:
		{
			if (auto TargetUIComp = Cast<UUIItem>(TargetComp))
			{
				ToggleTransitionTweener = ALTweenActor::To(FLTweenFloatGetterFunction::CreateUObject(TargetUIComp, &UUIItem::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(TargetUIComp, &UUIItem::SetAlpha), IsOn ? OnAlpha : OffAlpha, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
		break;
		case UIToggleTransitionType::ColorTint:
		{
			if (auto TargetUIComp = Cast<UUIItem>(TargetComp))
			{
				ToggleTransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(TargetUIComp, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(TargetUIComp, &UUIItem::SetColor), IsOn ? OnColor : OffColor, ToggleDuration)
					->SetEase(LTweenEase::InOutSine);
			}
		}
		break;
		case UIToggleTransitionType::TransitionComponent:
		{
			if (TransitionComp == nullptr)
			{
				TransitionComp = ToggleActor->FindComponentByClass<UUISelectableTransitionComponent>();
			}
			if (TransitionComp)
			{
				TransitionComp->OnStartCustomTransition(IsOn ? OnTransitionName : OffTransitionName);
			}
		}
		break;
		}

		if (GroupComp)
		{
			if (IsOn)
			{
				GroupComp->SelectItem(this);
			}
		}
	}
}
bool UUIToggleComponent::OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)
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
	auto delegateHandle = OnToggleCPP.AddLambda([InDelegate](bool IsOn) {
		if (InDelegate.IsBound())InDelegate.Execute(IsOn);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIToggleComponent::UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnToggleCPP.Remove(InDelegateHandle.DelegateHandle);
}