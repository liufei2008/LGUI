// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerFloat.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerFloat:public ULTweener
{
	GENERATED_BODY()
public:
	float startValue = 0.0f;//b
	float changeValue = 0.0f;//c

	float endValue = 0.0f;
	FloatGetterFunction getter;
	FloatSetterFunction setter;

	void SetInitialValue(const FloatGetterFunction& newGetter, const FloatSetterFunction& newSetter, float newEndValue, float newDuration)
	{
		this->duration = newDuration;
		this->endValue = newEndValue;
		this->getter = newGetter;
		this->setter = newSetter;
	}
protected:
	virtual void OnStartGetValue()
	{
		if (getter.IsBound())
			this->startValue = getter.Execute();
		this->changeValue = endValue - startValue;
	}

public:
	virtual void TweenAndApplyValue() override
	{
		auto value = tweenFunc.Execute(changeValue, startValue, elapseTime, duration);
		if (setter.IsBound()) 
			setter.Execute(value);
	}
};