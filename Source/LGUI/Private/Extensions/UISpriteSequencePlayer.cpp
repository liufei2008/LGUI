// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Extensions/UISpriteSequencePlayer.h"
#include "LTweenBPLibrary.h"
#include "Core/LGUISpriteData.h"
#include "Core/ActorComponent/UISpriteBase.h"

bool UUISpriteSequencePlayer::CanPlay()
{
	if (!sprite.IsValid())
	{
		sprite = GetOwner()->FindComponentByClass<UUISpriteBase>();
	}
	if (!sprite.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteSequencePlayer::Play]Need UISprite component!"));
		return false;
	}
	if (spriteSequence.Num() <= 0)
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteSequencePlayer::Play]SpriteSequence array is empty!"));
		return false;
	}
	return true;
}
float UUISpriteSequencePlayer::GetDuration()const
{
	return (float)(spriteSequence.Num()) / fps;
}
void UUISpriteSequencePlayer::PrepareForPlay()
{

}

void UUISpriteSequencePlayer::OnUpdateAnimation(int frameNumber)
{
	frameNumber = FMath::Clamp(frameNumber, 0, spriteSequence.Num() - 1);
	sprite->SetSprite(spriteSequence[frameNumber]);
}

void UUISpriteSequencePlayer::SetSpriteSequence(TArray<ULGUISpriteData*> value)
{
	spriteSequence = value;
}
