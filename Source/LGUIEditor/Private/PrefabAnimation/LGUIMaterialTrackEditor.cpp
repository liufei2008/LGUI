// Copyright Epic Games, Inc. All Rights Reserved.

#include "LGUIMaterialTrackEditor.h"
#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "PrefabAnimation/MovieSceneLGUIMaterialTrack.h"


FLGUIMaterialTrackEditor::FLGUIMaterialTrackEditor( TSharedRef<ISequencer> InSequencer )
	: FMaterialTrackEditor( InSequencer )
{
}


TSharedRef<ISequencerTrackEditor> FLGUIMaterialTrackEditor::CreateTrackEditor( TSharedRef<ISequencer> OwningSequencer )
{
	return MakeShareable( new FLGUIMaterialTrackEditor( OwningSequencer ) );
}


bool FLGUIMaterialTrackEditor::SupportsType( TSubclassOf<UMovieSceneTrack> Type ) const
{
	return Type == UMovieSceneLGUIMaterialTrack::StaticClass();
}


UMaterialInterface* FLGUIMaterialTrackEditor::GetMaterialInterfaceForTrack( FGuid ObjectBinding, UMovieSceneMaterialTrack* MaterialTrack )
{
	for (TWeakObjectPtr<> WeakObjectPtr : GetSequencer()->FindObjectsInCurrentSequence(ObjectBinding))
	{
		auto Renderable = Cast<UUIBatchMeshRenderable>( WeakObjectPtr.Get() );
		if (Renderable != nullptr)
		{
			return Renderable->GetCustomUIMaterial();
		}
	}
	return nullptr;
}
