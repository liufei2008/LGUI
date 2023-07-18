// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "LTweener.generated.h"

DECLARE_DELEGATE_RetVal_FourParams(float, FLTweenFunction, float, float, float, float);

DECLARE_DELEGATE_OneParam(FLTweenUpdateDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FLTweenUpdateMulticastDelegate, float);


DECLARE_DELEGATE_RetVal(float, FLTweenFloatGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenFloatSetterFunction, float);

DECLARE_DELEGATE_RetVal(int, FLTweenIntGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenIntSetterFunction, int);

DECLARE_DELEGATE_RetVal(FVector, FLTweenVectorGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenVectorSetterFunction, FVector);

DECLARE_DELEGATE_RetVal(FColor, FLTweenColorGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenColorSetterFunction, FColor);

DECLARE_DELEGATE_RetVal(FLinearColor, FLTweenLinearColorGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenLinearColorSetterFunction, FLinearColor);

DECLARE_DELEGATE_RetVal(FVector2D, FLTweenVector2DGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenVector2DSetterFunction, FVector2D);

DECLARE_DELEGATE_RetVal(FVector4, FLTweenVector4GetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenVector4SetterFunction, FVector4);

DECLARE_DELEGATE_RetVal(FQuat, FLTweenQuaternionGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenQuaternionSetterFunction, const FQuat&);

DECLARE_DELEGATE_RetVal(FRotator, FLTweenRotatorGetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenRotatorSetterFunction, FRotator);

DECLARE_DELEGATE_RetVal(FVector, FLTweenPositionGetterFunction);
DECLARE_DELEGATE_FourParams(FLTweenPositionSetterFunction, FVector, bool, FHitResult*, ETeleportType);

DECLARE_DELEGATE_RetVal(FQuat, FLTweenRotationQuatGetterFunction);
DECLARE_DELEGATE_FourParams(FLTweenRotationQuatSetterFunction, const FQuat&, bool, FHitResult*, ETeleportType);

DECLARE_DELEGATE_RetVal_OneParam(bool, FLTweenMaterialScalarGetterFunction, float&);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FLTweenMaterialScalarSetterFunction, int32, float);

DECLARE_DELEGATE_RetVal_OneParam(bool, FLTweenMaterialVectorGetterFunction, FLinearColor&);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FLTweenMaterialVectorSetterFunction, int32, const FLinearColor&);

/** for blueprint callback*/
DECLARE_DYNAMIC_DELEGATE(FTweenerSimpleDynamicDelegate);
/** @param InProgress Progress of this tween, from 0 to 1*/
DECLARE_DYNAMIC_DELEGATE_OneParam(FTweenerFloatDynamicDelegate, float, InProgress);

/**
 * Animation curve type
 */
UENUM(BlueprintType, Category = LTween)
enum class ELTweenEase :uint8
{
	Linear,
	InQuad,
	OutQuad,
	InOutQuad,
	InCubic,
	OutCubic,
	InOutCubic,
	InQuart,
	OutQuart,
	InOutQuart,
	InSine,
	OutSine,
	InOutSine,
	InExpo,
	OutExpo,
	InOutExpo,
	InCirc,
	OutCirc,
	InOutCirc,
	InElastic,
	OutElastic,
	InOutElastic,
	InBack,
	OutBack,
	InOutBack,
	InBounce,
	OutBounce,
	InOutBounce,
	/** Use CurveFloat to animate, only range 0-1 is valid. If use this you must assign curveFloat, or fallback to Linear. */
	CurveFloat,
};
/**
 * Loop type
 */
UENUM(BlueprintType, Category = LGUI)
enum class ELTweenLoop :uint8
{
	/** Play once, not loop */
	Once, 
	/** Each loop cycle restarts from beginning */
	Restart, 
	/** The tween move forward and backward at alternate cycles */
	Yoyo, 
	/** Continuously increments the tween at the end of each loop cycle (A to B, B to B+(A-B), and so on). */
	Incremental,
};

