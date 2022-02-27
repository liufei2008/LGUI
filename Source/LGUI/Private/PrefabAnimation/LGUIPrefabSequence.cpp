// Copyright Epic Games, Inc. All Rights Reserved.

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

UBlueprint* ULGUIPrefabSequence::GetParentBlueprint() const
{
	if (UBlueprintGeneratedClass* GeneratedClass = GetTypedOuter<UBlueprintGeneratedClass>())
	{
		return Cast<UBlueprint>(GeneratedClass->ClassGeneratedBy);
	}
	return nullptr;
}

void ULGUIPrefabSequence::PostInitProperties()
{
	Super::PostInitProperties();
}

void ULGUIPrefabSequence::BindPossessableObject(const FGuid& ObjectId, UObject& PossessedObject, UObject* Context)
{
	ObjectReferences.CreateBinding(ObjectId, FLGUIPrefabSequenceObjectReference::CreateForObject(&PossessedObject));
}

bool ULGUIPrefabSequence::CanPossessObject(UObject& Object, UObject* InPlaybackContext) const
{
	if (InPlaybackContext == nullptr)
	{
		return false;
	}

	AActor* ActorContext = CastChecked<AActor>(InPlaybackContext);

	if (AActor* Actor = Cast<AActor>(&Object))
	{
		return Actor == InPlaybackContext || Actor->GetLevel() == ActorContext->GetLevel();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(&Object))
	{
		return Component->GetOwner() ? Component->GetOwner()->GetLevel() == ActorContext->GetLevel() : false;
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

UObject* ULGUIPrefabSequence::CreateDirectorInstance(IMovieScenePlayer& Player, FMovieSceneSequenceID SequenceID)
{
	AActor* Actor = CastChecked<AActor>(Player.GetPlaybackContext(), ECastCheckedType::NullAllowed);
	if (!Actor)
	{
		return nullptr;
	}

	// If this sequence is inside a blueprint, or its component's archetype is from a blueprint, we use the actor as the instace (which will be an instance of the blueprint itself)
	if (GetTypedOuter<UBlueprintGeneratedClass>() || GetTypedOuter<ULGUIPrefabSequenceComponent>()->GetArchetype() != GetDefault<ULGUIPrefabSequenceComponent>())
	{
		return Actor;
	}

	// Otherwise we use the level script actor as the instance
	return Actor->GetLevel()->GetLevelScriptActor();
}

#if WITH_EDITOR
FText ULGUIPrefabSequence::GetDisplayName() const
{
	ULGUIPrefabSequenceComponent* Component = GetTypedOuter<ULGUIPrefabSequenceComponent>();

	if (Component)
	{
		FString OwnerName;
		
		if (UBlueprint* Blueprint = GetParentBlueprint())
		{
			OwnerName = Blueprint->GetName();
		}
		else if(AActor* Owner = Component->GetOwner())
		{
			OwnerName = Owner->GetActorLabel();
		}

		return OwnerName.IsEmpty()
			? FText::FromName(Component->GetFName())
			: FText::Format(NSLOCTEXT("LGUIPrefabSequence", "DisplayName", "{0} ({1})"), FText::FromName(Component->GetFName()), FText::FromString(OwnerName));
	}

	return UMovieSceneSequence::GetDisplayName();
}


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

#endif