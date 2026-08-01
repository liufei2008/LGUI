// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexSelectDeselectInterface.h"
#include "Event/Interface/LexNavigationInterface.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIImageBrush.h"
#include "UISelectable.generated.h"

class UUINavigationInputSelectionHandler;
class UUISelectable;
class ULexVisual;
class ULTweener;

UENUM(BlueprintType, Category = LGUI)
enum class EUISelectableTransitionType:uint8
{
	None,
	Color,
	/** This mode need a LexImage as TransitionTarget */
	ImageBrush,
	/** You can implement custom UISelectableTransition to do the transition */
	Custom,
};
UENUM(BlueprintType, Category = LGUI)
enum class EUISelectableSelectionState :uint8
{
	/** Not hovered by pointer, just a normal state. */
	Normal,
	/** Hovered by pointer. */
	Hovered,
	/** Pressed by pointer. */
	Pressed,
	/** Disabled, not interactable. */
	Disabled,
};
UENUM(BlueprintType, Category = LGUI)
enum class EUISelectableNavigationMode:uint8
{
	/** No navigation, cannot navigate out from this. */
	None,
	/** Navigation is controlled by LGUI. */
	Auto,
	/** Control your navigation behaviour on your own. */
	Explicit,
};


UCLASS(ClassGroup = (LexUI), Abstract, Blueprintable, meta=(BlueprintSpawnableComponent))
class LGUI_API UUITransition :public ULexUIBehaviour
{
	GENERATED_BODY()
public:
	UUITransition();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "LGUI-Transition")
		TArray<TObjectPtr<ULTweener>> TweenerCollection;
