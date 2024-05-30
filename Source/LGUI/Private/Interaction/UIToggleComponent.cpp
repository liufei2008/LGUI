// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIToggleComponent.h"
#include "LGUI.h"
#include "Interaction/UIToggleGroupComponent.h"
#include "LTweenManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UISprite.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/LGUISettings.h"


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
	ApplyValueToUI(true);
}

void UUIToggleComponent::OnEnable()
{
	Super::OnEnable();
	if (GroupComp.IsValid())
	{
		GroupComp->AddToggleComponent(this);
	}
}
void UUIToggleComponent::OnDisable()
{
	Super::OnDisable();
	if (GroupComp.IsValid())
	{
		GroupComp->RemoveToggleComponent(this);
	}
}

#if WITH_EDITOR
void UUIToggleComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIToggleComponent, IsOn))
		{
			ApplyValueToUI(true);
		}
	}
}
#endif

bool UUIToggleComponent::CheckTarget()
{
	if (ToggleActor.IsValid())return true;
	return false;
}

void UUIToggleComponent::SetValue(bool newValue, bool fireEvent)
{
	if (IsOn != newValue)
	{
		if (GroupComp.IsValid())
		{
			if (GroupComp->GetAllowNoneSelected() == false && GroupComp->GetSelectedItem() == this && newValue == false)//not allow none select
			{
				return;
			}
		}

		IsOn = newValue;
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
		if (fireEvent)
		{
			OnToggleCPP.Broadcast(IsOn);
			OnToggle.FireEvent(IsOn);
		}

		ApplyValueToUI(false);
	}
}
void UUIToggleComponent::ApplyValueToUI(bool immediateSet)
{
	if (!CheckTarget())return;
	if (ToggleTransition == UIToggleTransitionType::Fade)
	{
		if (auto UIRenderable = ToggleActor->GetUIRenderable())
		{
			if (ULTweenManager::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				UIRenderable->SetAlpha(IsOn ? OnAlpha : OffAlpha);
			}
			else
			{
				ToggleTransitionTweener = ULTweenManager::To(UIRenderable, FLTweenFloatGetterFunction::CreateUObject(UIRenderable, &UUIBaseRenderable::GetAlpha), FLTweenFloatSetterFunction::CreateUObject(UIRenderable, &UUIBaseRenderable::SetAlpha), IsOn ? OnAlpha : OffAlpha, ToggleDuration);
				if (ToggleTransitionTweener)
				{
					bool bAffectByGamePause = false;
					if (this->GetRootUIComponent())
					{
						if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
						{
							bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						}
						else
						{
							bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						}
					}
					ToggleTransitionTweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause);
				}
			}
		}
	}
	else if (ToggleTransition == UIToggleTransitionType::ColorTint)
	{
		if (auto UIRenderable = ToggleActor->GetUIRenderable())
		{
			if (ULTweenManager::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			if (ToggleDuration <= 0.0f || immediateSet)
			{
				UIRenderable->SetColor(IsOn ? OnColor : OffColor);
			}
			else
			{
				ToggleTransitionTweener = ULTweenManager::To(UIRenderable, FLTweenColorGetterFunction::CreateUObject(UIRenderable, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(UIRenderable, &UUIBaseRenderable::SetColor), IsOn ? OnColor : OffColor, ToggleDuration);
				if (ToggleTransitionTweener)
				{
					bool bAffectByGamePause = false;
					if (this->GetRootUIComponent())
					{
						if (this->GetRootUIComponent()->IsScreenSpaceOverlayUI())
						{
							bAffectByGamePause = GetDefault<ULGUISettings>()->bScreenSpaceUIAffectByGamePause;
						}
						else
						{
							bAffectByGamePause = GetDefault<ULGUISettings>()->bWorldSpaceUIAffectByGamePause;
						}
					}
					ToggleTransitionTweener->SetEase(ELTweenEase::InOutSine)->SetAffectByGamePause(bAffectByGamePause);
				}
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
		if (GroupComp.IsValid())
		{
			GroupComp->RemoveToggleComponent(this);
		}
		if (IsValid(InGroupComp))
		{
			InGroupComp->AddToggleComponent(this);
		}
		GroupComp = InGroupComp;
		UIToggleGroupActor = GroupComp->GetOwner();
	}
}

bool UUIToggleComponent::OnPointerClick_Implementation(ULGUIPointerEventData* eventData)
{
	SetValue(!IsOn);
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
		InDelegate.ExecuteIfBound(InIsOn);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIToggleComponent::UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnToggleCPP.Remove(InDelegateHandle.DelegateHandle);
}

int32 UUIToggleComponent::GetIndexInGroup()const
{
	if (GroupComp.IsValid())
	{
		return GroupComp->GetToggleIndex(this);
	}
	return -1;
}