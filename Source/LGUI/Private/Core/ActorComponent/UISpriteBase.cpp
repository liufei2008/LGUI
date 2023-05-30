// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISpriteBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "Core/UIDrawcall.h"

UUISpriteBase::UUISpriteBase(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUISpriteBase::BeginPlay()
{
	Super::BeginPlay();
	if (!bHasAddToSprite)
	{
		if (IsValid(sprite))
		{
			sprite->AddUISprite(this);
			bHasAddToSprite = true;
		}
	}
}

void UUISpriteBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (IsValid(sprite))
	{
		sprite->RemoveUISprite(this);
		bHasAddToSprite = false;
	}
}

void UUISpriteBase::ApplyAtlasTextureChange()
{
	geometry->texture = sprite->GetAtlasTexture();
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			drawcall->Texture = geometry->texture;
			drawcall->bTextureChanged = true;
		}
	}
	MarkCanvasUpdate(true, true, false);
}
void UUISpriteBase::ApplyAtlasTextureScaleUp()
{
	auto& vertices = geometry->vertices;
	if (vertices.Num() != 0)
	{
		for (int i = 0; i < vertices.Num(); i++)
		{
			auto& uv = vertices[i];
			uv.TextureCoordinate[0].X *= 0.5f;
			uv.TextureCoordinate[0].Y *= 0.5f;
		}
	}
	geometry->texture = sprite->GetAtlasTexture();
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			drawcall->Texture = geometry->texture;
			drawcall->bTextureChanged = true;
			drawcall->bNeedToUpdateVertex = true;
		}
	}
	MarkVerticesDirty(false, true, true, false);
	MarkCanvasUpdate(true, true, false);
}

void UUISpriteBase::SetSprite(ULGUISpriteData_BaseObject* newSprite, bool setSize)
{
	if (sprite != newSprite)
	{
		if((!IsValid(sprite) || !IsValid(newSprite))
			|| (sprite->GetAtlasTexture() != newSprite->GetAtlasTexture()))
		{
			//remove from old
			if (IsValid(sprite))
			{
				sprite->RemoveUISprite(this);
				bHasAddToSprite = false;
			}
			//add to new
			if (IsValid(newSprite))
			{
				newSprite->AddUISprite(this);
				bHasAddToSprite = true;
			}
			MarkTextureDirty();
		}
		sprite = newSprite;
		MarkUVDirty();
		if (setSize) SetSizeFromSpriteData();
	}
}
void UUISpriteBase::SetSizeFromSpriteData()
{
	if (IsValid(sprite))
	{
		SetWidth(sprite->GetSpriteInfo().GetSourceWidth());
		SetHeight(sprite->GetSpriteInfo().GetSourceHeight());
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Sprite is null!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}

void UUISpriteBase::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (!bHasAddToSprite)
		{
			if (IsValid(sprite))
			{
				sprite->AddUISprite(this);
				bHasAddToSprite = true;
			}
		}
	}
#endif
}
void UUISpriteBase::OnUnregister()
{
	Super::OnUnregister();
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (IsValid(sprite))
		{
			sprite->RemoveUISprite(this);
			bHasAddToSprite = false;
		}
	}
#endif
}
#if WITH_EDITOR
void UUISpriteBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUISpriteBase::OnPreChangeSpriteProperty()
{
	if (IsValid(sprite))
	{
		sprite->RemoveUISprite(this);
		bHasAddToSprite = false;
	}
}
void UUISpriteBase::OnPostChangeSpriteProperty()
{
	if (IsValid(sprite))
	{
		sprite->AddUISprite(this);
		bHasAddToSprite = true;
	}
}
#endif

void UUISpriteBase::CheckSpriteData()
{
	if (!IsValid(sprite))
	{
		sprite = ULGUISpriteData::GetDefaultWhiteSolid();
		sprite->AddUISprite(this);
	}
}
void UUISpriteBase::OnBeforeCreateOrUpdateGeometry()
{
	if (!bHasAddToSprite)
	{
		CheckSpriteData();
		if (IsValid(sprite))
		{
			sprite->AddUISprite(this);
			bHasAddToSprite = true;
		}
	}
}

UTexture* UUISpriteBase::GetTextureToCreateGeometry()
{
	if (!IsValid(sprite))
	{
		sprite = ULGUISpriteData::GetDefaultWhiteSolid();
	}
	if (IsValid(sprite) && IsValid(sprite->GetAtlasTexture()))
	{
		return sprite->GetAtlasTexture();
	}
	return nullptr;
}

bool UUISpriteBase::ReadPixelFromMainTexture(const FVector2D& InUV, FColor& OutPixel)const
{
	if (IsValid(sprite))
	{
		return sprite->ReadPixel(InUV, OutPixel);
	}
	return false;
}
