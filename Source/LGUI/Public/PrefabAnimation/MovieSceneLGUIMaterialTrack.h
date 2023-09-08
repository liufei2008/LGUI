// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "EntitySystem/IMovieSceneEntityProvider.h"
#include "Sections/MovieSceneParameterSection.h"
#include "Tracks/MovieSceneMaterialTrack.h"
#include "MovieSceneLGUIMaterialTrack.generated.h"

class UMovieSceneEntitySystemLinker;
class UMovieSceneSection;
struct FMovieSceneEntityComponentFieldBuilder;
struct FMovieSceneEvaluationFieldEntityMetaData;

/**
 * A material track which is specialized for LGUI's UIBatchGeometryRenderable's CustomUIMaterial.
 */
UCLASS(MinimalAPI)
class UMovieSceneLGUIMaterialTrack
	: public UMovieSceneMaterialTrack
	, public IMovieSceneEntityProvider
	, public IMovieSceneParameterSectionExtender
{
	GENERATED_BODY()

public:

	// UMovieSceneTrack interface
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual FName GetTrackName() const override;

	/*~ IMovieSceneEntityProvider */
	virtual void ImportEntityImpl(UMovieSceneEntitySystemLinker* EntityLinker, const FEntityImportParams& Params, FImportedEntity* OutImportedEntity) override;
	virtual bool PopulateEvaluationFieldImpl(const TRange<FFrameNumber>& EffectiveRange, const FMovieSceneEvaluationFieldEntityMetaData& InMetaData, FMovieSceneEntityComponentFieldBuilder* OutFieldBuilder) override;

	/*~ IMovieSceneParameterSectionExtender */
	virtual void ExtendEntityImpl(UMovieSceneParameterSection* Section, UMovieSceneEntitySystemLinker* EntityLinker, const UE::MovieScene::FEntityImportParams& Params, UE::MovieScene::FImportedEntity* OutImportedEntity) override;

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
