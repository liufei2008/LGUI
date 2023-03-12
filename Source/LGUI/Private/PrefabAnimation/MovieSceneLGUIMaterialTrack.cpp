// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIMaterialTrack.h"
#include "PrefabAnimation/MovieSceneLGUIMaterialTemplate.h"


FMovieSceneEvalTemplatePtr UMovieSceneLGUIMaterialTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneLGUIMaterialSectionTemplate(*CastChecked<UMovieSceneParameterSection>(&InSection), *this);
}


FName UMovieSceneLGUIMaterialTrack::GetTrackName() const
{ 
	return TrackName;
}


#if WITH_EDITORONLY_DATA
FText UMovieSceneLGUIMaterialTrack::GetDefaultDisplayName() const
{
	return FText::Format(NSLOCTEXT("UMGAnimation", "MaterialTrackFormat", "{0} Material"), FText::FromName( TrackName ) );
}
#endif


void UMovieSceneLGUIMaterialTrack::SetPropertyName(FName InPropertyName)
{
	PropertyName = InPropertyName;
	TrackName = PropertyName;
}
