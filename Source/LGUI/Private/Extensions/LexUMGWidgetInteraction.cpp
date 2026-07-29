// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/LexUMGWidgetInteraction.h"

#include "Core/Components/LexWidget.h"
#include "Extensions/LexUMGWidget.h"
#include "Framework/Application/SlateUser.h"
#include "Framework/Application/SlateApplication.h"
#include "Event/LexBaseRaycaster.h"

#define LOCTEXT_NAMESPACE "UIWidgetInteraction"

ULexUMGWidgetInteractionManager* ULexUMGWidgetInteractionManager::Instance = nullptr;

ULexUMGWidgetInteraction::ULexUMGWidgetInteraction()
{
	
}

void ULexUMGWidgetInteraction::OnPointerEnter_Implementation(ULexPointerEventData* EventData)
{
	if (CurrentPointerEventData == nullptr)
	{
		CurrentPointerEventData = EventData;

		auto& Interactions = ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
		if (Interactions.CurrentInteraction == nullptr)
		{
			Interactions.CurrentInteraction = this;
			this->SetCanExecuteTick(true);//hover in, enable update
		}
	}
}
void ULexUMGWidgetInteraction::OnPointerExit_Implementation(ULexPointerEventData* EventData)
{
	if (CurrentPointerEventData == EventData)
	{
		CurrentPointerEventData = nullptr;

		auto& Interactions = ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
		if (Interactions.CurrentInteraction == this)
		{
			SimulatePointerMovement();//pointer exit;
			Interactions.CurrentInteraction = nullptr;
			this->SetCanExecuteTick(false);//hover out, disable update
		}
	}
}
bool ULexUMGWidgetInteraction::OnPointerDown_Implementation(ULexPointerEventData* EventData)
{
	FKey PressKey;
	switch (EventData->MouseButtonType)
	{
	case ELexUIMouseButtonType::Left:
		PressKey = EKeys::LeftMouseButton;
		break;
	case ELexUIMouseButtonType::Middle:
		PressKey = EKeys::MiddleMouseButton;
		break;
	case ELexUIMouseButtonType::Right:
		PressKey = EKeys::RightMouseButton;
		break;
	}
	if (PressKey.IsValid())
	{
		PressPointerKey(PressKey);
	}
	return bAllowEventBubbleUp;
}
bool ULexUMGWidgetInteraction::OnPointerUp_Implementation(ULexPointerEventData* EventData)
{
	FKey ReleaseKey;
	switch (EventData->MouseButtonType)
	{
	case ELexUIMouseButtonType::Left:
		ReleaseKey = EKeys::LeftMouseButton;
		break;
	case ELexUIMouseButtonType::Middle:
		ReleaseKey = EKeys::MiddleMouseButton;
		break;
	case ELexUIMouseButtonType::Right:
		ReleaseKey = EKeys::RightMouseButton;
		break;
	}
	if (ReleaseKey.IsValid())
	{
		ReleasePointerKey(ReleaseKey);
	}
	return bAllowEventBubbleUp;
}
bool ULexUMGWidgetInteraction::OnPointerScroll_Implementation(ULexPointerEventData* EventData)
{
	auto inAxisValue = EventData->ScrollAxisValue;
	ScrollWheel(inAxisValue.Y);
	return bAllowEventBubbleUp;
}





void ULexUMGWidgetInteraction::Awake()
{
	Super::Awake();

	if (ULexUMGWidgetInteractionManager::Instance == nullptr)
	{
		ULexUMGWidgetInteractionManager::Instance = NewObject<ULexUMGWidgetInteractionManager>();
		ULexUMGWidgetInteractionManager::Instance->AddToRoot();
	}
	Helper = ULexUMGWidgetInteractionManager::Instance;
	// Only create another user in a real world. FindOrCreateVirtualUser changes focus
	if (FSlateApplication::IsInitialized() && !GetWorld()->IsPreviewWorld())
	{
		if (!VirtualUser.IsValid())
		{
			VirtualUser = FSlateApplication::Get().FindOrCreateVirtualUser(VirtualUserIndex);
			auto& Interactions = ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.FindOrAdd(VirtualUserIndex);
			Interactions.AllInteractions.Add(this);
		}
	}
	WidgetComponent = Cast<ULexUMGWidget>(GetWidget()->GetVisual());
	this->SetCanExecuteTick(false);//disable update by default
}

