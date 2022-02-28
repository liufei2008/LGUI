// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "IMovieScenePlayer.h"
#include "LGUIPrefabSequence.h"
#include "MovieSceneSequencePlayer.h"
#include "LGUIPrefabSequencePlayer.generated.h"

/**
 * ULGUIPrefabSequencePlayer is used to actually "play" an actor sequence asset at runtime.
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIPrefabSequencePlayer
	: public UMovieSceneSequencePlayer
{
public:
	GENERATED_BODY()

protected:

	//~ IMovieScenePlayer interface
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;
};

