// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIToggle.h"
#include "Interaction/UIToggleGroup.h"
#include "LTweenManager.h"
#include "Core/Components/LexWidget.h"
#include "Core/Components/LexVisual.h"
#include "Core/Components/LexImage.h"


UUIToggle* UUIToggleTransition::GetToggleComponent() const
{
	if (!IsValid(UIToggleComp))
	{
		UIToggleComp = GetWidget()->GetComponent<UUIToggle>();
	}
	return UIToggleComp;
}

void UUIToggleTransition::ToggleOn(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveToggleOn(InImmediateSet);
	}
}

void UUIToggleTransition::ToggleOff(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveToggleOff(InImmediateSet);
	}
}

UUIToggle::UUIToggle()
{
	OnColor = FColor(255, 255, 255, 255);
	OffColor = FColor(255, 255, 255, 0);
}
void UUIToggle::Awake()
{
	Super::Awake();
	//check toggle group
	if (!ToggleGroup.IsValid())
	{
		if (bAutoFindToggleGroupInParent)
		{
			ToggleGroup = GetWidget()->GetComponentInParent<UUIToggleGroup>();
		}
	}
	if (ToggleGroup.IsValid())
	{
		ToggleGroup->AddToggleComponent(this);
	}
}

void UUIToggle::Start()
{
	Super::Start();
	if (ToggleGroup.IsValid() && bIsOn)
	{
		ToggleGroup->SetSelection(this);//if default is selected, set to group
	}
	ApplyValueToVisual(true);
}

void UUIToggle::OnDestroy()
{
	Super::OnDestroy();
	if (ToggleGroup.IsValid())
	{
		ToggleGroup->RemoveToggleComponent(this);
	}
}

#if WITH_EDITOR
void UUIToggle::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIToggle, bIsOn))
		{
			ApplyValueToVisual(true);
		}
	}
}
#endif

void UUIToggle::SetValue(bool Value, bool SendCallback)
{
	if (bIsOn != Value)
	{
		if (ToggleGroup.IsValid())
		{
			if (ToggleGroup->GetAllowNoneSelected() == false && ToggleGroup->GetSelectedItem() == this && Value == false)//not allow none select
			{
				return;
			}
		}

		bIsOn = Value;
		if (ToggleGroup.IsValid())
		{
			if (bIsOn)
			{
				ToggleGroup->SetSelection(this);
			}
			else
			{
				if (ToggleGroup->GetSelectedItem() == this)
				{
					ToggleGroup->ClearSelection();
				}
			}
		}
		if (SendCallback)
		{
			OnValueChangedCPP.Broadcast(bIsOn);
			OnValueChanged.Broadcast(bIsOn);
			OnValueChangedED.FireEvent(bIsOn);
		}

		ApplyValueToVisual(false);
	}
}
void UUIToggle::ApplyValueToVisual(bool immediateSet)
{
	if (ToggleTransitionType != EUISelectableTransitionType::Custom)
	{
		if (!ToggleTransitionTarget.IsValid())return;
	}

	TOptional<FColor> Color;
	TOptional<FLexUIImageBrush> Brush;
	if (ToggleTransitionType == EUISelectableTransitionType::Color)
	{
		Color = bIsOn ? OnColor : OffColor;
	}
	else if (ToggleTransitionType == EUISelectableTransitionType::ImageBrush)
	{
		Brush = bIsOn ? OnImageBrush : OffImageBrush;
	}
	else if (ToggleTransitionType == EUISelectableTransitionType::Custom)
	{
		if (CustomToggleTransition.IsValid())
		{
			if (bIsOn)
			{
				CustomToggleTransition->ToggleOn(immediateSet);
			}
			else
			{
				CustomToggleTransition->ToggleOff(immediateSet);
			}
		}
	}

	if (Color.IsSet())
	{
		if (ToggleDuration <= 0.0f || immediateSet)
		{
			ToggleTransitionTarget->SetColor(Color.GetValue());
		}
		else
		{
			if (ULTweenManager::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
			ToggleTransitionTweener = ULTweenManager::To(ToggleTransitionTarget.Get()
				, FLTweenColorGetterFunction::CreateWeakLambda(ToggleTransitionTarget.Get(), [=, this]()
			{
				return ToggleTransitionTarget->GetColor();
			}), FLTweenColorSetterFunction::CreateUObject(ToggleTransitionTarget.Get(), &ULexVisual::SetColor), Color.GetValue(), ToggleDuration);
			if (ToggleTransitionTweener)
			{
				ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), ToggleTransitionTweener);
			}
		}
	}
	if (Brush.IsSet())
	{
		if (auto ToggleTransitionTargetAsLexImage = Cast<ULexImage>(ToggleTransitionTarget.Get()))
		{
			if (IsValid(Brush.GetValue().GetResourceObject()))
			{
				ToggleTransitionTargetAsLexImage->SetBrush(Brush.GetValue());
			}
			else
			{
				if (ToggleDuration <= 0.0f || immediateSet)
				{
					ToggleTransitionTargetAsLexImage->SetBrushTintColor(Brush.GetValue().TintColor);
				}
				else
				{
					if (ULTweenManager::IsTweening(this, ToggleTransitionTweener))ToggleTransitionTweener->Kill();
					ToggleTransitionTweener = ULTweenManager::To(ToggleTransitionTargetAsLexImage
						, FLTweenColorGetterFunction::CreateWeakLambda(ToggleTransitionTargetAsLexImage, [=, this]()
					{
						return ToggleTransitionTargetAsLexImage->GetBrush().TintColor;
					}), FLTweenColorSetterFunction::CreateUObject(ToggleTransitionTargetAsLexImage, &ULexImage::SetBrushTintColor), Brush.GetValue().TintColor, ToggleDuration);
					if (ToggleTransitionTweener)
					{
						ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), ToggleTransitionTweener);
					}
				}
			}
		}
	}
}

void UUIToggle::SetToggleGroup(UUIToggleGroup* InGroupComp)
{
	if (ToggleGroup != InGroupComp)
	{
		if (ToggleGroup.IsValid())
		{
			ToggleGroup->RemoveToggleComponent(this);
		}
		if (IsValid(InGroupComp))
		{
			InGroupComp->AddToggleComponent(this);
		}
		ToggleGroup = InGroupComp;
	}
}

void UUIToggle::SetValue(bool Value)
{
	SetValue(Value, true);
}

void UUIToggle::SetValueWithoutNotify(bool Value)
{
	SetValue(Value, false);
}

bool UUIToggle::OnPointerClick_Implementation(ULexPointerEventData* EventData)
{
	SetValue(!bIsOn);
	return AllowEventBubbleUp;
}

int32 UUIToggle::GetIndexInGroup()const
{
	if (ToggleGroup.IsValid())
	{
		return ToggleGroup->GetToggleIndex(this);
	}
	return -1;
}