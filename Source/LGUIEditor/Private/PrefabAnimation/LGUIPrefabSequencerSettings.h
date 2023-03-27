// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SequencerSettings.h"
#include "LGUIPrefabSequencerSettings.generated.h"

/**
 * Just create this to set default values.
 * These default values are copy from "Engine/Config/BaseEditorPerProjectUserSettings.ini"-> EmbeddedActorSequenceEditor
 */
UCLASS(config = EditorPerProjectUserSettings, PerObjectConfig)
class ULGUIPrefabSequencerSettings : public USequencerSettings
{
	GENERATED_BODY()
public:
	ULGUIPrefabSequencerSettings()
	{
		bShowRangeSlider = true;
		bKeepPlayRangeInSectionBounds = false;
		ZeroPadFrames = 4;
		bInfiniteKeyAreas = true;
		bAutoSetTrackDefaults = true;
		bCompileDirectorOnEvaluate = false;
	}
};