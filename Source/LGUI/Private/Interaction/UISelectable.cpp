// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UISelectable.h"
#include "LGUI.h"
#include "LTweenBPLibrary.h"
#include "Core/Components/LexVisual.h"
#include "Core/LexUIManager.h"
#include "LTweenManager.h"
#include "Core/LexWidgetPresenterComponentBase.h"
#include "Core/Components/LexCanvas.h"
#include "Event/LexEventSystem.h"
#include "Core/Components/LexImage.h"
#include "Core/Components/LexWidget.h"
#include "Interaction/UINavigationInputSelectionHandler.h"


UUITransition::UUITransition()
{
	bStartWithTickEnabled = false;
}

void UUITransition::StopTransition() 
{ 
	for (auto tweener : TweenerCollection)
	{
		ULTweenBPLibrary::KillIfIsTweening(this, tweener);
	}
	TweenerCollection.Reset();
}
void UUITransition::CollectTweener(ULTweener* InItem)
{
	TweenerCollection.Add(InItem);
}
void UUITransition::CollectTweeners(const TSet<ULTweener*>& InItems)
{
	TweenerCollection.Reserve(TweenerCollection.Num() + InItems.Num());
	for (auto item : InItems)
	{
		TweenerCollection.Add(item);
	}
}

UUISelectable* UUISelectableTransition::GetSelectableComponent() const
{
	if (!IsValid(UISelectableComp))
	{
		UISelectableComp = GetWidget()->GetComponent<UUISelectable>();
	}
	return UISelectableComp;
}

void UUISelectableTransition::OnNormal(bool InImmediateSet)
{ 
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnNormal(InImmediateSet);
	}
}
void UUISelectableTransition::OnHovered(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnHovered(InImmediateSet);
	}
}
void UUISelectableTransition::OnPressed(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnPressed(InImmediateSet);
	}
}
void UUISelectableTransition::OnDisabled(bool InImmediateSet)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnDisabled(InImmediateSet);
	}
}

UUISelectable::UUISelectable()
{
	NormalColor = FColor(255, 255, 255, 255);
	HoveredColor = FColor(200, 200, 200, 255);
	PressedColor = FColor(150, 150, 150, 255);
	DisabledColor = FColor(150, 150, 150, 128);
}

void UUISelectable::Awake()
{
	Super::Awake();
}

void UUISelectable::OnRegister()
{
	Super::OnRegister();
	ULexUIManagerWorldSubsystem::AddSelectable(this);
}
void UUISelectable::OnUnregister()
{
	Super::OnUnregister();
	ULexUIManagerWorldSubsystem::RemoveSelectable(this);
}

#if WITH_EDITOR
void UUISelectable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property)
	{
		ApplyPointerSelectionState(true);
	}
}
#endif

void UUISelectable::OnInteractableChanged(bool IsEnabled)
{
	Super::OnInteractableChanged(IsEnabled);
	CurrentSelectionState = GetSelectionState();
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())//is editor, just set properties immediately
	{
		ApplyPointerSelectionState(true);
	}
	else
#endif
	{
		ApplyPointerSelectionState(false);
	}
}

