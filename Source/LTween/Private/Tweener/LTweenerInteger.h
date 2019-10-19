// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerInteger.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerInteger :public ULTweener
{
	GENERATED_BODY()
public:
	float startValue = 0.0f;//b
	float changeValue = 0.0f;//c

	int endValue;
	FLTweenIntGetterFunction getter;
	IntSetterFunction setter;

	void SetInitialValue(const FLTweenIntGetterFunction& newGetter, const IntSetterFunction& newSetter, int newEndValue, float newDuration)
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
		this->changeValue = (float)endValue - startValue;
	}
	virtual void TweenAndApplyValue() override
	{
		auto value = tweenFunc.Execute(changeValue, startValue, elapseTime, duration);
		if (setter.IsBound()) 
			setter.Execute((int)value);
	}
};