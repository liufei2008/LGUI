// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequencePlayer.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SimpleConstructionScript.h"


UObject* ULGUIPrefabSequencePlayer::GetPlaybackContext() const
{
	ULGUIPrefabSequence* LGUIPrefabSequence = CastChecked<ULGUIPrefabSequence>(Sequence);
	if (LGUIPrefabSequence)
	{
		if (AActor* Actor = LGUIPrefabSequence->GetTypedOuter<AActor>())
		{
			return Actor;
		}
#if WITH_EDITOR
		else if (UBlueprintGeneratedClass* GeneratedClass = LGUIPrefabSequence->GetTypedOuter<UBlueprintGeneratedClass>())
		{
			return GeneratedClass->SimpleConstructionScript->GetComponentEditorActorInstance();
		}
#endif
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