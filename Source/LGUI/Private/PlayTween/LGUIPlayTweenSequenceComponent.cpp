// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenSequenceComponent.h"
#include "PlayTween/LGUIPlayTween.h"
#include "LTweener.h"
#include "LTweenActor.h"

void ULGUIPlayTweenSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (playTweenArray.Num() > 0)
	{
		for (auto playTween : playTweenArray)
		{
			playTween->RegisterOnComplete(FSimpleDelegate::CreateWeakLambda(this, [this] {
				OnTweenComplete();
				})
			);
		}
	}
	if (playOnStart)
	{
		Play();
	}
}
void ULGUIPlayTweenSequenceComponent::OnTweenComplete()
{
	currentTweenPlayIndex++;
	if (currentTweenPlayIndex >= playTweenArray.Num())
	{
		isPlaying = false;
		onComplete.FireEvent();
	}
	else
	{
		playTweenArray[currentTweenPlayIndex]->Start();
	}
}
void ULGUIPlayTweenSequenceComponent::Play()
{
	if (playTweenArray.Num() > 0)
	{
		if (!isPlaying)
		{
			isPlaying = true;
			currentTweenPlayIndex = -1;
			OnTweenComplete();
		}
	}
}
void ULGUIPlayTweenSequenceComponent::Stop()
{
	if (isPlaying)
	{
		isPlaying = false;
		ALTweenActor::KillIfIsTweening(this, playTweenArray[currentTweenPlayIndex]->GetTweener(), false);
	}
}
