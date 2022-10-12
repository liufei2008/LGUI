// Copyright 2019-2022 LexLiu. All Rights Reserved.

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

	FLTweenColorGetterFunction getter;
	FLTweenColorSetterFunction setter;

	void SetInitialValue(const FLTweenColorGetterFunction& newGetter, const FLTweenColorSetterFunction& newSetter, const FColor& newEndValue, float newDuration)
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
		FColor value;
		value.R = FMath::Lerp(startValue.R, endValue.R, lerpValue);
		value.G = FMath::Lerp(startValue.G, endValue.G, lerpValue);
		value.B = FMath::Lerp(startValue.B, endValue.B, lerpValue);
		value.A = FMath::Lerp(startValue.A, endValue.A, lerpValue);
		setter.ExecuteIfBound(value);
	}
	virtual void SetValueForIncremental() override
	{
		FColor diffValue;
		diffValue.R = endValue.R - startValue.R;
		diffValue.G = endValue.G - startValue.G;
		diffValue.B = endValue.B - startValue.B;
		diffValue.A = endValue.A - startValue.A;
		startValue = endValue;
		endValue += diffValue;
	}
};