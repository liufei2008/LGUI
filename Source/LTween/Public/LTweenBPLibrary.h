﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LTweenManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LTweenDelegateHandleWrapper.h"
#include "LTweenBPLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenFloatSetterDynamic, float, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenIntSetterDynamic, int, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenVector2SetterDynamic, const FVector2D&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenVector3SetterDynamic, const FVector&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenVector4SetterDynamic, const FVector4&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenColorSetterDynamic, const FColor&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenLinearColorSetterDynamic, const FLinearColor&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenQuaternionSetterDynamic, const FQuat&, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenRotatorSetterDynamic, const FRotator&, value);

UCLASS()
class LTWEEN_API ULTweenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* FloatTo(UObject* WorldContextObject, FLTweenFloatSetterDynamic setter, float startValue = 0.0f, float endValue = 1.0f, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* IntTo(UObject* WorldContextObject, FLTweenIntSetterDynamic setter, int startValue, int endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector2To(UObject* WorldContextObject, FLTweenVector2SetterDynamic setter, FVector2D startValue, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector3To(UObject* WorldContextObject, FLTweenVector3SetterDynamic setter, FVector startValue, FVector endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector4To(UObject* WorldContextObject, FLTweenVector4SetterDynamic setter, FVector4 startValue, FVector4 endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* ColorTo(UObject* WorldContextObject, FLTweenColorSetterDynamic setter, FColor startValue, FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* LinearColorTo(UObject* WorldContextObject, FLTweenLinearColorSetterDynamic setter, FLinearColor startValue, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* QuaternionTo(UObject* WorldContextObject, FLTweenQuaternionSetterDynamic setter, FQuat startValue, FQuat endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* RotatorTo(UObject* WorldContextObject, FLTweenRotatorSetterDynamic setter, FRotator startValue, FRotator endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma region PositionXYZ
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To"), Category = "LTween")
		static ULTweener* LocalPositionXTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To"), Category = "LTween")
		static ULTweener* LocalPositionYTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To"), Category = "LTween")
		static ULTweener* LocalPositionZTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To"), Category = "LTween")
		static ULTweener* WorldPositionXTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To"), Category = "LTween")
		static ULTweener* WorldPositionYTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To"), Category = "LTween")
		static ULTweener* WorldPositionZTo(USceneComponent* target, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionXTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionYTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionZTo_Sweep(USceneComponent* target, float endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion


#pragma region Position
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* LocalPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position To (Sweep)"), Category = LTween)
		static ULTweener* LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* WorldPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position To (Sweep)"), Category = LTween)
		static ULTweener* WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion Position

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* LocalScaleTo(USceneComponent* target, FVector endValue = FVector(1.0f, 1.0f, 1.0f), float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

#pragma region Rotation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* LocalRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* LocalRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion Rotation


	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* MaterialScalarParameterTo(UObject* WorldContextObject, class UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* MaterialVectorParameterTo(UObject* WorldContextObject, class UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MeshMaterialScalarParameterTo(class UMeshComponent* target, int materialIndex, FName parameterName, float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MeshMaterialVectorParameterTo(class UMeshComponent* target, int materialIndex, FName parameterName, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Assign start or update or omplete functions", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, FTweenerSimpleDynamicDelegate start, FTweenerFloatDynamicDelegate update, FTweenerSimpleDynamicDelegate complete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}
	static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, FSimpleDelegate start, LTweenUpdateDelegate update, FSimpleDelegate complete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}
	static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, const TFunction<void()>& start, const TFunction<void(float)>& update, const TFunction<void()>& complete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, duration)->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
	}

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay call function, Assign delayComplete to call", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, FTweenerSimpleDynamicDelegate delayComplete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, delayTime)->OnComplete(delayComplete);
	}
	static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, FSimpleDelegate delayComplete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, delayTime)->OnComplete(delayComplete);
	}
	static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, const TFunction<void()>& delayComplete)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, delayTime)->OnComplete(delayComplete);
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay frame call function, Assign delayComplete to call", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, FTweenerSimpleDynamicDelegate delayComplete)
	{
		return ULTweenManager::DelayFrameCall(WorldContextObject, frameCount)->OnComplete(delayComplete);
	}
	static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, FSimpleDelegate delayComplete)
	{
		return ULTweenManager::DelayFrameCall(WorldContextObject, frameCount)->OnComplete(delayComplete);
	}
	static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, const TFunction<void()>& delayComplete)
	{
		return ULTweenManager::DelayFrameCall(WorldContextObject, frameCount)->OnComplete(delayComplete);
	}

	UFUNCTION(BlueprintCallable, Category = LTween, meta = (WorldContext = "WorldContextObject"))
		static bool IsTweening(UObject* WorldContextObject, ULTweener* inTweener)
	{
		return ULTweenManager::IsTweening(WorldContextObject, inTweener);
	}
	/**
	 * Force stop this animation. if callComplete = true, OnComplete will call after stop.
	 * This method will check if the tweener is valid.
	 */
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "callComplete", WorldContext = "WorldContextObject"), Category = LTween)
		static void KillIfIsTweening(UObject* WorldContextObject, ULTweener* inTweener, bool callComplete = false)
	{
		ULTweenManager::KillIfIsTweening(WorldContextObject, inTweener, callComplete);
	}
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Kill If Is Tweening (Array)", AdvancedDisplay = "callComplete", WorldContext = "WorldContextObject"), Category = LTween)
		static void ArrayKillIfIsTweening(UObject* WorldContextObject, const TArray<ULTweener*>& inTweenerArray, bool callComplete = false)
	{
		auto Instance = ULTweenManager::GetLTweenInstance(WorldContextObject);
		if (!IsValid(Instance))return;

		for (auto tweener : inTweenerArray)
		{
			Instance->KillIfIsTweening(WorldContextObject, tweener, callComplete);
		}
	}

	static FDelegateHandle RegisterUpdateEvent(UObject* WorldContextObject, const LTweenUpdateDelegate& update)
	{
		return ULTweenManager::RegisterUpdateEvent(WorldContextObject, update);
	}
	static FDelegateHandle RegisterUpdateEvent(UObject* WorldContextObject, const TFunction<void(float)>& update)
	{
		LTweenUpdateDelegate updateDelegate = LTweenUpdateDelegate::CreateLambda(update);
		ULTweenManager::RegisterUpdateEvent(WorldContextObject, updateDelegate);
		return updateDelegate.GetHandle();
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Registerred update function will be called every frame from mainthread.", WorldContext = "WorldContextObject"), Category = LTween)
		static FLTweenDelegateHandleWrapper RegisterUpdateEvent(UObject* WorldContextObject, const FTweenerFloatDynamicDelegate& update)
	{
		LTweenUpdateDelegate updateDelegate = LTweenUpdateDelegate::CreateLambda([update](float deltaTime) {update.ExecuteIfBound(deltaTime); });
		FLTweenDelegateHandleWrapper delegateHandle(updateDelegate.GetHandle());
		ULTweenManager::RegisterUpdateEvent(WorldContextObject, updateDelegate);
		return delegateHandle;
	}
	static void UnregisterUpdateEvent(UObject* WorldContextObject, const FDelegateHandle& handle)
	{
		ULTweenManager::UnregisterUpdateEvent(WorldContextObject, handle);
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Unregister the update function. \"delegateHandle\" is the return value when use RegisterUpdateEvent.", WorldContext = "WorldContextObject"), Category = LTween)
		static void UnregisterUpdateEvent(UObject* WorldContextObject, const FLTweenDelegateHandleWrapper& delegateHandle)
	{
		ULTweenManager::UnregisterUpdateEvent(WorldContextObject, delegateHandle.DelegateHandle);
	}

	/**
	 * Repeatedly call function.
	 * \param delayTime delay time before the first call
	 * \param interval interval time between every call
	 * \param repeatCount repeat count, -1 means infinite
	 * \return tweener
	 */
	static ULTweener* RepeatCall(UObject* WorldContextObject, const TFunction<void()>& callFunction, float delayTime, float interval, int repeatCount = 1)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, interval)
			->SetDelay(delayTime)
			->SetLoop(repeatCount == 1 || repeatCount == 0 ? LTweenLoop::Once : LTweenLoop::Restart, repeatCount)
			->OnCycleStart(callFunction)
			;
	}
	/**
	 * Repeatedly call function.
	 * \param delayTime delay time before the first call
	 * \param interval interval time between every call
	 * \param repeatCount repeat count, -1 means infinite
	 * \return tweener
	 */
	static ULTweener* RepeatCall(UObject* WorldContextObject, const FSimpleDelegate& callFunction, float delayTime, float interval, int repeatCount = 1)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, interval)
			->SetDelay(delayTime)
			->SetLoop(repeatCount == 1 || repeatCount == 0 ? LTweenLoop::Once : LTweenLoop::Restart, repeatCount)
			->OnCycleStart(callFunction)
			;
	}
	/**
	 * Repeatedly call function.
	 * \param delayTime delay time before the first call
	 * \param interval interval time between every call
	 * \param repeatCount repeat count, -1 means infinite
	 * \return tweener
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* RepeatCall(UObject* WorldContextObject, FTweenerSimpleDynamicDelegate callFunction, float delayTime, float interval, int repeatCount = 1)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, interval)
			->SetDelay(delayTime)
			->SetLoop(repeatCount == 1 || repeatCount == 0 ? LTweenLoop::Once : LTweenLoop::Restart, repeatCount)
			->OnCycleStart(callFunction)
			;
	}
};
