// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LTween.h"
#include "Curves/CurveFloat.h"
#include "LTweener.generated.h"

DECLARE_DELEGATE_RetVal_FourParams(float, FLTweenFunction, float, float, float, float);

// @param	Progress of this tween, from 0 to 1.
DECLARE_DELEGATE_OneParam(LTweenUpdateDelegate, float);
DECLARE_MULTICAST_DELEGATE_OneParam(LTweenUpdateMulticastDelegate, float);


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

DECLARE_DELEGATE_RetVal(FVector, FLTweenVector4GetterFunction);
DECLARE_DELEGATE_OneParam(FLTweenVector4SetterFunction, FVector);

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

//for blueprint callback

DECLARE_DYNAMIC_DELEGATE(FTweenerSimpleDynamicDelegate);
//@param InProgress Progress of this tween, from 0 to 1
DECLARE_DYNAMIC_DELEGATE_OneParam(FTweenerFloatDynamicDelegate, float, InProgress);

//animation curve type
UENUM(BlueprintType)
enum class LTweenEase :uint8
{
	Linear			UMETA(DisplayName = "Linear"),
	InQuad			UMETA(DisplayName = "InQuad"),
	OutQuad			UMETA(DisplayName = "OutQuad"),
	InOutQuad		UMETA(DisplayName = "InOutQuad"),
	InCubic			UMETA(DisplayName = "InCubic"),
	OutCubic		UMETA(DisplayName = "OutCubic"),
	InOutCubic		UMETA(DisplayName = "InOutCubic"),
	InQuart			UMETA(DisplayName = "InQuart"),
	OutQuart		UMETA(DisplayName = "OutQuart"),
	InOutQuart		UMETA(DisplayName = "InOutQuart"),
	InSine			UMETA(DisplayName = "InSine"),
	OutSine			UMETA(DisplayName = "OutSine"),
	InOutSine		UMETA(DisplayName = "InOutSine"),
	InExpo			UMETA(DisplayName = "InExpo"),
	OutExpo			UMETA(DisplayName = "OutExpo"),
	InOutExpo		UMETA(DisplayName = "InOutExpo"),
	InCirc			UMETA(DisplayName = "InCirc"),
	OutCirc			UMETA(DisplayName = "OutCirc"),
	InOutCirc		UMETA(DisplayName = "InOutCirc"),
	InElastic		UMETA(DisplayName = "InElastic"),
	OutElastic		UMETA(DisplayName = "OutElastic"),
	InOutElastic	UMETA(DisplayName = "InOutElastic"),
	InBack			UMETA(DisplayName = "InBack"),
	OutBack			UMETA(DisplayName = "OutBack"),
	InOutBack		UMETA(DisplayName = "InOutBack"),
	InBounce		UMETA(DisplayName = "InBounce"),
	OutBounce		UMETA(DisplayName = "OutBounce"),
	InOutBounce		UMETA(DisplayName = "InOutBounce"),
	//Use CurveFloat to animate, only range 0-1 is valid. If use this you must assign curveFloat, or fallback to Linear
	CurveFloat		UMETA(DisplayName = "CurveFloat"),
};
//loop type
UENUM(BlueprintType)
enum class LTweenLoop :uint8
{
	Once, 
	Restart, 
	Yoyo, 
};


UCLASS(BlueprintType)
//class for manage single tween
class LTWEEN_API ULTweener : public UObject
{
	GENERATED_BODY()

public:
	ULTweener();

protected:
	//d, animation duration
	float duration = 0.0f;
	//delay time before animation start
	float delay = 0.0f;
	//t, elapse time
	float elapseTime = 0.0f;
	//use CurveFloat as animation function,horizontal is time (0-1),vertical is value (0-1)
	UPROPERTY(Transient) UCurveFloat* curveFloat = nullptr;
	//loop type
	LTweenLoop loopType = LTweenLoop::Once;
	//if loopType = Yoyo, reverse time
	bool reverseTween = false;
	//how many times is this animation loops
	int32 loopCount = 0;

	//if animation start play
	bool startToTween = false;
	//tween function
	FLTweenFunction tweenFunc;
	//
	bool isMarkedToKill = false;
	//
	bool isMarkedPause = false;

