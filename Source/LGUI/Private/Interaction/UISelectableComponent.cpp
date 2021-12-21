// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UISelectableComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "LTweenActor.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/LGUIEventSystem.h"
#include "Core/ActorComponent/UISprite.h"

void UUISelectableComponent::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}

void UUISelectableComponent::OnEnable()
{
	Super::OnEnable();
	ALGUIManagerActor::AddSelectable(this);
	ApplySelectionState(true);
}

void UUISelectableComponent::Start()
{
	Super::Start();
}

void UUISelectableComponent::OnDisable()
{
	Super::OnDisable();
	ALGUIManagerActor::RemoveSelectable(this);
}
void UUISelectableComponent::OnDestroy()
{
	Super::OnDestroy();
}

#if WITH_EDITOR
void UUISelectableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		auto propertyName = PropertyChangedEvent.Property->GetName();
		if (TransitionActor.IsValid())
		{
			auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionActor->GetUIRenderable());
			if (propertyName == TEXT("TransitionActor"))
			{
				if (TargetUISpriteComp) NormalSprite = TargetUISpriteComp->GetSprite();
				NormalColor = TransitionActor->GetUIRenderable()->GetColor();
			}
			else
			{
				bool isGroupAllowInteraction = RootUIComp->IsGroupAllowInteraction();
				if (Transition == UISelectableTransitionType::SpriteSwap)
				{
					if (IsValid(TargetUISpriteComp) && IsValid(NormalSprite))
					{
						TargetUISpriteComp->SetSprite(isGroupAllowInteraction ? NormalSprite : DisabledSprite, false);
					}
					TransitionActor->GetUIRenderable()->EditorForceUpdateImmediately();
				}
				else if (Transition == UISelectableTransitionType::ColorTint)
				{
					TransitionActor->GetUIRenderable()->SetColor(isGroupAllowInteraction ? NormalColor : DisabledColor);
					TransitionActor->GetUIRenderable()->EditorForceUpdateImmediately();
				}
			}
		}
	}
}
#endif

void UUISelectableComponent::OnUIInteractionStateChanged(bool interactableOrNot)
{
	Super::OnUIInteractionStateChanged(interactableOrNot);
	if (CheckRootUIComponent())
	{
		CurrentSelectionState = RootUIComp->IsGroupAllowInteraction()
			? (IsPointerInsideThis ? EUISelectableSelectionState::Highlighted : EUISelectableSelectionState::Normal)
			: EUISelectableSelectionState::Disabled;
#if WITH_EDITOR
		if (!this->GetWorld()->IsGameWorld())//is editor, just set properties immediately
		{
			ApplySelectionState(true);
		}
		else
#endif
		{
			ApplySelectionState(false);
		}
	}
}

