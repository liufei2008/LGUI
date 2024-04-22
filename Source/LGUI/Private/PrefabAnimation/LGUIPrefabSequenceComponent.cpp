// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "LGUI.h"
#include "PrefabSystem/LGUIPrefabManager.h"

ULGUIPrefabSequenceComponent::ULGUIPrefabSequenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SequenceEventHandler = FLGUIComponentReference(UActorComponent::StaticClass());
}

void ULGUIPrefabSequenceComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto ManagerInstance = ULGUIPrefabWorldSubsystem::GetInstance(this->GetWorld()))
	{
		if (!ManagerInstance->IsPrefabSystemProcessingActor(this->GetOwner()))//if not processing by PrefabSystem, then mannually call initialize function
		{
			Awake_Implementation();
		}
	}
}
void ULGUIPrefabSequenceComponent::Awake_Implementation()
{
	InitSequencePlayer();

	if (PlaybackSettings.bAutoPlay)
	{
		SequencePlayer->Play();
	}
}

void ULGUIPrefabSequenceComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (SequencePlayer)
	{
		SequencePlayer->Stop();
		SequencePlayer->TearDown();
	}
}

#if WITH_EDITOR
void ULGUIPrefabSequenceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
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

UBlueprint* ULGUIPrefabSequenceComponent::GetSequenceBlueprint()const
{
	if (auto Comp = SequenceEventHandler.GetComponent())
	{
		if (auto GeneratedClass = Cast<UBlueprintGeneratedClass>(Comp->GetClass()))
		{
			return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
		}
	}
	return nullptr;
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
ULGUIPrefabSequence* ULGUIPrefabSequenceComponent::GetSequenceByDisplayName(const FString& InName) const
{
	for (auto Item : SequenceArray)
	{
		if (Item->GetDisplayName().ToString() == InName)
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
	if (!SequencePlayer)
	{
		SequencePlayer = NewObject<ULGUIPrefabSequencePlayer>(this, "SequencePlayer");
		SequencePlayer->SetPlaybackClient(this);

		// Initialize this player for tick as soon as possible to ensure that a persistent
		// reference to the tick manager is maintained
		SequencePlayer->InitializeForTick(this);
	}
	if (auto CurrentSequence = GetCurrentSequence())
	{
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
void ULGUIPrefabSequenceComponent::SetSequenceByDisplayName(const FString& InName)
{
	int FoundIndex = -1;
	FoundIndex = SequenceArray.IndexOfByPredicate([InName](const ULGUIPrefabSequence* Item) {
		return Item->GetDisplayName().ToString() == InName;
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
	NewSequence->GetMovieScene()->Rename(*NewSequence->GetName(), nullptr, REN_DontCreateRedirectors | REN_ForceNoResetLoaders);
	NewSequence->SetDisplayName(NewSequence->GetName());
	{
		NewSequence->GetMovieScene()->SetTickResolutionDirectly(SourceSequence->GetMovieScene()->GetTickResolution());
		NewSequence->GetMovieScene()->SetPlaybackRange(SourceSequence->GetMovieScene()->GetPlaybackRange());
		NewSequence->GetMovieScene()->SetDisplayRate(SourceSequence->GetMovieScene()->GetDisplayRate());
	}
	SequenceArray.Insert(NewSequence, InIndex + 1);
	return NewSequence;
}
