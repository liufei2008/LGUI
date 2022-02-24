

#include "Animation/LGUIAnimation.h"
#include "UObject/Package.h"
#include "MovieScene.h"
#include "IMovieScenePlayer.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"
#include "UObject/SequencerObjectVersion.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "Core/Actor/UIBaseActor.h"
#include "Animation/LGUIAnimationBinding.h"
#include "Core/ActorComponent/UIAnimationComp.h"
#include "Utils/LGUIUtils.h"
#include "LGUI.h"


#define LOCTEXT_NAMESPACE "ULGUIAnimation"


ULGUIMovieScene::ULGUIMovieScene(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void ULGUIMovieScene::PostLoadMovieProperties()
{
	// update timerange
	UpgradeTimeRanges();
	//RemoveNullTracks(); // private function , we cannot call from derived class

	//CreateEditorOnlyDefaultSubobject<UMovieSceneNodeGroupCollection>("NodeGroupCollection");

	// fix ObjectBindings GUID-Object Mapping
}


/* ULGUIAnimation structors
 *****************************************************************************/

ULGUIAnimation::ULGUIAnimation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovieScene(nullptr)
{
	bParentContextsAreSignificant = false;
}


/* ULGUIAnimation interface
 *****************************************************************************/

#if WITH_EDITOR



ULGUIAnimation* ULGUIAnimation::GetNullAnimation()
{
	static ULGUIAnimation* NullAnimation = nullptr;

	if (!NullAnimation)
	{
		NullAnimation = NewObject<ULGUIAnimation>(GetTransientPackage(), NAME_None);
		NullAnimation->AddToRoot();
		NullAnimation->MovieScene = NewObject<ULGUIMovieScene>(NullAnimation, FName("No Animation"));
		NullAnimation->MovieScene->AddToRoot();

		NullAnimation->MovieScene->SetDisplayRate(FFrameRate(20, 1));
	}

	return NullAnimation;
}

void ULGUIAnimation::SetDisplayLabel(const FString& InDisplayLabel)
{
	DisplayLabel = InDisplayLabel;
}

FText ULGUIAnimation::GetDisplayName() const
{
	const bool bHasDisplayLabel = !DisplayLabel.IsEmpty();
	return bHasDisplayLabel ? FText::FromString(DisplayLabel) : Super::GetDisplayName();
}

ETrackSupport ULGUIAnimation::IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const
{
	// 暂不支持声音和事件，材质参数
	//if (InTrackClass == UMovieSceneAudioTrack::StaticClass() ||
	//	InTrackClass == UMovieSceneEventTrack::StaticClass() ||
	//	InTrackClass == UMovieSceneMaterialParameterCollectionTrack::StaticClass())
	//{
	//	return ETrackSupport::Supported;
	//}

	return Super::IsTrackSupported(InTrackClass);
}
#endif

float ULGUIAnimation::GetStartTime() const
{
	return MovieScene->GetPlaybackRange().GetLowerBoundValue() / MovieScene->GetTickResolution();
}

float ULGUIAnimation::GetEndTime() const
{
	return MovieScene->GetPlaybackRange().GetUpperBoundValue() / MovieScene->GetTickResolution();
}


/* UMovieSceneAnimation overrides
 *****************************************************************************/


void ULGUIAnimation::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	UUIAnimationComp* PreviewWidget = CastChecked<UUIAnimationComp>(Context);

	uint32 UIActorID = 0;
	if (AUIBaseActor* UIActor = Cast<AUIBaseActor>(&PossessedObject))
	{
		UIActorID = UIActor->UIActorID;
	}

	// If it's the Root Widget
	if (&PossessedObject == PreviewWidget->GetOwner())
	{
		FLGUIAnimationBinding NewBinding;
		{
			NewBinding.ActorGuid = ObjectId;
			NewBinding.ActorName = PossessedObject.GetFName();
			NewBinding.UIActorID = UIActorID;
			NewBinding.bIsRootActor = true;
		}

		AnimationBindings.Add(NewBinding);
		return;
	}
	
	FLGUIAnimationBinding NewBinding;
	{
		NewBinding.ActorGuid = ObjectId;
		NewBinding.ActorName = PossessedObject.GetFName();
		NewBinding.bIsRootActor = false;
		NewBinding.UIActorID = UIActorID;
	}
	AnimationBindings.Add(NewBinding);
}


