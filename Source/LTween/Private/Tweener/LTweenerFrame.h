// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LTweener.h"
#include "LTween.h"
#include "LTweenerFrame.generated.h"

UCLASS(NotBlueprintType)
class LTWEEN_API ULTweenerFrame:public ULTweener
{
	GENERATED_BODY()
public:
	uint32 startFrameNumber = 0;//b
	uint32 endFrameNumber = 0;//c

	void SetInitialValue(int newEndValue)
	{
		this->startFrameNumber = GFrameNumber;
		this->endFrameNumber = GFrameNumber + newEndValue;
	}
protected:
	virtual void OnStartGetValue() override
	{
		
	}
	virtual bool ToNext(float deltaTime) override
	{
		if (isMarkedToKill)return false;
		if (isMarkedPause)return true;//no need to tick time if pause
		if (!startToTween)
		{
			startToTween = true;
			onStartCpp.ExecuteIfBound();
		}

		if (GFrameNumber >= endFrameNumber)
		{
			onUpdateCpp.ExecuteIfBound(1.0f);
			onCompleteCpp.ExecuteIfBound();
			return false;
		}
		else
		{
			onUpdateCpp.ExecuteIfBound((float)(GFrameNumber - startFrameNumber) / (endFrameNumber - startFrameNumber));
			return true;
		}
	}
	virtual void TweenAndApplyValue(float currentTime) override
	{
		
	}
	virtual ULTweener* SetDelay(float newDelay)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetDelay]LTweenerFrame does not support delay!"));
		return this;
	}
	virtual ULTweener* SetLoop(ELTweenLoop newLoopType, int32 newLoopCount)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetLoop]LTweenerFrame does not support loop!"));
		return this;
	}
	virtual void SetValueForIncremental() override
	{
		
	}
	virtual void SetOriginValueForRestart() override
	{
		auto changeValue = endFrameNumber - startFrameNumber;
		startFrameNumber = GFrameNumber;
		endFrameNumber = GFrameNumber + changeValue;
	}
};