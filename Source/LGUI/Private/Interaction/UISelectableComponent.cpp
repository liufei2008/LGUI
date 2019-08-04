// Copyright 2019 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "LTweenActor.h"
#include "Interaction/UISelectableTransitionComponent.h"

UUISelectableComponent::UUISelectableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISelectableComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckTarget();
}

void UUISelectableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void UUISelectableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		auto propertyName = PropertyChangedEvent.Property->GetName();
		if (CheckTarget())
		{
			auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionActor->GetUIItem());
			if (propertyName == TEXT("TransitionActor"))
			{
				if (TargetUISpriteComp) NormalSprite = TargetUISpriteComp->GetSprite();
				NormalColor = TransitionActor->GetUIItem()->GetColor();
			}
			else
			{
				bool isGroupAllowInteraction = RootUIComp->IsGroupAllowInteraction();
				if (Transition == UISelectableTransitionType::SpriteSwap)
				{
					if (TargetUISpriteComp != nullptr && NormalSprite != nullptr)
					{
						TargetUISpriteComp->SetSprite(isGroupAllowInteraction ? NormalSprite : DisabledSprite, false);
					}
					TransitionActor->GetUIItem()->EditorForceUpdateImmediately();
				}
				else if (Transition == UISelectableTransitionType::ColorTint)
				{
					TransitionActor->GetUIItem()->SetColor(isGroupAllowInteraction ? NormalColor : DisabledColor);
					TransitionActor->GetUIItem()->EditorForceUpdateImmediately();
				}
			}
		}
	}
}
#endif

void UUISelectableComponent::OnUIInteractionStateChanged(bool interactableOrNot)
{
	if (CheckRootUIComponent())
	{
		CurrentSelectionState = RootUIComp->IsGroupAllowInteraction()
			? (IsPointerInsideThis ? EUISelectableSelectionState::Highlighted : EUISelectableSelectionState::Normal)
			: EUISelectableSelectionState::Disabled;
		ApplySelectionState();
	}
}