bool ULGUIAnimation::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	if (InPlaybackContext == nullptr)
	{
		return false;
	}

	UUIAnimationComp* AnimationComp = CastChecked<UUIAnimationComp>(InPlaybackContext);
	if (AnimationComp == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("CanPossessObject failed! InPlaybackContext is not valid AnimationComp!"), *GetNameSafe(InPlaybackContext));
		return false;
	}

	if (&Object == AnimationComp->GetOwner())
	{
		// root actor is ok!
		return true;
	}

	AUIBaseActor* UIActor = Cast<AUIBaseActor>(&Object);
	if (UIActor == nullptr)
	{
		return false;
	}

	// check if child actor
	AUIBaseActor* FoundObject = LGUIUtils::FindChildActorByUIActorID(AnimationComp->GetOwner(), UIActor->UIActorID, true);
	return FoundObject != nullptr;
}

// InContenxt: ULGUIAnimation
// 根据MovieSceneBinding中序列化的ObjectGUID来查找对象
// 由于ActorSerialization会重新创建UIBaseActor，而重新创建的UIBaseActor，其ObjectGUID可能会发生变化。
void ULGUIAnimation::LocateBoundObjects(const FGuid& ActorGuid, UObject* InContext, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	if (InContext == nullptr)
	{
		return;
	}

	UUIAnimationComp* AnimationComp = CastChecked<UUIAnimationComp>(InContext);
	for (const FLGUIAnimationBinding& Binding : AnimationBindings)
	{
		if (Binding.ActorGuid == ActorGuid)
		{
			UObject* FoundObject = Binding.FindRuntimeObject(*AnimationComp, this);

			if (FoundObject)
			{
				OutObjects.Add(FoundObject);
			}
		}
	}
}


UMovieScene* ULGUIAnimation::GetMovieScene() const
{
	return MovieScene;
}

UObject* ULGUIAnimation::CreateDirectorInstance(IMovieScenePlayer& Player)
{
	// Widget animations do not create separate director instances, but just re-use the UUserWidget from the playback context
	UUIAnimationComp* WidgetContext = CastChecked<UUIAnimationComp>(Player.GetPlaybackContext());
	return WidgetContext;
}

UObject* ULGUIAnimation::GetParentObject(UObject* Object) const
{
	AUIBaseActor* UIActor = Cast<AUIBaseActor>(Object);

	if (UIActor != nullptr)
	{
		AActor* ParentActor = UIActor->GetAttachParentActor();
		return ParentActor;
	}

	return nullptr;
}

void ULGUIAnimation::UnbindPossessableObjects(const FGuid& ActorGuid)
{
	// mark dirty
	Modify();

	// remove animation bindings
	AnimationBindings.RemoveAll([&](const FLGUIAnimationBinding& Binding) {
		return Binding.ActorGuid == ActorGuid;
	});
}

void ULGUIAnimation::RemoveBinding(const UObject& PossessedObject)
{
	Modify();

	uint32 UIActorID = LGUIUtils::GetUIActorID(&PossessedObject);

	AnimationBindings.RemoveAll([&](const FLGUIAnimationBinding& Binding) {
		return Binding.UIActorID == UIActorID;
	});
}

void ULGUIAnimation::RemoveBinding(const FLGUIAnimationBinding& Binding)
{
	Modify();

	AnimationBindings.Remove(Binding);
}

bool ULGUIAnimation::IsPostLoadThreadSafe() const
{
	return true;
}

#undef LOCTEXT_NAMESPACE
