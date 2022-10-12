// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LTweener.h"
#include "Curves/CurveFloat.h"
#include "LTween.h"

ULTweener::ULTweener()
{
	tweenFunc.BindStatic(&ULTweener::OutCubic);//OutCubic default animation curve function
}
ULTweener* ULTweener::SetEase(LTweenEase easetype)
{
	switch (easetype)
	{
	case LTweenEase::Linear:
		tweenFunc.BindStatic(&ULTweener::Linear);
		break;
	case LTweenEase::InQuad:
		tweenFunc.BindStatic(&ULTweener::InQuad);
		break;
	case LTweenEase::OutQuad:
		tweenFunc.BindStatic(&ULTweener::OutQuad);
		break;
	case LTweenEase::InOutQuad:
		tweenFunc.BindStatic(&ULTweener::InOutQuad);
		break;
	case LTweenEase::InCubic:
		tweenFunc.BindStatic(&ULTweener::InCubic);
		break;
	case LTweenEase::OutCubic:
		tweenFunc.BindStatic(&ULTweener::OutCubic);
		break;
	case LTweenEase::InOutCubic:
		tweenFunc.BindStatic(&ULTweener::InOutCubic);
		break;
	case LTweenEase::InQuart:
		tweenFunc.BindStatic(&ULTweener::InQuart);
		break;
	case LTweenEase::OutQuart:
		tweenFunc.BindStatic(&ULTweener::OutQuart);
		break;
	case LTweenEase::InOutQuart:
		tweenFunc.BindStatic(&ULTweener::InOutQuart);
		break;
	case LTweenEase::InSine:
		tweenFunc.BindStatic(&ULTweener::InSine);
		break;
	case LTweenEase::OutSine:
		tweenFunc.BindStatic(&ULTweener::OutSine);
		break;
	case LTweenEase::InOutSine:
		tweenFunc.BindStatic(&ULTweener::InOutSine);
		break;
	case LTweenEase::InExpo:
		tweenFunc.BindStatic(&ULTweener::InExpo);
		break;
	case LTweenEase::OutExpo:
		tweenFunc.BindStatic(&ULTweener::OutExpo);
		break;
	case LTweenEase::InOutExpo:
		tweenFunc.BindStatic(&ULTweener::InOutExpo);
		break;
	case LTweenEase::InCirc:
		tweenFunc.BindStatic(&ULTweener::InCirc);
		break;
	case LTweenEase::OutCirc:
		tweenFunc.BindStatic(&ULTweener::OutCirc);
		break;
	case LTweenEase::InOutCirc:
		tweenFunc.BindStatic(&ULTweener::InOutCirc);
		break;
	case LTweenEase::InElastic:
		tweenFunc.BindStatic(&ULTweener::InElastic);
		break;
	case LTweenEase::OutElastic:
		tweenFunc.BindStatic(&ULTweener::OutElastic);
		break;
	case LTweenEase::InOutElastic:
		tweenFunc.BindStatic(&ULTweener::InOutElastic);
		break;
	case LTweenEase::InBack:
		tweenFunc.BindStatic(&ULTweener::InBack);
		break;
	case LTweenEase::OutBack:
		tweenFunc.BindStatic(&ULTweener::OutBack);
		break;
	case LTweenEase::InOutBack:
		tweenFunc.BindStatic(&ULTweener::InOutBack);
		break;
	case LTweenEase::InBounce:
		tweenFunc.BindStatic(&ULTweener::InBounce);
		break;
	case LTweenEase::OutBounce:
		tweenFunc.BindStatic(&ULTweener::OutBounce);
		break;
	case LTweenEase::InOutBounce:
		tweenFunc.BindStatic(&ULTweener::InOutBounce);
		break;
	case LTweenEase::CurveFloat:
		tweenFunc.BindUObject(this, &ULTweener::CurveFloat);
		break;
	}
	return this;
}
ULTweener* ULTweener::SetEaseCurve(UCurveFloat* newCurve)
{
	if (IsValid(newCurve))
	{
		SetEase(LTweenEase::CurveFloat);
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
	curveFloat = newCurveFloat;
	return this;
}

bool ULTweener::ToNext(float deltaTime)
{
	if (isMarkedToKill)return false;
	if (isMarkedPause)return true;//no need to tick time if pause
	if (reverseTween)
		elapseTime -= deltaTime;
	else
		elapseTime += deltaTime;

	if (startToTween)//if already start tween animation
	{
		TWEEN_UPDATE:
		if (elapseTime >= duration || elapseTime < 0)//elapseTime less than 0 means loop reverse
		{
			bool returnValue = true;
			loopCycleCount++;
			switch (loopType)
			{
			case LTweenLoop::Once:
			{
				elapseTime = duration;
				returnValue = false;
			}
			break;
			case LTweenLoop::Restart:
			{
				elapseTime -= duration;
				returnValue = true;
			}
			break;
			case LTweenLoop::Yoyo:
			{
				if (elapseTime < 0)
				{
					elapseTime += deltaTime;
				}
				else
				{
					elapseTime -= deltaTime;
				}
				returnValue = true;
				reverseTween = !reverseTween;
			}
			break;
			case LTweenLoop::Incremental:
			{
				SetValueForIncremental();
				elapseTime -= duration;
				returnValue = true;
			}
			break;
			}
			
			TweenAndApplyValue();
			onUpdateCpp.ExecuteIfBound(1.0f);
			onCycleCompleteCpp.ExecuteIfBound();
			if (loopType == LTweenLoop::Once)
			{
				onCompleteCpp.ExecuteIfBound();
				returnValue = false;
			}
			else if (maxLoopCount == -1)//infinite loop
			{
				onCycleStartCpp.ExecuteIfBound();//start new cycle callback
				returnValue = true;
			}
			else if (maxLoopCount != -1)//-1 means infinite loop
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
			return returnValue;
		}
		else
		{
			TweenAndApplyValue();
			onUpdateCpp.ExecuteIfBound(elapseTime / duration);
			return true;
		}
	}
	else//if not start animation yet, check delay
	{
		if (elapseTime < delay)//if elapse time less than delay, means we need to keep waiting
		{
			return true;
		}
		else//if elapse time more than delay, start animation
		{
			elapseTime -= delay;
			startToTween = true;
			//set initialize value
			OnStartGetValue();
			//execute callback
			onCycleStartCpp.ExecuteIfBound();
			onStartCpp.ExecuteIfBound();
			goto TWEEN_UPDATE;
		}
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
	elapseTime = duration;
	TweenAndApplyValue();
	onUpdateCpp.ExecuteIfBound(elapseTime / duration);
	onCompleteCpp.ExecuteIfBound();
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