// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "LGUI.h"

ULGUIPrefabSequenceComponent::ULGUIPrefabSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

class ULGUIFixMovieScene : public UMovieScene
{
public:
	void FixUpAfterPrefabSerialize()
	{
		this->UpgradeTimeRanges();
	}
};

void ULGUIPrefabSequenceComponent::Awake()
{
	Super::Awake();

	if (this->bIsSerializedFromLGUIPrefab)
	{
		for (auto& SequenceItem : SequenceArray)
		{
			if (auto MovieScene = SequenceItem->GetMovieScene())
			{
				((ULGUIFixMovieScene*)MovieScene)->FixUpAfterPrefabSerialize();
			}
		}
	}
}
void ULGUIPrefabSequenceComponent::Start()
{
	Super::Start();

	this->SetCanExecuteUpdate(true);
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

void ULGUIPrefabSequenceComponent::OnDestroy()
{
	Super::OnDestroy();

	if (SequencePlayer)
	{
		SequencePlayer->Stop();
	}
}


void ULGUIPrefabSequenceComponent::Update(float DeltaSeconds)
{
	Super::Update(DeltaSeconds);

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

void ULGUIPrefabSequenceComponent::Play()
{
	if (auto CurrentSequence = GetCurrentSequence())
	{
		if (!SequencePlayer)
		{
			SequencePlayer = NewObject<ULGUIPrefabSequencePlayer>(this, "SequencePlayer");
		}
		SequencePlayer->Initialize(CurrentSequence, PlaybackSettings);
		SequencePlayer->Play();
	}
}
void ULGUIPrefabSequenceComponent::PlaySequenceByIndex(int32 InIndex)
{
	CurrentSequenceIndex = InIndex;
	Play();
}
void ULGUIPrefabSequenceComponent::PlaySequenceByName(FName InName)
{
	int FoundIndex = -1;
	FoundIndex = SequenceArray.IndexOfByPredicate([InName](const ULGUIPrefabSequence* Item) {
		return Item->GetFName() == InName;
		});
	if (FoundIndex != INDEX_NONE)
	{
		CurrentSequenceIndex = FoundIndex;
		Play();
	}
}
void ULGUIPrefabSequenceComponent::SetCurrentSequenceIndex(int32 InIndex) 
{ 
	CurrentSequenceIndex = InIndex; 
}

ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::AddNewAnimation()
{
	auto NewSequence = NewObject<ULGUIPrefabSequence>(this, NAME_None);
	NewSequence->SetFlags(RF_Public | RF_Transactional);
	auto MovieScene = NewSequence->GetMovieScene();
	SequenceArray.Add(NewSequence);
	return NewSequence;
}

bool ULGUIPrefabSequenceComponent::DeleteAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabSequenceComponent::DeleteAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return false;
	}
	SequenceArray.RemoveAt(InIndex);
	return true;
}
ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::DuplicateAnimationByIndex(int32 InIndex)
{
	if (InIndex < 0 || InIndex >= SequenceArray.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIPrefabSequenceComponent::DuplicateAnimationByIndex] Index out of range! Index: %d, ArrayNum: %d"), InIndex, SequenceArray.Num());
		return nullptr;
	}
	auto SourceSequence = SequenceArray[InIndex];
	auto NewSequence = DuplicateObject(SourceSequence, this);
	SequenceArray.Insert(NewSequence, InIndex + 1);
	return NewSequence;
}
