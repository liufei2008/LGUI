// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerColor.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerColor :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FColor startValue;
	FColor endValue;

	ColorGetterFunction getter;
	ColorSetterFunction setter;

	void SetInitialValue(const ColorGetterFunction& newGetter, const ColorSetterFunction& newSetter, FColor newEndValue, float newDuration)
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
		FColor value;
		value.R = FMath::Lerp(startValue.R, endValue.R, lerpValue);
		value.G = FMath::Lerp(startValue.G, endValue.G, lerpValue);
		value.B = FMath::Lerp(startValue.B, endValue.B, lerpValue);
		value.A = FMath::Lerp(startValue.A, endValue.A, lerpValue);
		if (setter.IsBound())
			setter.Execute(value);
	}
};