// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PlayTween/LexUIPlayTweenComponent.h"
#include "PlayTween/LexUIPlayTween.h"

void ULexUIPlayTweenComponent::Awake()
{
	if (bPlayOnStart)
	{
		if (IsValid(PlayTween))
		{
			PlayTween->Start();
		}
	}
}
void ULexUIPlayTweenComponent::Play()
{
	if (IsValid(PlayTween))
	{
		PlayTween->Start();
	}
}
void ULexUIPlayTweenComponent::Stop()
{
	if (IsValid(PlayTween))
	{
		PlayTween->Stop();
	}
}
