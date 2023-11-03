// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UISelectableComponent.h"
#include "LGUI.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/Actor/LGUIManager.h"
#include "LTweenManager.h"
#include "Interaction/UISelectableTransitionComponent.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/LGUIEventSystem.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/ActorComponent/UICanvasGroup.h"
#include "Core/LGUISpriteData_BaseObject.h"
#if WITH_EDITOR
#include "Utils/LGUIUtils.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

void UUISelectableComponent::Awake()
{
	Super::Awake();
	this->SetCanExecuteUpdate(false);
}

void UUISelectableComponent::OnEnable()
{
	Super::OnEnable();
	ULGUIManagerWorldSubsystem::AddSelectable(this);
	ApplySelectionState(true);
}

void UUISelectableComponent::Start()
{
	Super::Start();
}

void UUISelectableComponent::OnDisable()
{
	Super::OnDisable();
	ULGUIManagerWorldSubsystem::RemoveSelectable(this);
}
void UUISelectableComponent::OnDestroy()
{
	Super::OnDestroy();
}

void UUISelectableComponent::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	//Add/Remove selectable inside OnRegister/OnUnregister in edit mode, and inside OnEnable/OnDisable in runtime mode
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		ULGUIManagerWorldSubsystem::AddSelectable(this);
	}
#endif
}
void UUISelectableComponent::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		ULGUIManagerWorldSubsystem::RemoveSelectable(this);
	}