bool UUISelectableComponent::CheckTarget()
{
	if (TransitionActor)return true;
	return false;
}
void UUISelectableComponent::ApplySelectionState()
{
	if (!CheckTarget())return;
	auto TransitionTargetUIItemComp = TransitionActor->GetUIItem();
	switch (CurrentSelectionState)
	{
	case EUISelectableSelectionState::Normal:
	{
		switch (Transition)
		{
		case UISelectableTransitionType::ColorTint:
		{
			if(FadeDuration <= 0.0f
#if WITH_EDITOR
				|| !this->GetWorld()->IsGameWorld()
#endif
				)
			{
				TransitionTargetUIItemComp->SetColor(NormalColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(ColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), NormalColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				TargetUISpriteComp->SetSprite(NormalSprite, false);
			}
		}
		break;
		case UISelectableTransitionType::TransitionComponent:
		{
#if WITH_EDITOR
			if (this->GetWorld()->IsGameWorld())
#endif
			{
				if (TransitionComp == nullptr)
				{
					TransitionComp = TransitionActor->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp)
				{
					TransitionComp->OnNormal();
				}
			}
		}
		break;
		}
	}
	break;
	case EUISelectableSelectionState::Highlighted:
	{
		switch (Transition)
		{
		case UISelectableTransitionType::ColorTint:
		{
			if (FadeDuration <= 0.0f
#if WITH_EDITOR
				|| !this->GetWorld()->IsGameWorld()
#endif
				)
			{
				TransitionTargetUIItemComp->SetColor(HighlightedColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(ColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), HighlightedColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (HighlightedSprite != nullptr)
				{
					TargetUISpriteComp->SetSprite(HighlightedSprite, false);
				}
			}
		}
		break;
		case UISelectableTransitionType::TransitionComponent:
		{
#if WITH_EDITOR
			if (this->GetWorld()->IsGameWorld())
#endif
			{
				if (TransitionComp == nullptr)
				{
					TransitionComp = TransitionActor->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp)
				{
					TransitionComp->OnHighlighted();
				}
			}
		}
		break;
		}
	}
	break;
	case EUISelectableSelectionState::Pressed:
	{
		switch (Transition)
		{
		case UISelectableTransitionType::ColorTint:
		{
			if (FadeDuration <= 0.0f
#if WITH_EDITOR
				|| !this->GetWorld()->IsGameWorld()
#endif
				)
			{
				TransitionTargetUIItemComp->SetColor(PressedColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(ColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), PressedColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (PressedSprite != nullptr)
				{
					TargetUISpriteComp->SetSprite(PressedSprite, false);
				}
			}
		}
		break;
		case UISelectableTransitionType::TransitionComponent:
		{
#if WITH_EDITOR
			if (this->GetWorld()->IsGameWorld())
#endif
			{
				if (TransitionComp == nullptr)
				{
					TransitionComp = TransitionActor->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp)
				{
					TransitionComp->OnPressed();
				}
			}
		}
		break;
		}
	}
	break;
	case EUISelectableSelectionState::Disabled:
	{
		switch (Transition)
		{
		case UISelectableTransitionType::ColorTint:
		{
			if (FadeDuration <= 0.0f
#if WITH_EDITOR
				|| !this->GetWorld()->IsGameWorld()
#endif
				)
			{
				TransitionTargetUIItemComp->SetColor(DisabledColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(ColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), ColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), DisabledColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (DisabledSprite != nullptr)
				{
					TargetUISpriteComp->SetSprite(DisabledSprite, false);
				}
			}
		}
		break;
		case UISelectableTransitionType::TransitionComponent:
		{
#if WITH_EDITOR
			if (this->GetWorld()->IsGameWorld())
#endif
			{
				if (TransitionComp == nullptr)
				{
					TransitionComp = TransitionActor->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp)
				{
					TransitionComp->OnDisabled();
				}
			}
		}
		break;
		}
	}
	break;
	}
}

bool UUISelectableComponent::OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)
{
	IsPointerInsideThis = true;
	CurrentSelectionState = EUISelectableSelectionState::Highlighted;
	ApplySelectionState();
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)
{
	IsPointerInsideThis = false;
	CurrentSelectionState = EUISelectableSelectionState::Normal;
	ApplySelectionState();
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	CurrentSelectionState = EUISelectableSelectionState::Pressed;
	ApplySelectionState();
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	CurrentSelectionState = IsPointerInsideThis ? EUISelectableSelectionState::Highlighted : EUISelectableSelectionState::Normal;
	ApplySelectionState();
	return AllowEventBubbleUp;
}

void UUISelectableComponent::SetNormalSprite(ULGUISpriteData* NewSprite)
{
	if (NormalSprite != NewSprite)
	{
		NormalSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Normal)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetNormalColor(FColor NewColor)
{
	if (NormalColor != NewColor)
	{
		NormalColor = NewColor;
		if (CurrentSelectionState == EUISelectableSelectionState::Normal)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetHighlightedSprite(ULGUISpriteData* NewSprite)
{
	if (HighlightedSprite != NewSprite)
	{
		HighlightedSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Highlighted)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetHighlightedColor(FColor NewColor)
{
	if (HighlightedColor != NewColor)
	{
		HighlightedColor = NewColor;
		if (CurrentSelectionState == EUISelectableSelectionState::Highlighted)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetPressedSprite(ULGUISpriteData* NewSprite)
{
	if (PressedSprite != NewSprite)
	{
		PressedSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Pressed)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetPressedColor(FColor NewColor)
{
	if (PressedColor != NewColor)
	{
		PressedColor = NewColor;
		if (CurrentSelectionState == EUISelectableSelectionState::Pressed)
		{
			ApplySelectionState();
		}
	}
}
void UUISelectableComponent::SetSelectionState(EUISelectableSelectionState NewState)
{
	if (CurrentSelectionState != NewState)
	{
		CurrentSelectionState = NewState;
		ApplySelectionState();
	}
}
AUIBaseActor* UUISelectableComponent::GetTransitionTarget()
{ 
	return TransitionActor; 
}
