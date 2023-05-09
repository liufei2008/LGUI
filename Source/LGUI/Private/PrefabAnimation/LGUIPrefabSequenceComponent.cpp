// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "LGUI.h"

ULGUIPrefabSequenceComponent::ULGUIPrefabSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void ULGUIPrefabSequenceComponent::Awake()
{
	Super::Awake();

	InitSequencePlayer();
}
void ULGUIPrefabSequenceComponent::Start()
{
	Super::Start();

	this->SetCanExecuteUpdate(true);

	if (PlaybackSettings.bAutoPlay)
	{
		SequencePlayer->Play();
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

#if WITH_EDITOR
void ULGUIPrefabSequenceComponent::PreDuplicate(FObjectDuplicationParameters& DupParams)
{
	Super::PreDuplicate(DupParams);
	FixEditorHelpers();
}
#include "UObject/ObjectSaveContext.h"
void ULGUIPrefabSequenceComponent::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);
	FixEditorHelpers();
}
void ULGUIPrefabSequenceComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
}
void ULGUIPrefabSequenceComponent::PostInitProperties()
{
	Super::PostInitProperties();
}
void ULGUIPrefabSequenceComponent::PostLoad()
{
	Super::PostLoad();
}
void ULGUIPrefabSequenceComponent::FixEditorHelpers()
{
	for (auto& Sequence : SequenceArray)
	{
		if (Sequence->IsObjectReferencesGood(GetOwner()) && !Sequence->IsEditorHelpersGood(this->GetOwner()))
		{
			Sequence->FixEditorHelpers(this->GetOwner());
		}
	}
}
#endif

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

void ULGUIPrefabSequenceComponent::InitSequencePlayer()
{
	if (auto CurrentSequence = GetCurrentSequence())
	{
		if (!SequencePlayer)
		{
			SequencePlayer = NewObject<ULGUIPrefabSequencePlayer>(this, "SequencePlayer");
		}
		SequencePlayer->Initialize(CurrentSequence, PlaybackSettings);
	}
}
void ULGUIPrefabSequenceComponent::SetSequenceByIndex(int32 InIndex)
{
	CurrentSequenceIndex = InIndex;
	InitSequencePlayer();
}
void ULGUIPrefabSequenceComponent::SetSequenceByName(FName InName)
{
	int FoundIndex = -1;
	FoundIndex = SequenceArray.IndexOfByPredicate([InName](const ULGUIPrefabSequence* Item) {
		return Item->GetFName() == InName;
		});
	if (FoundIndex != INDEX_NONE)
	{
		CurrentSequenceIndex = FoundIndex;
		InitSequencePlayer();
	}
}

ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::AddNewAnimation()
{
	auto NewSequence = NewObject<ULGUIPrefabSequence>(this, NAME_None, RF_Public | RF_Transactional);
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
	if (auto SequenceItem = SequenceArray[InIndex])
	{
		SequenceItem->ConditionalBeginDestroy();
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
