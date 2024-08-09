// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "LTweener.h"
#include "LTweenManager.generated.h"

UCLASS(NotBlueprintable, NotBlueprintType, Transient)
class LTWEEN_API ULTweenTickHelperComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULTweenTickHelperComponent();
	UPROPERTY() TWeakObjectPtr<class ULTweenManager> Target = nullptr;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
};

UCLASS(NotBlueprintable, NotBlueprintType, Transient, NotPlaceable)
class LTWEEN_API ALTweenTickHelperActor : public AActor
{
	GENERATED_BODY()

public:
	ALTweenTickHelperActor();
	virtual void BeginPlay()override;
	virtual void Tick(float DeltaSeconds)override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason)override;
	UPROPERTY() TWeakObjectPtr<class ULTweenManager> Target = nullptr;
private:
	void OnLTweenManagerCreated(class ULTweenManager* LTweenManager);
	FDelegateHandle OnLTweenManagerCreatedDelegateHandle;

	void SetupTick(ULTweenManager* LTweenManager);
};

// This class is only for spawn ALTweenTickHelperActor for game world
UCLASS(NotBlueprintable, NotBlueprintType, Transient)
class LTWEEN_API ULTweenTickHelperWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void PostInitialize()override;
};

DECLARE_EVENT_OneParam(ULTweenManager, FLTweenManagerCreated, class ULTweenManager*);

UCLASS(NotBlueprintable, NotBlueprintType, Transient)
class LTWEEN_API ULTweenManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:	

	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface
	void Tick(ELTweenTickType TickType, float DeltaTime);
	
	UFUNCTION(BlueprintPure, Category = LTween, meta = (WorldContext = "WorldContextObject", DisplayName = "Get LTween Instance"))
	static ULTweenManager* GetLTweenInstance(UObject* WorldContextObject);
	static FLTweenManagerCreated OnLTweenManagerCreated;
private:
	/** current active tweener collection*/
	UPROPERTY(VisibleAnywhere, Category=LTween)TArray<TObjectPtr<ULTweener>> tweenerList;
	void OnTick(ELTweenTickType TickType, float DeltaTime, float UnscaledDeltaTime);
	FLTweenUpdateMulticastDelegate updateEvent;
	bool bTickPaused = false;
public:
	UE_DEPRECATED(5.1, "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick.")
	/** Use "CustomTick" instead of UE4's default Tick to control your tween animations. Call "DisableTick" function to disable UE4's default Tick function, then call this CustomTick function.*/
	UFUNCTION(BlueprintCallable, Category = LTween, meta=(DeprecatedFunction, DeprecationMessage = "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick."))
	void CustomTick(float DeltaTime);
	UE_DEPRECATED(5.1, "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick.")
	/**
	 * Disable default Tick function, so you can pause all tween or use CustomTick to do your own tick and use your own DeltaTime.
	 * This will only pause the tick with current LTweenManager instance, so after load a new level, default Tick will work again, and you need to call DisableTick again if you want to disable tick.
	 */ 
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (DeprecatedFunction, DeprecationMessage = "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick."))
	void DisableTick();
	UE_DEPRECATED(5.1, "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick.")
	/**
	 * Enable default Tick if it is disabled.
	 */
	UFUNCTION(BlueprintCallable, Category = LTween, meta = (DeprecatedFunction, DeprecationMessage = "Use Tweener->SetTickType(ELTweenTickType::Manual) then call this->ManualTick."))
	void EnableTick();


	UFUNCTION(BlueprintCallable, Category = LTween)
	void ManualTick(float DeltaTime);

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
	/**
	 * Remove tweener from list, so the tweener will not be managed by this LTweenManager.
	 * @param item tweener item
	 */
	static void RemoveTweener(UObject* WorldContextObject, ULTweener* item);

	static ULTweener* To(UObject* WorldContextObject, const FLTweenFloatGetterFunction& getter, const FLTweenFloatSetterFunction& setter, float endValue, float duration);
	static ULTweener* To(UObject* WorldContextObject, const FLTweenDoubleGetterFunction& getter, const FLTweenDoubleSetterFunction& setter, double endValue, float duration);
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
	static ULTweener* UpdateCall(UObject* WorldContextObject);

	static class ULTweenerSequence* CreateSequence(UObject* WorldContextObject);

	UE_DEPRECATED(5.2, "Use LTweenBPLibrary.UpdateCall instead.")
	static FDelegateHandle RegisterUpdateEvent(UObject* WorldContextObject, const FLTweenUpdateDelegate& update);
	UE_DEPRECATED(5.2, "Use LTweenBPLibrary.UpdateCall instead of RegisterUpdateEvent, and KillIfIsTweening for returned tweener instead of this UnregisterUpdateEvent.")
	static void UnregisterUpdateEvent(UObject* WorldContextObject, const FDelegateHandle& delegateHandle);
};