void UUISelectable::ApplyPointerSelectionState(bool ImmediateSet)
{
	if (TransitionType != EUISelectableTransitionType::Custom)
	{
		if (!TransitionTarget.IsValid())return;
	}

	TOptional<FColor> Color;
	TOptional<FLexUIImageBrush> Brush;
	switch (CurrentSelectionState)
	{
	case EUISelectableSelectionState::Normal:
		{
			switch (TransitionType)
			{
			case EUISelectableTransitionType::None:break;
			case EUISelectableTransitionType::Color:
				{
					Color = NormalColor;
				}
				break;
			case EUISelectableTransitionType::ImageBrush:
				{
					Brush = NormalImageBrush;
				}
				break;
			case EUISelectableTransitionType::Custom:
				{
					if (CustomTransition.IsValid())
					{
						CustomTransition->OnNormal(ImmediateSet);
					}
				}
				break;
			}
		}
		break;
	case EUISelectableSelectionState::Hovered:
		{
			switch (TransitionType)
			{
			case EUISelectableTransitionType::None:break;
			case EUISelectableTransitionType::Color:
				{
					Color = HoveredColor;
				}
				break;
			case EUISelectableTransitionType::ImageBrush:
				{
					Brush = HoveredImageBrush;
				}
				break;
			case EUISelectableTransitionType::Custom:
				{
					if (CustomTransition.IsValid())
					{
						CustomTransition->OnHovered(ImmediateSet);
					}
				}
				break;
			}
		}
		break;
	case EUISelectableSelectionState::Pressed:
		{
			switch (TransitionType)
			{
			case EUISelectableTransitionType::None:break;
			case EUISelectableTransitionType::Color:
				{
					Color = PressedColor;
				}
				break;
			case EUISelectableTransitionType::ImageBrush:
				{
					Brush = PressedImageBrush;
				}
				break;
			case EUISelectableTransitionType::Custom:
				{
					if (CustomTransition.IsValid())
					{
						CustomTransition->OnPressed(ImmediateSet);
					}
				}
				break;
			}
		}
		break;
	case EUISelectableSelectionState::Disabled:
		{
			switch (TransitionType)
			{
			case EUISelectableTransitionType::None:break;
			case EUISelectableTransitionType::Color:
				{
					Color = DisabledColor;
				}
				break;
			case EUISelectableTransitionType::ImageBrush:
				{
					Brush =  DisabledImageBrush;
				}
				break;
			case EUISelectableTransitionType::Custom:
				{
					if (CustomTransition.IsValid())
					{
						CustomTransition->OnDisabled(ImmediateSet);
					}
				}
				break;
			}
		}
		break;
	}

	if (Color.IsSet())
	{
		if (AnimDuration <= 0.0f || ImmediateSet)
		{
			TransitionTarget->SetColor(Color.GetValue());
		}
		else
		{
			if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
			TransitionTweener = ULTweenManager::To(TransitionTarget.Get()
				, FLTweenColorGetterFunction::CreateWeakLambda(TransitionTarget.Get(), [=, this]()
			{
				return TransitionTarget->GetColor();
			}), FLTweenColorSetterFunction::CreateUObject(TransitionTarget.Get(), &ULexVisual::SetColor), Color.GetValue(), AnimDuration);
			if (TransitionTweener)
			{
				ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), TransitionTweener);
			}
		}
	}
	if (Brush.IsSet())
	{
		if (auto TransitionTargetAsLexImage = Cast<ULexImage>(TransitionTarget.Get()))
		{
			if (IsValid(Brush.GetValue().GetResourceObject()))
			{
				TransitionTargetAsLexImage->SetBrush(Brush.GetValue());
			}
			else
			{
				if (AnimDuration <= 0.0f || ImmediateSet)
				{
					TransitionTargetAsLexImage->SetBrushTintColor(Brush.GetValue().TintColor);
				}
				else
				{
					if (ULTweenManager::IsTweening(this, TransitionTweener))TransitionTweener->Kill();
					TransitionTweener = ULTweenManager::To(TransitionTargetAsLexImage
						, FLTweenColorGetterFunction::CreateWeakLambda(TransitionTargetAsLexImage, [=, this]()
					{
						return TransitionTargetAsLexImage->GetBrush().TintColor;
					}), FLTweenColorSetterFunction::CreateUObject(TransitionTargetAsLexImage, &ULexImage::SetBrushTintColor), Brush.GetValue().TintColor, AnimDuration);
					if (TransitionTweener)
					{
						ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), TransitionTweener);
					}
				}
			}
		}
	}
}

bool UUISelectable::CheckNavigationSelectionState()
{
	if (!NavigationSelection.IsValid())
	{
		if (auto Widget = GetWidget())
		{
			if (auto Canvas = Widget->GetRootCanvas())
			{
				if (auto WidgetPresenter = Canvas->GetWidgetPresenterComponent())
				{
					NavigationSelection = WidgetPresenter->GetNavigationSelection();
				}
			}
		}
	}
	return NavigationSelection.IsValid();
}