#endif
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
						LGUIUtils::NotifyPropertyChanged(TargetUISpriteComp, UUISpriteBase::GetSpritePropertyName());
					}
					TransitionActor->GetUIRenderable()->EditorForceUpdate();
				}
				else if (Transition == UISelectableTransitionType::ColorTint)
				{
					TransitionActor->GetUIRenderable()->SetColor(isGroupAllowInteraction ? NormalColor : DisabledColor);
					LGUIUtils::NotifyPropertyChanged(TransitionActor->GetUIRenderable(), UUIBaseRenderable::GetColorPropertyName());
					TransitionActor->GetUIRenderable()->EditorForceUpdate();
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
				if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ULTweenManager::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), NormalColor, FadeDuration);
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
				if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ULTweenManager::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), HighlightedColor, FadeDuration);
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
				if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ULTweenManager::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), PressedColor, FadeDuration);
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
				if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
				TransitionTweener = ULTweenManager::To(TransitionTargetUIItemComp, FLTweenColorGetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::GetColor), FLTweenColorSetterFunction::CreateUObject(TransitionTargetUIItemComp, &UUIBaseRenderable::SetColor), DisabledColor, FadeDuration);
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
		eventSystemInstance->SetSelectComponent(GetRootSceneComponent(), eventData, eventData->enterComponentEventFireType);
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
void UUISelectableComponent::SetDisabledSprite(ULGUISpriteData_BaseObject* NewSprite)
{
	if (DisabledSprite != NewSprite)
	{
		DisabledSprite = NewSprite;
		if (CurrentSelectionState == EUISelectableSelectionState::Disabled)
		{
			ApplySelectionState(false);
		}
	}
}
void UUISelectableComponent::SetDisabledColor(FColor NewColor)
{
	if (DisabledColor != NewColor)
	{
		DisabledColor = NewColor;
		if (CurrentSelectionState == EUISelectableSelectionState::Disabled)
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
bool UUISelectableComponent::OnNavigate_Implementation(ELGUINavigationDirection direction, TScriptInterface<ILGUINavigationInterface>& result)
{
	UUISelectableComponent* Selectable = nullptr;
	switch (direction)
	{
	default:
	case ELGUINavigationDirection::None:
		return false;
		break;
	case ELGUINavigationDirection::Left:
		Selectable = FindSelectableOnLeft();
		break;
	case ELGUINavigationDirection::Right:
		Selectable = FindSelectableOnRight();
		break;
	case ELGUINavigationDirection::Up:
		Selectable = FindSelectableOnUp();
		break;
	case ELGUINavigationDirection::Down:
		Selectable = FindSelectableOnDown();
		break;
	case ELGUINavigationDirection::Prev:
		Selectable = FindSelectableOnPrev();
		break;
	case ELGUINavigationDirection::Next:
		Selectable = FindSelectableOnNext();
		break;
	}
	result = Selectable;
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
		if (RootUIComp->IsScreenSpaceOverlayUI() || RootUIComp->IsRenderTargetUI())
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
	auto LGUIManagerActor = ULGUIManagerWorldSubsystem::GetInstance(this->GetWorld());
	if (LGUIManagerActor == nullptr)return nullptr;
	const auto& SelectableArray = LGUIManagerActor->GetAllSelectableArray();

	auto GetPointOnRectEdge = [](UUIItem* rect, FVector2D dir)
	{
		if (dir != FVector2D::ZeroVector)
			dir /= FMath::Max(FMath::Abs(dir.X), FMath::Abs(dir.Y));
		auto center = rect->GetLocalSpaceCenter();
		dir = center + FVector2D(rect->GetWidth() * dir.X * 0.5f, rect->GetHeight() * dir.Y * 0.5f);
		return FVector(0, dir.X, dir.Y);
	};

	auto LocalPos = FVector::ZeroVector;
	USceneComponent* RestrictNavNode = nullptr;
	if (CheckRootUIComponent())
	{
		auto localDir = RootUIComp->GetComponentTransform().InverseTransformVectorNoScale(InDirection);
		LocalPos = GetPointOnRectEdge(RootUIComp.Get(), FVector2D(localDir.Y, localDir.Z));
		if (auto CanvasGroup = RootUIComp->GetCanvasGroup())
		{
			if (auto RestrictNavCanvasGroup = CanvasGroup->GetRestrictNavigationAreaCanvasGroup())
			{
				RestrictNavNode = RestrictNavCanvasGroup->GetOwner()->GetRootComponent();
			}
		}
	}
	auto pos = GetRootSceneComponent()->GetComponentTransform().TransformPosition(LocalPos);
	float maxScore = -MAX_flt;
	UUISelectableComponent* bestPick = this;
	for (int i = 0; i < SelectableArray.Num(); ++i)
	{
		auto sel = SelectableArray[i];

		if (sel == this || !sel.IsValid())
			continue;

		if (IsValid(InParent) && !sel->GetRootSceneComponent()->IsAttachedTo(InParent))
			continue;

		if (!sel->IsInteractable())
			continue;

		//if is UI node, not allow inactive one
		auto selRootUIComp = sel->GetRootUIComponent();
		if (selRootUIComp && !sel->GetRootUIComponent()->GetIsUIActiveInHierarchy())
		{
			continue;
		}

		//if navigation is restricted, only allow child of restric node
		if (RestrictNavNode && !sel->GetRootSceneComponent()->IsAttachedTo(RestrictNavNode))
		{
			continue;
		}

#if WITH_EDITOR
		if (this->GetWorld() != sel->GetWorld())
			continue;
#endif

		FVector selCenter;
		if (selRootUIComp)
		{
			auto LocalCenter2D = selRootUIComp->GetLocalSpaceCenter();
			selCenter = FVector(0, LocalCenter2D.X, LocalCenter2D.Y);
		}
		else
		{
			selCenter = sel->GetRootSceneComponent()->GetRelativeLocation();
		}
		FVector myVector = sel->GetRootSceneComponent()->GetComponentTransform().TransformPosition(selCenter) - pos;

		float dot = FVector::DotProduct(InDirection, myVector);
		if (dot <= 0.0f)
			continue;

		float score = dot / myVector.SizeSquared();
		if (score > maxScore)
		{
			maxScore = score;
			bestPick = sel.Get();
		}
	}
	return bestPick;
}
UUISelectableComponent* UUISelectableComponent::FindDefaultSelectable(UObject* WorldContextObject)
{
	if (auto LGUIManagerActor = ULGUIManagerWorldSubsystem::GetInstance(WorldContextObject->GetWorld()))
	{
		const auto& SelectableArray = LGUIManagerActor->GetAllSelectableArray();
		if (SelectableArray.Num() > 0)
		{
			auto Selectable = SelectableArray[0].Get();
			//default selectable is the most "prev" one, so we need to find it
			TSet<UUISelectableComponent*> FoundSelectables;
			while (true)
			{
				FoundSelectables.Add(Selectable);
				//change navigation mode to auto, so we can find selectable only by position (exclude explicit)
				auto OriginNavigationLeftMode = Selectable->NavigationLeft;
				auto OriginNavigationUpMode = Selectable->NavigationUp;
				auto OriginNavigationPrevMode = Selectable->NavigationPrev;
				Selectable->NavigationLeft = EUISelectableNavigationMode::Auto;
				Selectable->NavigationUp = EUISelectableNavigationMode::Auto;
				Selectable->NavigationPrev = EUISelectableNavigationMode::Auto;

				auto PrevSelectable = Selectable->FindSelectableOnPrev();

				//restore navigation mode
				Selectable->NavigationLeft = OriginNavigationLeftMode;
				Selectable->NavigationUp = OriginNavigationUpMode;
				Selectable->NavigationPrev = OriginNavigationPrevMode;

				if (!IsValid(PrevSelectable) 
					|| PrevSelectable == Selectable
					|| FoundSelectables.Contains(PrevSelectable)//incase cycle loop, eg: A is left and B is top, A's top return B, and B's left return A
					)
				{
					break;
				}
				else
				{
					Selectable = PrevSelectable;
				}
			}
			return Selectable;
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
		return FindSelectable(-GetRootSceneComponent()->GetRightVector());
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
		return FindSelectable(GetRootSceneComponent()->GetRightVector());
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
		return FindSelectable(GetRootSceneComponent()->GetUpVector());
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
		return FindSelectable(-GetRootSceneComponent()->GetUpVector());
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnNext()
{
	if (NavigationNext == EUISelectableNavigationMode::Explicit && NavigationNextSpecific.IsValidComponentReference())
	{
		return NavigationNextSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationNext == EUISelectableNavigationMode::Auto)
	{
		auto rightComp = FindSelectableOnRight();
		if (rightComp != this)
		{
			return rightComp;
		}
		return FindSelectableOnDown();
	}
	return nullptr;
}
UUISelectableComponent* UUISelectableComponent::FindSelectableOnPrev()
{
	if (NavigationPrev == EUISelectableNavigationMode::Explicit)
	{
		return NavigationPrevSpecific.GetComponent<UUISelectableComponent>();
	}
	if (NavigationPrev == EUISelectableNavigationMode::Auto)
	{
		auto leftComp = FindSelectableOnLeft();
		if (leftComp != this)
		{
			return leftComp;
		}
		return FindSelectableOnUp();
	}
	return nullptr;
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

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
