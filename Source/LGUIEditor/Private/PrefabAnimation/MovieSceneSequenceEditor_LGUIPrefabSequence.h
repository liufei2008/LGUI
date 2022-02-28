// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "MovieSceneSequenceEditor.h"
#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "Engine/Level.h"
#include "Engine/LevelScriptBlueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "K2Node_FunctionEntry.h"
#include "EdGraphSchema_K2.h"

struct FMovieSceneSequenceEditor_LGUIPrefabSequence : FMovieSceneSequenceEditor
{
	virtual bool CanCreateEvents(UMovieSceneSequence* InSequence) const
	{
		return true;
	}
};
