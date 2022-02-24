// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "IMovieScenePlayer.h"
//#include "Animation/UMGSequenceTickManager.h"
#include "Evaluation/MovieSceneEvaluationTemplateInstance.h"
#include "Misc/QualifiedFrameTime.h"
#include "LGUIAnimationPlayer.generated.h"

class ULGUIAnimation;
class UUIAnimationComp;

/*  映射关系: WidgetTree->UIMainContainer
 *			   UserWiget->UUIAnimationComp
 */
UCLASS(Transient, BlueprintType)
class LGUI_API ULGUIAnimationPlayer : public UObject, public IMovieScenePlayer
{
	GENERATED_UCLASS_BODY()

public:
	void InitSequencePlayer(ULGUIAnimation& InAnimation, UUIAnimationComp& InUserWidget);

	/** Updates the running movie */
	void Tick( float DeltaTime );

	/** Begins playing or restarts an animation */
	void Play(float StartAtTime, int32 InNumLoopsToPlay, int32 InPlayMode, float InPlaybackSpeed, bool bRestoreState);

	/** Begins playing or restarts an animation  and plays to the specified end time */
	void PlayTo(float StartAtTime, float EndAtTime, int32 InNumLoopsToPlay, int32 InPlayMode, float InPlaybackSpeed, bool bRestoreState);

	/** Stops a running animation and resets time */
	void Stop();

	/** Pauses a running animation */
	void Pause();

	/** Reverses a running animation */
	void Reverse();

	void SetCurrentTime(float InTime) { TimeCursorPosition = AnimationResolution.AsFrameTime(InTime); }
	FQualifiedFrameTime GetCurrentTime() const { return FQualifiedFrameTime(TimeCursorPosition, AnimationResolution); }

	/** @return The current animation being played */
	const ULGUIAnimation* GetAnimation() const { return Animation.Get(); }

	/** @return */
	UFUNCTION(BlueprintCallable, Category="Animation")
	FName GetUserTag() const { return UserTag; }

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void SetUserTag(FName InUserTag)
	{
		UserTag = InUserTag;
	}

	/** Sets the number of loops to play */
	void SetNumLoopsToPlay(int32 InNumLoopsToPlay);

	/** Sets the animation playback rate */
	void SetPlaybackSpeed(float PlaybackSpeed);

	/** Gets the current time position in the player (in seconds). */
	bool IsPlayingForward() const { return bIsPlayingForward; }

	/** IMovieScenePlayer interface */
	virtual FMovieSceneRootEvaluationTemplateInstance& GetEvaluationTemplate() override { return RootTemplateInstance; }
	virtual UMovieSceneEntitySystemLinker* ConstructEntitySystemLinker() override;
	virtual UObject* AsUObject() override { return this; }
	virtual void UpdateCameraCut(UObject* CameraObject, const EMovieSceneCameraCutParams& CameraCutParams) override {}
	virtual void SetViewportSettings(const TMap<FViewportClient*, EMovieSceneViewportParams>& ViewportParamsMap) override {}
	virtual void GetViewportSettings(TMap<FViewportClient*, EMovieSceneViewportParams>& ViewportParamsMap) const override {}
	virtual EMovieScenePlayerStatus::Type GetPlaybackStatus() const override;
	virtual UObject* GetPlaybackContext() const override;
	virtual TArray<UObject*> GetEventContexts() const override;
	virtual void SetPlaybackStatus(EMovieScenePlayerStatus::Type InPlaybackStatus) override;
	virtual void PreEvaluation(const FMovieSceneContext& Context) override;
	virtual void PostEvaluation(const FMovieSceneContext& Context) override;

	/** UObject interface */
	virtual void BeginDestroy() override;

	void TearDown();

	DECLARE_EVENT_OneParam(ULGUIAnimationPlayer, FOnSequenceFinishedPlaying, ULGUIAnimationPlayer&);
	FOnSequenceFinishedPlaying& OnSequenceFinishedPlaying() { return OnSequenceFinishedPlayingEvent; }

private:
	/** Internal play function with a verbose parameter set */
	void PlayInternal(double StartAtTime, double EndAtTime, int32 InNumLoopsToPlay, int32 InPlayMode, float InPlaybackSpeed, bool bRestoreState);

	//bool NeedsQueueLatentAction() const;
	//void QueueLatentAction(FMovieSceneSequenceLatentActionDelegate Delegate);
	//void ApplyLatentActions();

	/** Animation being played */
	UPROPERTY()
	TObjectPtr<ULGUIAnimation> Animation;

	/** The user widget this sequence is animating */
	TWeakObjectPtr<UUIAnimationComp> UIAnimationComp;

	UPROPERTY()
	FMovieSceneRootEvaluationTemplateInstance RootTemplateInstance;

	/** The resolution at which all FFrameNumbers are stored */
	FFrameRate AnimationResolution;

	FFrameNumber AbsolutePlaybackStart;

	/** The current time cursor position within the sequence, between 0 and Duration */
	FFrameTime TimeCursorPosition;

	/** The duration of the sequence */
	int32 Duration;

	/** Time at which to end the animation after looping */
	FFrameTime EndTime;

	/** Status of the player (e.g play, stopped) */
	EMovieScenePlayerStatus::Type PlayerStatus;

	/** Delegate to call when a sequence has finished playing */
	FOnSequenceFinishedPlaying OnSequenceFinishedPlayingEvent;

	/** The number of times to loop the animation playback */
	int32 NumLoopsToPlay;

	/** The number of loops completed since the last call to Play() */
	int32 NumLoopsCompleted;

	/** The speed at which the animation should be played */
	float PlaybackSpeed;

	/** Whether to restore pre-animated state */
	bool bRestoreState;

	/** The current playback mode. */
	int32 PlayMode;

	/**
	 * The 'state' tag the user may want to use to track what the animation is for.  It's very common in UI to use the
	 * same animation for intro / outro, so this allows you to tag what the animation is currently doing so that you can
	 * have some events just get called back when the animation finishes the outtro, to say, remove the UI then.
	 */
	FName UserTag;

	/** True if the animation is playing forward, otherwise false and it's playing in reverse. */
	bool bIsPlayingForward;

	/** Set to true while evaluating to prevent reentrancy */
	bool bIsEvaluating : 1;

	/** Set to true if we need to run the finishing logic in post-evaluation */
	bool bCompleteOnPostEvaluation : 1;

	/** List of latent action delegates. Only used when multi-threaded animation evaluation is disabled */
	//TArray<FMovieSceneSequenceLatentActionDelegate> LatentActions;
};
