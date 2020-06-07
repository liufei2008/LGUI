// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "Components/ActorComponent.h"
#include "UISelectableTransitionComponent.generated.h"

class ULTweener;
//This component is only used when UISelectableComponent's Transition = TransitionComponent
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUISelectableTransitionComponent :public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "LGUI-Selectable")
		TArray<ULTweener*> TweenerCollection;

	//Normal state
	//param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnNormal"))void OnNormalBP(bool InImmediateSet);
	//Highlight state
	//param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnHighlighted"))void OnHighlightedBP(bool InImmediateSet);
	//Trigger press state
	//param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnPressed"))void OnPressedBP(bool InImmediateSet);
	//Trigger release state
	//param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnDisabled"))void OnDisabledBP(bool InImmediateSet);
	//This gives us an opportunity to do transition on more case than just provided above
	//param InTransitionName: use this to tell different event type. eg.UIToggleComponent, "On"/"Off" for toggle on/off
	//param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI-Transition", meta = (DisplayName = "OnStartCustomTransition"))void OnStartCustomTransitionBP(FName InTransitionName, bool InImmediateSet);
public:
	//Called when UISelectableComponent's transition state is normal. Default will call blueprint implemented function. If you dont want that, just not use Super::OnNormal();
	//@param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	virtual void OnNormal(bool InImmediateSet) { OnNormalBP(InImmediateSet); };
	//Called when UISelectableComponent's transition state is highlighted. Default will call blueprint implemented function. If you dont want that, just not use Super::OnHighlighted();
	//@param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	virtual void OnHighlighted(bool InImmediateSet) { OnHighlightedBP(InImmediateSet); };
	//Called when UISelectableComponent's transition state is pressed. Default will call blueprint implemented function. If you dont want that, just not use Super::OnPressed();
	//@param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	virtual void OnPressed(bool InImmediateSet) { OnPressedBP(InImmediateSet); };
	//Called when UISelectableComponent's transition state is disabled. Default will call blueprint implemented function. If you dont want that, just not use Super::OnDisabled();
	//@param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	virtual void OnDisabled(bool InImmediateSet) { OnDisabledBP(InImmediateSet); };

	//This gives us an opportunity to do transition on more case than just provided above. Default will call blueprint implemented function. If you dont want that, just not use Super::OnStartCustomTransition();
	//@param InTransitionName: use this to tell different event type. eg.UIToggleComponent, "On"/"Off" for toggle on/off
	//@param InImmediateSet: set properties immediately or use tween animation. InImmediateSet is true when set initialize state.
	virtual void OnStartCustomTransition(FName InTransitionName, bool InImmediateSet) { OnStartCustomTransitionBP(InTransitionName, InImmediateSet); };

	//Stop any transition inside TweenerCollection if is playing, so remember to collect your tweener object by calling function CollectTweener.
	//Call this before start any transition, in case of other transition is in progress.
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	virtual void StopTransition();
	//Add tweener to TweenerCollection, so the function StopTransition will take effect.
	UFUNCTION(BlueprintCallable, Category = "LGUI-Transition")
	virtual void CollectTweener(ULTweener* InItem);
};