// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "Components/ActorComponent.h"
#include "UISelectableTransitionComponent.generated.h"

class ULTweener;
//This component is only used when UISelectableComponent's Transition = TransitionComponent
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUISelectableTransitionComponent :public UActorComponent
{
	GENERATED_BODY()
public:
	UUISelectableTransitionComponent();
protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		TArray<ULTweener*> TweenerCollection;

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnNormal"))void OnNormalBP();
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnHighlighted"))void OnHighlightedBP();
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnPressed"))void OnPressedBP();
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnDisabled"))void OnDisabledBP();

	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnStartCustomTransition"))void OnStartCustomTransitionBP(FName InTransitionName);
public:
	//Called when UISelectableComponent's transition state is normal. Default will call blueprint implemented function. If you dont want that, just not use Super::OnNormal();
	virtual void OnNormal() { OnNormalBP(); };
	//Called when UISelectableComponent's transition state is highlighted. Default will call blueprint implemented function. If you dont want that, just not use Super::OnHighlighted();
	virtual void OnHighlighted() { OnHighlightedBP(); };
	//Called when UISelectableComponent's transition state is pressed. Default will call blueprint implemented function. If you dont want that, just not use Super::OnPressed();
	virtual void OnPressed() { OnPressedBP(); };
	//Called when UISelectableComponent's transition state is disabled. Default will call blueprint implemented function. If you dont want that, just not use Super::OnDisabled();
	virtual void OnDisabled() { OnDisabledBP(); };

	//This gives us an opportunity to do transition on more case than just provided above
	virtual void OnStartCustomTransition(FName InTransitionName) { OnStartCustomTransitionBP(InTransitionName); };

	//Stop any transition inside TweenerCollection if is playing, so remember to collect your tweener object. Call this before start any transition, in case of other transition is in progress. Default will call blueprint implemented function. If you dont want that, just not use Super::StopTransition();
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	void StopTransition();
};