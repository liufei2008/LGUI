// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "PrefabAnimation/LGUIPrefabSequence.h"
#include "MovieScene.h"
#include "MovieSceneCommonHelpers.h"
#include "Modules/ModuleManager.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Blueprint.h"
#include "GameFramework/Actor.h"
#include "PrefabAnimation/LGUIPrefabSequenceComponent.h"
#include "Engine/LevelScriptActor.h"
#include "Tracks/MovieSceneAudioTrack.h"
#include "Tracks/MovieSceneEventTrack.h"
#include "Tracks/MovieSceneMaterialParameterCollectionTrack.h"

#if WITH_EDITOR
ULGUIPrefabSequence::FOnInitialize ULGUIPrefabSequence::OnInitializeSequenceEvent;
#endif

static TAutoConsoleVariable<int32> CVarDefaultEvaluationType(
	TEXT("LGUIPrefabSequence.DefaultEvaluationType"),
	0,
	TEXT("0: Playback locked to playback frames\n1: Unlocked playback with sub frame interpolation"),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultTickResolution(
	TEXT("LGUIPrefabSequence.DefaultTickResolution"),
	TEXT("24000fps"),
	TEXT("Specifies default a tick resolution for newly created level sequences. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

static TAutoConsoleVariable<FString> CVarDefaultDisplayRate(
	TEXT("LGUIPrefabSequence.DefaultDisplayRate"),
	TEXT("30fps"),
	TEXT("Specifies default a display frame rate for newly created level sequences; also defines frame locked frame rate where sequences are set to be frame locked. Examples: 30 fps, 120/1 (120 fps), 30000/1001 (29.97), 0.01s (10ms)."),
	ECVF_Default);

ULGUIPrefabSequence::ULGUIPrefabSequence(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MovieScene(nullptr)
#if WITH_EDITORONLY_DATA
	, bHasBeenInitialized(false)
#endif
{
	bParentContextsAreSignificant = true;

	MovieScene = ObjectInitializer.CreateDefaultSubobject<UMovieScene>(this, "MovieScene");
	MovieScene->SetFlags(RF_Transactional);
}

bool ULGUIPrefabSequence::IsEditable() const
{
	return true;
	//UObject* Template = GetArchetype();

	//if (Template == GetDefault<ULGUIPrefabSequence>())
	//{
	//	return false;
	//}

	//return !Template || Template->GetTypedOuter<ULGUIPrefabSequenceComponent>() == GetDefault<ULGUIPrefabSequenceComponent>();
}

void ULGUIPrefabSequence::PostInitProperties()
{
#if WITH_EDITOR && WITH_EDITORONLY_DATA

	// We do not run the default initialization for actor sequences that are CDOs, or that are going to be loaded (since they will have already been initialized in that case)
	EObjectFlags ExcludeFlags = RF_ClassDefaultObject | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded;

	UActorComponent* OwnerComponent = Cast<UActorComponent>(GetOuter());
	if (!bHasBeenInitialized && !HasAnyFlags(ExcludeFlags) && OwnerComponent && !OwnerComponent->HasAnyFlags(ExcludeFlags))
	{
		const bool bFrameLocked = CVarDefaultEvaluationType.GetValueOnGameThread() != 0;

		MovieScene->SetEvaluationType(bFrameLocked ? EMovieSceneEvaluationType::FrameLocked : EMovieSceneEvaluationType::WithSubFrames);

		FFrameRate TickResolution(60000, 1);
		TryParseString(TickResolution, *CVarDefaultTickResolution.GetValueOnGameThread());
		MovieScene->SetTickResolutionDirectly(TickResolution);

		FFrameRate DisplayRate(30, 1);
		TryParseString(DisplayRate, *CVarDefaultDisplayRate.GetValueOnGameThread());
		MovieScene->SetDisplayRate(DisplayRate);

		OnInitializeSequenceEvent.Broadcast(this);
		bHasBeenInitialized = true;
	}
#endif

	Super::PostInitProperties();
}

void ULGUIPrefabSequence::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	if (CanPossessObject(PossessedObject, Context))
	{
		FLGUIPrefabSequenceObjectReference ObjectRef;
		AActor* ActorContext = CastChecked<AActor>(Context);
		AActor* Actor = Cast<AActor>(&PossessedObject);
		if (Actor == nullptr)
		{
			if (auto Component = Cast<UActorComponent>(&PossessedObject))
			{
				Actor = Component->GetOwner();
			}
		}
		check(Actor != nullptr);
		if (FLGUIPrefabSequenceObjectReference::CreateForObject(Actor, &PossessedObject, ObjectRef))
		{
			ObjectReferences.CreateBinding(ObjectId, ObjectRef);
		}
	}
}

bool ULGUIPrefabSequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	if (InPlaybackContext == nullptr)
	{
		return false;
	}

	AActor* ActorContext = CastChecked<AActor>(InPlaybackContext);
	AActor* Actor = Cast<AActor>(&Object);
	if (Actor == nullptr)
	{
		if (auto Component = Cast<UActorComponent>(&Object))
		{
			Actor = Component->GetOwner();
		}
		if (Actor == nullptr)
		{
			Actor = Object.GetTypedOuter<AActor>();
		}
	}

	if (Actor != nullptr)
	{
		return Actor->GetLevel() == ActorContext->GetLevel()
			&& (Actor == InPlaybackContext || Actor->IsAttachedTo(ActorContext))//only allow actor self or child actor
			;
	}

	return false;
}

void ULGUIPrefabSequence::LocateBoundObjects(const FGuid& ObjectId, UObject* Context, TArray<UObject*, TInlineAllocator<1>>& OutObjects) const
{
	ObjectReferences.ResolveBinding(ObjectId, OutObjects);
}

UMovieScene* ULGUIPrefabSequence::GetMovieScene() const
{
	return MovieScene;
}

UObject* ULGUIPrefabSequence::GetParentObject(UObject* Object) const
{
	if (UActorComponent* Component = Cast<UActorComponent>(Object))
	{
		return Component->GetOwner();
	}

	return nullptr;
}

void ULGUIPrefabSequence::UnbindPossessableObjects(const FGuid& ObjectId)
{
	ObjectReferences.RemoveBinding(ObjectId);
}

#if WITH_EDITOR

ETrackSupport ULGUIPrefabSequence::IsTrackSupported(TSubclassOf<class UMovieSceneTrack> InTrackClass) const
{
	if (InTrackClass == UMovieSceneAudioTrack::StaticClass() ||
		InTrackClass == UMovieSceneEventTrack::StaticClass() ||
		InTrackClass == UMovieSceneMaterialParameterCollectionTrack::StaticClass())
	{
		return ETrackSupport::Supported;
	}

	return Super::IsTrackSupported(InTrackClass);
}

bool ULGUIPrefabSequence::IsObjectReferencesGood(AActor* InContextActor)const
{
	return ObjectReferences.IsObjectReferencesGood(InContextActor);
}
bool ULGUIPrefabSequence::IsEditorHelpersGood(AActor* InContextActor)const
{
	return ObjectReferences.IsEditorHelpersGood(InContextActor);
}
void ULGUIPrefabSequence::FixObjectReferences(AActor* InContextActor)
{
	if (ObjectReferences.FixObjectReferences(InContextActor))
	{
		this->Modify();
	}
}
void ULGUIPrefabSequence::FixEditorHelpers(AActor* InContextActor)
{
	if (ObjectReferences.FixEditorHelpers(InContextActor))
	{
		this->Modify();
	}
}
#endif