// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "LGUI.h"

ULGUIPrefabSequenceComponent::ULGUIPrefabSequenceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULGUIPrefabSequenceComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void ULGUIPrefabSequenceComponent::BeginPlay()
{
	Super::BeginPlay();

	if (auto CurrentSequence = GetCurrentSequence())
	{
		SequencePlayer = NewObject<ULGUIPrefabSequencePlayer>(this, "SequencePlayer");
		SequencePlayer->Initialize(CurrentSequence, PlaybackSettings);

		if (PlaybackSettings.bAutoPlay)
		{
			SequencePlayer->Play();
		}
	}
}

void ULGUIPrefabSequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SequencePlayer)
	{
		// Stop the internal sequence player during EndPlay when it's safer
		// to modify other actor's state during state restoration.
		SequencePlayer->Stop();
	}
	
	Super::EndPlay(EndPlayReason);
}


void ULGUIPrefabSequenceComponent::TickComponent(float DeltaSeconds, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaSeconds, TickType, ThisTickFunction);

	if (SequencePlayer)
	{
		SequencePlayer->Update(DeltaSeconds);
	}
}

ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::GetSequenceByName(FName InName) const
{
	for (auto Item : SequenceArray)
	{
		if (Item->GetFName() == InName)
		{
			return Item;
		}
	}
	return nullptr;
}
ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::GetSequenceByIndex(int32 InIndex) const
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabSequenceComponent::GetSequenceByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return nullptr;
	}
	return SequenceArray[InIndex];
}

void ULGUIPrefabSequenceComponent::AddNewAnimation()
{
	auto NewSequence = NewObject<ULGUIPrefabSequence>(this, NAME_None);
	NewSequence->SetFlags(RF_Public | RF_Transactional);
	auto MovieScene = NewSequence->GetMovieScene();
	SequenceArray.Add(NewSequence);
}

void ULGUIPrefabSequenceComponent::DeleteAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabSequenceComponent::DeleteAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return;
	}
	SequenceArray.RemoveAt(InIndex);
}
void ULGUIPrefabSequenceComponent::DuplicateAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabSequenceComponent::DuplicateAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return;
	}
	auto SourceSequence = SequenceArray[InIndex];
	auto NewSequence = DuplicateObject(SourceSequence, this);
	SequenceArray.Insert(NewSequence, InIndex + 1);
}
