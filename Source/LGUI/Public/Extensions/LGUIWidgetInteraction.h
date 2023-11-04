// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "Components/WidgetInteractionComponent.h"
#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "LGUIWidgetInteraction.generated.h"

class ULGUIWidget;
class ULGUIWidgetInteraction;

UCLASS()
class LGUI_API ULGUIWidgetInteractionManager : public UObject
{
	GENERATED_BODY()
public:

	static ULGUIWidgetInteractionManager* Instance;
	struct FInteractionContainer
	{
		TArray<ULGUIWidgetInteraction*> AllInteractions;
		ULGUIWidgetInteraction* CurrentInteraction = nullptr;
	};
	TMap<int, FInteractionContainer> MapVirtualUserIndexToInteraction;
};

/**
 * Perform a raycaster and interaction for LGUIRenderTargetGeometrySource object, which shows the LGUI RenderTarget UI.
 * This component should be placed on a actor which have a LGUIRenderTargetGeometrySource component.
 */
UCLASS(ClassGroup = LGUI, meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUIWidgetInteraction : public ULGUILifeCycleUIBehaviour
	, public ILGUIPointerEnterExitInterface
	, public ILGUIPointerDownUpInterface
	, public ILGUIPointerScrollInterface
{
	GENERATED_BODY()
	
public:	
	ULGUIWidgetInteraction();
	
protected:
	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bAllowEventBubbleUp = false;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI, AdvancedDisplay)
		ULGUIWidgetInteractionManager* Helper = nullptr;

	virtual bool OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerExit_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;

	ULGUIPointerEventData* CurrentPointerEventData = nullptr;

public:

	// Begin ActorComponent interface
	virtual void Awake() override;
	virtual void OnDestroy() override;
	virtual void Update(float DeltaTime) override;
	// End UActorComponent

	/**
	 * Presses a key as if the mouse/pointer were the source of it.  Normally you would just use
	 * Left/Right mouse button for the Key.  However - advanced uses could also be imagined where you
	 * send other keys to signal widgets to take special actions if they're under the cursor.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void PressPointerKey(FKey Key);

	/**
	 * Releases a key as if the mouse/pointer were the source of it.  Normally you would just use
	 * Left/Right mouse button for the Key.  However - advanced uses could also be imagined where you
	 * send other keys to signal widgets to take special actions if they're under the cursor.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void ReleasePointerKey(FKey Key);

	/**
	 * Press a key as if it had come from the keyboard.  Avoid using this for 'a-z|A-Z', things like
	 * the Editable Textbox in Slate expect OnKeyChar to be called to signal a specific character being
	 * send to the widget.  So for those cases you should use SendKeyChar.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual bool PressKey(FKey Key, bool bRepeat = false);

	/**
	 * Releases a key as if it had been released by the keyboard.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual bool ReleaseKey(FKey Key);

	/**
	 * Does both the press and release of a simulated keyboard key.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual bool PressAndReleaseKey(FKey Key);

	/**
	 * Transmits a list of characters to a widget by simulating a OnKeyChar event for each key listed in
	 * the string.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual bool SendKeyChar(FString Characters, bool bRepeat = false);

	/**
	 * Sends a scroll wheel event to the widget under the last hit result.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void ScrollWheel(float ScrollDelta);

	/**
	 * Returns true if a widget under the hit result is interactive.  e.g. Slate widgets
	 * that return true for IsInteractable().
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool IsOverInteractableWidget() const;

	/**
	 * Returns true if a widget under the hit result is focusable.  e.g. Slate widgets that
	 * return true for SupportsKeyboardFocus().
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool IsOverFocusableWidget() const;

	/**
	 * Returns true if a widget under the hit result is has a visibility that makes it hit test
	 * visible.  e.g. Slate widgets that return true for GetVisibility().IsHitTestVisible().
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool IsOverHitTestVisibleWidget() const;

	/**
	 * Gets the widget path for the slate widgets under the last hit result.
	 */
	const FWeakWidgetPath& GetHoveredWidgetPath() const;

	/**
	 * Gets the last hit location on the widget in 2D, local pixel units of the render target.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	FVector2D Get2DHitLocation() const;

	/**
	 * Set custom hit result.  This is only taken into account if InteractionSource is set to EWidgetInteractionSource::Custom.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetCustomHitResult(const FHitResult& HitResult);

	/**
	 * Set the focus target of the virtual user managed by this component
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetFocus(UWidget* FocusWidget);

protected:
	/**
	 * Represents the virtual user in slate.  When this component is registered, it gets a handle to the
	 * virtual slate user it will be, so virtual slate user 0, is probably real slate user 8, as that's the first
	 * index by default that virtual users begin - the goal is to never have them overlap with real input
	 * hardware as that will likely conflict with focus states you don't actually want to change - like where
	 * the mouse and keyboard focus input (the viewport), so that things like the player controller receive
	 * standard hardware input.
	 */
	TSharedPtr<class FSlateVirtualUserHandle> VirtualUser;

public:

	/**
	 * Represents the Virtual User Index.  Each virtual user should be represented by a different
	 * index number, this will maintain separate capture and focus states for them.  Each
	 * controller or finger-tip should get a unique PointerIndex.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI, meta = (ClampMin = "0", ExposeOnSpawn = true))
	int32 VirtualUserIndex;


protected:
	int32 PrevPointerIndex = -1;

	// Gets the key and char codes for sending keys for the platform.
	void GetKeyAndCharCodes(const FKey& Key, bool& bHasKeyCode, uint32& KeyCode, bool& bHasCharCode, uint32& CharCode);

	/** Is it safe for this interaction component to run?  Might not be in a server situation with no slate application. */
	bool CanSendInput();

	/** Performs the simulation of pointer movement.  Does not run if bEnableHitTesting is set to false. */
	void SimulatePointerMovement();

	struct FWidgetTraceResult
	{
		FWidgetTraceResult()
			: LocalHitLocation(FVector2D::ZeroVector)
			, HitWidgetPath()
		{
		}

		FVector2D LocalHitLocation;
		FWidgetPath HitWidgetPath;
	};

	/**
	 * Gets the list of components to ignore during hit testing.  Which is everything that is a parent/sibling of this
	 * component that's not a Widget Component.  This is so traces don't get blocked by capsules and such around the player.
	 */
	void GetRelatedComponentsToIgnoreInAutomaticHitTesting(TArray<UPrimitiveComponent*>& IgnorePrimitives) const;

	/** Returns true if the inteaction component can interact with the supplied widget component */
	bool CanInteractWithComponent(ULGUIWidget* Component) const;

protected:

	/** The last widget path under the hit result. */
	FWeakWidgetPath LastWidgetPath;

	/** The modifier keys to simulate during key presses. */
	FModifierKeysState ModifierKeys;

	/** The current set of pressed keys we maintain the state of. */
	TSet<FKey> PressedKeys;

	/** Stores the custom hit result set by the player. */
	UPROPERTY(Transient)
	FHitResult CustomHitResult;

	/** The 2D location on the widget component that was hit. */
	UPROPERTY(Transient)
	FVector2D LocalHitLocation;

	/** The last 2D location on the widget component that was hit. */
	UPROPERTY(Transient)
	FVector2D LastLocalHitLocation;

	/** The widget component we're currently hovering over. */
	UPROPERTY(Transient)
	TObjectPtr<ULGUIWidget> WidgetComponent;

	/** Are we hovering over any interactive widgets. */
	UPROPERTY(Transient)
	bool bIsHoveredWidgetInteractable;

	/** Are we hovering over any focusable widget? */
	UPROPERTY(Transient)
	bool bIsHoveredWidgetFocusable;

	/** Are we hovered over a widget that is hit test visible? */
	UPROPERTY(Transient)
	bool bIsHoveredWidgetHitTestVisible;

private:

	/** Returns the path to the widget that is currently beneath the pointer */
	FWidgetPath DetermineWidgetUnderPointer();
};