class UCurveFloat;

/** Class for manage single tween */
UCLASS(BlueprintType, Abstract)
class LTWEEN_API ULTweener : public UObject
{
	GENERATED_BODY()

public:
	ULTweener();

protected:
	friend class ULTweenerSequence;
	/** animation duration */
	float duration = 0.0f;
	/** delay time before animation start */
	float delay = 0.0f;
	/** total elapse time, include delay */
	float elapseTime = 0.0f;
	/** use CurveFloat as animation function,horizontal is time (0-1),vertical is value (0-1) */
	TWeakObjectPtr<UCurveFloat> curveFloat = nullptr;
	/** loop type */
	ELTweenLoop loopType = ELTweenLoop::Once;
	/** max loop count when loop type is Restart/Yoyo/Incremental */
	int32 maxLoopCount = 0;
	/** current completed cycle count */
	int32 loopCycleCount = 0;

	/** reverse animation */
	bool reverseTween = false;
	/** if animation start play */
	bool startToTween = false;
	/** mark this tween for kill, so when the next update came, the tween will be killed */
	bool isMarkedToKill = false;
	/** mark this tween as pause, it will keep pause until call Resume() */
	bool isMarkedPause = false;

	/** tween function */
	FLTweenFunction tweenFunc;

	/** call once after animation complete */
	FSimpleDelegate onCompleteCpp;
	/** if use loop, this will call every time when begin tween in every cycle */
	FSimpleDelegate onCycleStartCpp;
	/** if use loop, this will call every time after tween complete in every cycle */
	FSimpleDelegate onCycleCompleteCpp;
	/** call every frame after animation starts */
	FLTweenUpdateDelegate onUpdateCpp;
	/** call once when animation starts */
	FSimpleDelegate onStartCpp;
public:
	/**
	 * Set animation curve type.
	 * Has no effect if the Tween has already started.
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* SetEase(ELTweenEase easetype);
	/** set ease to CurveFloat and use CurveFloat as animation function, horizontal is time (0-1),vertical is value (0-1) */
	UE_DEPRECATED(4.23, "SetEaseCurve is not valid anymore, use SetEase and SetCurveFloat instead.")
	UFUNCTION(BlueprintCallable, Category = "LTween", meta = (DeprecatedFunction, DeprecationMessage = "SetEaseCurve is not valid anymore, use SetEase and SetCurveFloat instead."))
		ULTweener* SetEaseCurve(UCurveFloat* newCurve);
	/**
	 * Set delay time before start animation.
	 * Has no effect if the Tween has already started.
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual ULTweener* SetDelay(float newDelay);
	UE_DEPRECATED(4.23, "SetLoopType not valid anymore, use SetLoop instead.")
	UFUNCTION(BlueprintCallable, Category = "LTween", meta = (DeprecatedFunction, DeprecationMessage = "SetLoopType not valid anymore, use SetLoop instead."))
		virtual ULTweener* SetLoopType(ELTweenLoop newLoopType)
	{
		return SetLoop(newLoopType, -1);
	}
	/**
	 * Set loop of tween.
	 * Has no effect if the Tween has already started.
	 * @param newLoopType	loop type
	 * @param newLoopCount	number of cycles to play (-1 for infinite)
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual ULTweener* SetLoop(ELTweenLoop newLoopType, int32 newLoopCount = 1);
	UE_DEPRECATED(4.23, "GetLoopCount not valid anymore, use GetLoopCycleCount instead.")
	UFUNCTION(BlueprintCallable, Category = "LTween", meta = (DeprecatedFunction, DeprecationMessage = "GetLoopCount not valid anymore, use GetLoopCycleCount instead."))
		int32 GetLoopCount() { return loopCycleCount; }
	/** curently completed loop cycle count */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		int32 GetLoopCycleCount()const { return loopCycleCount; }

