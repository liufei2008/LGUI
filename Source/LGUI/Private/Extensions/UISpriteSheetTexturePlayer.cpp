// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Extensions/UISpriteSheetTexturePlayer.h"
#include "LTweenBPLibrary.h"
#include "Core/ActorComponent/UITexture.h"

#if WITH_EDITOR
void UUISpriteSheetTexturePlayer::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propertyName = Property->GetFName();
		if (
			propertyName == GET_MEMBER_NAME_CHECKED(UUISpriteSheetTexturePlayer, preview)
			)
		{
			if (!texture.IsValid())
			{
				texture = GetOwner()->FindComponentByClass<UUITexture>();
			}
			if (texture.IsValid())
			{
				if (preview < 0)
				{
					texture->SetUVRect(FVector4(0, 0, 1, 1));
				}
				else
				{
					PrepareForPlay();
					OnUpdateAnimation((widthCount * heightCount) * preview);
				}
			}
		}
	}
}
#endif
bool UUISpriteSheetTexturePlayer::CanPlay()
{
	if (!texture.IsValid())
	{
		texture = GetOwner()->FindComponentByClass<UUITexture>();
	}
	if (!texture.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteSheetTexturePlayer::Play]Need UITexture component!"));
		return false;
	}
	if (!IsValid(texture->GetTexture()))
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteSheetTexturePlayer::Play]UITexture component must have valid texture!"));
		return false;
	}
	if (widthCount <= 0 || heightCount <= 0)
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteSheetTexturePlayer::Play]WidthCount & HeightCount must greater then 0!"));
		return false;
	}
	return true;
}
float UUISpriteSheetTexturePlayer::GetDuration()const
{
	return (float)(widthCount * heightCount) / fps;
}
void UUISpriteSheetTexturePlayer::PrepareForPlay()
{
	widthUVInterval = 1.0f / widthCount;
	heightUVInterval = 1.0f / heightCount;
}

void UUISpriteSheetTexturePlayer::OnUpdateAnimation(int frameNumber)
{
	int verticalFrame = (int)(frameNumber / widthCount);
	int horizontalFrame = (int)(frameNumber % widthCount);
	verticalFrame = FMath::Clamp(verticalFrame, 0, heightCount);
	horizontalFrame = FMath::Clamp(horizontalFrame, 0, widthCount);
	texture->SetUVRect(FVector4(widthUVInterval * horizontalFrame, heightUVInterval * verticalFrame, widthUVInterval, heightUVInterval));
}

void UUISpriteSheetTexturePlayer::SetWidthCount(int value)
{
	widthCount = value;
}
void UUISpriteSheetTexturePlayer::SetHeightCount(int value)
{
	heightCount = value;
}
