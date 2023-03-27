// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerQuaternion.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerQuaternion :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FQuat startValue;
	FQuat endValue;

	FLTweenQuaternionGetterFunction getter;
	FLTweenQuaternionSetterFunction setter;

	FQuat originStartValue;

	void SetInitialValue(const FLTweenQuaternionGetterFunction& newGetter, const FLTweenQuaternionSetterFunction& newSetter, const FQuat& newEndValue, float newDuration)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;

		this->startFloat = 0.0f;
		this->changeFloat = 1.0f;
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
		FQuat value = FQuat::Slerp(startValue, endValue, lerpValue);
		setter.ExecuteIfBound(value);
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