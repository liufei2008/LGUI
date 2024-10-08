// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTweenerVirtual.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerVirtual:public ULTweener
{
	GENERATED_BODY()
public:

	void SetInitialValue(float newDuration)
	{
		this->duration = newDuration;
	}
protected:
	virtual void OnStartGetValue() override
	{
		
	}
	virtual void TweenAndApplyValue(float currentTime) override
	{
		
	}
	virtual void SetValueForIncremental() override
	{
		
	}
	virtual void SetOriginValueForRestart() override
	{
		
	}
};