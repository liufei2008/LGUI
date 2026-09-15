// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LTweenManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LTweenDelegateHandleWrapper.h"
#include "LTweenBPLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenFloatSetterDynamic, float, value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLTweenDoubleSetterDynamic, double, value);
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
		static ULTweener* FloatTo(UObject* WorldContextObject, const FLTweenFloatSetterDynamic& setter, float startValue = 0.0f, float endValue = 1.0f, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* DoubleTo(UObject* WorldContextObject, const FLTweenDoubleSetterDynamic& setter, double startValue = 0.0f, double endValue = 1.0f, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* IntTo(UObject* WorldContextObject, const FLTweenIntSetterDynamic& setter, int startValue, int endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector2To(UObject* WorldContextObject, const FLTweenVector2SetterDynamic& setter, FVector2D startValue, FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector3To(UObject* WorldContextObject, const FLTweenVector3SetterDynamic& setter, FVector startValue, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* Vector4To(UObject* WorldContextObject, const FLTweenVector4SetterDynamic& setter, FVector4 startValue, FVector4 endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* ColorTo(UObject* WorldContextObject, const FLTweenColorSetterDynamic& setter, FColor startValue, FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* LinearColorTo(UObject* WorldContextObject, const FLTweenLinearColorSetterDynamic& setter, FLinearColor startValue, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* QuaternionTo(UObject* WorldContextObject, const FLTweenQuaternionSetterDynamic& setter, FQuat startValue, FQuat endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = "LTween")
		static ULTweener* RotatorTo(UObject* WorldContextObject, const FLTweenRotatorSetterDynamic& setter, FRotator startValue, FRotator endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma region LocationXYZ
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location X To"), Category = "LTween")
		static ULTweener* RelativeLocationXTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location Y To"), Category = "LTween")
		static ULTweener* RelativeLocationYTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location Z To"), Category = "LTween")
		static ULTweener* RelativeLocationZTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location X To (Sweep)"), Category = LTween)
		static ULTweener* RelativeLocationXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location Y To (Sweep)"), Category = LTween)
		static ULTweener* RelativeLocationYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location Z To (Sweep)"), Category = LTween)
		static ULTweener* RelativeLocationZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location X To"), Category = "LTween")
		static ULTweener* WorldLocationXTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location Y To"), Category = "LTween")
		static ULTweener* WorldLocationYTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location Z To"), Category = "LTween")
		static ULTweener* WorldLocationZTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location X To (Sweep)"), Category = LTween)
		static ULTweener* WorldLocationXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location Y To (Sweep)"), Category = LTween)
		static ULTweener* WorldLocationYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location Z To (Sweep)"), Category = LTween)
		static ULTweener* WorldLocationZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	// Deprecated, use RelativeLocation*To / WorldLocation*To instead
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationXTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeLocationXTo instead.")
		static ULTweener* LocalPositionXTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationYTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeLocationYTo instead.")
		static ULTweener* LocalPositionYTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationZTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeLocationZTo instead.")
		static ULTweener* LocalPositionZTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position X To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationXTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use RelativeLocationXTo_Sweep instead.")
		static ULTweener* LocalPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Y To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationYTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use RelativeLocationYTo_Sweep instead.")
		static ULTweener* LocalPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position Z To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationZTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use RelativeLocationZTo_Sweep instead.")
		static ULTweener* LocalPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To", DeprecatedFunction, DeprecationMessage = "Use WorldLocationXTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use WorldLocationXTo instead.")
		static ULTweener* WorldPositionXTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To", DeprecatedFunction, DeprecationMessage = "Use WorldLocationYTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use WorldLocationYTo instead.")
		static ULTweener* WorldPositionYTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To", DeprecatedFunction, DeprecationMessage = "Use WorldLocationZTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use WorldLocationZTo instead.")
		static ULTweener* WorldPositionZTo(USceneComponent* target, double endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position X To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use WorldLocationXTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use WorldLocationXTo_Sweep instead.")
		static ULTweener* WorldPositionXTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Y To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use WorldLocationYTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use WorldLocationYTo_Sweep instead.")
		static ULTweener* WorldPositionYTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position Z To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use WorldLocationZTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use WorldLocationZTo_Sweep instead.")
		static ULTweener* WorldPositionZTo_Sweep(USceneComponent* target, double endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion


#pragma region Location
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* RelativeLocationTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Location To (Sweep)"), Category = LTween)
		static ULTweener* RelativeLocationTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTween")
		static ULTweener* WorldLocationTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Location To (Sweep)"), Category = LTween)
		static ULTweener* WorldLocationTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	// Deprecated, use RelativeLocationTo / WorldLocationTo instead
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeLocationTo instead.")
		static ULTweener* LocalPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Position To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use RelativeLocationTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use RelativeLocationTo_Sweep instead.")
		static ULTweener* LocalPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use WorldLocationTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use WorldLocationTo instead.")
		static ULTweener* WorldPositionTo(USceneComponent* target, FVector endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Position To (Sweep)", DeprecatedFunction, DeprecationMessage = "Use WorldLocationTo_Sweep instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use WorldLocationTo_Sweep instead.")
		static ULTweener* WorldPositionTo_Sweep(USceneComponent* target, FVector endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion Location

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* RelativeScaleTo(USceneComponent* target, FVector endValue = FVector(1.0f, 1.0f, 1.0f), float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DeprecatedFunction, DeprecationMessage = "Use RelativeScaleTo instead."), Category = LTween)
	UE_DEPRECATED(5.7, "Use RelativeScaleTo instead.")
		static ULTweener* LocalScaleTo(USceneComponent* target, FVector endValue = FVector(1.0f, 1.0f, 1.0f), float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

#pragma region Rotation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* RelativeRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* RelativeRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* RelativeRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* RelativeRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* RelativeRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Relative Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* RelativeRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value"), Category = "LTween")
		static ULTweener* WorldRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value"), Category = "LTween")
		static ULTweener* WorldRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "World Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value"), Category = "LTween")
		static ULTweener* WorldRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	// Deprecated, use RelativeRotationQuaternionTo / RelativeRotatorTo / RelativeRotateEulerAngleTo instead
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate eulerAngle relative to current rotation value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotateEulerAngleTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotateEulerAngleTo instead.")
		static ULTweener* LocalRotateEulerAngleTo(USceneComponent* target, FVector eulerAngle, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute quaternion rotation value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotationQuaternionTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotationQuaternionTo instead.")
		static ULTweener* LocalRotationQuaternionTo(USceneComponent* target, const FQuat& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", ToolTip = "Rotate absolute rotator value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotatorTo instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotatorTo instead.")
		static ULTweener* LocalRotatorTo(USceneComponent* target, FRotator endValue, bool shortestPath, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotate Euler Angle To (Sweep)", ToolTip = "Rotate eulerAngle relative to current rotation value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotateEulerAngleTo_Sweep instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotateEulerAngleTo_Sweep instead.")
		static ULTweener* LocalRotateEulerAngleTo_Sweep(USceneComponent* target, FVector eulerAngle, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotation Quaternion To (Sweep)", ToolTip = "Rotate absolute quaternion rotation value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotationQuaternionTo_Sweep instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotationQuaternionTo_Sweep instead.")
		static ULTweener* LocalRotationQuaternionTo_Sweep(USceneComponent* target, const FQuat& endValue, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", DisplayName = "Local Rotator To (Sweep)", ToolTip = "Rotate absolute rotator value", DeprecatedFunction, DeprecationMessage = "Use RelativeRotatorTo_Sweep instead."), Category = "LTween")
	UE_DEPRECATED(5.7, "Use RelativeRotatorTo_Sweep instead.")
		static ULTweener* LocalRotatorTo_Sweep(USceneComponent* target, FRotator endValue, bool shortestPath, FHitResult& sweepHitResult, bool sweep, bool teleport, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion Rotation

#pragma region Material
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* MaterialScalarParameterTo(UObject* WorldContextObject, class UMaterialInstanceDynamic* target, FName parameterName, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* MaterialVectorParameterTo(UObject* WorldContextObject, class UMaterialInstanceDynamic* target, FName parameterName, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MeshMaterialScalarParameterTo(class UPrimitiveComponent* target, int materialIndex, FName parameterName, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = LTween)
		static ULTweener* MeshMaterialVectorParameterTo(class UPrimitiveComponent* target, int materialIndex, FName parameterName, FLinearColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion

#pragma region UMG
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_CanvasPanelSlot_PositionTo(UObject* WorldContextObject, class UCanvasPanelSlot* target, const FVector2D& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_CanvasPanelSlot_SizeTo(UObject* WorldContextObject, class UCanvasPanelSlot* target, const FVector2D& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_HorizontalBoxSlot_PaddingTo(UObject* WorldContextObject, class UHorizontalBoxSlot* target, const FMargin& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_VerticalBoxSlot_PaddingTo(UObject* WorldContextObject, class UVerticalBoxSlot* target, const FMargin& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_OverlaySlot_PaddingTo(UObject* WorldContextObject, class UOverlaySlot* target, const FMargin& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_ButtonSlot_PaddingTo(UObject* WorldContextObject, class UButtonSlot* target, const FMargin& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_BorderSlot_PaddingTo(UObject* WorldContextObject, class UBorderSlot* target, const FMargin& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_RenderTransform_TranslationTo(UObject* WorldContextObject, class UWidget* target, const FVector2D& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_RenderTransform_AngleTo(UObject* WorldContextObject, class UWidget* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_RenderTransform_ScaleTo(UObject* WorldContextObject, class UWidget* target, const FVector2D& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_RenderTransform_ShearTo(UObject* WorldContextObject, class UWidget* target, const FVector2D& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_RenderOpacityTo(UObject* WorldContextObject, class UWidget* target, float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_UserWidget_ColorAndOpacityTo(UObject* WorldContextObject, class UUserWidget* target, const FLinearColor& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_Image_ColorAndOpacityTo(UObject* WorldContextObject, class UImage* target, const FLinearColor& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_Button_ColorAndOpacityTo(UObject* WorldContextObject, class UButton* target, const FLinearColor& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease", WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* UMG_Border_ContentColorAndOpacityTo(UObject* WorldContextObject, class UBorder* target, const FLinearColor& endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Assign start or update or omplete functions", WorldContext = "WorldContextObject", AutoCreateRefTerm="start,update,complete"), Category = LTween)
		static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, const FLTweenSimpleDynamicDelegate& start, const FLTweenFloatDynamicDelegate& update, const FLTweenSimpleDynamicDelegate& complete)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
		}
		return Tweener;
	}
	static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, FSimpleDelegate start, FLTweenUpdateDelegate update, FSimpleDelegate complete)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
		};
		return Tweener;
	}
	static ULTweener* VirtualCall(UObject* WorldContextObject, float duration, float delay, const TFunction<void()>& start, const TFunction<void(float)>& update, const TFunction<void()>& complete)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->OnStart(start)->OnUpdate(update)->OnComplete(complete);
		}
		return Tweener;
	}
	static ULTweener* VirtualCall(UObject* WorldContextObject, float duration)
	{
		return ULTweenManager::VirtualTo(WorldContextObject, duration);
	}

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay call function, Assign delayComplete to call", WorldContext = "WorldContextObject", AdvancedDisplay="affectByGamePause,affectByTimeDilation"), Category = LTween)
		static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, const FLTweenSimpleDynamicDelegate& delayComplete, bool affectByGamePause = true, bool affectByTimeDilation = true)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, delayTime);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause)->SetAffectByTimeDilation(affectByTimeDilation);
		}
		return Tweener;
	}
	static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, FSimpleDelegate delayComplete, bool affectByGamePause = true, bool affectByTimeDilation = true)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, delayTime);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause)->SetAffectByTimeDilation(affectByTimeDilation);
		}
		return Tweener;
	}
	static ULTweener* DelayCall(UObject* WorldContextObject, float delayTime, const TFunction<void()>& delayComplete, bool affectByGamePause = true, bool affectByTimeDilation = true)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, delayTime);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause)->SetAffectByTimeDilation(affectByTimeDilation);
		}
		return Tweener;
	}
	UFUNCTION(BlueprintCallable, meta = (ToolTip = "MainThread delay frame call function, Assign delayComplete to call", WorldContext = "WorldContextObject", AdvancedDisplay = "affectByGamePause"), Category = LTween)
		static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, const FLTweenSimpleDynamicDelegate& delayComplete, bool affectByGamePause = true)
	{
		auto Tweener = ULTweenManager::DelayFrameCall(WorldContextObject, frameCount);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause);
		}
		return Tweener;
	}
	static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, FSimpleDelegate delayComplete, bool affectByGamePause = true)
	{
		auto Tweener = ULTweenManager::DelayFrameCall(WorldContextObject, frameCount);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause);
		}
		return Tweener;
	}
	static ULTweener* DelayFrameCall(UObject* WorldContextObject, int frameCount, const TFunction<void()>& delayComplete, bool affectByGamePause = true)
	{
		auto Tweener = ULTweenManager::DelayFrameCall(WorldContextObject, frameCount);
		if (Tweener)
		{
			Tweener->OnComplete(delayComplete)->SetAffectByGamePause(affectByGamePause);
		}
		return Tweener;
	}

	UFUNCTION(BlueprintCallable, meta = (ToolTip = "Assign start or update or omplete functions", WorldContext = "WorldContextObject", AutoCreateRefTerm = "start,update,complete"), Category = LTween)
		static ULTweener* UpdateCall(UObject* WorldContextObject, const FLTweenFloatDynamicDelegate& update)
	{
		auto Tweener = ULTweenManager::UpdateCall(WorldContextObject);
		if (Tweener)
		{
			Tweener->OnUpdate(update);
		}
		return Tweener;
	}
	static ULTweener* UpdateCall(UObject* WorldContextObject, FLTweenUpdateDelegate update)
	{
		auto Tweener = ULTweenManager::UpdateCall(WorldContextObject);
		if (Tweener)
		{
			Tweener->OnUpdate(update);
		};
		return Tweener;
	}
	static ULTweener* UpdateCall(UObject* WorldContextObject, const TFunction<void(float)>& update)
	{
		auto Tweener = ULTweenManager::UpdateCall(WorldContextObject);
		if (Tweener)
		{
			Tweener->OnUpdate(update);
		}
		return Tweener;
	}

	UFUNCTION(BlueprintPure, Category = LTween, meta = (WorldContext = "WorldContextObject"))
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
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "callComplete", WorldContext = "WorldContextObject"), Category = LTween)
	static void KillAllTweensOnTarget(UObject* WorldContextObject, UObject* Target, bool callComplete = false)
	{
		ULTweenManager::KillAllTweensOnTarget(WorldContextObject, Target, callComplete);
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
	static void ArrayKillIfIsTweening(UObject* WorldContextObject, const TArray<TWeakObjectPtr<ULTweener>>& inTweenerArray, bool callComplete = false)
	{
		auto Instance = ULTweenManager::GetLTweenInstance(WorldContextObject);
		if (!IsValid(Instance))return;

		for (auto tweener : inTweenerArray)
		{
			Instance->KillIfIsTweening(tweener.Get(), callComplete);
		}
	}

	/**
	 * Repeatedly call function.
	 * @param delayTime delay time before the first call
	 * @param interval interval time between every call
	 * @param repeatCount repeat count, -1 means infinite
	 * @return tweener
	 */
	static ULTweener* RepeatCall(UObject* WorldContextObject, const TFunction<void()>& callFunction, float delayTime, float interval, int repeatCount = 1)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, interval);
		if (Tweener)
		{
			Tweener
				->SetDelay(delayTime)
				->SetLoop(repeatCount == 1 || repeatCount == 0 ? ELTweenLoop::Once : ELTweenLoop::Restart, repeatCount)
				->OnCycleStart(callFunction)
				;
		}
		return Tweener;
	}
	/**
	 * Repeatedly call function.
	 * @param delayTime delay time before the first call
	 * @param interval interval time between every call
	 * @param repeatCount repeat count, -1 means infinite
	 * \return tweener
	 */
	static ULTweener* RepeatCall(UObject* WorldContextObject, const FSimpleDelegate& callFunction, float delayTime, float interval, int repeatCount = 1)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, interval);
		if (Tweener)
		{
			Tweener
				->SetDelay(delayTime)
				->SetLoop(repeatCount == 1 || repeatCount == 0 ? ELTweenLoop::Once : ELTweenLoop::Restart, repeatCount)
				->OnCycleStart(callFunction)
				;
		}
		return Tweener;
	}
	/**
	 * Repeatedly call function.
	 * @param delayTime delay time before the first call
	 * @param interval interval time between every call
	 * @param repeatCount repeat count, -1 means infinite
	 * @return tweener
	 */
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		static ULTweener* RepeatCall(UObject* WorldContextObject, FLTweenSimpleDynamicDelegate callFunction, float delayTime, float interval = 1.0f, int repeatCount = 1)
	{
		auto Tweener = ULTweenManager::VirtualTo(WorldContextObject, interval);
		if (Tweener)
		{
			Tweener
				->SetDelay(delayTime)
				->SetLoop(repeatCount == 1 || repeatCount == 0 ? ELTweenLoop::Once : ELTweenLoop::Restart, repeatCount)
				->OnCycleStart(callFunction)
				;
		}
		return Tweener;
	}
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"), Category = LTween)
		static class ULTweenerSequence* CreateSequence(UObject* WorldContextObject)
	{
		return ULTweenManager::CreateSequence(WorldContextObject);
	}
};
