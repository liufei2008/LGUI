// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneParameterTemplate.h"
#include "MovieSceneLGUIMaterialTemplate.generated.h"

class UMovieSceneLGUIMaterialTrack;

USTRUCT()
struct FMovieSceneLGUIMaterialSectionTemplate : public FMovieSceneParameterSectionTemplate
{
	GENERATED_BODY()

	FMovieSceneLGUIMaterialSectionTemplate() {}
	FMovieSceneLGUIMaterialSectionTemplate(const UMovieSceneParameterSection& Section, const UMovieSceneLGUIMaterialTrack& Track);

private:

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
	FName PropertyName;
};