void UUISelectable::OnPointerEnter_Implementation(ULexPointerEventData* EventData)
{
	bIsPointerInsideThis = true;
	CurrentSelectionState = GetSelectionState();
	ApplyPointerSelectionState(false);
	if (EventData->InputType == ELexUIPointerInputType::Navigation)
	{
		if (CheckNavigationSelectionState())
		{
			NavigationSelection->SelectWidget(GetWidget());
		}
	}
	else
	{
		if (NavigationSelection.IsValid())
		{
			NavigationSelection->SelectNone();
		}
	}
}
void UUISelectable::OnPointerExit_Implementation(ULexPointerEventData* EventData)
{
	bIsPointerInsideThis = false;
	CurrentSelectionState = GetSelectionState();
	ApplyPointerSelectionState(false);
}
bool UUISelectable::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
	bIsPointerDown = true;
	CurrentSelectionState = GetSelectionState();
	ApplyPointerSelectionState(false);
	if (auto EventSystem = ULexEventSystem::GetLexEventSystemInstance(this, IsValid(EventData) ? EventData->UserIndex : 0))
	{
		EventSystem->SetSelectWidget(GetWidget(), EventData);
	}
	return AllowEventBubbleUp;
}
bool UUISelectable::OnPointerUp_Implementation(ULexPointerEventData* EventData)
{
	bIsPointerDown = false;
	CurrentSelectionState = GetSelectionState();
	ApplyPointerSelectionState(false);
	return AllowEventBubbleUp;
}
bool UUISelectable::OnSelect_Implementation(ULexBaseEventData* EventData)
{
	return AllowEventBubbleUp;
}
bool UUISelectable::OnDeselect_Implementation(ULexBaseEventData* EventData)
{
	return AllowEventBubbleUp;
}

EUISelectableSelectionState UUISelectable::GetSelectionState()const
{
	if (!IsInteractable())
		return EUISelectableSelectionState::Disabled;
	if (bIsPointerDown)
		return EUISelectableSelectionState::Pressed;
	if (bIsPointerInsideThis)
		return EUISelectableSelectionState::Hovered;
	return EUISelectableSelectionState::Normal;
}

