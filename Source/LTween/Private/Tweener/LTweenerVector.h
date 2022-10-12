// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerVector.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerVector :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FVector startValue;
	FVector endValue;

	FLTweenVectorGetterFunction getter;
	FLTweenVectorSetterFunction setter;

	void SetInitialValue(const FLTweenVectorGetterFunction& newGetter, const FLTweenVectorSetterFunction& newSetter, const FVector& newEndValue, float newDuration)
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
	}
	virtual void TweenAndApplyValue() override
	{
		float lerpValue = tweenFunc.Execute(changeFloat, startFloat, elapseTime, duration);
		auto value = FMath::Lerp(startValue, endValue, lerpValue);
		setter.ExecuteIfBound(value);
	}
	virtual void SetValueForIncremental() override
	{
		auto diffValue = endValue - startValue;
		startValue = endValue;
		endValue += diffValue;
	}
};