public:
	/**
	 * Stop any transition inside TweenerCollection if playing, so remember to collect your tweener object by calling function CollectTweener.
	 * Call this before start any transition, in case of other transition is in progress.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	virtual void StopTransition();
	/** Add tweener to TweenerCollection, so the function StopTransition will take effect. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	virtual void CollectTweener(ULTweener* InItem);
	/** Add tweener set to TweenerCollection, so the function StopTransition will take effect. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	virtual void CollectTweeners(const TSet<ULTweener*>& InItems);
};

UCLASS(ClassGroup = (LexUI), Abstract, Blueprintable)
class LGUI_API UUISelectableTransition :public UUITransition
{
	GENERATED_BODY()
public:

	UFUNCTION()
	UUISelectable* GetSelectableComponent()const;
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Getter=GetSelectableComponent, Category = "LGUI-Transition", DisplayName=UISelectable)
	mutable TObjectPtr<UUISelectable> UISelectableComp;

	/** 
	 * Called when UISelectableComponent's transition state = normal.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnNormal"))
		void ReceiveOnNormal(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = highlighted.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnHovered"))
		void ReceiveOnHovered(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = pressed.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnPressed"))
		void ReceiveOnPressed(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = disabled.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnDisabled"))
		void ReceiveOnDisabled(bool InImmediateSet);
public:
	/**
	 * Called when UISelectableComponent's transition state = normal.
	 * Default will call blueprint implemented function. If you dont want that, just not use Super::OnNormal();
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	virtual void OnNormal(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = highlighted.
	 * Default will call blueprint implemented function. If you dont want that, just not use Super::OnHighlighted();
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	virtual void OnHovered(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = pressed.
	 * Default will call blueprint implemented function. If you dont want that, just not use Super::OnPressed();
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	virtual void OnPressed(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = disabled.
	 * Default will call blueprint implemented function. If you dont want that, just not use Super::OnDisabled();
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	virtual void OnDisabled(bool InImmediateSet);
};

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISelectable : public ULexUIBehaviour
	, public ILexPointerEnterExitInterface
	, public ILexPointerDownUpInterface
	, public ILexSelectDeselectInterface
	, public ILexNavigationInterface
{
	GENERATED_BODY()
public:
	UUISelectable();
protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void Awake() override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	friend class FUISelectableCustomization;
	
	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		bool AllowEventBubbleUp = false;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		bool bInteractable = true;

	virtual void OnInteractableChanged(bool IsEnabled) override;

#pragma region Transition
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
	TWeakObjectPtr<ULexVisual> TransitionTarget;
	
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
	EUISelectableTransitionType TransitionType = EUISelectableTransitionType::Color;

	UPROPERTY(EditAnywhere, Category="LGUI-Selectable", meta=(EditCondition="TransitionType==EUISelectableTransitionType::Custom"))
	TWeakObjectPtr<UUISelectableTransition> CustomTransition = nullptr;
	UPROPERTY(Transient)TObjectPtr<class ULTweener> TransitionTweener = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor NormalColor = FColor(255, 255, 255, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor HoveredColor = FColor(200, 200, 200, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor PressedColor = FColor(150, 150, 150, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor DisabledColor = FColor(150, 150, 150, 128);

	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush NormalImageBrush;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush HoveredImageBrush;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush PressedImageBrush;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush DisabledImageBrush;

	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (ClampMin = "0.0"))
	float AnimDuration = 0.2f;

	EUISelectableSelectionState CurrentSelectionState = EUISelectableSelectionState::Normal;
	void ApplyPointerSelectionState(bool ImmediateSet);
	bool bIsPointerInsideThis = false;
	bool bIsPointerDown = false;
	bool CheckNavigationSelectionState();
	TWeakObjectPtr<UUINavigationInputSelectionHandler> NavigationSelection;
#pragma endregion
	/**
	 * Can we navigate from other selectable object to this one?
	 * If other selectable use EUISelectableNavigationMode.Explicit and use this selectable as specific one, then this selectable can still be navigate to.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		bool bCanNavigateHere = true;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationLeft = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationLeftSpecific;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationRight = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationRightSpecific;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationUp = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationUpSpecific;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationDown = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationDownSpecific;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationNext = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationNextSpecific;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationPrev = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationPrevSpecific;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		ULexVisual* GetTransitionTarget()const { return TransitionTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	FColor GetNormalColor()const { return NormalColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	FColor GetHoveredColor()const { return HoveredColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	FColor GetPressedColor()const { return PressedColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	FColor GetDisabledColor()const { return DisabledColor; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	const FLexUIImageBrush& GetNormalImageBrush()const { return NormalImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	const FLexUIImageBrush& GetHoveredImageBrush()const { return HoveredImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
	const FLexUIImageBrush& GetPressedImageBrush()const { return PressedImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	const FLexUIImageBrush& GetDisabledImageBrush()const { return DisabledImageBrush; }
	
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") 
		EUISelectableSelectionState GetSelectionState()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetTransitionTarget(ULexVisual* Value);
	
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetNormalColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetHoveredColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetPressedColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetDisabledColor(FColor Value);
	
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetNormalImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetHoveredImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetPressedImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
	void SetDisabledImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetSelectionState(EUISelectableSelectionState NewState);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		bool IsInteractable()const;

#pragma region Navigation
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		bool GetCanNavigateHere()const { return bCanNavigateHere; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationLeft()const { return NavigationLeft; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationRight()const { return NavigationRight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationUp()const { return NavigationUp; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationDown()const { return NavigationDown; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationPrev()const { return NavigationPrev; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationNext()const { return NavigationNext; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationLeftExplicit()const { return NavigationLeftSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationRightExplicit()const { return NavigationRightSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationUpExplicit()const { return NavigationUpSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationDownExplicit()const { return NavigationDownSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationPrevExplicit()const { return NavigationPrevSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		UUISelectable* GetNavigationNextExplicit()const { return NavigationNextSpecific.Get(); }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetCanNavigateHere(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationLeft(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationRight(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationUp(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationDown(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationPrev(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationNext(EUISelectableNavigationMode Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationLeftExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationRightExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationUpExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationDownExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationPrevExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable-Navigation")
		void SetNavigationNextExplicit(UUISelectable* Value);

	/**
	 * Find UISelectable component on specific direction.
	 */
	virtual UUISelectable* FindSelectable(FVector InDirection);
	/**
	 * Find UISelectable component inside InParent on specific direction.
	 */
	virtual UUISelectable* FindSelectable(FVector InDirection, ULexWidget* InParent);
	/**
     * Default selectable is the most "Prev" one (left top most).
	 */
	static UUISelectable* FindDefaultSelectable(UObject* WorldContextObject);
	virtual UUISelectable* FindSelectableOnLeft();
	virtual UUISelectable* FindSelectableOnRight();
	virtual UUISelectable* FindSelectableOnUp();
	virtual UUISelectable* FindSelectableOnDown();
	virtual UUISelectable* FindSelectableOnNext();
	virtual UUISelectable* FindSelectableOnPrev();
#pragma endregion
protected:
	virtual void OnPointerEnter_Implementation(ULexPointerEventData* EventData)override;
	virtual void OnPointerExit_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnSelect_Implementation(ULexBaseEventData* EventData)override;
	virtual bool OnDeselect_Implementation(ULexBaseEventData* EventData)override;
	virtual bool CanNavigateHere_Implementation() const override;
	virtual bool OnNavigate_Implementation(ELexUINavigationDirection direction, TScriptInterface<ILexNavigationInterface>& result)override;
};
