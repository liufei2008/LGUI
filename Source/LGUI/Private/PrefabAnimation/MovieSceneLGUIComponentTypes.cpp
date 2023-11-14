// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrefabAnimation/MovieSceneLGUIComponentTypes.h"
#include "EntitySystem/BuiltInComponentTypes.h"
#include "EntitySystem/MovieSceneComponentRegistry.h"
#include "EntitySystem/MovieSceneEntityFactoryTemplates.h"
#include "EntitySystem/MovieScenePropertyComponentHandler.h"
#include "MovieSceneTracksComponentTypes.h"
#include "Systems/MovieScenePiecewiseDoubleBlenderSystem.h"
#include "Core/ActorComponent/UIBatchMeshRenderable.h"

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
	ComponentRegistry->NewComponentType(&LGUIMaterialHandle, TEXT("LGUI Material Handle"), EComponentTypeFlags::CopyToOutput);
	/** Initializer that initializes the value of an FLGUIMaterialHandle derived from an FLGUIMaterialPath */
	struct FLGUIMaterialHandleInitializer : TChildEntityInitializer<FLGUIMaterialPath, FLGUIMaterialHandle>
	{
		explicit FLGUIMaterialHandleInitializer(TComponentTypeID<FLGUIMaterialPath> Path, TComponentTypeID<FLGUIMaterialHandle> Handle)
			: TChildEntityInitializer<FLGUIMaterialPath, FLGUIMaterialHandle>(Path, Handle)
		{}

		virtual void Run(const FEntityRange& ChildRange, const FEntityAllocation* ParentAllocation, TArrayView<const int32> ParentAllocationOffsets)
		{
			TComponentReader<FLGUIMaterialPath>   PathComponents = ParentAllocation->ReadComponents(this->GetParentComponent());
			TComponentWriter<FLGUIMaterialHandle> HandleComponents = ChildRange.Allocation->WriteComponents(this->GetChildComponent(), FEntityAllocationWriteContext::NewAllocation());
			TOptionalComponentReader<UObject*>      BoundObjectComponents = ChildRange.Allocation->TryReadComponents(FBuiltInComponentTypes::Get()->BoundObject);
			if (!ensure(BoundObjectComponents))
			{
				return;
			}

			for (int32 Index = 0; Index < ChildRange.Num; ++Index)
			{
				const int32 ParentIndex = ParentAllocationOffsets[Index];
				const int32 ChildIndex = ChildRange.ComponentStartOffset + Index;

				auto Renderable = Cast<UUIBatchMeshRenderable>(BoundObjectComponents[ChildIndex]);
				if (Renderable)
				{
					auto Property = FindFProperty<FProperty>(Renderable->GetClass(), UUIBatchMeshRenderable::GetCustomUIMaterialPropertyName());
					HandleComponents[ChildIndex] = FLGUIMaterialHandle(Property->ContainerPtrToValuePtr<void>(Renderable));
				}
			}
		}
	};

	ComponentRegistry->Factories.DefineChildComponent(FLGUIMaterialHandleInitializer(LGUIMaterialPath, LGUIMaterialHandle));
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
