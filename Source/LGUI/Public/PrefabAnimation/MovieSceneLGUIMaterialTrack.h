// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "MovieSceneLGUIMaterialTrack.generated.h"

/**
 * A material track which is specialized for LGUI's UIBatchGeometryRenderable's CustomUIMaterial.
 */
UCLASS(MinimalAPI)
class UMovieSceneLGUIMaterialTrack
	: public UMovieSceneMaterialTrack
	, public IMovieSceneTrackTemplateProducer
{
	GENERATED_BODY()

public:

	// UMovieSceneTrack interface

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;
	virtual FName GetTrackName() const override;

#if WITH_EDITORONLY_DATA
	virtual FText GetDefaultDisplayName() const override;
#endif

public:

	/** Gets name of the material property. */
	const FName& GetPropertyName() const { return PropertyName; }

	/** Sets the name of the material property. */
	LGUI_API void SetPropertyName( FName InPropertyName);

private:

	/** The name of the material property. */
	UPROPERTY()
	FName PropertyName;
	/** The name of this track, generated from the property name path. */
	UPROPERTY()
	FName TrackName;
};
