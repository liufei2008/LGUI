// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITextureBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Materials/MaterialInterface.h"
#include "Utils/LGUIUtils.h"
#include "TextureResource.h"

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
		auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load default texture error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
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

bool UUITextureBase::ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const
{
	if (IsValid(texture))
	{
		if (auto texture2D = Cast<UTexture2D>(texture))
		{
			auto PlatformData = texture2D->GetPlatformData();
			if (PlatformData && PlatformData->Mips.Num() > 0)
			{
				if (auto Pixels = (FColor*)(PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY)))
				{
					auto uvInFullSize = FIntPoint(InUV.X * texture2D->GetSizeX(), InUV.Y * texture2D->GetSizeY());
					auto PixelIndex = uvInFullSize.Y * texture2D->GetSizeX() + uvInFullSize.X;
					OutPixel = Pixels[PixelIndex];
				}
				PlatformData->Mips[0].BulkData.Unlock();
				return true;
			}
		}
	}
	return false;
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
		UE_LOG(LGUI, Error, TEXT("[%s].%d Texture is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}

#undef LOCTEXT_NAMESPACE
