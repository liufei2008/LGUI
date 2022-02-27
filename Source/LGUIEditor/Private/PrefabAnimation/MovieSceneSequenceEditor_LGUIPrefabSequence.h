// Copyright Epic Games, Inc. All Rights Reserved.

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

	virtual UBlueprint* GetBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		ULGUIPrefabSequence* LGUIPrefabSequence = CastChecked<ULGUIPrefabSequence>(InSequence);
		if (UBlueprint* Blueprint = LGUIPrefabSequence->GetParentBlueprint())
		{
			return Blueprint;
		}

		ULGUIPrefabSequenceComponent* Component = LGUIPrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
		ULevel* Level = (Component && Component->GetOwner()) ? Component->GetOwner()->GetLevel() : nullptr;

		bool bDontCreateNewBlueprint = true;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}

	virtual UBlueprint* CreateBlueprintForSequence(UMovieSceneSequence* InSequence) const override
	{
		ULGUIPrefabSequence* LGUIPrefabSequence = CastChecked<ULGUIPrefabSequence>(InSequence);
		check(!LGUIPrefabSequence->GetParentBlueprint());

		ULGUIPrefabSequenceComponent* Component = LGUIPrefabSequence->GetTypedOuter<ULGUIPrefabSequenceComponent>();
		ULevel* Level = (Component && Component->GetOwner()) ? Component->GetOwner()->GetLevel() : nullptr;

		bool bDontCreateNewBlueprint = false;
		return Level ? Level->GetLevelScriptBlueprint(bDontCreateNewBlueprint) : nullptr;
	}
};
