// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PlayTween/LGUIPlayTweenSequenceComponent.h"
#include "PlayTween/LGUIPlayTween.h"
#include "LTweener.h"
#include "LTweenManager.h"
#include "PrefabSystem/LGUIPrefabManager.h"

void ULGUIPlayTweenSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!ULGUIPrefabWorldSubsystem::IsLGUIPrefabSystemProcessingActor(this->GetOwner()))
	{
		Awake_Implementation();
	}
}
void ULGUIPlayTweenSequenceComponent::Awake_Implementation()
{
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
		onComplete_Delegate.Broadcast();
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

FDelegateHandle ULGUIPlayTweenSequenceComponent::RegisterOnComplete(const FSimpleDelegate& InDelegate)
{
	return onComplete_Delegate.Add(InDelegate);
}
FDelegateHandle ULGUIPlayTweenSequenceComponent::RegisterOnComplete(const TFunction<void()>& InFunction)
{
	return onComplete_Delegate.AddLambda(InFunction);
}
void ULGUIPlayTweenSequenceComponent::UnregisterOnComplete(const FDelegateHandle& InDelegateHandle)
{
	onComplete_Delegate.Remove(InDelegateHandle);
}

FLGUIDelegateHandleWrapper ULGUIPlayTweenSequenceComponent::RegisterOnComplete(const FLGUIPlayTweenCompleteDynamicDelegate& InDelegate)
{
	return FLGUIDelegateHandleWrapper(onComplete_Delegate.AddLambda([=] {
		InDelegate.ExecuteIfBound();
		})
	);
}
void ULGUIPlayTweenSequenceComponent::UnregisterOnComplete(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	onComplete_Delegate.Remove(InDelegateHandle.DelegateHandle);
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
