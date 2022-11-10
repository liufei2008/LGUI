// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTween.h"
#include "LGUI.h"
#include "LTweenManager.h"

void ULGUIPlayTween::Stop()
{
	ULTweenManager::KillIfIsTweening(this, tweener, false);
}
void ULGUIPlayTween::Start()
{
	tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateLambda([] { return 0.0f; })
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULGUIPlayTween::OnUpdate)
		, 1.0f, duration)
		->SetDelay(startDelay)
		->SetLoop(loopType, loopCount)
		->SetEase(easeType)
		->SetCurveFloat(easeCurve)
		->OnStart([&] {
			onStart.FireEvent();
		})
		->OnUpdate([&](float progress) {
			onUpdateProgress.FireEvent(progress);
		})
		->OnCycleComplete([&] {
			onCycleComplete.FireEvent();
			onCycleComplete_Delegate.Broadcast(tweener->GetLoopCycleCount());
		})
		->OnComplete([&] {
			onComplete.FireEvent();
			onComplete_Delegate.Broadcast();
		});
}
FDelegateHandle ULGUIPlayTween::RegisterOnComplete(const FSimpleDelegate& InDelegate)
{
	return onComplete_Delegate.Add(InDelegate);
}
FDelegateHandle ULGUIPlayTween::RegisterOnComplete(const TFunction<void()>& InFunction)
{
	return onComplete_Delegate.AddLambda(InFunction);
}
void ULGUIPlayTween::UnregisterOnComplete(const FDelegateHandle& InDelegateHandle)
{
	onComplete_Delegate.Remove(InDelegateHandle);
}

FDelegateHandle ULGUIPlayTween::RegisterOnCycleComplete(const FLGUIInt32Delegate& InDelegate)
{
	return onCycleComplete_Delegate.Add(InDelegate);
}
FDelegateHandle ULGUIPlayTween::RegisterOnCycleComplete(const TFunction<void(int32)>& InFunction)
{
	return onCycleComplete_Delegate.AddLambda(InFunction);
}
void ULGUIPlayTween::UnregisterOnCycleComplete(const FDelegateHandle& InDelegateHandle)
{
	onCycleComplete_Delegate.Remove(InDelegateHandle);
}

FLGUIDelegateHandleWrapper ULGUIPlayTween::RegisterOnComplete(const FLGUIPlayTweenCompleteDynamicDelegate& InDelegate)
{
	return FLGUIDelegateHandleWrapper(onComplete_Delegate.AddLambda([=] {
		InDelegate.ExecuteIfBound();
		})
	);
}
void ULGUIPlayTween::UnregisterOnComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	onComplete_Delegate.Remove(InDelegateHandle.DelegateHandle);
}

FLGUIDelegateHandleWrapper ULGUIPlayTween::RegisterOnCycleComplete(const FLGUIPlayTweenCycleCompleteDynamicDelegate& InDelegate)
{
	return FLGUIDelegateHandleWrapper(onCycleComplete_Delegate.AddLambda([=](int count) {
		InDelegate.ExecuteIfBound(count);
		})
	);
}
void ULGUIPlayTween::UnregisterOnCycleComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	onCycleComplete_Delegate.Remove(InDelegateHandle.DelegateHandle);
}
