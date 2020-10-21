// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTween.h"
#include "LGUI.h"
#include "LTweenActor.h"

void ULGUIPlayTween::Stop()
{
	ALTweenActor::KillIfIsTweening(tweener, false);
}
void ULGUIPlayTween::Start()
{
	tweener = ALTweenActor::VirtualTo(this->GetWorld(), duration)
		->SetDelay(startDelay)
		->SetLoopType(loopType)
		->OnStart([&] {
			onStart.FireEvent();
		})
		->OnUpdate([&](float progress) {
			onUpdateProgress.FireEvent(progress);
			OnUpdate(progress);
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
