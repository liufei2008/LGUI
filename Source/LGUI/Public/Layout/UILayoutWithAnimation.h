// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "UILayoutWithAnimation.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class EUILayoutAnimationType :uint8
{
	/** Immediately change position and size */
	Immediately,
	/** Change position and size with ease animation */
	EaseAnimation,
	/** Use implemented CustomAnimation object to do the transition */
	Custom,
};

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API UUILayoutWithAnimation_CustomAnimation : public UObject
{
	GENERATED_BODY()
public:
	UUILayoutWithAnimation_CustomAnimation();

	virtual void BeginSetupAnimations();

	virtual void ApplyAnchoredPositionAnimation(const FVector2D& Value, UUIItem* Target);
	virtual void ApplyRotatorAnimation(const FRotator& Value, UUIItem* Target);
	virtual void ApplyWidthAnimation(float Value, UUIItem* Target);
	virtual void ApplyHeightAnimation(float Value, UUIItem* Target);
	virtual void ApplySizeDeltaAnimation(const FVector2D& Value, UUIItem* Target);

	virtual void EndSetupAnimations();
protected:
	/** use this to tell if the class is compiled from blueprint, only blueprint can execute ReceiveXXX. */
	bool bCanExecuteBlueprintEvent = false;
	/** Called before setup any animation when trying to calculate layout. Use this to initialize, eg cancel previous animations. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BeginSetupAnimations"), Category = "LGUI")
		void ReceiveBeginSetupAnimations();
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyAnchoredPositionAnimation"), Category = "LGUI")
		void ReceiveApplyAnchoredPositionAnimation(const FVector2D& Value, UUIItem* Target);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyRotatorAnimation"), Category = "LGUI")
		void ReceiveApplyRotatorAnimation(const FRotator& Value, UUIItem* Target);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyWidthAnimation"), Category = "LGUI")
		void ReceiveApplyWidthAnimation(float Value, UUIItem* Target);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplyHeightAnimation"), Category = "LGUI")
		void ReceiveApplyHeightAnimation(float Value, UUIItem* Target);
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "ApplySizeDeltaAnimation"), Category = "LGUI")
		void ReceiveApplySizeDeltaAnimation(const FVector2D& Value, UUIItem* Target);
	/** Called after setup all animations when finish calculate layout. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "EndSetupAnimations"), Category = "LGUI")
		void ReceiveEndSetupAnimations();
};

UCLASS(Abstract)
class LGUI_API UUILayoutWithAnimation : public UUILayoutBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUILayoutAnimationType AnimationType = EUILayoutAnimationType::Immediately;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "AnimationType==EUILayoutAnimationType::EaseAnimation"))
		float AnimationDuration = 0.3f;
	/** Will fallback to Immediately if object is not valid */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI", meta = (EditCondition = "AnimationType==EUILayoutAnimationType::Custom"))
		TObjectPtr<UUILayoutWithAnimation_CustomAnimation> CustomAnimation;
	UPROPERTY(Transient)
		TArray<TObjectPtr<class ULTweener>> TweenerArray;

	bool bIsAnimationPlaying = false;
	bool bShouldRebuildLayoutAfterAnimation = false;
	/** Called before setup any animation when trying to calculate layout. Use this to initialize, eg cancel previous animations. */
	virtual void BeginSetupAnimations();
	virtual void ApplyAnchoredPositionWithAnimation(EUILayoutAnimationType AnimationType, FVector2D Value, UUIItem* Target);
	virtual void ApplyRotatorWithAnimation(EUILayoutAnimationType AnimationType, const FRotator& Value, UUIItem* Target);
	virtual void ApplyWidthWithAnimation(EUILayoutAnimationType AnimationType, float Value, UUIItem* Target);
	virtual void ApplyHeightWithAnimation(EUILayoutAnimationType AnimationType, float Value, UUIItem* Target);
	virtual void ApplySizeDeltaWithAnimation(EUILayoutAnimationType AnimationType, FVector2D Value, UUIItem* Target);
	/** Called after setup all animations when finish calculate layout. */
	virtual void EndSetupAnimations();
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUILayoutAnimationType GetAnimationType()const { return AnimationType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAnimationDuration()const { return AnimationDuration; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUILayoutWithAnimation_CustomAnimation* GetCustomAnimation()const { return CustomAnimation; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAnimationType(EUILayoutAnimationType Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAnimationDuration(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomAnimation(UUILayoutWithAnimation_CustomAnimation* Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void CancelAllAnimations(bool callComplete = false);
};
