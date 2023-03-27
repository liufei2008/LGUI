// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTween.h"
#include "LTweenerMaterialScalar.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerMaterialScalar:public ULTweener
{
	GENERATED_BODY()
public:
	float startValue = 0.0f;//b
	float changeValue = 0.0f;//c
	int32 parameterIndex;
	float endValue = 0.0f;
	FLTweenMaterialScalarGetterFunction getter;
	FLTweenMaterialScalarSetterFunction setter;

	float originStartValue = 0.0f;

	void SetInitialValue(const FLTweenMaterialScalarGetterFunction& newGetter, const FLTweenMaterialScalarSetterFunction& newSetter, float newEndValue, float newDuration, int32 newParameterIndex)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;
		this->parameterIndex = newParameterIndex;
	}
protected:
	virtual void OnStartGetValue() override
	{
		if (getter.IsBound())
		{
			if (getter.Execute(startValue))
			{
				this->changeValue = endValue - startValue;
				this->originStartValue = this->changeValue;
			}
			else
			{
				UE_LOG(LTween, Error, TEXT("[ULTweenerMaterialScalar/OnStartGetValue]Get paramter value error!"));
			}
		}
	}
	virtual void TweenAndApplyValue(float currentTime) override
	{
		auto value = tweenFunc.Execute(changeValue, startValue, currentTime, duration);
		if (setter.Execute(parameterIndex, value) == false)
		{
			UE_LOG(LTween, Warning, TEXT("[ULTweenerMaterialScalar/TweenAndApplyValue]Set paramter value error!"));
		}
	}
	virtual void SetValueForIncremental() override
	{
		startValue = endValue;
		endValue += changeValue;
	}
	virtual void SetOriginValueForRestart() override
	{
		startValue = originStartValue;
		endValue = originStartValue + changeValue;
	}
};