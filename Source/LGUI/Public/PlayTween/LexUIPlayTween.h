// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "Event/LexUIEventDelegate.h"
#include "LexUIPlayTween.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLexUIPlayTweenCompleteDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLexUIPlayTweenCycleCompleteDynamicDelegate, int32, InCycleCompleteCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLexUIPlayTweenStartDynamicDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLexUIPlayTweenUpdateProgressDynamicDelegate, float, InProgress);

UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew, DisplayName="LexUIPlayTween")
class LGUI_API ULexUIPlayTween : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenLoop LoopType = ELTweenLoop::Once;
	/** number of cycles to play (-1 for infinite) */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (EditCondition = "LoopType != ELTweenLoop::Once"))
		int32 LoopCount = -1;
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenEase EaseType = ELTweenEase::Linear;
	/** only valid if easeType=CurveFloat */
	UPROPERTY(EditAnywhere, Category = "Property", meta=(EditCondition = "EaseType == ELTweenEase::CurveFloat"))
		TObjectPtr<UCurveFloat> EaseCurve;
	UPROPERTY(EditAnywhere, Category = "Property")
		float Duration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float StartDelay = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnStart = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Empty);
	/** parameter float is the progress in range 0-1, not affected by ease type (linear on time) */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnUpdateProgress = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Float);
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnComplete = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Empty);
	/** if LoopType is not Once, then this will be called every time when the cycle end, with parameter "cycle complete count". */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLexUIEventDelegate OnCycleComplete = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Int32);
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bAffectByGamePause = false;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool bAffectByTimeDilation = false;
	UPROPERTY(Transient)
		TObjectPtr<ULTweener> Tweener;
	DECLARE_EVENT(ULexUIPlayTween, FOnStartEvent);
	DECLARE_EVENT_OneParam(ULexUIPlayTween, FOnUpdateProgressEvent, float);
	DECLARE_EVENT(ULexUIPlayTween, FOnCompleteEvent);
	DECLARE_EVENT_OneParam(ULexUIPlayTween, FOnCycleCompleteEvent, int32);
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void Start();
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void Stop();
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ULTweener* GetTweener()const { return Tweener; }

	FOnStartEvent OnStartCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI", meta=(DisplayName="OnStart"))
	FOnLexUIPlayTweenStartDynamicDelegate OnStartBP;

	FOnUpdateProgressEvent OnUpdateProgressCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI", meta=(DisplayName="OnUpdateProgress"))
	FOnLexUIPlayTweenUpdateProgressDynamicDelegate OnUpdateProgressBP;
	
	FOnCompleteEvent OnCompleteCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI", meta=(DisplayName="OnComplete"))
	FOnLexUIPlayTweenCompleteDynamicDelegate OnCompleteBP;
	
	FOnCycleCompleteEvent OnCycleCompleteCPP;
	UPROPERTY(BlueprintAssignable, Category = "LexUI", meta=(DisplayName="OnCycleComplete"))
	FOnLexUIPlayTweenCycleCompleteDynamicDelegate OnCycleCompleteBP;


	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ELTweenLoop GetLoopType()const { return LoopType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		int GetLoopCount()const { return LoopCount; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		ELTweenEase GetEaseType()const { return EaseType; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		UCurveFloat* GetEaseCurve()const { return EaseCurve; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetDuration()const { return Duration; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetStartDelay()const { return StartDelay; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetAffectByGamePause()const { return bAffectByGamePause; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetAffectByTimeDilation()const { return bAffectByTimeDilation; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetLoopType(ELTweenLoop Value){ LoopType = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetLoopCount(int Value) { LoopCount = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetEaseType(ELTweenEase Value) { EaseType = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetEaseCurve(UCurveFloat* Value) { EaseCurve = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetDuration(float Value) { Duration = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetStartDelay(float Value) { StartDelay = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetAffectByGamePause(bool Value) { bAffectByGamePause = Value; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetAffectByTimeDilation(bool Value) { bAffectByTimeDilation = Value; }
protected:
	virtual void OnUpdate(float progress)PURE_VIRTUAL(ULGUIPlayTween::OnUpdate, );
};
