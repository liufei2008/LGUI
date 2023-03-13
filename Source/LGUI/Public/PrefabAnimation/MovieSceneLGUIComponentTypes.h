// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Layout/Margin.h"
#include "Slate/WidgetTransform.h"
#include "EntitySystem/MovieSceneEntityIDs.h"
#include "EntitySystem/MovieScenePropertySystemTypes.h"
#include "EntitySystem/MovieScenePropertyTraits.h"
#include "EntitySystem/MovieScenePropertyMetaDataTraits.h"

#include "Containers/ArrayView.h"


namespace UE
{
namespace MovieScene
{

struct FLGUIMaterialPath
{
	FLGUIMaterialPath() = default;
	FLGUIMaterialPath(FName Name)
		: Path(Name)
	{}

	FName Path;
};

struct LGUI_API FMovieSceneLGUIComponentTypes
{
	~FMovieSceneLGUIComponentTypes();

	TComponentTypeID<FLGUIMaterialPath> LGUIMaterialPath;

	static void Destroy();

	static FMovieSceneLGUIComponentTypes* Get();

private:
	FMovieSceneLGUIComponentTypes();
};


} // namespace MovieScene
} // namespace UE
