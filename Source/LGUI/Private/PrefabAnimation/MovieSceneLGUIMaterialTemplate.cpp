// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIMaterialTemplate.h"
#include "PrefabAnimation/MovieSceneLGUIMaterialTrack.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"

// Container to ensure unique IDs per property path
TMovieSceneAnimTypeIDContainer<FName> MaterialPropertyIDs;

struct FLGUIMaterialAccessor : FDefaultMaterialAccessor
{
	FLGUIMaterialAccessor(FName InPropertyName)
		: AnimTypeID(MaterialPropertyIDs.GetAnimTypeID(InPropertyName))
	{}

	FMovieSceneAnimTypeID GetAnimTypeID() const
	{
		return AnimTypeID;
	}

	UMaterialInterface* GetMaterialForObject(UObject& Object)
	{
		if (auto Renderable = Cast<UUIBatchGeometryRenderable>(&Object))
		{
			return Renderable->GetCustomUIMaterial();
		}
		return nullptr;
	}

	void SetMaterialForObject(UObject& Object, UMaterialInterface& Material)
	{
		if (auto Renderable = Cast<UUIBatchGeometryRenderable>(&Object))
		{
			Renderable->SetCustomUIMaterial(&Material);
		}
	}

	UMaterialInstanceDynamic* CreateMaterialInstanceDynamic(UObject& Object, UMaterialInterface& Material, FName UniqueDynamicName)
	{
		return UMaterialInstanceDynamic::Create(&Material, &Object, UniqueDynamicName );
	}

	FMovieSceneAnimTypeID AnimTypeID;
};

FMovieSceneLGUIMaterialSectionTemplate::FMovieSceneLGUIMaterialSectionTemplate(const UMovieSceneParameterSection& Section, const UMovieSceneLGUIMaterialTrack& Track)
	: FMovieSceneParameterSectionTemplate(Section)
	, PropertyName(Track.GetPropertyName())
{
}

void FMovieSceneLGUIMaterialSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	TMaterialTrackExecutionToken<FLGUIMaterialAccessor> ExecutionToken(PropertyName);

	EvaluateCurves(Context, ExecutionToken.Values);

	ExecutionTokens.Add(MoveTemp(ExecutionToken));
}