	/** execute when animation complete */
	ULTweener* OnComplete(const FSimpleDelegate& newComplete)
	{
		this->onCompleteCpp = newComplete;
		return this;
	}
	/** execute when animation complete */
	ULTweener* OnComplete(const TFunction<void()>& newComplete)
	{
		if (newComplete != nullptr)
		{
			this->onCompleteCpp.BindLambda(newComplete);
		}
		return this;
	}
	/** execute when animation complete */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnComplete(const FTweenerSimpleDynamicDelegate& newComplete)
	{
		this->onCompleteCpp.BindLambda([newComplete] {
			newComplete.ExecuteIfBound();
		});
		return this;
	}
	
	/** if use loop, this will call every time after tween complete in every cycle */
	ULTweener* OnCycleComplete(const FSimpleDelegate& newCycleComplete)
	{
		this->onCycleCompleteCpp = newCycleComplete;
		return this;
	}
	/** if use loop, this will call every time after tween complete in every cycle */
	ULTweener* OnCycleComplete(const TFunction<void()>& newCycleComplete)
	{
		if (newCycleComplete != nullptr)
		{
			this->onCycleCompleteCpp.BindLambda(newCycleComplete);
		}
		return this;
	}
	/** if use loop, this will call every time after tween complete in every cycle */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnCycleComplete(const FTweenerSimpleDynamicDelegate& newCycleComplete)
	{
		this->onCycleCompleteCpp.BindLambda([newCycleComplete] {
			newCycleComplete.ExecuteIfBound();
			});
		return this;
	}

	/** if use loop, this will call every time when begin tween in every cycle */
	ULTweener* OnCycleStart(const FSimpleDelegate& newCycleStart)
	{
		this->onCycleStartCpp = newCycleStart;
		return this;
	}
	/** if use loop, this will call every time when begin tween in every cycle */
	ULTweener* OnCycleStart(const TFunction<void()>& newCycleStart)
	{
		if (newCycleStart != nullptr)
		{
			this->onCycleStartCpp.BindLambda(newCycleStart);
		}
		return this;
	}
	/** if use loop, this will call every time when begin tween in every cycle */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnCycleStart(const FTweenerSimpleDynamicDelegate& newCycleStart)
	{
		this->onCycleStartCpp.BindLambda([newCycleStart] {
			newCycleStart.ExecuteIfBound();
			});
		return this;
	}

	/** execute every frame if animation is playing */
	ULTweener* OnUpdate(const FLTweenUpdateDelegate& newUpdate)
	{
		this->onUpdateCpp = newUpdate;
		return this;
	}
	/** execute every frame if animation is playing */
	ULTweener* OnUpdate(const TFunction<void(float)>& newUpdate)
	{
		if (newUpdate != nullptr)
		{
			this->onUpdateCpp.BindLambda(newUpdate);
		}
		return this;
	}
	/** execute every frame if animation is playing */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnUpdate(const FTweenerFloatDynamicDelegate& newUpdate)
	{
		this->onUpdateCpp.BindLambda([newUpdate](float progress) {
			newUpdate.ExecuteIfBound(progress);
		});
		return this;
	}
	
