// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EntitySystem/MovieSceneEntityIDs.h"
#include "EntitySystem/MovieScenePropertySystemTypes.h"

class LGUI_API FLGUIMaterialHandle
{
public:
	FLGUIMaterialHandle()
		: Data(nullptr)
	{}

	FLGUIMaterialHandle(void* InData)
		: Data(InData)
	{}

	friend uint32 GetTypeHash(const FLGUIMaterialHandle& In)
	{
		return GetTypeHash(In.Data);
	}
	friend bool operator==(const FLGUIMaterialHandle& A, const FLGUIMaterialHandle& B)
	{
		return A.Data == B.Data;
	}
	friend bool operator!=(const FLGUIMaterialHandle& A, const FLGUIMaterialHandle& B)
	{
		return !(A == B);
	}

	/** @return true if this handle points to valid data */
	bool IsValid() const { return Data != nullptr; }

private:
	/** Pointer to the struct data holding the material */
	void* Data;
};

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
	TComponentTypeID<FLGUIMaterialHandle> LGUIMaterialHandle;

	static void Destroy();

	static FMovieSceneLGUIComponentTypes* Get();

private:
	FMovieSceneLGUIComponentTypes();
};


} // namespace MovieScene
} // namespace UE
