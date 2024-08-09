// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LTweener.h"
#include "Curves/CurveFloat.h"
#include "LTween.h"
#include "Engine/World.h"

ULTweener::ULTweener()
{
	tweenFunc.BindStatic(&ULTweener::OutCubic);//OutCubic default animation curve function
}
ULTweener* ULTweener::SetEase(ELTweenEase easetype)
{
	if (elapseTime > 0 || startToTween)return this;
	switch (easetype)
	{
	case ELTweenEase::Linear:
		tweenFunc.BindStatic(&ULTweener::Linear);
		break;
	case ELTweenEase::InQuad:
		tweenFunc.BindStatic(&ULTweener::InQuad);
		break;
	case ELTweenEase::OutQuad:
		tweenFunc.BindStatic(&ULTweener::OutQuad);
		break;
	case ELTweenEase::InOutQuad:
		tweenFunc.BindStatic(&ULTweener::InOutQuad);
		break;
	case ELTweenEase::InCubic:
		tweenFunc.BindStatic(&ULTweener::InCubic);
		break;
	case ELTweenEase::OutCubic:
		tweenFunc.BindStatic(&ULTweener::OutCubic);
		break;
	case ELTweenEase::InOutCubic:
		tweenFunc.BindStatic(&ULTweener::InOutCubic);
		break;
	case ELTweenEase::InQuart:
		tweenFunc.BindStatic(&ULTweener::InQuart);
		break;
	case ELTweenEase::OutQuart:
		tweenFunc.BindStatic(&ULTweener::OutQuart);
		break;
	case ELTweenEase::InOutQuart:
		tweenFunc.BindStatic(&ULTweener::InOutQuart);
		break;
	case ELTweenEase::InSine:
		tweenFunc.BindStatic(&ULTweener::InSine);
		break;
	case ELTweenEase::OutSine:
		tweenFunc.BindStatic(&ULTweener::OutSine);
		break;
	case ELTweenEase::InOutSine:
		tweenFunc.BindStatic(&ULTweener::InOutSine);
		break;
	case ELTweenEase::InExpo:
		tweenFunc.BindStatic(&ULTweener::InExpo);
		break;
	case ELTweenEase::OutExpo:
		tweenFunc.BindStatic(&ULTweener::OutExpo);
		break;
	case ELTweenEase::InOutExpo:
		tweenFunc.BindStatic(&ULTweener::InOutExpo);
		break;
	case ELTweenEase::InCirc:
		tweenFunc.BindStatic(&ULTweener::InCirc);
		break;
	case ELTweenEase::OutCirc:
		tweenFunc.BindStatic(&ULTweener::OutCirc);
		break;
	case ELTweenEase::InOutCirc:
		tweenFunc.BindStatic(&ULTweener::InOutCirc);
		break;
	case ELTweenEase::InElastic:
		tweenFunc.BindStatic(&ULTweener::InElastic);
		break;
	case ELTweenEase::OutElastic:
		tweenFunc.BindStatic(&ULTweener::OutElastic);
		break;
	case ELTweenEase::InOutElastic:
		tweenFunc.BindStatic(&ULTweener::InOutElastic);
		break;
	case ELTweenEase::InBack:
		tweenFunc.BindStatic(&ULTweener::InBack);
		break;
	case ELTweenEase::OutBack:
		tweenFunc.BindStatic(&ULTweener::OutBack);
		break;
	case ELTweenEase::InOutBack:
		tweenFunc.BindStatic(&ULTweener::InOutBack);
		break;
	case ELTweenEase::InBounce:
		tweenFunc.BindStatic(&ULTweener::InBounce);
		break;
	case ELTweenEase::OutBounce:
		tweenFunc.BindStatic(&ULTweener::OutBounce);
		break;
	case ELTweenEase::InOutBounce:
		tweenFunc.BindStatic(&ULTweener::InOutBounce);
		break;
	case ELTweenEase::CurveFloat:
		tweenFunc.BindUObject(this, &ULTweener::CurveFloat);
		break;
	}
	return this;
}
ULTweener* ULTweener::SetDelay(float newDelay)
{
	if (elapseTime > 0 || startToTween)return this;
	this->delay = newDelay;
	if (this->delay < 0)
	{
		this->delay = 0;
	}
	return this;
}
ULTweener* ULTweener::SetLoop(ELTweenLoop newLoopType, int32 newLoopCount)
{
	if (elapseTime > 0 || startToTween)return this;
	this->loopType = newLoopType;
	this->maxLoopCount = newLoopCount;
	return this;
}
ULTweener* ULTweener::SetEaseCurve(UCurveFloat* newCurve)
{
	if (IsValid(newCurve))
	{
		SetEase(ELTweenEase::CurveFloat);
		curveFloat = newCurve;
	}
	else
	{
		UE_LOG(LTween, Error, TEXT("[ULTweener::SetEaseCurve]newCurve is not valid!"));
	}
	return this;
}

