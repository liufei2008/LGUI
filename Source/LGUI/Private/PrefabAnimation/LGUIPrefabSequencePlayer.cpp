// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"


UObject* ULGUIPrefabSequencePlayer::GetPlaybackContext() const
{
	ULGUIPrefabSequence* PrefabSequence = CastChecked<ULGUIPrefabSequence>(Sequence);
	if (PrefabSequence)
	{
		auto Component = PrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
		return Component->GetSequenceBlueprintInstance();
	}

	return nullptr;
}

TArray<UObject*> ULGUIPrefabSequencePlayer::GetEventContexts() const
{
	TArray<UObject*> Contexts;
	if (UObject* PlaybackContext = GetPlaybackContext())
	{
		Contexts.Add(PlaybackContext);
	}
	return Contexts;
}