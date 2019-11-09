// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LTweenActor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "LTweenDelegateHandleWrapper.h"
#include "LTweenBPLibrary.generated.h"

UCLASS()
class LTWEEN_API ULTweenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#pragma region PositionXYZ
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To"), Category = "LTween")
		static ULTweener* LocalPositionXTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To"), Category = "LTween")
		static ULTweener* LocalPositionYTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To"), Category = "LTween")
		static ULTweener* LocalPositionZTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To"), Category = "LTween")
		static ULTweener* WorldPositionXTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To"), Category = "LTween")
		static ULTweener* WorldPositionYTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To"), Category = "LTween")
		static ULTweener* WorldPositionZTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion


#pragma region Position
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* LocalPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* WorldPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion Position


	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* LocalScaleTo(USceneComponent* target, FVector endValue = FVector(1.0f, 1.0f, 1.0f), float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);


#pragma region Rotation
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* LocalRotationQuaternionTo(USceneComponent* target, FQuat endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* LocalRotationQuaternionTo_Sweep(USceneComponent* target, FQuat endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo(USceneComponent* target, FQuat endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo_Sweep(USceneComponent* target, FQuat endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion Rotation


	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MaterialScalarParameterTo(UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	//curve is only for CurveFloat ease type
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MaterialVectorParameterTo(UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Assign onComplete or onUpdate or onStart"), Category = LTween)
		static ULTweener* VirtualCall(float duration, float delay, const FTweenerSimpleDynamicDelegate& start, const FTweenerFloatDynamicDelegate& update, const FTweenerSimpleDynamicDelegate& complete)
	{
		return ALTweenActor::VirtualTo(duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}
	static ULTweener* VirtualCall(float duration, float delay, FSimpleDelegate start, LTweenUpdateDelegate update, FSimpleDelegate complete)
	{
		return ALTweenActor::VirtualTo(duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}
	static ULTweener* VirtualCall(float duration, float delay, const TFunction<void()>& start, const TFunction<void(float)>& update, const TFunction<void()>& complete)
	{
		return ALTweenActor::VirtualTo(duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay call function, Assign delayComplete to call"), Category = LTween)
		static ULTweener* DelayCall(float delayTime, const FTweenerSimpleDynamicDelegate& delayComplete)
	{
		return ALTweenActor::VirtualTo(delayTime)->OnComplete(delayComplete);
	}
	static ULTweener* DelayCall(float delayTime, FSimpleDelegate delayComplete)
	{
		return ALTweenActor::VirtualTo(delayTime)->OnComplete(delayComplete);
	}
	static ULTweener* DelayCall(float delayTime, const TFunction<void()>& delayComplete)
	{
		return ALTweenActor::VirtualTo(delayTime)->OnComplete(delayComplete);
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay frame call function, Assign delayComplete to call"), Category = LTween)
		static ULTweener* DelayFrameCall(int frameCount, const FTweenerSimpleDynamicDelegate& delayComplete)
	{
		return ALTweenActor::DelayFrameCall(frameCount)->OnComplete(delayComplete);
	}
	static ULTweener* DelayFrameCall(int frameCount, FSimpleDelegate delayComplete)
	{
		return ALTweenActor::DelayFrameCall(frameCount)->OnComplete(delayComplete);
	}
	static ULTweener* DelayFrameCall(int frameCount, const TFunction<void()>& delayComplete)
	{
		return ALTweenActor::DelayFrameCall(frameCount)->OnComplete(delayComplete);
	}

	UFUNCTION(BlueprintCallable, Category = LTween)
		static bool IsTweening(ULTweener* inTweener)
	{
		return ALTweenActor::IsTweening(inTweener);
	}
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "callComplete"), Category = LTween)
		static void KillIfIsTweening(ULTweener* inTweener, bool callComplete = false)
	{
		ALTweenActor::KillIfIsTweening(inTweener, callComplete);
	}
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Kill If Is Tweening (Array)", AdvancedDisplay = "callComplete"), Category = LTween)
		static void ArrayKillIfIsTweening(const TArray<ULTweener*>& inTweenerArray, bool callComplete = false)
	{
		for (auto tweener : inTweenerArray)
		{
			ALTweenActor::KillIfIsTweening(tweener, callComplete);
		}
	}

	static void RegisterUpdateEvent(const LTweenUpdateDelegate& update)
	{
		ALTweenActor::RegisterUpdateEvent(update);
	}
	static FLTweenDelegateHandleWrapper RegisterUpdateEvent(const TFunction<void(float)>& update)
	{
		LTweenUpdateDelegate updateDelegate = LTweenUpdateDelegate::CreateLambda(update);
		FLTweenDelegateHandleWrapper delegateHandle(updateDelegate.GetHandle());
		ALTweenActor::RegisterUpdateEvent(updateDelegate);
		return delegateHandle;
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Registerred update function will be called every frame from mainthread."), Category = LTween)
		static FLTweenDelegateHandleWrapper RegisterUpdateEvent(const FTweenerFloatDynamicDelegate& update)
	{
		LTweenUpdateDelegate updateDelegate = LTweenUpdateDelegate::CreateLambda([update](float deltaTime) {update.ExecuteIfBound(deltaTime); });
		FLTweenDelegateHandleWrapper delegateHandle(updateDelegate.GetHandle());
		ALTweenActor::RegisterUpdateEvent(updateDelegate);
		return delegateHandle;
	}
	static void UnregisterUpdateEvent(const LTweenUpdateDelegate& update)
	{
		ALTweenActor::UnregisterUpdateEvent(update);
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Unregister the update function. \"delegateHandle\" is the return value when use RegisterUpdateEvent."), Category = LTween)
		static void UnregisterUpdateEvent(const FLTweenDelegateHandleWrapper& delegateHandle)
	{
		ALTweenActor::UnregisterUpdateEvent(delegateHandle.DelegateHandle);
	}
};
