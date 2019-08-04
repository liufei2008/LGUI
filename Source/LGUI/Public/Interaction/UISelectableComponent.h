// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerEnterExitInterface.h"
#include "Event/LGUIPointerDownUpInterface.h"
#include "Components/ActorComponent.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/UIComponentBase.h"
#include "UISelectableComponent.generated.h"

UENUM(BlueprintType)
enum class UISelectableTransitionType:uint8
{
	None				UMETA(DisplayName = "None"),
	ColorTint			UMETA(DisplayName = "ColorTint"),
	SpriteSwap			UMETA(DisplayName = "SpriteSwap"),
	//You can implement a UISelectableTransitionComponent in c++ or blueprint to do the transition, and add this component to transition actor
	TransitionComponent			UMETA(DisplayName = "TransitionComponent"),
};
UENUM(BlueprintType)
enum class EUISelectableSelectionState :uint8
{
	Normal,
	Highlighted,
	Pressed,
	Disabled,
};

UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUISelectableComponent : public UUIComponentBase, public ILGUIPointerEnterExitInterface, public ILGUIPointerDownUpInterface
{
	GENERATED_BODY()
	
public:	
	UUISelectableComponent();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	friend class FUISelectableCustomization;
	//If not assigned, use self. must have UIItem component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		class AUIBaseActor* TransitionActor;
	//inherited events of this component can bubble up?
	UPROPERTY(EditAnywhere, Category = "LGUI-Selectable")
		bool AllowEventBubbleUp = false;

	virtual void OnUIInteractionStateChanged(bool interactableOrNot)override;

	bool CheckTarget();
	
#pragma region Transition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		UISelectableTransitionType Transition;

	UPROPERTY(Transient)class ULTweener* TransitionTweener = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		FColor NormalColor = FColor(255, 255, 255, 255);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		FColor HighlightedColor = FColor(200, 200, 200, 255);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		FColor PressedColor = FColor(150, 150, 150, 255);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		FColor DisabledColor = FColor(150, 150, 150, 128);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable", meta = (ClampMin = "0.0"))
		float FadeDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		ULGUISpriteData* NormalSprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		ULGUISpriteData* HighlightedSprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		ULGUISpriteData* PressedSprite;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		ULGUISpriteData* DisabledSprite;

	EUISelectableSelectionState CurrentSelectionState;
	void ApplySelectionState();
	bool IsPointerInsideThis = false;
	UPROPERTY(Transient) class UUISelectableTransitionComponent* TransitionComp = nullptr;
#pragma endregion
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable")
		class AUIBaseActor* GetTransitionTarget();

	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetNormalSprite()const { return NormalSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetNormalColor()const { return NormalColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetHighlightedSprite()const { return HighlightedSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetHighlightedColor()const { return HighlightedColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") ULGUISpriteData* GetPressedSprite() { return PressedSprite; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") FColor GetPressedColor()const { return PressedColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Selectable") EUISelectableSelectionState GetSelectionState()const { return CurrentSelectionState; }

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
public:
	virtual bool OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)override;

};
