// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	}
	virtual void TweenAndApplyValue() override
	{
		auto lerpValue = tweenFunc.Execute(changeFloat, startFloat, elapseTime, duration);
		setter.ExecuteIfBound(FMath::Lerp(startValue, endValue, lerpValue));
	}
	virtual void SetValueForIncremental() override
	{
		auto diffValue = endValue - startValue;
		startValue = endValue;
		endValue += diffValue;
	}
};