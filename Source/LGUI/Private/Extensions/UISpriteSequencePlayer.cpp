// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Extensions/UISpriteSequencePlayer.h"
#include "LTweenBPLibrary.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "Core/ActorComponent/UISpriteBase.h"

#if WITH_EDITOR
void UUISpriteSequencePlayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (
			propertyName == GET_MEMBER_NAME_CHECKED(UUISpriteSequencePlayer, preview)
			)
		{
			if (!sprite.IsValid())
			{
				sprite = GetOwner()->FindComponentByClass<UUISpriteBase>();
			}
			if (sprite.IsValid())
			{
				PrepareForPlay();
				OnUpdateAnimation((spriteSequence.Num() - 1) * preview);
			}
		}
	}
}
#endif
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

void UUISpriteSequencePlayer::SetSpriteSequence(TArray<ULGUISpriteData_BaseObject*> value)
{
	spriteSequence = value;
}
