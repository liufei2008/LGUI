// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTween.h"
#include "LTweenerMaterialVector.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerMaterialVector :public ULTweener
{
	GENERATED_BODY()
public:
	float startFloat = 0.0f;//b
	float changeFloat = 1.0f;//c
	FLinearColor startValue;
	FLinearColor endValue;
	int32 parameterIndex;

	FLTweenMaterialVectorGetterFunction getter;
	FLTweenMaterialVectorSetterFunction setter;

	void SetInitialValue(const FLTweenMaterialVectorGetterFunction& newGetter, const FLTweenMaterialVectorSetterFunction& newSetter, const FLinearColor& newEndValue, float newDuration, int32 newParameterIndex)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->parameterIndex = newParameterIndex;
		this->endValue = newEndValue;

		this->startFloat = 0.0f;
		this->changeFloat = 1.0f;
	}
protected:
	virtual void OnStartGetValue() override
	{
		if (getter.IsBound())
		{
			if (getter.Execute(startValue) == false)
			{
				UE_LOG(LTween, Error, TEXT("[ULTweenerMaterialVector/OnStartGetValue]Get paramter value error!"));
			}
		}
	}
	virtual void TweenAndApplyValue() override
	{
		float lerpValue = tweenFunc.Execute(changeFloat, startFloat, elapseTime, duration);
		FLinearColor value;
		value.R = FMath::Lerp(startValue.R, endValue.R, lerpValue);
		value.G = FMath::Lerp(startValue.G, endValue.G, lerpValue);
		value.B = FMath::Lerp(startValue.B, endValue.B, lerpValue);
		value.A = FMath::Lerp(startValue.A, endValue.A, lerpValue);
		if (setter.IsBound()) 
			if (setter.Execute(parameterIndex, value) == false)
			{
				UE_LOG(LTween, Warning, TEXT("[ULTweenerMaterialScalar/TweenAndApplyValue]Set paramter value error!"));
			}
	}
	virtual void SetValueForIncremental() override
	{
		auto diffValue = endValue - startValue;
		startValue = endValue;
		endValue += diffValue;
	}
};