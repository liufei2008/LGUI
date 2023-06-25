// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "MovieSceneSequenceEditor_LGUIPrefabSequence.h"
#include "ISequencerModule.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"

#define LOCTEXT_NAMESPACE "MovieSceneSequenceEditor_LGUIPrefabSequence"

UBlueprint* FMovieSceneSequenceEditor_LGUIPrefabSequence::GetBlueprintForSequence(UMovieSceneSequence* InSequence) const
{
	auto PrefabSequence = CastChecked<ULGUIPrefabSequence>(InSequence);
	auto Component = PrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
	return Component->GetSequenceBlueprint();
}

UBlueprint* FMovieSceneSequenceEditor_LGUIPrefabSequence::CreateBlueprintForSequence(UMovieSceneSequence* InSequence) const
{
	auto PrefabSequence = CastChecked<ULGUIPrefabSequence>(InSequence);
	auto Component = PrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
	check(!Component->GetSequenceBlueprint());
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
