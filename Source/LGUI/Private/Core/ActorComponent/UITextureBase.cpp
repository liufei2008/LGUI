// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITextureBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"


UUITextureBase::UUITextureBase(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITextureBase::BeginPlay()
{
	Super::BeginPlay();
}
#if WITH_EDITOR
void UUITextureBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUITextureBase::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
void UUITextureBase::CheckTexture()
{
	if (!IsValid(texture))
	{
		static auto defaultWhiteSolid = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
		if (IsValid(defaultWhiteSolid))
		{
			texture = defaultWhiteSolid;
		}
		else
		{
			UE_LOG(LGUI, Error, TEXT("[UUITextureBase::CheckTexture]Load default texture error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
		}
	}
}
#endif

UTexture* UUITextureBase::GetTextureToCreateGeometry()
{
	return texture;
}

void UUITextureBase::SetTexture(UTexture* newTexture)
{
	if (texture != newTexture)
	{
		texture = newTexture;
		MarkTextureDirty();
	}
}
void UUITextureBase::SetSizeFromTexture()
{
	if (IsValid(texture))
	{
		SetWidth(texture->GetSurfaceWidth());
		SetHeight(texture->GetSurfaceHeight());
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUITextureBase::SetSizeFromTexture]texture is null!"));
	}
}