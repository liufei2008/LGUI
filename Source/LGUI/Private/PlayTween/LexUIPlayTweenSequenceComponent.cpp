// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PlayTween/LexUIPlayTweenSequenceComponent.h"
#include "PlayTween/LexUIPlayTween.h"
#include "LTweenManager.h"

void ULexUIPlayTweenSequenceComponent::Awake()
{
	if (bPlayOnStart)
	{
		Play();
	}
}
void ULexUIPlayTweenSequenceComponent::OnTweenComplete()
{
	if (bPlayNextWhenCycleComplete)
	{
		PlayTweenArray[CurrentTweenPlayIndex]->OnCycleCompleteCPP.Remove(OnCompleteDelegateHandle);
	}
	else
	{
		PlayTweenArray[CurrentTweenPlayIndex]->OnCycleCompleteCPP.Remove(OnCompleteDelegateHandle);
	}

	CurrentTweenPlayIndex++;
	if (CurrentTweenPlayIndex >= PlayTweenArray.Num())
	{
		bIsPlaying = false;
		OnComplete.FireEvent();
		OnCompleteCPP.Broadcast();
		OnCompleteBP.Broadcast();
	}
	else
	{
		auto& tweenItem = PlayTweenArray[CurrentTweenPlayIndex];
		if (bPlayNextWhenCycleComplete)
		{
			OnCompleteDelegateHandle = tweenItem->OnCycleCompleteCPP.AddWeakLambda(this, [this](int count) {
				OnTweenComplete();
				});
		}
		else
		{
			OnCompleteDelegateHandle = tweenItem->OnCompleteCPP.AddUObject(this, &ULexUIPlayTweenSequenceComponent::OnTweenComplete);
		}
		tweenItem->Start();
	}
}

void ULexUIPlayTweenSequenceComponent::Play()
{
	if (PlayTweenArray.Num() > 0)
	{
		if (!bIsPlaying)
		{
			bIsPlaying = true;
			CurrentTweenPlayIndex = 0;
			auto& tweenItem = PlayTweenArray[CurrentTweenPlayIndex];
			if (bPlayNextWhenCycleComplete)
			{
				OnCompleteDelegateHandle = tweenItem->OnCycleCompleteCPP.AddWeakLambda(this, [this](int count) {
					OnTweenComplete();
					});
			}
			else
			{
				OnCompleteDelegateHandle = tweenItem->OnCompleteCPP.AddUObject(this, &ULexUIPlayTweenSequenceComponent::OnTweenComplete);
			}
			tweenItem->Start();
		}
	}
}
void ULexUIPlayTweenSequenceComponent::Stop()
{
	if (bIsPlaying)
	{
		bIsPlaying = false;
		auto& tweenItem = PlayTweenArray[CurrentTweenPlayIndex];
		tweenItem->OnCompleteCPP.Remove(OnCompleteDelegateHandle);
		ULTweenManager::KillIfIsTweening(this, tweenItem->GetTweener(), false);
	}
}
