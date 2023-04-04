// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedStateStorage.h"

#include "Systems/MovieSceneMaterialSystem.h"
#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"

#include "MovieSceneLGUIMaterialSystem.generated.h"

class UUIBatchGeometryRenderable;
class UMovieScenePiecewiseDoubleBlenderSystem;

namespace UE::MovieScene
{

struct FLGUIMaterialKey
{
	FObjectKey Object;
	FLGUIMaterialPath LGUIMaterialPath;

	friend uint32 GetTypeHash(const FLGUIMaterialKey& In)
	{
		uint32 Accumulator = GetTypeHash(In.Object);
		Accumulator ^= GetTypeHash(In.LGUIMaterialPath.Path);
		return Accumulator;
	}
	friend bool operator==(const FLGUIMaterialKey& A, const FLGUIMaterialKey& B)
	{
		return A.Object == B.Object && A.LGUIMaterialPath.Path == B.LGUIMaterialPath.Path;
	}
};

struct FLGUIMaterialAccessor
{
	using KeyType = FLGUIMaterialKey;

	UUIBatchGeometryRenderable* Renderable;
	FLGUIMaterialPath LGUIMaterialPath;

	FLGUIMaterialAccessor(const FLGUIMaterialKey& InKey);
	FLGUIMaterialAccessor(UObject* InObject, FLGUIMaterialPath InWidgetMaterialPath);

	UMaterialInterface* GetMaterial() const;
	void SetMaterial(UMaterialInterface* InMaterial) const;
	UMaterialInstanceDynamic* CreateDynamicMaterial(UMaterialInterface* InMaterial);
	FString ToString() const;
};

using FPreAnimatedWidgetMaterialTraits          = TPreAnimatedMaterialTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialPath>;
using FPreAnimatedWidgetMaterialParameterTraits = TPreAnimatedMaterialParameterTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialPath>;

struct FPreAnimatedWidgetMaterialSwitcherStorage
	: public TPreAnimatedStateStorage<TPreAnimatedMaterialTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialPath>>
{
	static TAutoRegisterPreAnimatedStorageID<FPreAnimatedWidgetMaterialSwitcherStorage> StorageID;
};

struct FPreAnimatedWidgetMaterialParameterStorage
	: public TPreAnimatedStateStorage<TPreAnimatedMaterialParameterTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialPath>>
{
	static TAutoRegisterPreAnimatedStorageID<FPreAnimatedWidgetMaterialParameterStorage> StorageID;
};

} // namespace UE::MovieScene


UCLASS(MinimalAPI)
class UMovieSceneLGUIMaterialSystem
	: public UMovieSceneEntitySystem
	, public IMovieScenePreAnimatedStateSystemInterface
{
public:

	GENERATED_BODY()

	UMovieSceneLGUIMaterialSystem(const FObjectInitializer& ObjInit);

private:

	virtual void OnLink() override;
	virtual void OnUnlink() override;
	virtual void OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents) override;

	virtual void SavePreAnimatedState(const FPreAnimationParameters& InParameters) override;

private:

	UE::MovieScene::TMovieSceneMaterialSystem<UE::MovieScene::FLGUIMaterialAccessor, UObject*, UE::MovieScene::FLGUIMaterialPath> SystemImpl;
};