	//call once after animation complete. if use loop, this will call every time after complete in every cycle
	FSimpleDelegate onCompleteCpp;
	//call every frame after animation starts
	LTweenUpdateDelegate onUpdateCpp;
	//call once when animation starts
	FSimpleDelegate onStartCpp;
public:
	//set animation curve type
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* SetEase(LTweenEase easetype);
	//use CurveFloat as animation function,horizontal is time (0-1),vertical is value (0-1)
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* SetEaseCurve(UCurveFloat* newCurve);
	//delay start animation
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual ULTweener* SetDelay(float newDelay)
	{
		if (newDelay > 0)
		{
			this->delay = newDelay;
		}
		return this;
	}
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual ULTweener* SetLoopType(LTweenLoop newLoopType)
	{
		this->loopType = newLoopType;
		return this;
	}
	UFUNCTION(BlueprintCallable, Category = "LTween")
		int32 GetLoopCount() { return loopCount; }
	//finish animation callback function
	ULTweener* OnComplete(const FSimpleDelegate& newComplete)
	{
		this->onCompleteCpp = newComplete;
		return this;
	}
	//finish animation callback function for blueprint
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnComplete(const FTweenerSimpleDynamicDelegate& newComplete)
	{
		this->onCompleteCpp.BindLambda([newComplete] {
			if (newComplete.IsBound())newComplete.Execute();
		});
		return this;
	}
	//finish animation callback function for lambda
	ULTweener* OnComplete(const TFunction<void()>& newComplete)
	{
		this->onCompleteCpp.BindLambda(newComplete);
		return this;
	}
	//execute every frame if animation is playing
	ULTweener* OnUpdate(const LTweenUpdateDelegate& newUpdate)
	{
		this->onUpdateCpp = newUpdate;
		return this;
	}
	//execute every frame if animation is playing, blueprint version
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnUpdate(const FTweenerFloatDynamicDelegate& newUpdate)
	{
		this->onUpdateCpp.BindLambda([newUpdate](float progress) {
			if (newUpdate.IsBound())newUpdate.Execute(progress);
		});
		return this;
	}
	//execute every frame if animation is playing, lambda version
	ULTweener* OnUpdate(const TFunction<void(float)>& newUpdate)
	{
		this->onUpdateCpp.BindLambda(newUpdate);
		return this;
	}
	//execute when animation start
	ULTweener* OnStart(const FSimpleDelegate& newStart)
	{
		this->onStartCpp = newStart;
		return this;
	}
	//execute when animation start, blueprint version
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* OnStart(const FTweenerSimpleDynamicDelegate& newStart)
	{
		this->onStartCpp.BindLambda([newStart] {
			if (newStart.IsBound())newStart.Execute();
		});
		return this;
	}
	//execute when animation start, lambda version
	ULTweener* OnStart(const TFunction<void()>& newStart)
	{
		this->onStartCpp.BindLambda(newStart);
		return this;
	}
	//set CurveFloat as animation curve
	UFUNCTION(BlueprintCallable, Category = "LTween")
		ULTweener* SetCurveFloat(UCurveFloat* newCurveFloat)
	{
		curveFloat = newCurveFloat;
		return this;
	}

