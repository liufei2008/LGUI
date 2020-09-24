// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenComponent.h"
#include "PlayTween/LGUIPlayTween.h"

void ULGUIPlayTweenComponent::Start()
{
	Super::Start();
	if (IsValid(playTween))
	{
		playTween->Start();
	}
}