ULTweener* ULTweener::SetCurveFloat(UCurveFloat* newCurveFloat)
{
	if (elapseTime > 0 || startToTween)return this;
	curveFloat = newCurveFloat;
	return this;
}
ULTweener* ULTweener::SetAffectByGamePause(bool value)
{
	affectByGamePause = value;
	return this;
}
ULTweener* ULTweener::SetAffectByTimeDilation(bool value)
{
	affectByTimeDilation = value;
	return this;
}

bool ULTweener::ToNext(float deltaTime, float unscaledDeltaTime)
{
	if (auto world = GetWorld())
	{
		if (world->IsPaused() && affectByGamePause)return true;
	}
	if (isMarkedToKill)return false;
	if (isMarkedPause)return true;//no need to tick time if pause
	return this->ToNextWithElapsedTime(elapseTime + (affectByTimeDilation ? deltaTime : unscaledDeltaTime));
}
bool ULTweener::ToNextWithElapsedTime(float InElapseTime)
{
	this->elapseTime = InElapseTime;
	if (elapseTime > delay)//if elapseTime bigger than delay, do animation
	{
		if (!startToTween)
		{
			startToTween = true;
			//set initialize value
			OnStartGetValue();
			//execute callback
			onCycleStartCpp.ExecuteIfBound();
			onStartCpp.ExecuteIfBound();
		}

		float elapseTimeWithoutDelay = elapseTime - delay;
		float currentTime = elapseTimeWithoutDelay - duration * loopCycleCount;
		if (currentTime >= duration)
		{
			bool returnValue = true;
			loopCycleCount++;

			TweenAndApplyValue(reverseTween ? 0 : duration);
			onUpdateCpp.ExecuteIfBound(1.0f);
			onCycleCompleteCpp.ExecuteIfBound();
			if (loopType == ELTweenLoop::Once)
			{
				onCompleteCpp.ExecuteIfBound();
				returnValue = false;
			}
			else if (maxLoopCount <= -1)//infinite loop
			{
				onCycleStartCpp.ExecuteIfBound();//start new cycle callback
				returnValue = true;
			}
			else
			{
				if (loopCycleCount >= maxLoopCount)//reach end cycle
				{
					onCompleteCpp.ExecuteIfBound();
					returnValue = false;
				}
				else//not reach end cycle
				{
					onCycleStartCpp.ExecuteIfBound();//start new cycle callback
					returnValue = true;
				}
			}
			switch (loopType)
			{
			case ELTweenLoop::Restart:
			{
				SetValueForRestart();
			}
			break;
			case ELTweenLoop::Yoyo:
			{
				reverseTween = !reverseTween;
				SetValueForYoyo();
			}
			break;
			case ELTweenLoop::Incremental:
			{
				SetValueForIncremental();
			}
			break;
			}
			return returnValue;
		}
		else
		{
			if (reverseTween)
			{
				currentTime = duration - currentTime;
			}
			TweenAndApplyValue(currentTime);
			onUpdateCpp.ExecuteIfBound(currentTime / duration);
			return true;
		}
	}
	else
	{
		return true;//waiting delay
	}
}

void ULTweener::Kill(bool callComplete)
{
	if (callComplete)
	{
		onCompleteCpp.ExecuteIfBound();
	}
	isMarkedToKill = true;
}

void ULTweener::ForceComplete()
{
	isMarkedToKill = true;
	elapseTime = delay + duration;
	TweenAndApplyValue(duration);
	onUpdateCpp.ExecuteIfBound(1.0f);
	onCompleteCpp.ExecuteIfBound();
}

void ULTweener::Restart()
{
	if (elapseTime == 0)
	{
		return;
	}
	isMarkedPause = false;//incase it is paused.
	//reset parameter to initial
	loopCycleCount = 0;
	reverseTween = false;
	SetOriginValueForRestart();

	this->ToNextWithElapsedTime(0);
}

void ULTweener::Goto(float timePoint)
{
	timePoint = FMath::Clamp(timePoint, 0.0f, duration);
	//reset parameter to initial
	loopCycleCount = 0;
	reverseTween = false;

	this->ToNextWithElapsedTime(timePoint);
}

float ULTweener::GetProgress()const
{
	if (elapseTime > delay)
	{
		float elapseTimeWithoutDelay = elapseTime - delay;
		float currentTime = elapseTimeWithoutDelay - duration * loopCycleCount;
		if (currentTime >= duration)
		{
			return 1;
		}
		else
		{
			if (reverseTween)
			{
				currentTime = duration - currentTime;
			}
			return currentTime / duration;
		}
	}
	else
	{
		return 0;
	}
}

ULTweener* ULTweener::SetTickType(ELTweenTickType value)
{
	if (elapseTime > 0 || startToTween)return this;
	this->tickType = value;
	return this;
}

float ULTweener::CurveFloat(float c, float b, float t, float d)
{
	if (d < KINDA_SMALL_NUMBER)return c + b;
	if (curveFloat.IsValid())
	{
		return curveFloat->GetFloatValue(t / d) * c + b;
	}
	else
	{
		UE_LOG(LTween, Warning, TEXT("[ULTweener::CurveFloat]CurveFloat not valid! Fallback to linear. You should always call SetCurveFloat(and pass a valid curve) if set Easetype to CurveFloat."));
		return Linear(c, b, t, d);
	}
}