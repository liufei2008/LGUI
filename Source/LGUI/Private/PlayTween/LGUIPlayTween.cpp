// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTween.h"
#include "LGUI.h"
#include "LTweenActor.h"

void ULGUIPlayTween::Stop()
{
	ALTweenActor::KillIfIsTweening(this, tweener, false);
}
void ULGUIPlayTween::Start()
{
	tweener = ALTweenActor::To(this
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
		->OnComplete([&] {
			onComplete.FireEvent();
			if (onComplete_Delegate.IsBound())
			{
				onComplete_Delegate.Broadcast();
			}
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
