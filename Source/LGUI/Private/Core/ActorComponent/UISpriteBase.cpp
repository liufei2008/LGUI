// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISpriteBase.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"


UUISpriteBase::UUISpriteBase(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUISpriteBase::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(sprite))
	{
		sprite->AddUISprite(this);
	}
}

void UUISpriteBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (IsValid(sprite))
	{
		sprite->RemoveUISprite(this);
	}
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
		MarkUVDirty();
	}
	geometry->texture = sprite->InitAndGetAtlasTexture();
	if (CheckRenderCanvas())
	{
		RenderCanvas->SetDrawcallTexture(geometry->drawcallIndex, sprite->InitAndGetAtlasTexture());
	}
}

void UUISpriteBase::SetSprite(ULGUISpriteData* newSprite, bool setSize)
{
	if (sprite != newSprite)
	{
		if((!IsValid(sprite) || !IsValid(newSprite))
			|| (sprite->InitAndGetAtlasTexture() != newSprite->InitAndGetAtlasTexture()))
		{
			MarkTextureDirty();
			//remove from old
			if (IsValid(sprite))
			{
				sprite->RemoveUISprite(this);
			}
			//add to new
			if (IsValid(newSprite))
			{
				newSprite->AddUISprite(this);
			}
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
		SetWidth(sprite->InitAndGetSpriteInfo().width);
		SetHeight(sprite->InitAndGetSpriteInfo().height);
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[UUISpriteBase::SetSizeFromSpriteData]sprite is null!"));
	}
}

void UUISpriteBase::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (this->GetWorld() && this->GetWorld()->WorldType == EWorldType::Editor)
	{
		if (IsValid(sprite))
		{
			sprite->AddUISprite(this);
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
		}
	}
#endif
}
#if WITH_EDITOR
void UUISpriteBase::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
void UUISpriteBase::CheckSpriteData()
{
	if (!IsValid(sprite))
	{
		sprite = ULGUISpriteData::GetDefaultWhiteSolid();
		sprite->AddUISprite(this);
	}
}
void UUISpriteBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUISpriteBase::OnPreChangeSpriteProperty()
{
	if (IsValid(sprite))
	{
		sprite->RemoveUISprite(this);
	}
}
void UUISpriteBase::OnPostChangeSpriteProperty()
{
	if (IsValid(sprite))
	{
		sprite->AddUISprite(this);
	}
}
#endif

UTexture* UUISpriteBase::GetTextureToCreateGeometry()
{
	if (!IsValid(sprite))
	{
		sprite = ULGUISpriteData::GetDefaultWhiteSolid();
	}
	if (IsValid(sprite) && IsValid(sprite->InitAndGetAtlasTexture()))
	{
		return sprite->InitAndGetAtlasTexture();
	}
	return nullptr;
}
