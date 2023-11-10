// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/LGUIWidgetInteraction.h"
#include "LGUI.h"
#include "Extensions/LGUIWidget.h"
#include "Event/LGUIEventSystem.h"
#include "Framework/Application/SlateUser.h"
#include "Framework/Application/SlateApplication.h"
#include "Event/LGUIBaseRaycaster.h"

#define LOCTEXT_NAMESPACE "LGUIWidgetInteraction"

ULGUIWidgetInteractionManager* ULGUIWidgetInteractionManager::Instance = nullptr;

ULGUIWidgetInteraction::ULGUIWidgetInteraction()
{
	
}

bool ULGUIWidgetInteraction::OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)
{
	if (CurrentPointerEventData == nullptr)
	{
		CurrentPointerEventData = eventData;

		auto& Interactions = ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
		if (Interactions.CurrentInteraction == nullptr)
		{
			Interactions.CurrentInteraction = this;
			this->SetEnable(true);//hover in, enable update
		}
	}
	return bAllowEventBubbleUp;
}
bool ULGUIWidgetInteraction::OnPointerExit_Implementation(ULGUIPointerEventData* eventData)
{
	if (CurrentPointerEventData == eventData)
	{
		CurrentPointerEventData = nullptr;

		auto& Interactions = ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
		if (Interactions.CurrentInteraction == this)
		{
			SimulatePointerMovement();//pointer exit;
			Interactions.CurrentInteraction = nullptr;
			this->SetEnable(false);//hover out, disable update
		}
	}
	return bAllowEventBubbleUp;
}
bool ULGUIWidgetInteraction::OnPointerDown_Implementation(ULGUIPointerEventData* eventData)
{
	FKey PressKey;
	switch (eventData->mouseButtonType)
	{
	case EMouseButtonType::Left:
		PressKey = EKeys::LeftMouseButton;
		break;
	case EMouseButtonType::Middle:
		PressKey = EKeys::MiddleMouseButton;
		break;
	case EMouseButtonType::Right:
		PressKey = EKeys::RightMouseButton;
		break;
	}
	if (PressKey.IsValid())
	{
		PressPointerKey(PressKey);
	}
	return bAllowEventBubbleUp;
}
bool ULGUIWidgetInteraction::OnPointerUp_Implementation(ULGUIPointerEventData* eventData)
{
	FKey ReleaseKey;
	switch (eventData->mouseButtonType)
	{
	case EMouseButtonType::Left:
		ReleaseKey = EKeys::LeftMouseButton;
		break;
	case EMouseButtonType::Middle:
		ReleaseKey = EKeys::MiddleMouseButton;
		break;
	case EMouseButtonType::Right:
		ReleaseKey = EKeys::RightMouseButton;
		break;
	}
	if (ReleaseKey.IsValid())
	{
		ReleasePointerKey(ReleaseKey);
	}
	return bAllowEventBubbleUp;
}
bool ULGUIWidgetInteraction::OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)
{
	auto inAxisValue = eventData->scrollAxisValue;
	ScrollWheel(inAxisValue.Y);
	return bAllowEventBubbleUp;
}





void ULGUIWidgetInteraction::Awake()
{
	Super::Awake();

	if (ULGUIWidgetInteractionManager::Instance == nullptr)
	{
		ULGUIWidgetInteractionManager::Instance = NewObject<ULGUIWidgetInteractionManager>();
		ULGUIWidgetInteractionManager::Instance->AddToRoot();
	}
	Helper = ULGUIWidgetInteractionManager::Instance;
	// Only create another user in a real world. FindOrCreateVirtualUser changes focus
	if (FSlateApplication::IsInitialized() && !GetWorld()->IsPreviewWorld())
	{
		if (!VirtualUser.IsValid())
		{
			VirtualUser = FSlateApplication::Get().FindOrCreateVirtualUser(VirtualUserIndex);
			auto& Interactions = ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.FindOrAdd(VirtualUserIndex);
			Interactions.AllInteractions.Add(this);
		}
	}
	WidgetComponent = GetOwner()->FindComponentByClass<ULGUIWidget>();
	this->SetEnable(false);//disable update by default
}

void ULGUIWidgetInteraction::OnDestroy()
{
	Super::OnDestroy();

	if (FSlateApplication::IsInitialized())
	{
		if (VirtualUser.IsValid())
		{
			FSlateApplication::Get().UnregisterUser(VirtualUser->GetUserIndex());
			VirtualUser.Reset();
			auto& Interactions = ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction[VirtualUserIndex];
			Interactions.AllInteractions.Remove(this);
			if (Interactions.AllInteractions.Num() == 0)
			{
				ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.Remove(VirtualUserIndex);
			}
		}
	}

	if (ULGUIWidgetInteractionManager::Instance->MapVirtualUserIndexToInteraction.Num() == 0)
	{
		ULGUIWidgetInteractionManager::Instance->ConditionalBeginDestroy();
		ULGUIWidgetInteractionManager::Instance = nullptr;
	}
}

void ULGUIWidgetInteraction::Update(float DeltaTime)
{
	Super::Update(DeltaTime);
	
	SimulatePointerMovement();
}

