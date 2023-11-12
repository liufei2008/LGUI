// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "EntitySystem/MovieSceneEntitySystem.h"
#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedStateStorage.h"

#include "Systems/MovieSceneMaterialSystem.h"
#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"

#include "MovieSceneLGUIMaterialSystem.generated.h"

class UUIBatchMeshRenderable;
class UMovieScenePiecewiseDoubleBlenderSystem;

namespace UE::MovieScene
{

struct FLGUIMaterialKey
{
	FObjectKey Object;
	FLGUIMaterialHandle LGUIMaterialHandle;

	friend uint32 GetTypeHash(const FLGUIMaterialKey& In)
	{
		uint32 Accumulator = GetTypeHash(In.Object);
		Accumulator ^= GetTypeHash(In.LGUIMaterialHandle);
		return Accumulator;
	}
	friend bool operator==(const FLGUIMaterialKey& A, const FLGUIMaterialKey& B)
	{
		return A.Object == B.Object && A.LGUIMaterialHandle == B.LGUIMaterialHandle;
	}
};

struct FLGUIMaterialAccessor
{
	using KeyType = FLGUIMaterialKey;

	UUIBatchMeshRenderable* Renderable;
	FLGUIMaterialHandle LGUIMaterialHandle;

	FLGUIMaterialAccessor(const FLGUIMaterialKey& InKey);
	FLGUIMaterialAccessor(UObject* InObject, FLGUIMaterialHandle InLGUIMaterialPath);

	explicit operator bool() const;

	UMaterialInterface* GetMaterial() const;
	void SetMaterial(UMaterialInterface* InMaterial) const;
	UMaterialInstanceDynamic* CreateDynamicMaterial(UMaterialInterface* InMaterial);
	FString ToString() const;
};

using FPreAnimatedLGUIMaterialTraits          = TPreAnimatedMaterialTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialHandle>;
using FPreAnimatedLGUIMaterialParameterTraits = TPreAnimatedMaterialParameterTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialHandle>;

struct FPreAnimatedLGUIMaterialSwitcherStorage
	: public TPreAnimatedStateStorage<TPreAnimatedMaterialTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialHandle>>
{
	static TAutoRegisterPreAnimatedStorageID<FPreAnimatedLGUIMaterialSwitcherStorage> StorageID;
};

struct FPreAnimatedLGUIMaterialParameterStorage
	: public TPreAnimatedStateStorage<TPreAnimatedMaterialParameterTraits<FLGUIMaterialAccessor, UObject*, FLGUIMaterialHandle>>
{
	static TAutoRegisterPreAnimatedStorageID<FPreAnimatedLGUIMaterialParameterStorage> StorageID;
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

	UE::MovieScene::TMovieSceneMaterialSystem<UE::MovieScene::FLGUIMaterialAccessor, UObject*, FLGUIMaterialHandle> SystemImpl;
};