void ULexUMGWidgetInteraction::OnDestroy()
{
	Super::OnDestroy();

	if (FSlateApplication::IsInitialized())
	{
		if (VirtualUser.IsValid())
		{
			FSlateApplication::Get().UnregisterUser(VirtualUser->GetUserIndex());
			VirtualUser.Reset();
			auto& Interactions = ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
			Interactions.AllInteractions.Remove(this);
			if (Interactions.AllInteractions.Num() == 0)
			{
				ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.Remove(VirtualUserIndex);
			}
		}
	}

	if (ULexUMGWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.Num() == 0)
	{
		ULexUMGWidgetInteractionManager::Instance->ConditionalBeginDestroy();
		ULexUMGWidgetInteractionManager::Instance = nullptr;
	}
}

void ULexUMGWidgetInteraction::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	SimulatePointerMovement();
}

bool ULexUMGWidgetInteraction::CanSendInput()
{
	return FSlateApplication::IsInitialized() && VirtualUser.IsValid() && WidgetComponent != nullptr;
}

void ULexUMGWidgetInteraction::SetCustomHitResult(const FLexUIHitResult& HitResult)
{
	CustomHitResult = HitResult;
}

void ULexUMGWidgetInteraction::SetFocus(UWidget* FocusWidget)
{
	if (VirtualUser.IsValid())
	{
		FSlateApplication::Get().SetUserFocus(VirtualUser->GetUserIndex(), FocusWidget->GetCachedWidget(), EFocusCause::SetDirectly);
	}
}

bool ULexUMGWidgetInteraction::CanInteractWithComponent(ULexUMGWidget* Component) const
{
	bool bCanInteract = false;

	if (Component)
	{
		bCanInteract = !GetWorld()->IsPaused()
		//|| Component->PrimaryComponentTick.bTickEvenWhenPaused;
		;
	}

	return bCanInteract;
}

FWidgetPath ULexUMGWidgetInteraction::DetermineWidgetUnderPointer()
{
	FWidgetPath WidgetPathUnderPointer;

	bIsHoveredWidgetInteractable = false;
	bIsHoveredWidgetFocusable = false;
	bIsHoveredWidgetHitTestVisible = false;

	LastLocalHitLocation = LocalHitLocation;
	FWidgetTraceResult TraceResult;
	if (CurrentPointerEventData != nullptr && CurrentPointerEventData->Raycaster != nullptr)
	{
		auto RayOrigin = CurrentPointerEventData->Raycaster->GetRayOrigin();
		auto RayDirection = CurrentPointerEventData->Raycaster->GetRayDirection();
		auto RayEnd = RayOrigin + RayDirection * CurrentPointerEventData->Raycaster->GetRayLength();

		WidgetComponent->GetLocalHitLocation(CurrentPointerEventData->FaceIndex, CurrentPointerEventData->WorldPoint, RayOrigin, RayEnd, TraceResult.LocalHitLocation);
		TraceResult.HitWidgetPath = FWidgetPath(WidgetComponent->GetHitWidgetPath(TraceResult.LocalHitLocation, /*bIgnoreEnabledStatus*/ false));

		LocalHitLocation = TraceResult.LocalHitLocation;
	}
	WidgetPathUnderPointer = TraceResult.HitWidgetPath;

	WidgetComponent->RequestRenderUpdate();

	if (WidgetPathUnderPointer.IsValid())
	{
		const FArrangedChildren::FArrangedWidgetArray& AllArrangedWidgets = WidgetPathUnderPointer.Widgets.GetInternalArray();
		for (const FArrangedWidget& ArrangedWidget : AllArrangedWidgets)
		{
			const TSharedRef<SWidget>& Widget = ArrangedWidget.Widget;
			if (Widget->IsEnabled())
			{
				if (Widget->IsInteractable())
				{
					bIsHoveredWidgetInteractable = true;
				}

				if (Widget->SupportsKeyboardFocus())
				{
					bIsHoveredWidgetFocusable = true;
				}
			}

			if (Widget->GetVisibility().IsHitTestVisible())
			{
				bIsHoveredWidgetHitTestVisible = true;
			}
		}
	}

	return WidgetPathUnderPointer;
}

