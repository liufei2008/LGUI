// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UUITextureBase, texture))
		{
			MarkTextureDirty();
		}
	}
}
void UUITextureBase::CheckTexture()
{
	if (!IsValid(texture))
	{
		auto defaultWhiteSolid = GetDefaultWhiteTexture();
		if (IsValid(defaultWhiteSolid))
		{
			texture = defaultWhiteSolid;
		}
	}
}
#endif

UTexture* UUITextureBase::GetDefaultWhiteTexture()
{
	auto defaultWhiteSolid = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
	if (!IsValid(defaultWhiteSolid))
	{
		UE_LOG(LGUI, Error, TEXT("[UUITextureBase::GetDefaultWhiteTexture]Load default texture error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
	}
	return defaultWhiteSolid;
}

UTexture* UUITextureBase::GetTextureToCreateGeometry()
{
	if (!IsValid(texture))
	{
		texture = GetDefaultWhiteTexture();
	}
	return texture;
}

void UUITextureBase::SetTexture(UTexture* newTexture)
{
	if (texture != newTexture)
	{
		texture = newTexture;
		if (texture == nullptr)
		{
			texture = GetDefaultWhiteTexture();
		}
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