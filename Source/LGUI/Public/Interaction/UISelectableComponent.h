// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"
#include "Components/ActorComponent.h"
#include "Core/LGUIBehaviour.h"
#include "LGUIComponentReference.h"
#include "UISelectableComponent.generated.h"

UENUM(BlueprintType)
enum class UISelectableTransitionType:uint8
{
	None				UMETA(DisplayName = "None"),
	/** In this mode, TransitionActor's color property will be override by this component. */
	ColorTint			UMETA(DisplayName = "ColorTint"),
	/** In this mode, TransitionActor's root component need to be a UISpriteBase. */
	/** Target UISprite's sprite will be override by this component. */
	SpriteSwap			UMETA(DisplayName = "SpriteSwap"),
	/** You can implement a UISelectableTransitionComponent in c++ or blueprint to do the transition, and add this component to transition actor */
	TransitionComponent			UMETA(DisplayName = "TransitionComponent"),
};
UENUM(BlueprintType)
enum class EUISelectableSelectionState :uint8
{
	/** Not hovered by pointer, just a normal state. */
	Normal,
	/** Hovered by pointer. */
	Highlighted,
	/** Pressed by pointer. */
	Pressed,
	/** Disabled, not interactable. Check the "OnUIInteractionStateChanged" function of UISelectableComponent, to see why it is disabled. */
	Disabled,
};
UENUM(BlueprintType)
enum class EUISelectableNavigationMode:uint8
{
	/** No navigation. */
	None,
	/** Navigation is controlled by LGUI. */
	Auto,
	/** Control your navigation behaviour on your own. */
	Explicit,
};

class ULGUISpriteData;

UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISelectableComponent : public ULGUIBehaviour, public ILGUIPointerEnterExitInterface, public ILGUIPointerDownUpInterface, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()
	
protected:

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void OnDestroy()override;

	friend class FUISelectableCustomization;
	/** If not assigned, use self. must have UIItem component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		class AUIBaseActor* TransitionActor;
	/** inherited events of this component can bubble up? */
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		bool AllowEventBubbleUp = false;

	virtual void OnUIInteractionStateChanged(bool interactableOrNot)override;

	bool CheckTarget();
	
#pragma region Transition
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		UISelectableTransitionType Transition;

	UPROPERTY(Transient)class ULTweener* TransitionTweener = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor NormalColor = FColor(255, 255, 255, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor HighlightedColor = FColor(200, 200, 200, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor PressedColor = FColor(150, 150, 150, 255);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		FColor DisabledColor = FColor(150, 150, 150, 128);
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable", meta = (ClampMin = "0.0"))
		float FadeDuration = 0.2f;

	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		ULGUISpriteData* NormalSprite;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		ULGUISpriteData* HighlightedSprite;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		ULGUISpriteData* PressedSprite;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		ULGUISpriteData* DisabledSprite;

	EUISelectableSelectionState CurrentSelectionState = EUISelectableSelectionState::Normal;
	void ApplySelectionState(bool immediateSet);
	bool IsPointerInsideThis = false;
	bool IsPointerDown = false;
	UPROPERTY(Transient) class UUISelectableTransitionComponent* TransitionComp = nullptr;
#pragma endregion
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationLeft = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationLeftSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationRight = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationRightSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());

	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationUp = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationUpSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationDown = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationDownSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());
	
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationNext = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationNextSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		EUISelectableNavigationMode NavigationPrev = EUISelectableNavigationMode::Auto;
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable-Navigation")
		FLGUIComponentReference NavigationPrevSpecific = FLGUIComponentReference(UUISelectableComponent::StaticClass());
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		class AUIBaseActor* GetTransitionTarget()const { return TransitionActor; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetNormalSprite()const { return NormalSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetNormalColor()const { return NormalColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetHighlightedSprite()const { return HighlightedSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetHighlightedColor()const { return HighlightedColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetPressedSprite()const { return PressedSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetPressedColor()const { return PressedColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") EUISelectableSelectionState GetSelectionState()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetNormalSprite(ULGUISpriteData* NewSprite);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetNormalColor(FColor NewColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetHighlightedSprite(ULGUISpriteData* NewSprite);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetHighlightedColor(FColor NewColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetPressedSprite(ULGUISpriteData* NewSprite);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetPressedColor(FColor NewColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		void SetSelectionState(EUISelectableSelectionState NewState);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		bool IsInteractable()const;

	UUISelectableComponent* FindSelectable(FVector InDirection);
	UUISelectableComponent* FindSelectable(FVector InDirection, USceneComponent* InParent);
	virtual UUISelectableComponent* FindSelectableOnLeft();
	virtual UUISelectableComponent* FindSelectableOnRight();
	virtual UUISelectableComponent* FindSelectableOnUp();
	virtual UUISelectableComponent* FindSelectableOnDown();
	virtual UUISelectableComponent* FindSelectableOnNext();
	virtual UUISelectableComponent* FindSelectableOnPrev();
public:
	virtual bool OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerExit_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)override;
	virtual bool OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)override;
};