void UUISelectableComponent::ApplySelectionState(bool immediateSet)
{
	if (!this->GetIsActiveAndEnable())return;
	if (Transition != UISelectableTransitionType::TransitionComponent)
	{
		if (!TransitionActor.IsValid())return;
	}

	switch (CurrentSelectionState)
	{
	case EUISelectableSelectionState::Normal:
	{
		switch (Transition)
		{
		case UISelectableTransitionType::ColorTint:
		{
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if(FadeDuration <= 0.0f || immediateSet)
			{
				TransitionTargetUIItemComp->SetColor(NormalColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), NormalColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
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
				if (!TransitionComp.IsValid())
				{
					TransitionComp = this->GetOwner()->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp.IsValid() && TransitionComp->GetIsActiveAndEnable())
				{
					TransitionComp->OnNormal(immediateSet);
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
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (FadeDuration <= 0.0f || immediateSet)
			{
				TransitionTargetUIItemComp->SetColor(HighlightedColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), HighlightedColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (IsValid(HighlightedSprite))
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
				if (!TransitionComp.IsValid())
				{
					TransitionComp = this->GetOwner()->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp.IsValid() && TransitionComp->GetIsActiveAndEnable())
				{
					TransitionComp->OnHighlighted(immediateSet);
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
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (FadeDuration <= 0.0f || immediateSet)
			{
				TransitionTargetUIItemComp->SetColor(PressedColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), PressedColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (IsValid(PressedSprite))
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
				if (!TransitionComp.IsValid())
				{
					TransitionComp = this->GetOwner()->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp.IsValid() && TransitionComp->GetIsActiveAndEnable())
				{
					TransitionComp->OnPressed(immediateSet);
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
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (FadeDuration <= 0.0f || immediateSet)
			{
				TransitionTargetUIItemComp->SetColor(DisabledColor);
			}
			else
			{
				if (ALTweenActor::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ALTweenActor::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), DisabledColor, FadeDuration);
			}
		}
		break;
		case UISelectableTransitionType::SpriteSwap:
		{
			auto TransitionTargetUIItemComp = TransitionActor->GetUIRenderable();
			if (auto TargetUISpriteComp = Cast<UUISpriteBase>(TransitionTargetUIItemComp))
			{
				if (IsValid(DisabledSprite))
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
				if (!TransitionComp.IsValid())
				{
					TransitionComp = this->GetOwner()->FindComponentByClass<UUISelectableTransitionComponent>();
				}
				if (TransitionComp.IsValid() && TransitionComp->GetIsActiveAndEnable())
				{
					TransitionComp->OnDisabled(immediateSet);
				}
			}
		}
		break;
		}
	}
	break;
	}
}

bool UUISelectableComponent::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	IsPointerInsideThis = true;
	CurrentSelectionState = GetSelectionState();
	ApplySelectionState(false);
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	IsPointerInsideThis = false;
	CurrentSelectionState = GetSelectionState();
	ApplySelectionState(false);
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	IsPointerDown = true;
	CurrentSelectionState = GetSelectionState();
	ApplySelectionState(false);
	if (auto eventSystemInstance = ULGUIEventSystem::GetLGUIEventSystemInstance(this))
	{
		eventSystemInstance->SetSelectComponent(GetRootUIComponent(), eventData, eventData->enterComponentEventFireType);
	}
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	IsPointerDown = false;
	CurrentSelectionState = GetSelectionState();
	ApplySelectionState(false);
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)
{
	return AllowEventBubbleUp;
}
bool UUISelectableComponent::OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)
{
	return AllowEventBubbleUp;
}

EUISelectableSelectionState UUISelectableComponent::GetSelectionState()const
{
	if (!IsInteractable())
		return EUISelectableSelectionState::Disabled;
	if (IsPointerDown)
		return EUISelectableSelectionState::Pressed;
	if (IsPointerInsideThis)
		return EUISelectableSelectionState::Highlighted;
	return EUISelectableSelectionState::Normal;
}

void UUISelectableComponent::SetTransitionTarget(class AUIBaseRenderableActor* value)
{
	if (TransitionActor != value)
	{
		TransitionActor = value;
		ApplySelectionState(false);
	}
}
void UUISelectableComponent::SetNormalSprite(ULGUISpriteData_BaseObject* NewSprite)
{
	if (NormalSprite != NewSprite)
	{
		NormalSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Normal)
		{
			ApplySelectionState(false);
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
			ApplySelectionState(false);
		}
	}
}
void UUISelectableComponent::SetHighlightedSprite(ULGUISpriteData_BaseObject* NewSprite)
{
	if (HighlightedSprite != NewSprite)
	{
		HighlightedSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Highlighted)
		{
			ApplySelectionState(false);
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
			ApplySelectionState(false);
		}
	}
}
void UUISelectableComponent::SetPressedSprite(ULGUISpriteData_BaseObject* NewSprite)
{
	if (PressedSprite != NewSprite)
	{
		PressedSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Pressed)
		{
			ApplySelectionState(false);
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
			ApplySelectionState(false);
		}
	}
}
void UUISelectableComponent::SetSelectionState(EUISelectableSelectionState NewState)
{
	if (CurrentSelectionState != NewState)
	{
		CurrentSelectionState = NewState;
		ApplySelectionState(false);
	}
}
bool UUISelectableComponent::IsInteractable()const
{
	if (CheckRootUIComponent())
	{
		return RootUIComp->IsGroupAllowInteraction();
	}
	return true;
}

