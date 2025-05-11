// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIMaterialTrack.h"
#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"
#include "EntitySystem/BuiltInComponentTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneLGUIMaterialTrack)

UMovieSceneLGUIMaterialTrack::UMovieSceneLGUIMaterialTrack(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{

}

bool UMovieSceneLGUIMaterialTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneParameterSection::StaticClass();
}

UMovieSceneSection* UMovieSceneLGUIMaterialTrack::CreateNewSection()
{
	UMovieSceneSection* NewSection = NewObject<UMovieSceneParameterSection>(this, NAME_None, RF_Transactional);
	NewSection->SetBlendType(EMovieSceneBlendType::Absolute);
	NewSection->SetRange(TRange<FFrameNumber>::All());
	return NewSection;
}

void UMovieSceneLGUIMaterialTrack::ImportEntityImpl(UMovieSceneEntitySystemLinker* EntityLinker, const FEntityImportParams& Params, FImportedEntity* OutImportedEntity)
{
	// These tracks don't define any entities for themselves
	checkf(false, TEXT("This track should never have created entities for itself - this assertion indicates an error in the entity-component field"));
}

void UMovieSceneLGUIMaterialTrack::ExtendEntityImpl(UMovieSceneParameterSection* Section, UMovieSceneEntitySystemLinker* EntityLinker, const UE::MovieScene::FEntityImportParams& Params, UE::MovieScene::FImportedEntity* OutImportedEntity)
{
	using namespace UE::MovieScene;

	FBuiltInComponentTypes* BuiltInComponents = FBuiltInComponentTypes::Get();
	FMovieSceneLGUIComponentTypes* LGUIComponents = FMovieSceneLGUIComponentTypes::Get();

	// Material parameters are always absolute blends for the time being
	OutImportedEntity->AddBuilder(
		FEntityBuilder()
		.Add(LGUIComponents->LGUIMaterialPath, FLGUIMaterialPath(PropertyName))
		.AddTagConditional(BuiltInComponents->Tags.AbsoluteBlend, !Section->GetBlendType().IsValid())
	);
}

bool UMovieSceneLGUIMaterialTrack::PopulateEvaluationFieldImpl(const TRange<FFrameNumber>& EffectiveRange, const FMovieSceneEvaluationFieldEntityMetaData& InMetaData, FMovieSceneEntityComponentFieldBuilder* OutFieldBuilder)
{
	const FMovieSceneTrackEvaluationField& LocalEvaluationField = GetEvaluationField();

	// Define entities for every entry in our evaluation field
	for (const FMovieSceneTrackEvaluationFieldEntry& Entry : LocalEvaluationField.Entries)
	{
		UMovieSceneParameterSection* ParameterSection = Cast<UMovieSceneParameterSection>(Entry.Section);
		if (!ParameterSection || IsRowEvalDisabled(ParameterSection->GetRowIndex()))
		{
			continue;
		}

		TRange<FFrameNumber> SectionEffectiveRange = TRange<FFrameNumber>::Intersection(EffectiveRange, Entry.Range);
		if (!SectionEffectiveRange.IsEmpty())
		{
			FMovieSceneEvaluationFieldEntityMetaData SectionMetaData = InMetaData;
			SectionMetaData.Flags = Entry.Flags;
			SectionMetaData.Condition = MovieSceneHelpers::GetSequenceCondition(this, ParameterSection, true);

			ParameterSection->ExternalPopulateEvaluationField(SectionEffectiveRange, SectionMetaData, OutFieldBuilder);
		}
	}

	return true;
}

FName UMovieSceneLGUIMaterialTrack::GetTrackName() const
{ 
	return TrackName;
}


#if WITH_EDITORONLY_DATA
FText UMovieSceneLGUIMaterialTrack::GetDefaultDisplayName() const
{
	return FText::Format(NSLOCTEXT("LGUIAnimation", "MaterialTrackFormat", "{0} Material"), FText::FromName( TrackName ) );
}
#endif


void UMovieSceneLGUIMaterialTrack::SetPropertyName(FName InPropertyName)
{
	PropertyName = InPropertyName;
	TrackName = PropertyName;
}
