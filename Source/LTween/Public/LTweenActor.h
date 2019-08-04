// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LTweener.h"
#include "LTweenActor.generated.h"

UCLASS(NotBlueprintable, notplaceable)
class LTWEEN_API ALTweenActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALTweenActor();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	//current active tweener collection
	UPROPERTY(VisibleAnywhere, Category=LTween)TArray<ULTweener*> tweenerList;
	static ALTweenActor* Instance;
	static bool InitCheck();
	virtual void Tick(float DeltaSeconds) override;
	FORCEINLINE void OnTick(float DeltaTime);
	LTweenUpdateMulticastDelegate updateEvent;
	bool TickPaused = false;
public:
	//this function can use your own DeltaSeconds instead of UE4's Tick. use DisableTick to disable UE4's Tick function, then call this CustomTick function.
	static void CustomTick(float DeltaTime);
	//disable default Tick function, so you can pause all tween or use CustomTick to do your own tick and use your own DeltaTime.
	//this will only pause the tick with current LTweenActor instance, so after load a new level, default Tick will work again, and you need to call DisableTick again if you want to disable tick
	static void DisableTick();
	static void EnableTick();
	static bool IsTweening(ULTweener* item);
	static void KillIfIsTweening(ULTweener* item, bool callComplete);

	static ULTweener* To(const FloatGetterFunction& getter, const FloatSetterFunction& setter, float endValue, float duration);
	static ULTweener* To(const IntGetterFunction& getter, const IntSetterFunction& setter, int endValue, float duration);
	static ULTweener* To(const PositionGetterFunction& getter, const PositionSetterFunction& setter, FVector endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const VectorGetterFunction& getter, const VectorSetterFunction& setter, FVector endValue, float duration);
	static ULTweener* To(const ColorGetterFunction& getter, const ColorSetterFunction& setter, FColor endValue, float duration);
	static ULTweener* To(const LinearColorGetterFunction& getter, const LinearColorSetterFunction& setter, FLinearColor endValue, float duration);
	static ULTweener* To(const Vector2DGetterFunction& getter, const Vector2DSetterFunction& setter, FVector2D endValue, float duration);
	static ULTweener* To(const QuaternionGetterFunction& getter, const QuaternionSetterFunction& setter, FQuat endValue, float duration);
	static ULTweener* To(const RotatorGetterFunction& getter, const RotatorSetterFunction& setter, FRotator endValue, float duration);
	static ULTweener* To(const RotationQuatGetterFunction& getter, const RotationQuatSetterFunction& setter, FVector eulerAngle, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const RotationQuatGetterFunction& getter, const RotationQuatSetterFunction& setter, FQuat endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const MaterialScalarGetterFunction& getter, const MaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex);
	static ULTweener* To(const MaterialVectorGetterFunction& getter, const MaterialVectorSetterFunction& setter, FLinearColor endValue, float duration, int32 parameterIndex);

	static ULTweener* VirtualTo(float duration);

	static ULTweener* DelayFrameCall(int delayFrame);

	static void RegisterUpdateEvent(const LTweenUpdateDelegate& update);
	static void UnregisterUpdateEvent(const LTweenUpdateDelegate& update);
	static void UnregisterUpdateEvent(const FDelegateHandle& delegateHandle);
};
