// Copyright 2019 LexLiu. All Rights Reserved.

#include "LTweener.h"

ULTweener::ULTweener()
{
	tweenFunc.BindUObject(this, &ULTweener::OutCubic);//OutCubic default animation curve function
}
ULTweener* ULTweener::SetEase(LTweenEase easetype)
{
	switch (easetype)
	{
	case LTweenEase::Linear:
		tweenFunc.BindUObject(this, &ULTweener::Linear);
		break;
	case LTweenEase::InQuad:
		tweenFunc.BindUObject(this, &ULTweener::InQuad);
		break;
	case LTweenEase::OutQuad:
		tweenFunc.BindUObject(this, &ULTweener::OutQuad);
		break;
	case LTweenEase::InOutQuad:
		tweenFunc.BindUObject(this, &ULTweener::InOutQuad);
		break;
	case LTweenEase::InCubic:
		tweenFunc.BindUObject(this, &ULTweener::InCubic);
		break;
	case LTweenEase::OutCubic:
		tweenFunc.BindUObject(this, &ULTweener::OutCubic);
		break;
	case LTweenEase::InOutCubic:
		tweenFunc.BindUObject(this, &ULTweener::InOutCubic);
		break;
	case LTweenEase::InQuart:
		tweenFunc.BindUObject(this, &ULTweener::InQuart);
		break;
	case LTweenEase::OutQuart:
		tweenFunc.BindUObject(this, &ULTweener::OutQuart);
		break;
	case LTweenEase::InOutQuart:
		tweenFunc.BindUObject(this, &ULTweener::InOutQuart);
		break;
	case LTweenEase::InSine:
		tweenFunc.BindUObject(this, &ULTweener::InSine);
		break;
	case LTweenEase::OutSine:
		tweenFunc.BindUObject(this, &ULTweener::OutSine);
		break;
	case LTweenEase::InOutSine:
		tweenFunc.BindUObject(this, &ULTweener::InOutSine);
		break;
	case LTweenEase::InExpo:
		tweenFunc.BindUObject(this, &ULTweener::InExpo);
		break;
	case LTweenEase::OutExpo:
		tweenFunc.BindUObject(this, &ULTweener::OutExpo);
		break;
	case LTweenEase::InOutExpo:
		tweenFunc.BindUObject(this, &ULTweener::InOutExpo);
		break;
	case LTweenEase::InCirc:
		tweenFunc.BindUObject(this, &ULTweener::InCirc);
		break;
	case LTweenEase::OutCirc:
		tweenFunc.BindUObject(this, &ULTweener::OutCirc);
		break;
	case LTweenEase::InOutCirc:
		tweenFunc.BindUObject(this, &ULTweener::InOutCirc);
		break;
	case LTweenEase::InElastic:
		tweenFunc.BindUObject(this, &ULTweener::InElastic);
		break;
	case LTweenEase::OutElastic:
		tweenFunc.BindUObject(this, &ULTweener::OutElastic);
		break;
	case LTweenEase::InOutElastic:
		tweenFunc.BindUObject(this, &ULTweener::InOutElastic);
		break;
	case LTweenEase::InBack:
		tweenFunc.BindUObject(this, &ULTweener::InBack);
		break;
	case LTweenEase::OutBack:
		tweenFunc.BindUObject(this, &ULTweener::OutBack);
		break;
	case LTweenEase::InOutBack:
		tweenFunc.BindUObject(this, &ULTweener::InOutBack);
		break;
	case LTweenEase::InBounce:
		tweenFunc.BindUObject(this, &ULTweener::InBounce);
		break;
	case LTweenEase::OutBounce:
		tweenFunc.BindUObject(this, &ULTweener::OutBounce);
		break;
	case LTweenEase::InOutBounce:
		tweenFunc.BindUObject(this, &ULTweener::InOutBounce);
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
				loopCount++;
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
				loopCount++;
			}
			break;
			}
			
			TweenAndApplyValue();
			if (onUpdateCpp.IsBound()) 
				onUpdateCpp.Execute(1.0f);
			if (onCompleteCpp.IsBound()) 
				onCompleteCpp.Execute();
			return returnValue;
		}
		else
		{
			TweenAndApplyValue();
			if (onUpdateCpp.IsBound()) 
				onUpdateCpp.Execute(elapseTime / duration);
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
			if (onStartCpp.IsBound())
				onStartCpp.Execute();
			goto TWEEN_UPDATE;
		}
	}
}

void ULTweener::Kill(bool callComplete)
{
	if (callComplete)
	{
		if (onCompleteCpp.IsBound())
			onCompleteCpp.Execute();
	}
	isMarkedToKill = true;
}

void ULTweener::ForceComplete()
{
	isMarkedToKill = true;
	elapseTime = duration;
	TweenAndApplyValue();
	if (onUpdateCpp.IsBound()) 
		onUpdateCpp.Execute(elapseTime / duration);
	if (onCompleteCpp.IsBound()) 
		onCompleteCpp.Execute();
}