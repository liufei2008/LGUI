// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerInteger.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerInteger :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	int startValue;
	int endValue;

	FLTweenIntGetterFunction getter;
	FLTweenIntSetterFunction setter;

	int originStartValue = 0;

	void SetInitialValue(const FLTweenIntGetterFunction& newGetter, const FLTweenIntSetterFunction& newSetter, int newEndValue, float newDuration)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;
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
		auto lerpValue = tweenFunc.Execute(changeFloat, startFloat, currentTime, duration);
		setter.ExecuteIfBound(FMath::Lerp(startValue, endValue, lerpValue));
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
		startValue = GFrameNumber;
		endValue = GFrameNumber + diffValue;
	}
};