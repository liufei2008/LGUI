// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/LexImageSequencePlayer.h"
#include "LTweenBPLibrary.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexTexture.h"

ULexImageSequencePlayer::ULexImageSequencePlayer()
{
}
void ULexImageSequencePlayer::Awake()
{
	Super::Awake();	
	if (bPlayOnStart)
	{
		Play();
	}
}

void ULexImageSequencePlayer::OnDestroy()
{
	Super::OnDestroy();
	Stop();
}

void ULexImageSequencePlayer::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	RegisterEditorTick();
#endif
}
void ULexImageSequencePlayer::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	UnregisterEditorTick();
#endif
}

#if WITH_EDITOR
void ULexImageSequencePlayer::RegisterEditorTick()
{
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		if (auto LexUIManagerObject = ULexUIManagerObject::GetInstance(true))
		{
			if (!EditorPlayDelegateHandle.IsValid())
			{
				EditorPlayDelegateHandle = LexUIManagerObject->GetEditorTickDelegate().AddWeakLambda(this, [this](float deltaTime) {
					if (!bPreviewInEditor)return;
					if (!CanPlay())return;
					Duration = GetDuration();
					PrepareForPlay();
					UpdateAnimation(deltaTime);
					});
			}
		}
	}
}
void ULexImageSequencePlayer::UnregisterEditorTick()
{
	if (EditorPlayDelegateHandle.IsValid())
	{
		if (auto LexUIManagerObject = ULexUIManagerObject::GetInstance(false))
		{
			LexUIManagerObject->GetEditorTickDelegate().Remove(EditorPlayDelegateHandle);
		}
		EditorPlayDelegateHandle.Reset();
	}
}
void ULexImageSequencePlayer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	auto PropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexImageSequencePlayer, bPreviewInEditor))
	{
		if (bPreviewInEditor)
		{
			RegisterEditorTick();
		}
		else
		{
			UnregisterEditorTick();
		}
	}
}
#endif

void ULexImageSequencePlayer::Play()
{
	if (!CanPlay())return;
	if (!bIsPlaying)
	{
		bIsPlaying = true;
		ElapsedTime = 0.0f;
		Duration = GetDuration();
		PrepareForPlay();
		PlayTweener = ULTweenBPLibrary::UpdateCall(this, FLTweenUpdateDelegate::CreateUObject(this, &ULexImageSequencePlayer::UpdateAnimation));
		if (PlayTweener.IsValid())
		{
			PlayTweener->SetAffectByGamePause(bAffectByGamePause);
		}
		UpdateAnimation(0);
	}
	if (bIsPaused)
	{
		bIsPaused = false;
	}
}

void ULexImageSequencePlayer::Stop()
{
	if (bIsPlaying)
	{
		bIsPlaying = false;
		ULTweenBPLibrary::KillIfIsTweening(this, PlayTweener.Get());
	}
}

void ULexImageSequencePlayer::SeekFrame(int frameNumber)
{
	ElapsedTime = frameNumber / Fps;
	if (CanPlay())
	{
		OnUpdateAnimation(frameNumber);
	}
}
void ULexImageSequencePlayer::SeekTime(float time)
{
	ElapsedTime = time;
	if (CanPlay())
	{
		OnUpdateAnimation(ElapsedTime * Fps);
	}
}

void ULexImageSequencePlayer::UpdateAnimation(float deltaTime)
{
	if (bIsPaused)return;
	ElapsedTime += deltaTime;
	if (ElapsedTime > Duration)
	{
		if (bLoop)
		{
			ElapsedTime -= Duration;
		}
		else
		{
			Stop();
			return;
		}
	}
	int frameNumber = (int)(ElapsedTime * Fps);
	OnUpdateAnimation(frameNumber);
}

void ULexImageSequencePlayer::SetFps(float value)
{
	Fps = value;
}
void ULexImageSequencePlayer::SetLoop(bool value)
{
	bLoop = value;
}
