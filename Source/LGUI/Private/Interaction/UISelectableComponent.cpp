// Copyright 2019 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LTweenActor.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/ActorComponent/LGUICanvas.h"

UUISelectableComponent::UUISelectableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISelectableComponent::BeginPlay()
{
	Super::BeginPlay();
	CheckTarget();
	ALGUIManagerActor::AddSelectable(this);
}

void UUISelectableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ALGUIManagerActor::RemoveSelectable(this);
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
				TransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), NormalColor, FadeDuration);
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
				TransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), HighlightedColor, FadeDuration);
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
				TransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), PressedColor, FadeDuration);
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
				TransitionTweener = ALTweenActor::To(FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIItem::SetColor), DisabledColor, FadeDuration);
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
bool UUISelectableComponent::OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)
{
	IsPointerInsideThis = true;
	CurrentSelectionState = EUISelectableSelectionState::Highlighted;
	ApplySelectionState();
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)
{
	IsPointerInsideThis = false;
	CurrentSelectionState = EUISelectableSelectionState::Normal;
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
bool UUISelectableComponent::IsInteractable()
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->IsGroupAllowInteraction();
	}
	return true;
}

UUISelectableComponent* UUISelectableComponent::FindSelectable(FVector InDirection)
{
	InDirection.Normalize();
	if (CheckRootUIComponent())
	{
		if (RootUIComp->GetRenderCanvas() == nullptr || RootUIComp->GetRenderCanvas()->GetRootCanvas() == nullptr)
		{
			return nullptr;//not active render
		}
		bool isScreenSpaceUI = RootUIComp->IsScreenSpaceOverlayUI();
		if (isScreenSpaceUI)
		{
			auto rootCanvasTf = RootUIComp->GetRenderCanvas()->GetRootCanvas()->CheckAndGetUIItem();
			return FindSelectable(InDirection, rootCanvasTf);
		}
		else
		{
			return FindSelectable(InDirection, nullptr);
		}
	}
	else
	{
		return FindSelectable(InDirection, nullptr);
	}
	return nullptr;
}

UUISelectableComponent* UUISelectableComponent::FindSelectable(FVector InDirection, USceneComponent* InParent)
{
	const auto& uiSelectables = ALGUIManagerActor::Instance->GetSelectables();
	FVector pos = CheckRootUIComponent() ? FVector(RootUIComp->GetLocalSpaceCenter(), 0) : FVector::ZeroVector;
	pos = GetOwner()->GetRootComponent()->GetComponentTransform().TransformPosition(pos);
	float maxScore = MIN_flt;
	UUISelectableComponent* bestPick = this;
	for (int i = 0; i < uiSelectables.Num(); ++i)
	{
		auto sel = uiSelectables[i];

		if (sel == this || sel == nullptr)
			continue;

		if (InParent != nullptr && !sel->GetOwner()->GetRootComponent()->IsAttachedTo(InParent))
			continue;

		if (!sel->IsInteractable())
			continue;

		UUIItem* selRect = Cast<UUIItem>(sel->GetOwner()->GetRootComponent());
		FVector selCenter = selRect != nullptr ? FVector(selRect->GetLocalSpaceCenter(), 0) : FVector::ZeroVector;
		FVector myVector = sel->GetOwner()->GetRootComponent()->GetComponentTransform().TransformPosition(selCenter) - pos;

		float dot = FVector::DotProduct(InDirection, myVector);
		if (dot <= 0)
			continue;

		float score = dot / myVector.SizeSquared();
		if (score > maxScore)
		{
			maxScore = score;
			bestPick = sel;
		}
	}
	return bestPick;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnLeft()
{
	if (NavigationLeft == EUISelectableNavigationMode::Explicit)
	{
		return NavigationLeftSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationLeft == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(-GetOwner()->GetRootComponent()->GetForwardVector());
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnRight()
{
	if (NavigationRight == EUISelectableNavigationMode::Explicit)
	{
		return NavigationRightSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationRight == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(GetOwner()->GetRootComponent()->GetForwardVector());//forward as right
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnUp()
{
	if (NavigationUp == EUISelectableNavigationMode::Explicit)
	{
		return NavigationUpSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationUp == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(GetOwner()->GetRootComponent()->GetRightVector());//right as up 
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnDown()
{
	if (NavigationDown == EUISelectableNavigationMode::Explicit)
	{
		return NavigationDownSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationDown == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(-GetOwner()->GetRootComponent()->GetRightVector());
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnNext()
{
	auto rightComp = FindSelectableOnRight();
	if (rightComp != this)
	{
		return rightComp;
	}
	return FindSelectableOnDown();
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnPrev()
{
	auto leftComp = FindSelectableOnLeft();
	if (leftComp != this)
	{
		return leftComp;
	}
	return FindSelectableOnUp();
}