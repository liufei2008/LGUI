// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIMaterialSystem.h"
#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"

#include "EntitySystem/MovieSceneEntityMutations.h"

#include "Evaluation/PreAnimatedState/MovieScenePreAnimatedStorageID.inl"

#include "Systems/FloatChannelEvaluatorSystem.h"
#include "Systems/MovieScenePiecewiseDoubleBlenderSystem.h"

#include "Materials/MaterialInstanceDynamic.h"

#include "Core/ActorComponent/UIBatchMeshRenderable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovieSceneLGUIMaterialSystem)

namespace UE::MovieScene
{

FLGUIMaterialAccessor::FLGUIMaterialAccessor(const FLGUIMaterialKey& InKey)
	: Renderable(Cast<UUIBatchMeshRenderable>(InKey.Object.ResolveObjectPtr()))
	, LGUIMaterialPath(InKey.LGUIMaterialPath)
{}

FLGUIMaterialAccessor::FLGUIMaterialAccessor(UObject* InObject, FLGUIMaterialPath InLGUIMaterialPath)
	: Renderable(Cast<UUIBatchMeshRenderable>(InObject))
	, LGUIMaterialPath(MoveTemp(InLGUIMaterialPath))
{
	check(!InObject || Renderable);
}

FString FLGUIMaterialAccessor::ToString() const
{
	return FString::Printf(TEXT("CustomUIMaterial %s.%s"), *Renderable->GetPathName(), *LGUIMaterialPath.Path.ToString());
}

UMaterialInterface* FLGUIMaterialAccessor::GetMaterial() const
{
	if (Renderable)
	{
		return Renderable->GetCustomUIMaterial();
	}
	return nullptr;
}

void FLGUIMaterialAccessor::SetMaterial(UMaterialInterface* InMaterial) const
{
	if (Renderable)
	{
		Renderable->SetCustomUIMaterial(InMaterial);
	}
}

UMaterialInstanceDynamic* FLGUIMaterialAccessor::CreateDynamicMaterial(UMaterialInterface* InMaterial)
{
	// Need to create a new MID, either because the parent has changed, or because one doesn't already exist
	TStringBuilder<128> DynamicName;
	InMaterial->GetFName().ToString(DynamicName);
	DynamicName.Append(TEXT("_Animated"));
	FName UniqueDynamicName = MakeUniqueObjectName(Renderable, UMaterialInstanceDynamic::StaticClass() , DynamicName.ToString());

	UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(InMaterial, Renderable, UniqueDynamicName);
	SetMaterial(MID);
	return MID;
}

TAutoRegisterPreAnimatedStorageID<FPreAnimatedLGUIMaterialSwitcherStorage> FPreAnimatedLGUIMaterialSwitcherStorage::StorageID;
TAutoRegisterPreAnimatedStorageID<FPreAnimatedLGUIMaterialParameterStorage> FPreAnimatedLGUIMaterialParameterStorage::StorageID;

} // namespace UE::MovieScene

UMovieSceneLGUIMaterialSystem::UMovieSceneLGUIMaterialSystem(const FObjectInitializer& ObjInit)
	: Super(ObjInit)
{
	using namespace UE::MovieScene;

	FBuiltInComponentTypes*          BuiltInComponents = FBuiltInComponentTypes::Get();
	FMovieSceneLGUIComponentTypes*    LGUIComponents  = FMovieSceneLGUIComponentTypes::Get();
	FMovieSceneTracksComponentTypes* TracksComponents  = FMovieSceneTracksComponentTypes::Get();

	RelevantComponent = LGUIComponents->LGUIMaterialPath;
	Phase = ESystemPhase::Instantiation | ESystemPhase::Evaluation;

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		DefineComponentConsumer(GetClass(), BuiltInComponents->ObjectResult);
		DefineComponentConsumer(GetClass(), BuiltInComponents->BoundObject);
		DefineComponentProducer(GetClass(), TracksComponents->BoundMaterial);
		DefineImplicitPrerequisite(UMovieSceneCachePreAnimatedStateSystem::StaticClass(), GetClass());
	}
}

void UMovieSceneLGUIMaterialSystem::OnLink()
{
	using namespace UE::MovieScene;

	FBuiltInComponentTypes*       BuiltInComponents = FBuiltInComponentTypes::Get();
	FMovieSceneLGUIComponentTypes* LGUIComponents  = FMovieSceneLGUIComponentTypes::Get();

	SystemImpl.MaterialSwitcherStorage = Linker->PreAnimatedState.GetOrCreateStorage<FPreAnimatedLGUIMaterialSwitcherStorage>();
	SystemImpl.MaterialParameterStorage = Linker->PreAnimatedState.GetOrCreateStorage<FPreAnimatedLGUIMaterialParameterStorage>();

	SystemImpl.OnLink(Linker, BuiltInComponents->BoundObject, LGUIComponents->LGUIMaterialPath);
}

void UMovieSceneLGUIMaterialSystem::OnUnlink()
{
	SystemImpl.OnUnlink(Linker);
}

void UMovieSceneLGUIMaterialSystem::OnRun(FSystemTaskPrerequisites& InPrerequisites, FSystemSubsequentTasks& Subsequents)
{
	using namespace UE::MovieScene;

	FBuiltInComponentTypes*       BuiltInComponents = FBuiltInComponentTypes::Get();
	FMovieSceneLGUIComponentTypes* LGUIComponents  = FMovieSceneLGUIComponentTypes::Get();

	SystemImpl.OnRun(Linker, BuiltInComponents->BoundObject, LGUIComponents->LGUIMaterialPath, InPrerequisites, Subsequents);
}

void UMovieSceneLGUIMaterialSystem::SavePreAnimatedState(const FPreAnimationParameters& InParameters)
{
	using namespace UE::MovieScene;

	FBuiltInComponentTypes*       BuiltInComponents = FBuiltInComponentTypes::Get();
	FMovieSceneLGUIComponentTypes* LGUIComponents  = FMovieSceneLGUIComponentTypes::Get();

	SystemImpl.SavePreAnimatedState(Linker, BuiltInComponents->BoundObject, LGUIComponents->LGUIMaterialPath, InParameters);
}
