// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"
#include "EntitySystem/BuiltInComponentTypes.h"
#include "EntitySystem/MovieSceneComponentRegistry.h"
#include "EntitySystem/MovieSceneEntityFactoryTemplates.h"
#include "EntitySystem/MovieScenePropertyComponentHandler.h"
#include "MovieSceneTracksComponentTypes.h"
#include "Systems/MovieScenePiecewiseDoubleBlenderSystem.h"

namespace UE
{
namespace MovieScene
{

static bool GMovieSceneLGUIComponentTypesDestroyed = false;
static TUniquePtr<FMovieSceneLGUIComponentTypes> GMovieSceneLGUIComponentTypes;

FMovieSceneLGUIComponentTypes::FMovieSceneLGUIComponentTypes()
{
	FComponentRegistry* ComponentRegistry = UMovieSceneEntitySystemLinker::GetComponents();

	ComponentRegistry->NewComponentType(&LGUIMaterialPath, TEXT("LGUI Material Path"), EComponentTypeFlags::CopyToChildren | EComponentTypeFlags::CopyToOutput);
}

FMovieSceneLGUIComponentTypes::~FMovieSceneLGUIComponentTypes()
{
}

void FMovieSceneLGUIComponentTypes::Destroy()
{
	GMovieSceneLGUIComponentTypes.Reset();
	GMovieSceneLGUIComponentTypesDestroyed = true;
}

FMovieSceneLGUIComponentTypes* FMovieSceneLGUIComponentTypes::Get()
{
	if (!GMovieSceneLGUIComponentTypes.IsValid())
	{
		check(!GMovieSceneLGUIComponentTypesDestroyed);
		GMovieSceneLGUIComponentTypes.Reset(new FMovieSceneLGUIComponentTypes);
	}
	return GMovieSceneLGUIComponentTypes.Get();
}


} // namespace MovieScene
} // namespace UE