#pragma region Navigation
bool UUISelectableComponent::OnNavigate(ELGUINavigationDirection InDirection)
{
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
			auto rootCanvasUIItem = RootUIComp->GetRootCanvas()->GetUIItem();
			return FindSelectable(InDirection, rootCanvasUIItem);
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
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetWorld()))
	{
		const auto& uiSelectables = LGUIManagerActor->GetSelectables();
		FVector pos = CheckRootUIComponent() ? FVector(RootUIComp->GetLocalSpaceCenter(), 0) : FVector::ZeroVector;
		pos = GetRootUIComponent()->GetComponentTransform().TransformPosition(pos);
		float maxScore = MIN_flt;
		UUISelectableComponent* bestPick = this;
		for (int i = 0; i < uiSelectables.Num(); ++i)
		{
			auto sel = uiSelectables[i];

			if (sel == this || !IsValid(sel))
				continue;

			if (IsValid(InParent) && !sel->GetRootUIComponent()->IsAttachedTo(InParent))
				continue;

			if (!sel->IsInteractable())
				continue;

			if(!sel->GetRootUIComponent()->GetIsUIActiveInHierarchy())
				continue;

			FVector selCenter = sel->CheckRootUIComponent() ? FVector(sel->GetRootUIComponent()->GetLocalSpaceCenter(), 0) : FVector::ZeroVector;
			FVector myVector = sel->GetRootUIComponent()->GetComponentTransform().TransformPosition(selCenter) - pos;

			float dot = FVector::DotProduct(InDirection, myVector);
			if (dot <= 0.1f)
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
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindDefaultSelectable(UObject* WorldContextObject)
{
	if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(WorldContextObject->GetWorld()))
	{
		const auto& uiSelectables = LGUIManagerActor->GetSelectables();
		if (uiSelectables.Num() > 0)
		{
			return uiSelectables[0];
		}
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnLeft()
{
	if (NavigationLeft == EUISelectableNavigationMode::Explicit)
	{
		return NavigationLeftSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationLeft == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(-GetRootUIComponent()->GetRightVector());
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
		return FindSelectable(GetRootUIComponent()->GetRightVector());
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
		return FindSelectable(GetRootUIComponent()->GetUpVector());
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
		return FindSelectable(-GetRootUIComponent()->GetUpVector());
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

void UUISelectableComponent::SetNavigationLeft(EUISelectableNavigationMode value)
{
	NavigationLeft = value;
}
void UUISelectableComponent::SetNavigationRight(EUISelectableNavigationMode value)
{
	NavigationRight = value;
}
void UUISelectableComponent::SetNavigationUp(EUISelectableNavigationMode value)
{
	NavigationUp = value;
}
void UUISelectableComponent::SetNavigationDown(EUISelectableNavigationMode value)
{
	NavigationDown = value;
}
void UUISelectableComponent::SetNavigationPrev(EUISelectableNavigationMode value)
{
	NavigationPrev = value;
}
void UUISelectableComponent::SetNavigationNext(EUISelectableNavigationMode value)
{
	NavigationNext = value;
}

void UUISelectableComponent::SetNavigationLeftExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationLeftSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationLeftExplicit] value is not valid!"));
	}
}
void UUISelectableComponent::SetNavigationRightExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationRightSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationRightExplicit] value is not valid!"));
	}
}
void UUISelectableComponent::SetNavigationUpExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationUpSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationUpExplicit] value is not valid!"));
	}
}
void UUISelectableComponent::SetNavigationDownExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationDownSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationDownExplicit] value is not valid!"));
	}
}
void UUISelectableComponent::SetNavigationPrevExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationPrevSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationPrevExplicit] value is not valid!"));
	}
}
void UUISelectableComponent::SetNavigationNextExplicit(UUISelectableComponent* value)
{
	if (IsValid(value))
	{
		NavigationNextSpecific = FLGUIComponentReference(value);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISelectableComponent::SetNavigationNextExplicit] value is not valid!"));
	}
}
#pragma endregion