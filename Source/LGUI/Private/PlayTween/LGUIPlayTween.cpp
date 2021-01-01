// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTween.h"
#include "LGUI.h"
#include "LTweenActor.h"

void ULGUIPlayTween::Stop()
{
	ALTweenActor::KillIfIsTweening(this, tweener, false);
}
void ULGUIPlayTween::Start()
{
	tweener = ALTweenActor::To(this->GetWorld()
		, FLTweenFloatGetterFunction::CreateLambda([] { return 0.0f; })
		, FLTweenFloatSetterFunction::CreateLambda([=](float value) { OnUpdate(value); })
		, 1.0f, duration)
		->SetDelay(startDelay)
		->SetLoopType(loopType)
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
void ULGUIPlayTween::UnregisterOnComplete(const FDelegateHandle& InDelegateHandle)
{
	onComplete_Delegate.Remove(InDelegateHandle);
}
