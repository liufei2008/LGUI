﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "Event/LGUIEventDelegate.h"
#include "LGUIPlayTween.generated.h"

DECLARE_DYNAMIC_DELEGATE(FLGUIPlayTweenCompleteDynamicDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIPlayTweenCycleCompleteDynamicDelegate, int32, InCycleCompleteCount);

UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUIPlayTween : public UObject
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenLoop loopType = ELTweenLoop::Once;
	/** number of cycles to play (-1 for infinite) */
	UPROPERTY(EditAnywhere, Category = "Property", meta = (EditCondition = "loopType != ELTweenLoop::Once"))
		int32 loopCount = -1;
	UPROPERTY(EditAnywhere, Category = "Property")
		ELTweenEase easeType = ELTweenEase::Linear;
	/** only valid if easeType=CurveFloat */
	UPROPERTY(EditAnywhere, Category = "Property", meta=(EditCondition = "easeType == ELTweenEase::CurveFloat"))
		TObjectPtr<UCurveFloat> easeCurve;
	UPROPERTY(EditAnywhere, Category = "Property")
		float duration = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Property")
		float startDelay = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onStart = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Empty);
	/** parameter float is the progress in range 0-1, not affected by ease type (linear on time) */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onUpdateProgress = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Float);
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onComplete = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Empty);
	/** if LoopType is not Once, then this will be called every time when the cycle end, with parameter "cycle complete count". */
	UPROPERTY(EditAnywhere, Category = "Event")
		FLGUIEventDelegate onCycleComplete = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Int32);
	UPROPERTY(EditAnywhere, Category = "Property")
		bool affectByGamePause = false;
	UPROPERTY(EditAnywhere, Category = "Property")
		bool affectByTimeDilation = false;
	UPROPERTY(Transient)
		TObjectPtr<ULTweener> tweener;
	FSimpleMulticastDelegate onComplete_Delegate;
	FLGUIMulticastInt32Delegate onCycleComplete_Delegate;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Start();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Stop();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULTweener* GetTweener()const { return tweener; }
	FDelegateHandle RegisterOnComplete(const FSimpleDelegate& InDelegate);
	FDelegateHandle RegisterOnComplete(const TFunction<void()>& InFunction);
	void UnregisterOnComplete(const FDelegateHandle& InDelegateHandle);

	FDelegateHandle RegisterOnCycleComplete(const FLGUIInt32Delegate& InDelegate);
	FDelegateHandle RegisterOnCycleComplete(const TFunction<void(int32)>& InFunction);
	void UnregisterOnCycleComplete(const FDelegateHandle& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FLGUIDelegateHandleWrapper RegisterOnComplete(const FLGUIPlayTweenCompleteDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UnregisterOnComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FLGUIDelegateHandleWrapper RegisterOnCycleComplete(const FLGUIPlayTweenCycleCompleteDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void UnregisterOnCycleComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle);
protected:
	virtual void OnUpdate(float progress)PURE_VIRTUAL(ULGUIPlayTween::OnUpdate, );
};
