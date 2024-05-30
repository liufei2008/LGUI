// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTween.h"
#include "LTweenerUpdate.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerUpdate :public ULTweener
{
	GENERATED_BODY()
public:
	void SetInitialValue(int newEndValue)
	{
		
	}
protected:
	virtual void OnStartGetValue() override
	{
		
	}
	virtual bool ToNext(float deltaTime) override
	{
		if (auto world = GetWorld())
		{
			if (world->IsPaused() && affectByGamePause)return true;
		}
		if (isMarkedToKill)return false;
		if (isMarkedPause)return true;//no need to tick time if pause
		onUpdateCpp.ExecuteIfBound(deltaTime);
		return true;
	}
	virtual void TweenAndApplyValue(float currentTime) override
	{
		
	}
	virtual ULTweener* SetDelay(float newDelay)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetDelay]LTweenerUpdate does not support delay!"));
		return this;
	}
	virtual ULTweener* SetLoop(ELTweenLoop newLoopType, int32 newLoopCount)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetLoop]LTweenerUpdate does not support loop!"));
		return this;
	}
	virtual void SetValueForIncremental() override
	{
		
	}
	virtual void SetOriginValueForRestart() override
	{
		
	}
};