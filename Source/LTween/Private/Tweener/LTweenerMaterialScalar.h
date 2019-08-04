// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
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
	MaterialScalarGetterFunction getter;
	MaterialScalarSetterFunction setter;

	void SetInitialValue(const MaterialScalarGetterFunction& newGetter, const MaterialScalarSetterFunction& newSetter, float newEndValue, float newDuration, int32 newParameterIndex)
	{
		this->duration = newDuration;
		this->getter = newGetter;
		this->setter = newSetter;
		this->endValue = newEndValue;
		this->parameterIndex = newParameterIndex;
	}
protected:
	virtual void OnStartGetValue()
	{
		if (getter.IsBound())
		{
			if (getter.Execute(startValue))
			{
				this->changeValue = endValue - startValue;
			}
			else
			{
				UE_LOG(LTween, Error, TEXT("[ULTweenerMaterialScalar/OnStartGetValue]Get paramter value error!"));
			}
		}
	}

public:
	virtual void TweenAndApplyValue() override
	{
		auto value = tweenFunc.Execute(changeValue, startValue, elapseTime, duration);
		if (setter.IsBound()) 
			if (setter.Execute(parameterIndex, value) == false)
			{
				UE_LOG(LTween, Warning, TEXT("[ULTweenerMaterialScalar/TweenAndApplyValue]Set paramter value error!"));
			}
	}
};