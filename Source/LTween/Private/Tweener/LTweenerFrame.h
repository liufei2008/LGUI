﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
			if (onStartCpp.IsBound())
				onStartCpp.Execute();
		}

		if (GFrameNumber >= endFrameNumber)
		{
			if (onUpdateCpp.IsBound())
				onUpdateCpp.Execute(1.0f);
			if (onCompleteCpp.IsBound())
				onCompleteCpp.Execute();
			return false;
		}
		else
		{
			if (onUpdateCpp.IsBound())
				onUpdateCpp.Execute((float)(GFrameNumber - startFrameNumber) / (endFrameNumber - startFrameNumber));
			return true;
		}
	}
	virtual void TweenAndApplyValue() override
	{
		
	}
	virtual ULTweener* SetDelay(float newDelay)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetDelay]LTweenerFrame does not support delay!"));
		return this;
	}
	virtual ULTweener* SetLoopType(LTweenLoop newLoopType)override
	{
		UE_LOG(LTween, Error, TEXT("[LTweenerFrame::SetLoopType]LTweenerFrame does not support loop!"));
		return this;
	}
};