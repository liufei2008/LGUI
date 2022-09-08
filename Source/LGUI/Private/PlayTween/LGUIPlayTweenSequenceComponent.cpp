// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenSequenceComponent.h"
#include "PlayTween/LGUIPlayTween.h"
#include "LTweener.h"
#include "LTweenManager.h"

void ULGUIPlayTweenSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (playOnStart)
	{
		Play();
	}
}
void ULGUIPlayTweenSequenceComponent::OnTweenComplete()
{
	if (bPlayNextWhenCycleComplete)
	{
		playTweenArray[currentTweenPlayIndex]->UnregisterOnCycleComplete(onCompleteDelegateHandle);
	}
	else
	{
		playTweenArray[currentTweenPlayIndex]->UnregisterOnComplete(onCompleteDelegateHandle);
	}

	currentTweenPlayIndex++;
	if (currentTweenPlayIndex >= playTweenArray.Num())
	{
		isPlaying = false;
		onComplete.FireEvent();
	}
	else
	{
		auto& tweenItem = playTweenArray[currentTweenPlayIndex];
		if (bPlayNextWhenCycleComplete)
		{
			onCompleteDelegateHandle = tweenItem->RegisterOnCycleComplete(FLGUIInt32Delegate::CreateWeakLambda(this, [this](int count) {
				OnTweenComplete();
				}));
		}
		else
		{
			onCompleteDelegateHandle = tweenItem->RegisterOnComplete(FSimpleDelegate::CreateUObject(this, &ULGUIPlayTweenSequenceComponent::OnTweenComplete));
		}
		tweenItem->Start();
	}
}
void ULGUIPlayTweenSequenceComponent::Play()
{
	if (playTweenArray.Num() > 0)
	{
		if (!isPlaying)
		{
			isPlaying = true;
			currentTweenPlayIndex = 0;
			auto& tweenItem = playTweenArray[currentTweenPlayIndex];
			if (bPlayNextWhenCycleComplete)
			{
				onCompleteDelegateHandle = tweenItem->RegisterOnCycleComplete(FLGUIInt32Delegate::CreateWeakLambda(this, [this](int count) {
					OnTweenComplete();
					}));
			}
			else
			{
				onCompleteDelegateHandle = tweenItem->RegisterOnComplete(FSimpleDelegate::CreateUObject(this, &ULGUIPlayTweenSequenceComponent::OnTweenComplete));
			}
			tweenItem->Start();
		}
	}
}
void ULGUIPlayTweenSequenceComponent::Stop()
{
	if (isPlaying)
	{
		isPlaying = false;
		auto& tweenItem = playTweenArray[currentTweenPlayIndex];
		tweenItem->UnregisterOnComplete(onCompleteDelegateHandle);
		ULTweenManager::KillIfIsTweening(this, tweenItem->GetTweener(), false);
	}
}
