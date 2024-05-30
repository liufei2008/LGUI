// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "Components/ActorComponent.h"
#include "LGUIImageSequencePlayer.generated.h"

UCLASS(Abstract)
class LGUI_API ULGUIImageSequencePlayer : public UActorComponent
{
	GENERATED_BODY()
public:
	ULGUIImageSequencePlayer();
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool previewInEditor = true;
#endif
	/** fps: Frame per second */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float fps = 24;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool loop = true;
	/** Auto play when BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool playOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool affectByGamePause = false;
	bool isPlaying = false;

	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
#if WITH_EDITOR
	FDelegateHandle editorPlayDelegateHandle;
#endif
	TWeakObjectPtr<class ULTweener> playTweener;
	void UpdateAnimation(float deltaTime);
	virtual bool CanPlay() { return true; }
	virtual void PrepareForPlay() {};
	virtual void OnUpdateAnimation(int frameNumber)PURE_VIRTUAL(ULGUIImageSequencePlayer::OnUpdateAnimation, );
	float elapsedTime = 0.0f;
	float duration = 1.0f;
	bool isPaused = false;
public:
	/** Play the animation sequence. If is paused then resume play */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Play();
	/** Stop the animation. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Stop();
	/** Pause the animation, call Play to resume. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Pause() { isPaused = true; }
	/** Seek to desired frame and play. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SeekFrame(int frameNumber);
	/** Seek to desired time and play. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SeekTime(float time);
	/** Is the animation playing? */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetIsPlaying()const { return isPlaying && !isPaused; }
	/** Get the animation time length */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual float GetDuration()const PURE_VIRTUAL(ULGUIImageSequencePlayer::GetDuration, return 0.0f;);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFps()const { return fps; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetLoop()const { return loop; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFps(float value);
	/** Will take effect on nexe cycle. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetLoop(bool value);
};
