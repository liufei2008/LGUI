// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UILayoutWithAnimation.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EUILayoutChangePositionAnimationType :uint8
{
	/** Immediately change position and size */
	Immediately,
	/** Change position and size with ease animation */
	EaseAnimation,
	/** Register custom event and do the transition */
	//Custom,
};

UCLASS(Abstract)
class LGUI_API UUILayoutWithAnimation : public UUILayoutBase
{
	GENERATED_BODY()

protected:

	friend class FUIGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUILayoutChangePositionAnimationType AnimationType = EUILayoutChangePositionAnimationType::Immediately;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AnimationDuration = 0.3f;
	UPROPERTY(Transient)
		TArray<class ULTweener*> TweenerArray;

	bool bIsAnimationPlaying = false;
	bool bShouldRebuildLayoutAfterAnimation = false;
	/** This function can be override. */
	virtual void SetOnCompleteTween();
	/** This function can be override to achive your own animation effect. */
	virtual void ApplyAnchoredPositionWithAnimation(EUILayoutChangePositionAnimationType animationType, FVector2D offset, UUIItem* target);
	/** This function can be override to achive your own animation effect. */
	virtual void ApplyWidthWithAnimation(EUILayoutChangePositionAnimationType animationType, float width, UUIItem* target);
	/** This function can be override to achive your own animation effect. */
	virtual void ApplyHeightWithAnimation(EUILayoutChangePositionAnimationType animationType, float height, UUIItem* target);
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUILayoutChangePositionAnimationType GetAnimationType()const { return AnimationType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAnimationDuration()const { return AnimationDuration; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAnimationType(EUILayoutChangePositionAnimationType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAnimationDuration(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void CancelAnimation(bool callComplete = false);
};
