﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LTweenerSequence.h"
#include "LTween.h"
#include "LTweenManager.h"
#include "Tweener/LTweenerFrame.h"
#include "Tweener/LTweenerVirtual.h"

ULTweenerSequence* ULTweenerSequence::Append(UObject* WorldContextObject, ULTweener* tweener)
{
	return this->Insert(WorldContextObject, duration, tweener);
}
ULTweenerSequence* ULTweenerSequence::AppendInterval(UObject* WorldContextObject, float interval)
{
	if (elapseTime > 0 || startToTween)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d can't do this because this tween already started"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	duration += interval;
	return this;
}
ULTweenerSequence* ULTweenerSequence::Insert(UObject* WorldContextObject, float timePosition, ULTweener* tweener)
{
	if (!IsValid(tweener))
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d tweener is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweener->IsA<ULTweenerFrame>() || tweener->IsA<ULTweenerVirtual>())
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d sequence not support this tweener type: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(tweener->GetClass()->GetName()));
		return this;
	}
	if (elapseTime > 0 || startToTween)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d can't do this because this tween already started"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweenerList.Contains(tweener))
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d tweener already contains in the list"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweener->loopType != LTweenLoop::Once && tweener->maxLoopCount == -1)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d infinite tweener is not supported in sequence, will convert to 1"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		tweener->maxLoopCount = 1;
	}
	ULTweenManager::RemoveTweener(WorldContextObject, tweener);
	int loopCount = tweener->loopType == LTweenLoop::Once ? 1 : tweener->maxLoopCount;
	float tweenerTime = tweener->delay + tweener->duration * loopCount;
	tweener->SetDelay(tweener->delay + timePosition);
	tweenerList.Add(tweener);
	lastTweenStartTime = timePosition;
	float inputDuration = tweenerTime + timePosition;
	if (duration < inputDuration)
	{
		duration = inputDuration;
	}
	return this;
}
ULTweenerSequence* ULTweenerSequence::Prepend(UObject* WorldContextObject, ULTweener* tweener)
{
	if (!IsValid(tweener))
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d tweener is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweener->IsA<ULTweenerFrame>() || tweener->IsA<ULTweenerVirtual>())
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d sequence not support this tweener type: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(tweener->GetClass()->GetName()));
		return this;
	}
	if (elapseTime > 0 || startToTween)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d can't do this because this tween already started"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweenerList.Contains(tweener))
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d tweener already contains in the list"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweener->loopType != LTweenLoop::Once && tweener->maxLoopCount == -1)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d infinite tweener is not supported in sequence, will convert to 1"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		tweener->maxLoopCount = 1;
	}
	ULTweenManager::RemoveTweener(WorldContextObject, tweener);
	int loopCount = tweener->loopType == LTweenLoop::Once ? 1 : tweener->maxLoopCount;
	float inputDuration = tweener->delay + tweener->duration * loopCount;
	//offset others 
	for (auto& item : tweenerList)
	{
		item->SetDelay(item->delay + inputDuration);
	}
	tweenerList.Insert(tweener, 0);
	duration += inputDuration;
	lastTweenStartTime = 0;
	return this;
}
ULTweenerSequence* ULTweenerSequence::PrependInterval(UObject* WorldContextObject, float interval)
{
	if (elapseTime > 0 || startToTween)
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d can't do this because this tween already started"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	//offset others 
	for (auto& item : tweenerList)
	{
		item->SetDelay(item->delay + interval);
	}
	duration += interval;
	lastTweenStartTime += interval;
	return this;
}
ULTweenerSequence* ULTweenerSequence::Join(UObject* WorldContextObject, ULTweener* tweener)
{
	if (!IsValid(tweener))
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d tweener is null"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		return this;
	}
	if (tweener->IsA<ULTweenerFrame>() || tweener->IsA<ULTweenerVirtual>())
	{
		UE_LOG(LTween, Error, TEXT("[%s].%d sequence not support this tweener type: %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *(tweener->GetClass()->GetName()));
		return this;
	}
	if (tweenerList.Num() == 0)return this;
	return this->Insert(WorldContextObject, lastTweenStartTime, tweener);
}

void ULTweenerSequence::TweenAndApplyValue(float currentTime)
{
	for(int i = 0; i < tweenerList.Num(); i++)
	{
		auto& item = tweenerList[i];
		if (!item->ToNextWithElapsedTime(currentTime))
		{
			finishedTweenerList.Add(item);
			tweenerList.RemoveAt(i);
			i--;
		}
	}
}

void ULTweenerSequence::SetOriginValueForRestart()
{
	for (auto& item : finishedTweenerList)
	{
		//add tweener to tweenerList
		tweenerList.Add(item);
	}
	finishedTweenerList.Reset();

	for (auto& item : tweenerList)
	{
		item->SetOriginValueForRestart();
		//set parameter to initial
		item->elapseTime = 0;
		item->loopCycleCount = 0;
		item->reverseTween = false;
		item->TweenAndApplyValue(0);
	}
}

void ULTweenerSequence::SetValueForIncremental()
{
	for (auto& item : finishedTweenerList)
	{
		item->SetValueForIncremental();
		//set parameter to initial
		item->elapseTime = 0;
		item->loopCycleCount = 0;
		item->reverseTween = false;
		item->TweenAndApplyValue(0);

		//add tweener to tweenerList
		tweenerList.Add(item);
	}
	finishedTweenerList.Reset();
}
void ULTweenerSequence::SetValueForYoyo()
{
	this->reverseTween = !this->reverseTween;//reverse it again, so it will keep value false, because we only need to reverse tweenerList
	for (auto& item : finishedTweenerList)
	{
		if (item->loopType != LTweenLoop::Yoyo)//if it is already yoyo, then we no need to change reverseTween for it
		{
			item->reverseTween = !item->reverseTween;
		}

		//set parameter to initial
		item->elapseTime = 0;
		item->loopCycleCount = 0;
		//flip tweener
		int loopCount = item->loopType == LTweenLoop::Once ? 1 : item->maxLoopCount;
		float tweenerDelay = duration - (item->delay + item->duration * loopCount);
		item->delay = tweenerDelay;

		//add tweener to tweenerList
		tweenerList.Add(item);
	}
	finishedTweenerList.Reset();
}
void ULTweenerSequence::SetValueForRestart()
{
	for (auto& item : finishedTweenerList)
	{
		//set parameter to initial
		item->elapseTime = 0;
		item->loopCycleCount = 0;
		item->reverseTween = false;
		item->TweenAndApplyValue(0);

		//add tweener to tweenerList
		tweenerList.Add(item);
	}
	finishedTweenerList.Reset();
}
