// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTweener.h"
#include "LTweenActor.generated.h"

UCLASS(NotBlueprintable, NotPlaceable)
class LTWEEN_API ALTweenActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALTweenActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy()override;
	
	UFUNCTION(BlueprintPure, Category = LTween, meta = (WorldContext = "WorldContextObject", DisplayName = "Get LTween Instance"))
	static ALTweenActor* GetLTweenInstance(UObject* WorldContextObject);
private:
	/** current active tweener collection*/
	UPROPERTY(VisibleAnywhere, Category=LTween)TArray<ULTweener*> tweenerList;
	/** a world should only have one LTweenActor */
	static TMap<UWorld*, ALTweenActor*> WorldToInstanceMap;
	bool existInInstanceMap = false;
	virtual void Tick(float DeltaSeconds) override;
	void OnTick(float DeltaTime);
	LTweenUpdateMulticastDelegate updateEvent;
	bool TickPaused = false;
public:
	/** Use "CustomTick" instead of UE4's default Tick to control your tween animations. Call "DisableTick" function to disable UE4's default Tick function, then call this CustomTick function.*/
	UFUNCTION(BlueprintCallable, Category = LTween)
	void CustomTick(float DeltaTime);
	/**
	 * Disable default Tick function, so you can pause all tween or use CustomTick to do your own tick and use your own DeltaTime.
	 * This will only pause the tick with current LTweenActor instance, so after load a new level, default Tick will work again, and you need to call DisableTick again if you want to disable tick.
	 */ 
	UFUNCTION(BlueprintCallable, Category = LTween)
	void DisableTick();
	/**
	 * Enable default Tick if it is disabled.
	 */
	UFUNCTION(BlueprintCallable, Category = LTween)
	void EnableTick();

	/**
	 * Kill all tweens
	 */
	UFUNCTION(BlueprintCallable, Category = LTween)
	void KillAllTweens(bool callComplete = false);

	/**
	 * Is the tweener is currently tweening? 
	 * @param item tweener item
	 */
	static bool IsTweening(UObject* WorldContextObject, ULTweener* item);
	/**
	 * Kill the tweener if it is tweening.
	 * @param item tweener item
	 * @param callComplete true-execute onComplete event.
	 */
	static void KillIfIsTweening(UObject* WorldContextObject, ULTweener* item, bool callComplete);

	static ULTweener* To(UObject* WorldContextObject, const FLTweenFloatGetterFunction& getter, const FLTweenFloatSetterFunction& setter, float endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenIntGetterFunction& getter, const FLTweenIntSetterFunction& setter, int endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenPositionGetterFunction& getter, const FLTweenPositionSetterFunction& setter, const FVector& endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenVectorGetterFunction& getter, const FLTweenVectorSetterFunction& setter, const FVector& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenColorGetterFunction& getter, const FLTweenColorSetterFunction& setter, const FColor& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenLinearColorGetterFunction& getter, const FLTweenLinearColorSetterFunction& setter, const FLinearColor& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenVector2DGetterFunction& getter, const FLTweenVector2DSetterFunction& setter, const FVector2D& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenVector4GetterFunction& getter, const FLTweenVector4SetterFunction& setter, const FVector4& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenQuaternionGetterFunction& getter, const FLTweenQuaternionSetterFunction& setter, const FQuat& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenRotatorGetterFunction& getter, const FLTweenRotatorSetterFunction& setter, const FRotator& endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FVector& eulerAngle, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, const FQuat& endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenMaterialScalarGetterFunction& getter, const FLTweenMaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenMaterialVectorGetterFunction& getter, const FLTweenMaterialVectorSetterFunction& setter, const FLinearColor& endValue, float duration, int32 parameterIndex);

	static ULTweener* VirtualTo(UObject* WorldContextObject, float duration);

	static ULTweener* DelayFrameCall(UObject* WorldContextObject, int delayFrame);

	static FDelegateHandle RegisterUpdateEvent(UObject* WorldContextObject, const LTweenUpdateDelegate& update);
	static void UnregisterUpdateEvent(UObject* WorldContextObject, const FDelegateHandle& delegateHandle);
};