	/** execute when animation start*/
	ULTweener* OnStart(const FSimpleDelegate& newStart)
	{
		this->onStartCpp = newStart;
		return this;
	}
	/** execute when animation start, blueprint version*/
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnStart(const FTweenerSimpleDynamicDelegate& newStart)
	{
		this->onStartCpp.BindLambda([newStart] {
			newStart.ExecuteIfBound();
		});
		return this;
	}
	/** execute when animation start, lambda version*/
	ULTweener* OnStart(const TFunction<void()>& newStart)
	{
		if (newStart != nullptr)
		{
			this->onStartCpp.BindLambda(newStart);
		}
		return this;
	}
	/**
	 * Set CurveFloat as animation curve. Make sure ease type is CurveFloat. (Call SetEase function to set ease type)
	 * Has no effect if the Tween has already started.
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* SetCurveFloat(UCurveFloat* newCurveFloat);
	/**
	 * @return false: the tween is complete and need to be killed. true: the tween is still processing.
	 */
	virtual bool ToNext(float deltaTime);
	/**
	 * @return false: the tween is complete and need to be killed. true: the tween is still processing.
	 */
	virtual bool ToNextWithElapsedTime(float InElapseTime);
	/** Force stop this animation. if callComplete = true, will call OnComplete after stop*/
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void Kill(bool callComplete = false);
	/** Force stop this animation at this frame, set value to end, call OnComplete. */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void ForceComplete();
	/** Pause this animation. */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		void Pause()
	{
		isMarkedPause = true;
	}
	/** Continue play animation if is paused. */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		void Resume()
	{
		isMarkedPause = false;
	}
	/**
	 * Restart animation.
	 * Has no effect if the Tween is not started.
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void Restart();
	/**
	 * Send the tween to the given position in time.
	 * @param timePoint Time position to reach (if higher than the whole tween duration the tween will simply reach its end).
	 */
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void Goto(float timePoint);

protected:
	/** get value when start. child class must override this, check LTweenerFloat for reference */
	virtual void OnStartGetValue() PURE_VIRTUAL(ULTweener::OnStartGetValue, );
	/** set value when tweening. child class must override this, check LTweenerFloat for reference */
	virtual void TweenAndApplyValue(float currentTime) PURE_VIRTUAL(ULTweener::TweenAndApplyValue, );
	/** set start and end value if looptype is Incremental. */
	virtual void SetValueForIncremental() PURE_VIRTUAL(ULTweener::SetValueForIncremental, );
	/** set start and end value before the animation wan't to restart */
	virtual void SetOriginValueForRestart() PURE_VIRTUAL(ULTweener::SetOriginValueForRestart, );
	virtual void SetValueForYoyo() {};
	virtual void SetValueForRestart() {};
#pragma region tweenFunctions
public:
	/**
	* @param c		Change needed in value.
	* @param b		Starting value.
	* @param t		Current time (in frames or seconds).
	* @param d		Expected eaSing duration (in frames or seconds).
	* @return		The correct value.
	*/
	static float Linear(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return c*t / d + b;
	}
	static float InQuad(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		return c*t*t + b;
	}
	static float OutQuad(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		return -c*t*(t - 2) + b;
	}
	static float InOutQuad(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d * 0.5f;
		if (t < 1)
		{
			return c * 0.5f * t*t + b;
		}
		else
		{
			--t;
			return -c * 0.5f * (t*(t - 2) - 1) + b;
		}
	}
	static float InCubic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		return c*t*t*t + b;
	}
	static float OutCubic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t = t / d - 1.0f;
		return c*(t*t*t + 1) + b;
	}
	static float InOutCubic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d * 0.5f;
		if (t < 1)
		{
			return c * 0.5f * t*t*t + b;
		}
		else
		{
			t -= 2;
			return c * 0.5f * (t*t*t + 2) + b;
		}
	}
	static float InQuart(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		return c*t*t*t*t + b;
	}
	static float OutQuart(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t = t / d - 1.0f;
		return -c * (t*t*t*t - 1) + b;
	}
	static float InOutQuart(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d * 0.5f;
		if (t < 1)
		{
			return c * 0.5f * t*t*t*t + b;
		}
		else
		{
			t -= 2;
			return -c * 0.5f * (t*t*t*t - 2) + b;
		}
	}
	static float InSine(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return -c * FMath::Cos(t / d * HALF_PI) + c + b;
	}
	static float OutSine(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return c * FMath::Sin(t / d * HALF_PI) + b;
	}
	static float InOutSine(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return -c * 0.5f * (FMath::Cos(PI * t / d) - 1) + b;
	}
	static float InExpo(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return (t == 0.0f) ? b : c * FMath::Pow(2.0f, 10.0f * (t / d - 1.0f)) + b - c * 0.001f;
	}
	static float OutExpo(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return (t == d) ? b + c : c * 1.001f * (-FMath::Pow(2.0f, -10.0f * t / d) + 1.0f) + b;
	}
	static float InOutExpo(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t == 0) return b;
		if (t == d) return b + c;
		t /= d * 0.5f;
		if (t < 1.0f)
		{
			return c * 0.5f * FMath::Pow(2.0f, 10.0f * (t - 1.0f)) + b - c * 0.0005f;
		}
		else
		{
			return c * 0.5f * 1.0005f * (-FMath::Pow(2.0f, -10.0f * (t - 1.0f)) + 2.0f) + b;
		}
	}
	static float InCirc(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		return -c * (FMath::Sqrt(1.0f - t*t) - 1.0f) + b;
	}
	static float OutCirc(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t = t / d - 1.0f;
		return c * FMath::Sqrt(1.0f - t*t) + b;
	}
	static float InOutCirc(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d * 0.5f;
		if (t < 1)
		{
			return -c * 0.5f * (FMath::Sqrt(1 - t * t) - 1) + b;
		}
		else
		{
			t -= 2.0f;
			return c * 0.5f * (FMath::Sqrt(1.0f - t*t) + 1.0f) + b;
		}
	}
	static float InElastic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t == 0) return b;
		t /= d;
		if (t == 1) return b + c;
		float p = d*0.3f;
		float s = p / 4;
		float a = c;
		t -= 1.0f;
		return -(a*FMath::Pow(2.0f, 10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p)) + b;
	}
	static float OutElastic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t == 0) return b;
		t /= d;
		if (t == 1) return b + c;
		float p = d*0.3f;
		float s = p / 4;
		float a = c;
		return (a*FMath::Pow(2.0f, -10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p) + c + b);
	}
	static float InOutElastic(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t == 0) return b;
		t /= d * 0.5f;
		if (t == 2) return b + c;
		float p = d*0.3f;
		float s = p / 4;
		float a = c;
		if (t < 1.0f)
		{
			t -= 1.0f;
			return -0.5f*(a*FMath::Pow(2.0f, 10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p)) + b;
		}
		else
		{
			t -= 1.0f;
			return a * FMath::Pow(2.0f, -10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p)*0.5f + c + b;
		}
	}
	static float InBack(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		static float s = 1.70158f;
		t /= d;
		return c*t*t*((s + 1)*t - s) + b;
	}
	static float OutBack(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		static float s = 1.70158f;
		t = t / d - 1;
		return c*(t*t*((s + 1)*t + s) + 1) + b;
	}
	static float InOutBack(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t < d * 0.5f) return InBack(c, 0, t * 2, d) * .5f + b;
		else return OutBack(c, 0, t * 2 - d, d) * .5f + c * .5f + b;
	}

	static float OutBounce(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		t /= d;
		if (t < (1.0f / 2.75f)) {
			return c*(7.5625f*t*t) + b;
		}
		else if (t < (2.0f / 2.75f)) {
			t -= (1.5f / 2.75f);
			return c*(7.5625f*t*t + .75f) + b;
		}
		else if (t < (2.5f / 2.75f)) {
			t -= (2.25f / 2.75f);
			return c*(7.5625f*t*t + .9375f) + b;
		}
		else {
			t -= (2.625f / 2.75f);
			return c*(7.5625f*t*t + .984375f) + b;
		}
	}
	static float InBounce(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		return c - OutBounce(c, 0, d - t, d) + b;
	}
	static float InOutBounce(float c, float b, float t, float d)
	{
		if (d < KINDA_SMALL_NUMBER)return c + b;
		if (t < d * 0.5f) return InBounce(c, 0, t * 2, d) * .5f + b;
		else return OutBounce(c, 0, t * 2 - d, d) * .5f + c * .5f + b;
	}
	/** Tween use CurveFloat, in range of 0-1. if curveFloat is null, fallback to Linear */
	float CurveFloat(float c, float b, float t, float d);
#pragma endregion
};