// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerRotator.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerRotator :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FRotator startValue;
	FRotator endValue;

	FLTweenRotatorGetterFunction getter;
	FLTweenRotatorSetterFunction setter;

	void SetInitialValue(const FLTweenRotatorGetterFunction& newGetter, const FLTweenRotatorSetterFunction& newSetter, const FRotator& newEndValue, float newDuration)
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
		FRotator value;
		value = startValue + lerpValue * (endValue - startValue);
		setter.ExecuteIfBound(value);
	}
	virtual void SetValueForIncremental() override
	{
		auto diffValue = endValue - startValue;
		startValue = endValue;
		endValue += diffValue;
	}
};