// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenComponent.h"
#include "PlayTween/LGUIPlayTween.h"
#include "PrefabSystem/LGUIPrefabManager.h"

void ULGUIPlayTweenComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ULGUIPrefabWorldSubsystem::IsLGUIPrefabSystemProcessingActor(this->GetOwner()))
	{
		Awake_Implementation();
	}
}
void ULGUIPlayTweenComponent::Awake_Implementation()
{
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