void ULexUMGWidgetInteraction::SimulatePointerMovement()
{
	if (!CanSendInput())
	{
		return;
	}

	FWidgetPath WidgetPathUnderFinger = DetermineWidgetUnderPointer();
	if (CurrentPointerEventData != nullptr)
	{
		PrevPointerIndex = CurrentPointerEventData->PointerID;
	}
	if (PrevPointerIndex >= 0)
	{
		FPointerEvent PointerEvent(
			VirtualUser->GetUserIndex(),
			(uint32)PrevPointerIndex,
			LocalHitLocation,
			LastLocalHitLocation,
			PressedKeys,
			FKey(),
			0.0f,
			ModifierKeys);

		if (WidgetPathUnderFinger.IsValid())
		{
			check(WidgetComponent);
			LastWidgetPath = WidgetPathUnderFinger;
			FSlateApplication::Get().RoutePointerMoveEvent(WidgetPathUnderFinger, PointerEvent, false);
		}
		else
		{
			FWidgetPath EmptyWidgetPath;
			FSlateApplication::Get().RoutePointerMoveEvent(EmptyWidgetPath, PointerEvent, false);

			LastWidgetPath = FWeakWidgetPath();
		}
	}
}

void ULexUMGWidgetInteraction::PressPointerKey(FKey Key)
{
	if (!CanSendInput())
	{
		return;
	}

	if (PressedKeys.Contains(Key))
	{
		return;
	}

	PressedKeys.Add(Key);

	if (!LastWidgetPath.IsValid())
	{
		// If the cached widget path isn't valid, attempt to find a valid widget since we might have received a touch input
		LastWidgetPath = DetermineWidgetUnderPointer();
	}

	FWidgetPath WidgetPathUnderFinger = LastWidgetPath.ToWidgetPath();
	if (PrevPointerIndex >= 0)
	{
		FPointerEvent PointerEvent;
		if (Key.IsTouch())
		{
			PointerEvent = FPointerEvent(
				VirtualUser->GetUserIndex(),
				(uint32)PrevPointerIndex,
				LocalHitLocation,
				LastLocalHitLocation,
				1.0f,
				false);

		}
		else
		{
			PointerEvent = FPointerEvent(
				VirtualUser->GetUserIndex(),
				(uint32)PrevPointerIndex,
				LocalHitLocation,
				LastLocalHitLocation,
				PressedKeys,
				Key,
				0.0f,
				ModifierKeys);
		}


		FReply Reply = FSlateApplication::Get().RoutePointerDownEvent(WidgetPathUnderFinger, PointerEvent);

		// @TODO Something about double click, expose directly, or automatically do it if key press happens within
		// the double click timeframe?
		//Reply = FSlateApplication::Get().RoutePointerDoubleClickEvent( WidgetPathUnderFinger, PointerEvent );
	}
}

void ULexUMGWidgetInteraction::ReleasePointerKey(FKey Key)
{
	if (!CanSendInput())
	{
		return;
	}

	if (!PressedKeys.Contains(Key))
	{
		return;
	}

	PressedKeys.Remove(Key);

	FWidgetPath WidgetPathUnderFinger = LastWidgetPath.ToWidgetPath();
	// Need to clear the widget path for cases where the component isn't ticking/clearing itself.
	LastWidgetPath = FWeakWidgetPath();
	if (PrevPointerIndex >= 0)
	{
		FPointerEvent PointerEvent;
		if (Key.IsTouch())
		{
			PointerEvent = FPointerEvent(
				VirtualUser->GetUserIndex(),
				(uint32)PrevPointerIndex,
				LocalHitLocation,
				LastLocalHitLocation,
				1.0f,
				false);
		}
		else
		{
			PointerEvent = FPointerEvent(
				VirtualUser->GetUserIndex(),
				(uint32)PrevPointerIndex,
				LocalHitLocation,
				LastLocalHitLocation,
				PressedKeys,
				Key,
				0.0f,
				ModifierKeys);
		}

		FReply Reply = FSlateApplication::Get().RoutePointerUpEvent(WidgetPathUnderFinger, PointerEvent);
	}
}

bool ULexUMGWidgetInteraction::PressKey(FKey Key, bool bRepeat)
{
	if (!CanSendInput())
	{
		return false;
	}

	bool bHasKeyCode, bHasCharCode;
	uint32 KeyCode, CharCode;
	GetKeyAndCharCodes(Key, bHasKeyCode, KeyCode, bHasCharCode, CharCode);

	FKeyEvent KeyEvent(Key, ModifierKeys, VirtualUser->GetUserIndex(), bRepeat, CharCode, KeyCode);
	bool bDownResult = FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent);

	bool bKeyCharResult = false;
	if (bHasCharCode)
	{
		FCharacterEvent CharacterEvent(CharCode, ModifierKeys, VirtualUser->GetUserIndex(), bRepeat);
		bKeyCharResult = FSlateApplication::Get().ProcessKeyCharEvent(CharacterEvent);
	}

	return bDownResult || bKeyCharResult;
}

