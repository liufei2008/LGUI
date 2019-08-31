// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITexture.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"


UUITexture::UUITexture()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITexture::BeginPlay()
{
	Super::BeginPlay();
	WidthChanged();
	HeightChanged();
	CheckSpriteData();
}
#if WITH_EDITOR
void UUITexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	WidthChanged();
	HeightChanged();
	CheckSpriteData();
}
void UUITexture::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
	WidthChanged();
	HeightChanged();
	CheckSpriteData();
}
#endif

void UUITexture::CheckSpriteData()
{
	if (texture != nullptr)
	{
		spriteData.width = texture->GetSurfaceWidth();
		spriteData.height = texture->GetSurfaceHeight();
		if (type != UITextureType::Tiled)
		{
			spriteData.ApplyUV(uvRect.X * spriteData.width, uvRect.Y * spriteData.height, uvRect.Z * spriteData.width, uvRect.W * spriteData.height, 1.0f / spriteData.width, 1.0f / spriteData.height);
			spriteData.ApplyBorderUV(1.0f / spriteData.width, 1.0f / spriteData.height);
		}
	}
}

void UUITexture::OnCreateGeometry()
{
	switch (type)
	{
	case UITextureType::Normal:
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, &spriteData, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
		break;
	case UITextureType::Sliced:
	case UITextureType::SlicedFrame:
	{
		if (spriteData.HasBorder())
			UIGeometry::FromUIRectBorder(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, &spriteData, type == UITextureType::Sliced, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
		else
			UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, &spriteData, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
	}
	break;
	case UITextureType::Tiled:
	{
		FLGUISpriteInfo tempSpriteInfo;
		tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / texture->GetSurfaceWidth(), 1.0f / texture->GetSurfaceHeight());
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, &tempSpriteInfo, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
	}
		break;
	}
}
void UUITexture::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexPositionChanged)
	{
		switch (type)
		{
		case UITextureType::Normal:
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot);
			break;
		case UITextureType::Sliced:
		case UITextureType::SlicedFrame:
		{
			if (spriteData.HasBorder())
				UIGeometry::UpdateUIRectBorderVertex(geometry, widget.width, widget.height, widget.pivot, &spriteData);
			else
				UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot);
		}
		break;
		case UITextureType::Tiled:
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot);
			break;
		}
	}
	if (InVertexUVChanged)
	{
		switch (type)
		{
		case UITextureType::Normal:
			UIGeometry::UpdateUIRectSimpleUV(geometry, &spriteData);
			break;
		case UITextureType::Sliced:
		case UITextureType::SlicedFrame:
		{
			if (spriteData.HasBorder())
				UIGeometry::UpdateUIRectBorderUV(geometry, &spriteData);
			else
				UIGeometry::UpdateUIRectSimpleUV(geometry, &spriteData);
		}
		break;
		case UITextureType::Tiled:
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / texture->GetSurfaceWidth(), 1.0f / texture->GetSurfaceHeight());
			UIGeometry::UpdateUIRectSimpleUV(geometry, &tempSpriteInfo);
		}
			break;
		}
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
}

void UUITexture::WidthChanged()
{
	if (texture == nullptr)return;
	if (type != UITextureType::Tiled)return;
	spriteData.ApplyUV(0, 0, widget.width, widget.height, 1.0f / spriteData.width, 1.0f / spriteData.height);
	MarkVertexPositionDirty();
	MarkUVDirty();
}
void UUITexture::HeightChanged()
{
	if (texture == nullptr)return;
	if (type != UITextureType::Tiled)return;
	spriteData.ApplyUV(0, 0, widget.width, widget.height, 1.0f / spriteData.width, 1.0f / spriteData.height);
	MarkVertexPositionDirty();
	MarkUVDirty();
}


void UUITexture::SetTextureType(UITextureType newType)
{
	if (type != newType)
	{
		type = newType;
		MarkTriangleDirty();
	}
}
void UUITexture::SetSpriteData(FLGUISpriteInfo newSpriteData) 
{
	if (spriteData != newSpriteData)
	{
		spriteData = newSpriteData;
		MarkUVDirty();
		CheckSpriteData();
	}
}

void UUITexture::SetUVRect(FVector4 newUVRect)
{
	if (uvRect != newUVRect)
	{
		uvRect = newUVRect;
		MarkUVDirty();
		CheckSpriteData();
	}
}

void UUITexture::SetTexture(UTexture* newTexture)
{
	if (texture != newTexture)
	{
		texture = newTexture;
		MarkTextureDirty();
		CheckSpriteData();
	}
}