// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "LTweener.h"
#include "UICanvasGroup.generated.h"

/**
 * The UICanvasGroup can be used to control certain aspects of a whole group of UI elements from one place without needing to handle them each individually.
 * The properties of the UICanvasGroup affect the UI element it is on as well as all children.
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUICanvasGroup : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	UUICanvasGroup();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	void OnRegister();
	void OnUnregister();

	bool CheckUIItem();

	FDelegateHandle UIHierarchyChangeDelegateHandle;
	void OnUIHierarchyChanged();

	/**
	 * The opacity of the UI elements in this group.
	 * The value is between 0 and 1 where 0 is fully transparent and 1 is fully opaque.
	 * Note that elements retain their own transparency as well, so the UICanvasGroup alpha and the alpha values of the individual UI elements are multiplied with each other.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float Alpha = 1.0f;
	/**
	 * Determines if this component will accept input.
	 * When it is set to false interaction is disabled.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bInteractable = true;
	/**
	 * Restrict navigation area to only children of this UI node when navigate out.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bRestrictNavigationArea = false;
	/**
	 * Will this group also be affected by the settings in UICanvasGroup components further up in the Actor hierarchy, or will it ignore those and hence override them?
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bIgnoreParentGroup = false;
	/** The UI element this component attached to */
	TWeakObjectPtr<UUIItem> UIItem = nullptr;
	/** Nearest up parent UICanvasGroup */
	TWeakObjectPtr<UUICanvasGroup> ParentUICanvasGroup = nullptr;
	FSimpleMulticastDelegate InteractableStateChangeDelegate;
	FSimpleMulticastDelegate AlphaChangeDelegate;

	FDelegateHandle ParentInteractableStateChangeDelegateHandle;
	void OnParentInteractableStateChange();
	FDelegateHandle ParentAlphaChangeDelegteHandle;
	void OnAlphaChange();

	mutable float CacheFinalAlpha = 1.0f;
	mutable bool bIsAlphaDirty = true;
	bool bPrevIsInteractable = true;
	void CheckInteractableStateChange();
public:
	FDelegateHandle RegisterInteractableStateChange(const FSimpleDelegate& InCallback);
	void UnregisterInteractableStateChange(const FDelegateHandle& InHandle);
	FDelegateHandle RegisterAlphaChange(const FSimpleDelegate& InCallback);
	void UnregisterAlphaChange(const FDelegateHandle& InHandle);
	void SetParentCanvasGroup(UUICanvasGroup* InParentCanvasGroup);
	/**
	 * Return final calculated alpha, will concern parent's alpha value.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI) float GetFinalAlpha() const;
	/**
	 * Return alpha property value directly, not concern parent's alpha.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI) float GetAlpha() const { return Alpha; }
	/** Return final calculated interactable, will concern parent's value */
	UFUNCTION(BlueprintCallable, Category = LGUI) bool GetFinalInteractable() const;
	/** Return Interactable property value directly, not concern parent's property. */
	UFUNCTION(BlueprintCallable, Category = LGUI) bool GetInteractable() const { return bInteractable; }
	UFUNCTION(BlueprintCallable, Category = LGUI) bool GetIgnoreParentGroup() const { return bIgnoreParentGroup; }
	/** Return RestrictNavigationArea property value directly, not concern parent's property. */
	UFUNCTION(BlueprintCallable, Category = LGUI) bool GetRestrictNavigationArea() const { return bRestrictNavigationArea; }
	/**
	 * Return self or upper nearest UICanvasGroup that use restrict navigation, if none found then return nullptr.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI) const UUICanvasGroup* GetRestrictNavigationAreaCanvasGroup() const;

	UFUNCTION(BlueprintCallable, Category = LGUI) void SetAlpha(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI) void SetInteractable(bool value);
	UFUNCTION(BlueprintCallable, Category = LGUI) void SetIgnoreParentGroup(bool value);
public:
#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaFrom(float startValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion
};
