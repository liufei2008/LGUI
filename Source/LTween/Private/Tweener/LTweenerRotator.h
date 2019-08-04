// Copyright 2019 LexLiu. All Rights Reserved.

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

	RotatorGetterFunction getter;
	RotatorSetterFunction setter;

	void SetInitialValue(const RotatorGetterFunction& newGetter, const RotatorSetterFunction& newSetter, FRotator newEndValue, float newDuration)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;

		this->startFloat = 0.0f;
		this->changeFloat = 1.0f;
	}
protected:
	virtual void OnStartGetValue()
	{
		if (getter.IsBound())
			this->startValue = getter.Execute();
	}

public:

	virtual void TweenAndApplyValue() override
	{
		float lerpValue = tweenFunc.Execute(changeFloat, startFloat, elapseTime, duration);
		FRotator value;
		value = startValue + lerpValue * (endValue - startValue);
		if (setter.IsBound()) 
			setter.Execute(value);
	}
};