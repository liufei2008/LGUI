// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITextureBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"
#include "Utils/LGUIUtils.h"

#define LOCTEXT_NAMESPACE "LGUISpriteData"

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
		auto errMsg = LOCTEXT("MissingDefaultContent", "[UUITextureBase::GetDefaultWhiteTexture] Load default texture error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.");
		UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errMsg, 10);
#endif
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

#undef LOCTEXT_NAMESPACE
