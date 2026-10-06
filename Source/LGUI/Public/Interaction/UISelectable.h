// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexSelectDeselectInterface.h"
#include "Event/Interface/LexNavigationInterface.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIImageBrush.h"
#include "UISelectable.generated.h"

class ULexVisualBatchMesh;
class UUINavigationInputSelectionHandler;
class UUISelectable;
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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "LexUI-Transition")
		TArray<TObjectPtr<ULTweener>> TweenerCollection;
public:
	/**
	 * Stop any transition inside TweenerCollection if playing, so remember to collect your tweener object by calling function CollectTweener.
	 * Call this before start any transition, in case of other transition is in progress.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Transition")
	virtual void StopTransition();
	/** Add tweener to TweenerCollection, so the function StopTransition will take effect. */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Transition")
	virtual void CollectTweener(ULTweener* InItem);
	/** Add tweener set to TweenerCollection, so the function StopTransition will take effect. */
	UFUNCTION(BlueprintCallable, Category = "LexUI-Transition")
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
	UPROPERTY(Transient, BlueprintReadOnly, Getter=GetSelectableComponent, Category = "LexUI-Transition", DisplayName=UISelectable)
	mutable TObjectPtr<UUISelectable> UISelectableComp;

	/** 
	 * Called when UISelectableComponent's transition state = normal.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LexUI-Transition", meta = (DisplayName = "OnNormal"))
		void ReceiveOnNormal(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = highlighted.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LexUI-Transition", meta = (DisplayName = "OnHovered"))
		void ReceiveOnHovered(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = pressed.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LexUI-Transition", meta = (DisplayName = "OnPressed"))
		void ReceiveOnPressed(bool InImmediateSet);
	/**
	 * Called when UISelectableComponent's transition state = disabled.
	 * @param InImmediateSet	set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LexUI-Transition", meta = (DisplayName = "OnDisabled"))
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
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		bool AllowEventBubbleUp = false;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		bool bInteractable = true;

	virtual void OnInteractableChanged(bool IsEnabled) override;

#pragma region Transition
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
	TWeakObjectPtr<ULexVisualBatchMesh> TransitionTarget;
	
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
	EUISelectableTransitionType TransitionType = EUISelectableTransitionType::Color;

	UPROPERTY(EditAnywhere, Category="LexUI-Selectable", meta=(EditCondition="TransitionType==EUISelectableTransitionType::Custom"))
	TWeakObjectPtr<UUISelectableTransition> CustomTransition = nullptr;
	UPROPERTY(Transient)TObjectPtr<class ULTweener> TransitionTweener = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		FColor NormalColor = FColor(255, 255, 255, 255);
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		FColor HoveredColor = FColor(200, 200, 200, 255);
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		FColor PressedColor = FColor(150, 150, 150, 255);
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable")
		FColor DisabledColor = FColor(150, 150, 150, 128);

	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush NormalImageBrush;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush HoveredImageBrush;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush PressedImageBrush;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable", meta = (DisplayThumbnail = "false"))
		FLexUIImageBrush DisabledImageBrush;

	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable", meta = (ClampMin = "0.0"))
	float AnimDuration = 0.2f;

	EUISelectableSelectionState CurrentSelectionState = EUISelectableSelectionState::Normal;
	void ApplyPointerSelectionState(bool ImmediateSet);
	bool bIsPointerInsideThis = false;
	bool bIsPointerDown = false;
	bool CheckNavigationSelectionState(bool CreateIfNotValid);
	TWeakObjectPtr<UUINavigationInputSelectionHandler> NavigationSelection;
#pragma endregion
	/**
	 * Can we navigate from other selectable object to this one?
	 * If other selectable use EUISelectableNavigationMode.Explicit and use this selectable as specific one, then this selectable can still be navigate to.
	 */
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		bool bCanNavigateHere = true;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationLeft = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationLeftSpecific;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationRight = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationRightSpecific;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationUp = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationUpSpecific;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationDown = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationDownSpecific;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationNext = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationNextSpecific;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationPrev = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LexUI-Selectable-Navigation")
		TWeakObjectPtr<UUISelectable> NavigationPrevSpecific;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
		ULexVisualBatchMesh* GetTransitionTarget()const { return TransitionTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	FColor GetNormalColor()const { return NormalColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	FColor GetHoveredColor()const { return HoveredColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	FColor GetPressedColor()const { return PressedColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	FColor GetDisabledColor()const { return DisabledColor; }

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	const FLexUIImageBrush& GetNormalImageBrush()const { return NormalImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	const FLexUIImageBrush& GetHoveredImageBrush()const { return HoveredImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
	const FLexUIImageBrush& GetPressedImageBrush()const { return PressedImageBrush; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	const FLexUIImageBrush& GetDisabledImageBrush()const { return DisabledImageBrush; }
	
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable") 
		EUISelectableSelectionState GetSelectionState()const;

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
		void SetTransitionTarget(ULexVisualBatchMesh* Value);
	
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetNormalColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetHoveredColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetPressedColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetDisabledColor(FColor Value);
	
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetNormalImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetHoveredImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetPressedImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
	void SetDisabledImageBrush(const FLexUIImageBrush& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
		void SetSelectionState(EUISelectableSelectionState NewState);

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable")
		bool IsInteractable()const;

#pragma region Navigation
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		bool GetCanNavigateHere()const { return bCanNavigateHere; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationLeft()const { return NavigationLeft; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationRight()const { return NavigationRight; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationUp()const { return NavigationUp; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationDown()const { return NavigationDown; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationPrev()const { return NavigationPrev; }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		EUISelectableNavigationMode GetNavigationNext()const { return NavigationNext; }

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationLeftExplicit()const { return NavigationLeftSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationRightExplicit()const { return NavigationRightSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationUpExplicit()const { return NavigationUpSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationDownExplicit()const { return NavigationDownSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationPrevExplicit()const { return NavigationPrevSpecific.Get(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		UUISelectable* GetNavigationNextExplicit()const { return NavigationNextSpecific.Get(); }

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetCanNavigateHere(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationLeft(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationRight(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationUp(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationDown(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationPrev(EUISelectableNavigationMode Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationNext(EUISelectableNavigationMode Value);

	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationLeftExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationRightExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationUpExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationDownExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
		void SetNavigationPrevExplicit(UUISelectable* Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI-Selectable-Navigation")
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