void UUISelectable::SetTransitionTarget(ULexVisual* Value)
{
	if (TransitionTarget != Value)
	{
		TransitionTarget = Value;
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetNormalColor(FColor Value)
{
	NormalColor = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Normal)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetHoveredColor(FColor Value)
{
	HoveredColor = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Hovered)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetPressedColor(FColor Value)
{
	PressedColor = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Pressed)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetDisabledColor(FColor Value)
{
	DisabledColor = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Disabled)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetNormalImageBrush(const FLexUIImageBrush& Value)
{
	NormalImageBrush = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Normal)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetHoveredImageBrush(const FLexUIImageBrush& Value)
{
	HoveredImageBrush = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Hovered)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetPressedImageBrush(const FLexUIImageBrush& Value)
{
	PressedImageBrush = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Pressed)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetDisabledImageBrush(const FLexUIImageBrush& Value)
{
	DisabledImageBrush = Value;
	if (CurrentSelectionState == EUISelectableSelectionState::Disabled)
	{
		ApplyPointerSelectionState(false);
	}
}
void UUISelectable::SetSelectionState(EUISelectableSelectionState NewState)
{
	if (CurrentSelectionState != NewState)
	{
		CurrentSelectionState = NewState;
		ApplyPointerSelectionState(false);
	}
}
bool UUISelectable::IsInteractable()const
{
	if (auto Widget = GetWidget())
	{
		return Widget->GetWidgetActiveInHierarchy() && Widget->GetInteractableInHierarchy() && bInteractable;
	}
	return bInteractable;
}

#pragma region Navigation
bool UUISelectable::CanNavigateHere_Implementation() const
{
	return IsInteractable() && GetCanNavigateHere();
}
bool UUISelectable::OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)
{
	UUISelectable* Selectable = nullptr;
	switch (direction)
	{
	default:
	case ELexUINavigationDirection::None:
		return false;
		break;
	case ELexUINavigationDirection::Left:
		Selectable = FindSelectableOnLeft();
		break;
	case ELexUINavigationDirection::Right:
		Selectable = FindSelectableOnRight();
		break;
	case ELexUINavigationDirection::Up:
		Selectable = FindSelectableOnUp();
		break;
	case ELexUINavigationDirection::Down:
		Selectable = FindSelectableOnDown();
		break;
	case ELexUINavigationDirection::Prev:
		Selectable = FindSelectableOnPrev();
		break;
	case ELexUINavigationDirection::Next:
		Selectable = FindSelectableOnNext();
		break;
	}
	result = Selectable;
	return true;
}
UUISelectable* UUISelectable::FindSelectable(FVector InDirection)
{
	InDirection.Normalize();
	if (auto Widget = GetWidget())
	{
		if (Widget->GetRenderCanvas() == nullptr || Widget->GetRenderCanvas()->GetRootCanvas() == nullptr)
		{
			return nullptr;//not active render
		}
		if (Widget->IsScreenSpaceOverlayUI() || Widget->IsRenderTargetUI())
		{
			auto rootCanvasUIItem = Widget->GetRootCanvas()->GetWidget();
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
}

UUISelectable* UUISelectable::FindSelectable(FVector InDirection, ULexWidget* InParent)
{
	auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld());
	if (LexUIManager == nullptr)return nullptr;
	const auto& SelectableArray = LexUIManager->GetAllSelectableArray();

	auto GetPointOnRectEdge = [](ULexWidget* rect, FVector2D dir)
	{
		if (dir != FVector2D::ZeroVector)
			dir /= FMath::Max(FMath::Abs(dir.X), FMath::Abs(dir.Y));
		auto center = rect->GetLocalSpaceCenter();
		dir = center + FVector2D(rect->GetWidth() * dir.X * 0.5f, rect->GetHeight() * dir.Y * 0.5f);
		return FVector(0, dir.X, dir.Y);
	};

	auto LocalPos = FVector::ZeroVector;
	const ULexWidget* RestrictNavNode = nullptr;
	if (auto Widget = GetWidget())
	{
		auto localDir = Widget->GetWorldTransform().InverseTransformVectorNoScale(InDirection);
		LocalPos = GetPointOnRectEdge(Widget, FVector2D(localDir.Y, localDir.Z));
		if (auto RestrictNavWidget = Widget->GetRestrictNavigationAreaWidget())
		{
			RestrictNavNode = RestrictNavWidget;
		}
	}
	auto pos = this->GetWidget()->GetWorldTransform().TransformPosition(LocalPos);
	auto thisWidget = this->GetWidget();
	float maxScore = -MAX_flt;
	UUISelectable* bestPick = this;
	for (int i = 0; i < SelectableArray.Num(); ++i)
	{
		auto sel = SelectableArray[i];

		if (sel == this || !sel.IsValid())
			continue;

		if (IsValid(InParent) && !sel->GetWidget()->IsChildOf(InParent))
			continue;

		if (!sel->IsInteractable())
			continue;

		if (!sel->GetCanNavigateHere())
			continue;

		//if is UI node, not allow inactive one
		auto selWidget = sel->GetWidget();
		if (selWidget && !sel->GetWidget()->GetInteractableInHierarchy())
		{
			continue;
		}

		if (selWidget && thisWidget)
		{
			if (selWidget->IsWorldSpaceUI() != thisWidget->IsWorldSpaceUI())
			{
				continue;
			}
		}

		//if navigation is restricted, only allow child of restrict node
		if (RestrictNavNode && !sel->GetWidget()->IsChildOf(RestrictNavNode))
		{
			continue;
		}

#if WITH_EDITOR
		if (this->GetWorld() != sel->GetWorld())
			continue;
#endif

		FVector selCenter;
		if (selWidget)
		{
			auto LocalCenter2D = selWidget->GetLocalSpaceCenter();
			selCenter = FVector(0, LocalCenter2D.X, LocalCenter2D.Y);
		}
		else
		{
			selCenter = sel->GetWidget()->GetRelativeLocation();
		}
		auto selCenterInWorld = sel->GetWidget()->GetWorldTransform().TransformPosition(selCenter);
		if (selWidget)
		{
			if (!selWidget->IsPointVisibleOnClip(selCenterInWorld))
			{
				continue;//if not visible then skip it
			}
		}
		FVector myVector = selCenterInWorld - pos;

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
UUISelectable* UUISelectable::FindDefaultSelectable(UObject* WorldContextObject)
{
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(WorldContextObject->GetWorld()))
	{
		const auto& SelectableArray = LexUIManager->GetAllSelectableArray();
		if (SelectableArray.Num() > 0)
		{
			UUISelectable* Selectable = nullptr;
			for (int i = 0; i < SelectableArray.Num(); i++)
			{
				auto SelectableItem = SelectableArray[i];
				if (SelectableItem->IsInteractable() && SelectableItem->GetCanNavigateHere())
				{
					Selectable = SelectableItem.Get();//find a interactable one
					break;
				}
			}
			if (Selectable)
			{
				//default selectable is the most "prev" one, so we need to find it
				TSet<UUISelectable*> FoundSelectables;
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
	}
	return nullptr;
}
UUISelectable* UUISelectable::FindSelectableOnLeft()
{
	if (NavigationLeft == EUISelectableNavigationMode::Explicit)
	{
		return NavigationLeftSpecific.Get();
	}
	if (NavigationLeft == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(-GetWidget()->GetRightVector());
	}
	return nullptr;
}
UUISelectable* UUISelectable::FindSelectableOnRight()
{
	if (NavigationRight == EUISelectableNavigationMode::Explicit)
	{
		return NavigationRightSpecific.Get();
	}
	if (NavigationRight == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(GetWidget()->GetRightVector());
	}
	return nullptr;
}
UUISelectable* UUISelectable::FindSelectableOnUp()
{
	if (NavigationUp == EUISelectableNavigationMode::Explicit)
	{
		return NavigationUpSpecific.Get();
	}
	if (NavigationUp == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(GetWidget()->GetUpVector());
	}
	return nullptr;
}
UUISelectable* UUISelectable::FindSelectableOnDown()
{
	if (NavigationDown == EUISelectableNavigationMode::Explicit)
	{
		return NavigationDownSpecific.Get();
	}
	if (NavigationDown == EUISelectableNavigationMode::Auto)
	{
		return FindSelectable(-GetWidget()->GetUpVector());
	}
	return nullptr;
}
UUISelectable* UUISelectable::FindSelectableOnNext()
{
	if (NavigationNext == EUISelectableNavigationMode::Explicit)
	{
		return NavigationNextSpecific.Get();
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
UUISelectable* UUISelectable::FindSelectableOnPrev()
{
	if (NavigationPrev == EUISelectableNavigationMode::Explicit)
	{
		return NavigationPrevSpecific.Get();
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

void UUISelectable::SetCanNavigateHere(bool Value)
{
	bCanNavigateHere = Value;
}
void UUISelectable::SetNavigationLeft(EUISelectableNavigationMode Value)
{
	NavigationLeft = Value;
}
void UUISelectable::SetNavigationRight(EUISelectableNavigationMode Value)
{
	NavigationRight = Value;
}
void UUISelectable::SetNavigationUp(EUISelectableNavigationMode Value)
{
	NavigationUp = Value;
}
void UUISelectable::SetNavigationDown(EUISelectableNavigationMode Value)
{
	NavigationDown = Value;
}
void UUISelectable::SetNavigationPrev(EUISelectableNavigationMode Value)
{
	NavigationPrev = Value;
}
void UUISelectable::SetNavigationNext(EUISelectableNavigationMode Value)
{
	NavigationNext = Value;
}

void UUISelectable::SetNavigationLeftExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationLeftSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUISelectable::SetNavigationRightExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationRightSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUISelectable::SetNavigationUpExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationUpSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUISelectable::SetNavigationDownExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationDownSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUISelectable::SetNavigationPrevExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationPrevSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void UUISelectable::SetNavigationNextExplicit(UUISelectable* Value)
{
	if (IsValid(Value))
	{
		NavigationNextSpecific = Value;
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Value is not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
#pragma endregion


