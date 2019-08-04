// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIText.h"
#include "Extensions/UISector.h"
#include "LTweenActor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LGUITweenBPLibrary.generated.h"

UCLASS()
class LGUI_API ULGUITweenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region QuickEntry
	//Tween alpha if root component is UIItem component
	static ULTweener* AlphaFrom(AActor* target, float startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//Tween alpha if root component is UIItem component
	static ULTweener* AlphaTo(AActor* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion
#pragma region UIItem
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* WidthTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* HeightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* ColorTo(UUIItem* target, FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* ColorFrom(UUIItem* target, FColor startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* AlphaTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* AlphaFrom(UUIItem* target, float startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* AnchorOffsetXTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* AnchorOffsetYTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* PivotTo(UUIItem* target, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* StretchLeftTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* StretchRightTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* StretchTopTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* StretchBottomTo(UUIItem* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion

#pragma region UISector
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* StartAngleTo(UUISector* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* EndAngleTo(UUISector* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion
};
