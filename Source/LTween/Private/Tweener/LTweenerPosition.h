// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerPosition.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerPosition :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FVector startValue;
	FVector endValue;

	bool sweep = false;
	FHitResult* sweepHitResult = nullptr;
	ETeleportType teleportType = ETeleportType::None;

	FLTweenPositionGetterFunction getter;
	FLTweenPositionSetterFunction setter;

	FVector originStartValue;

	void SetInitialValue(const FLTweenPositionGetterFunction& newGetter, const FLTweenPositionSetterFunction& newSetter, const FVector& newEndValue, float newDuration, bool newSweep = false, FHitResult* newSweepHitResult = nullptr, ETeleportType newTeleportType = ETeleportType::None)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;

		this->startFloat = 0.0f;
		this->changeFloat = 1.0f;

		this->sweep = newSweep;
		this->sweepHitResult = newSweepHitResult;
		this->teleportType = newTeleportType;
	}
protected:
	virtual void OnStartGetValue() override
	{
		if (getter.IsBound())
			this->startValue = getter.Execute();
		this->originStartValue = this->startValue;
	}
	virtual void TweenAndApplyValue(float currentTime) override
	{
		float lerpValue = tweenFunc.Execute(changeFloat, startFloat, currentTime, duration);
		FVector value = FMath::Lerp(startValue, endValue, lerpValue);
		setter.ExecuteIfBound(value, sweep, sweepHitResult, teleportType);
	}
	virtual void SetValueForIncremental() override
	{
		auto diffValue = endValue - startValue;
		startValue = endValue;
		endValue += diffValue;
	}
	virtual void SetOriginValueForRestart() override
	{
		auto diffValue = endValue - startValue;
		startValue = originStartValue;
		endValue = originStartValue + diffValue;
	}
};