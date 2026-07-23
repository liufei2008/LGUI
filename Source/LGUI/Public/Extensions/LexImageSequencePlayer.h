// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "Core/LexUIBehaviour.h"
#include "LexImageSequencePlayer.generated.h"

UCLASS(Abstract)
class LGUI_API ULexImageSequencePlayer : public ULexUIBehaviour
{
	GENERATED_BODY()
public:
	ULexImageSequencePlayer();
protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPreviewInEditor = true;
#endif
	/** fps: Frame per second */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float Fps = 24;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bLoop = true;
	/** Autoplay when BeginPlay. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bPlayOnStart = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bAffectByGamePause = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bAffectByTimeDilation = false;
	bool bIsPlaying = false;

	virtual void Awake()override;
	virtual void OnDestroy()override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
#if WITH_EDITOR
	FDelegateHandle EditorPlayDelegateHandle;
	void RegisterEditorTick();
	void UnregisterEditorTick();
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	TWeakObjectPtr<class ULTweener> PlayTweener;
	void UpdateAnimation(float deltaTime);
	virtual bool CanPlay() { return true; }
	virtual void PrepareForPlay() {};
	virtual void OnUpdateAnimation(int FrameNumber)PURE_VIRTUAL(ULexUIImageSequencePlayer::OnUpdateAnimation, );
	float ElapsedTime = 0.0f;
	float Duration = 1.0f;
	bool bIsPaused = false;
public:
	/** Play the animation sequence. If is paused then resume play */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Play();
	/** Stop the animation. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Stop();
	/** Pause the animation, call Play to resume. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void Pause() { bIsPaused = true; }
	/** Seek to desired frame and play. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SeekFrame(int frameNumber);
	/** Seek to desired time and play. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SeekTime(float time);
	/** Is the animation playing? */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetIsPlaying()const { return bIsPlaying && !bIsPaused; }
	/** Get the animation time length */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual float GetDuration()const PURE_VIRTUAL(ULGUIImageSequencePlayer::GetDuration, return 0.0f;);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFps()const { return Fps; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetLoop()const { return bLoop; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetFps(float value);
	/** Will take effect on nexe cycle. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetLoop(bool value);
};