bool ULGUIWidgetInteraction::CanSendInput()
{
	return FSlateApplication::IsInitialized() && VirtualUser.IsValid() && WidgetComponent != nullptr;
}

void ULGUIWidgetInteraction::SetCustomHitResult(const FHitResult& HitResult)
{
	CustomHitResult = HitResult;
}

void ULGUIWidgetInteraction::SetFocus(UWidget* FocusWidget)
{
	if (VirtualUser.IsValid())
	{
		FSlateApplication::Get().SetUserFocus(VirtualUser->GetUserIndex(), FocusWidget->GetCachedWidget(), EFocusCause::SetDirectly);
	}
}

void ULGUIWidgetInteraction::GetRelatedComponentsToIgnoreInAutomaticHitTesting(TArray<UPrimitiveComponent*>& IgnorePrimitives) const
{
	TArray<USceneComponent*> SceneChildren;
	if (AActor* Owner = GetOwner())
	{
		if (USceneComponent* Root = Owner->GetRootComponent())
		{
			Root = Root->GetAttachmentRoot();
			Root->GetChildrenComponents(true, SceneChildren);
			SceneChildren.Add(Root);
		}
	}

	for (USceneComponent* SceneComponent : SceneChildren)
	{
		if (UPrimitiveComponent* PrimtiveComponet = Cast<UPrimitiveComponent>(SceneComponent))
		{
			// Don't ignore widget components that are siblings.
			if (SceneComponent->IsA<ULGUIWidget>())
			{
				continue;
			}

			IgnorePrimitives.Add(PrimtiveComponet);
		}
	}
}

bool ULGUIWidgetInteraction::CanInteractWithComponent(ULGUIWidget* Component) const
{
	bool bCanInteract = false;

	if (Component)
	{
		bCanInteract = !GetWorld()->IsPaused() || Component->PrimaryComponentTick.bTickEvenWhenPaused;
	}

	return bCanInteract;
}

FWidgetPath ULGUIWidgetInteraction::DetermineWidgetUnderPointer()
{
	FWidgetPath WidgetPathUnderPointer;

	bIsHoveredWidgetInteractable = false;
	bIsHoveredWidgetFocusable = false;
	bIsHoveredWidgetHitTestVisible = false;

	LastLocalHitLocation = LocalHitLocation;
	FWidgetTraceResult TraceResult;
	if (CurrentPointerEventData != nullptr && CurrentPointerEventData->raycaster != nullptr)
	{
		auto RayOrigin = CurrentPointerEventData->raycaster->GetRayOrigin();
		auto RayDirection = CurrentPointerEventData->raycaster->GetRayDirection();
		auto RayEnd = RayOrigin + RayDirection * CurrentPointerEventData->raycaster->GetRayLength();

		WidgetComponent->GetLocalHitLocation(CurrentPointerEventData->faceIndex, CurrentPointerEventData->worldPoint, RayOrigin, RayEnd, TraceResult.LocalHitLocation);
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

void ULGUIWidgetInteraction::SimulatePointerMovement()
{
	if (!CanSendInput())
	{
		return;
	}

	FWidgetPath WidgetPathUnderFinger = DetermineWidgetUnderPointer();
	if (CurrentPointerEventData != nullptr)
	{
		PrevPointerIndex = CurrentPointerEventData->pointerID;
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

void ULGUIWidgetInteraction::PressPointerKey(FKey Key)
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

void ULGUIWidgetInteraction::ReleasePointerKey(FKey Key)
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

bool ULGUIWidgetInteraction::PressKey(FKey Key, bool bRepeat)
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

bool ULGUIWidgetInteraction::ReleaseKey(FKey Key)
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

void ULGUIWidgetInteraction::GetKeyAndCharCodes(const FKey& Key, bool& bHasKeyCode, uint32& KeyCode, bool& bHasCharCode, uint32& CharCode)
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

bool ULGUIWidgetInteraction::PressAndReleaseKey(FKey Key)
{
	const bool PressResult = PressKey(Key, false);
	const bool ReleaseResult = ReleaseKey(Key);

	return PressResult || ReleaseResult;
}

bool ULGUIWidgetInteraction::SendKeyChar(FString Characters, bool bRepeat)
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

void ULGUIWidgetInteraction::ScrollWheel(float ScrollDelta)
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

bool ULGUIWidgetInteraction::IsOverInteractableWidget() const
{
	return bIsHoveredWidgetInteractable;
}

bool ULGUIWidgetInteraction::IsOverFocusableWidget() const
{
	return bIsHoveredWidgetFocusable;
}

bool ULGUIWidgetInteraction::IsOverHitTestVisibleWidget() const
{
	return bIsHoveredWidgetHitTestVisible;
}

const FWeakWidgetPath& ULGUIWidgetInteraction::GetHoveredWidgetPath() const
{
	return LastWidgetPath;
}

FVector2D ULGUIWidgetInteraction::Get2DHitLocation() const
{
	return LocalHitLocation;
}

#undef LOCTEXT_NAMESPACE