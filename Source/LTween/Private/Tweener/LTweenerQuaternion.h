// Copyright 2019 LexLiu. All Rights Reserved.

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

	QuaternionGetterFunction getter;
	QuaternionSetterFunction setter;

	void SetInitialValue(const QuaternionGetterFunction& newGetter, const QuaternionSetterFunction& newSetter, FQuat newEndValue, float newDuration)
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
		FQuat value = FQuat::Slerp(startValue, endValue, lerpValue);
		if (setter.IsBound()) 
			setter.Execute(value);
	}
};