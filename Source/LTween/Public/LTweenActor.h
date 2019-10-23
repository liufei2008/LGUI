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
	void OnTick(float DeltaTime);
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

	static ULTweener* To(const FLTweenFloatGetterFunction& getter, const FLTweenFloatSetterFunction& setter, float endValue, float duration);
	static ULTweener* To(const FLTweenIntGetterFunction& getter, const IntSetterFunction& setter, int endValue, float duration);
	static ULTweener* To(const FLTweenPositionGetterFunction& getter, const FLTweenPositionSetterFunction& setter, FVector endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const FLTweenVectorGetterFunction& getter, const FLTweenVectorSetterFunction& setter, FVector endValue, float duration);
	static ULTweener* To(const FLTweenColorGetterFunction& getter, const FLTweenColorSetterFunction& setter, FColor endValue, float duration);
	static ULTweener* To(const FLTweenLinearColorGetterFunction& getter, const FLTweenLinearColorSetterFunction& setter, FLinearColor endValue, float duration);
	static ULTweener* To(const FLTweenVector2DGetterFunction& getter, const FLTweenVector2DSetterFunction& setter, FVector2D endValue, float duration);
	static ULTweener* To(const FLTweenQuaternionGetterFunction& getter, const FLTweenQuaternionSetterFunction& setter, FQuat endValue, float duration);
	static ULTweener* To(const FLTweenRotatorGetterFunction& getter, const FLTweenRotatorSetterFunction& setter, FRotator endValue, float duration);
	static ULTweener* To(const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, FVector eulerAngle, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const FLTweenRotationQuatGetterFunction& getter, const FLTweenRotationQuatSetterFunction& setter, FQuat endValue, float duration, bool sweep = false, FHitResult* sweepHitResult = nullptr, ETeleportType teleportType = ETeleportType::None);
	static ULTweener* To(const FLTweenMaterialScalarGetterFunction& getter, const FLTweenMaterialScalarSetterFunction& setter, float endValue, float duration, int32 parameterIndex);
	static ULTweener* To(const FLTweenMaterialVectorGetterFunction& getter, const FLTweenMaterialVectorSetterFunction& setter, FLinearColor endValue, float duration, int32 parameterIndex);

	static ULTweener* VirtualTo(float duration);

	static ULTweener* DelayFrameCall(int delayFrame);

	static void RegisterUpdateEvent(const LTweenUpdateDelegate& update);
	static void UnregisterUpdateEvent(const LTweenUpdateDelegate& update);
	static void UnregisterUpdateEvent(const FDelegateHandle& delegateHandle);
};