	virtual bool ToNext(float deltaTime);
	//force stop this animation. if callComplete = true, OnComplete will call after stop
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void Kill(bool callComplete = false);
	//force complete this animation at this frame, call OnComplete
	UFUNCTION(BlueprintCallable, Category = "LTween")
		virtual void ForceComplete();
	//pause this animation
	UFUNCTION(BlueprintCallable, Category = "LTween")
		void Pause()
	{
		isMarkedPause = true;
	}
	//resume animation after pause
	UFUNCTION(BlueprintCallable, Category = "LTween")
		void Resume()
	{
		isMarkedPause = false;
	}

protected:
	//get value when start. child class must override this, check LTweenerFloat for reference
	virtual void OnStartGetValue()
	{
		UE_LOG(LTween, Warning, TEXT("The function \"OnStartGetValue\" must be override by a child class!"));
	}
	//set value when tweening. child class must override this, check LTweenerFloat for reference
	virtual void TweenAndApplyValue()
	{
		UE_LOG(LTween, Warning, TEXT("The function \"TweenAndApplyValue\" must be override by a child class!"));
	};
#pragma region tweenFunctions
private:
	/**
	* @param c		Change needed in value.
	* @param b		Starting value.
	* @param t		Current time (in frames or seconds).
	* @param d		Expected eaSing duration (in frames or seconds).
	* @return		The correct value.
	*/
	float Linear(float c, float b, float t, float d)
	{
		return c*t / d + b;
	}
	float InQuad(float c, float b, float t, float d)
	{
		t /= d;
		return c*t*t + b;
	}
	float OutQuad(float c, float b, float t, float d)
	{
		t /= d;
		return -c*t*(t - 2) + b;
	}
	float InOutQuad(float c, float b, float t, float d)
	{
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
	float InCubic(float c, float b, float t, float d)
	{
		t /= d;
		return c*t*t*t + b;
	}
	float OutCubic(float c, float b, float t, float d)
	{
		t = t / d - 1;
		return c*(t*t*t + 1) + b;
	}
	float InOutCubic(float c, float b, float t, float d)
	{
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
	float InQuart(float c, float b, float t, float d)
	{
		t /= d;
		return c*t*t*t*t + b;
	}
	float OutQuart(float c, float b, float t, float d)
	{
		t = t / d - 1;
		return -c * (t*t*t*t - 1) + b;
	}
	float InOutQuart(float c, float b, float t, float d)
	{
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
	float InSine(float c, float b, float t, float d)
	{
		return -c * FMath::Cos(t / d * HALF_PI) + c + b;
	}
	float OutSine(float c, float b, float t, float d)
	{
		return c * FMath::Sin(t / d * HALF_PI) + b;
	}
	float InOutSine(float c, float b, float t, float d)
	{
		return -c * 0.5f * (FMath::Cos(PI*t / d) - 1) + b;
	}
	float InExpo(float c, float b, float t, float d)
	{
		return (t == 0.0f) ? b : c * FMath::Pow(2.0f, 10.0f * (t / d - 1.0f)) + b - c * 0.001f;
	}
	float OutExpo(float c, float b, float t, float d)
	{
		return (t == d) ? b + c : c * 1.001f * (-FMath::Pow(2.0f, -10.0f * t / d) + 1.0f) + b;
	}
	float InOutExpo(float c, float b, float t, float d)
	{
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
	float InCirc(float c, float b, float t, float d)
	{
		t /= d;
		return -c * (FMath::Sqrt(1.0f - t*t) - 1.0f) + b;
	}
	float OutCirc(float c, float b, float t, float d)
	{
		t /= d;
		return c * FMath::Sqrt(1.0f - (t - 1.0f)*t) + b;
	}
	float InOutCirc(float c, float b, float t, float d)
	{
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
	float InElastic(float c, float b, float t, float d)
	{
		if (t == 0) return b;
		t /= d;
		if (t == 1) return b + c;
		float p = d*0.3f;
		float s = p / 4;
		float a = c;
		t -= 1.0f;
		return -(a*FMath::Pow(2.0f, 10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p)) + b;
	}
	float OutElastic(float c, float b, float t, float d)
	{
		if (t == 0) return b;
		t /= d;
		if (t == 1) return b + c;
		float p = d*0.3f;
		float s = p / 4;
		float a = c;
		return (a*FMath::Pow(2.0f, -10.0f * t) * FMath::Sin((t*d - s)*(2.0f * PI) / p) + c + b);
	}
	float InOutElastic(float c, float b, float t, float d)
	{
		if (t == 0) return b;
		t /= d;
		if (t * 0.5f == 2) return b + c;
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
	float InBack(float c, float b, float t, float d)
	{
		static float s = 1.70158f;
		t /= d;
		return c*t*t*((s + 1)*t - s) + b;
	}
	float OutBack(float c, float b, float t, float d)
	{
		static float s = 1.70158f;
		t = t / d - 1;
		return c*(t*t*((s + 1)*t + s) + 1) + b;
	}
	float InOutBack(float c, float b, float t, float d)
	{
		if (t < d * 0.5f) return InBack(t * 2, 0, c, d) * .5f + b;
		else return OutBack(t * 2 - d, 0, c, d) * .5f + c * .5f + b;
	}

	float OutBounce(float c, float b, float t, float d)
	{
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
	float InBounce(float c, float b, float t, float d)
	{
		return c - OutBounce(d - t, 0, c, d) + b;
	}
	float InOutBounce(float c, float b, float t, float d)
	{
		if (t < d * 0.5f) return InBounce(t * 2, 0, c, d) * .5f + b;
		else return OutBounce(t * 2 - d, 0, c, d) * .5f + c*.5f + b;
	}
	//Tween use CurveFloat, in range of 0-1. if curveFloat is null, fallback to Linear
	float CurveFloat(float c, float b, float t, float d);
#pragma endregion
};