bool ULexUMGWidgetInteraction::ReleaseKey(FKey Key)
{
	if (!CanSendInput())
	{
		return false;
	}

	bool bHasKeyCode, bHasCharCode;
	uint32 KeyCode, CharCode;
	GetKeyAndCharCodes(Key, bHasKeyCode, KeyCode, bHasCharCode, CharCode);

	FKeyEvent KeyEvent(Key, ModifierKeys, VirtualUser->GetUserIndex(), false, CharCode, KeyCode);
	return FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
}

void ULexUMGWidgetInteraction::GetKeyAndCharCodes(const FKey& Key, bool& bHasKeyCode, uint32& KeyCode, bool& bHasCharCode, uint32& CharCode)
{
	const uint32* KeyCodePtr;
	const uint32* CharCodePtr;
	FInputKeyManager::Get().GetCodesFromKey(Key, KeyCodePtr, CharCodePtr);

	bHasKeyCode = KeyCodePtr ? true : false;
	bHasCharCode = CharCodePtr ? true : false;

	KeyCode = KeyCodePtr ? *KeyCodePtr : 0;
	CharCode = CharCodePtr ? *CharCodePtr : 0;

	// These special keys are not handled by the platform layer, and while not printable
	// have character mappings that several widgets look for, since the hardware sends them.
	if (CharCodePtr == nullptr)
	{
		if (Key == EKeys::Tab)
		{
			CharCode = '\t';
			bHasCharCode = true;
		}
		else if (Key == EKeys::BackSpace)
		{
			CharCode = '\b';
			bHasCharCode = true;
		}
		else if (Key == EKeys::Enter)
		{
			CharCode = '\n';
			bHasCharCode = true;
		}
	}
}

bool ULexUMGWidgetInteraction::PressAndReleaseKey(FKey Key)
{
	const bool PressResult = PressKey(Key, false);
	const bool ReleaseResult = ReleaseKey(Key);

	return PressResult || ReleaseResult;
}

bool ULexUMGWidgetInteraction::SendKeyChar(FString Characters, bool bRepeat)
{
	if (!CanSendInput())
	{
		return false;
	}

	bool bProcessResult = false;

	for (int32 CharIndex = 0; CharIndex < Characters.Len(); CharIndex++)
	{
		TCHAR CharKey = Characters[CharIndex];

		FCharacterEvent CharacterEvent(CharKey, ModifierKeys, VirtualUser->GetUserIndex(), bRepeat);
		bProcessResult |= FSlateApplication::Get().ProcessKeyCharEvent(CharacterEvent);
	}

	return bProcessResult;
}

void ULexUMGWidgetInteraction::ScrollWheel(float ScrollDelta)
{
	if (!CanSendInput())
	{
		return;
	}

	if (PrevPointerIndex >= 0)
	{
		FWidgetPath WidgetPathUnderFinger = LastWidgetPath.ToWidgetPath();
		FPointerEvent MouseWheelEvent(
			VirtualUser->GetUserIndex(),
			(uint32)PrevPointerIndex,
			LocalHitLocation,
			LastLocalHitLocation,
			PressedKeys,
			EKeys::MouseWheelAxis,
			ScrollDelta,
			ModifierKeys);

		FSlateApplication::Get().RouteMouseWheelOrGestureEvent(WidgetPathUnderFinger, MouseWheelEvent, nullptr);
	}
}

bool ULexUMGWidgetInteraction::IsOverInteractableWidget() const
{
	return bIsHoveredWidgetInteractable;
}

bool ULexUMGWidgetInteraction::IsOverFocusableWidget() const
{
	return bIsHoveredWidgetFocusable;
}

bool ULexUMGWidgetInteraction::IsOverHitTestVisibleWidget() const
{
	return bIsHoveredWidgetHitTestVisible;
}

const FWeakWidgetPath& ULexUMGWidgetInteraction::GetHoveredWidgetPath() const
{
	return LastWidgetPath;
}

FVector2D ULexUMGWidgetInteraction::Get2DHitLocation() const
{
	return LocalHitLocation;
}
#undef LOCTEXT_NAMESPACE