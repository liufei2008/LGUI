// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenComponent.h"
#include "PlayTween/LGUIPlayTween.h"

void ULGUIPlayTweenComponent::BeginPlay()
{
	Super::BeginPlay();
	if (playOnStart)
	{
		if (IsValid(playTween))
		{
			playTween->Start();
		}
	}
}
void ULGUIPlayTweenComponent::Play()
{
	if (IsValid(playTween))
	{
		playTween->Start();
	}
}
void ULGUIPlayTweenComponent::Stop()
{
	if (IsValid(playTween))
	{
		playTween->Stop();
	}
}
