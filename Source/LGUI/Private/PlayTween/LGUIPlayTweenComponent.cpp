// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

