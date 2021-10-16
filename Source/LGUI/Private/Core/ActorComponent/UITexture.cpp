// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITexture.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"


UUITexture::UUITexture(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUITexture::BeginPlay()
{
	Super::BeginPlay();
	WidthChanged();
	HeightChanged();
}
#if WITH_EDITOR
void UUITexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	WidthChanged();
	HeightChanged();
	CheckSpriteData();
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propName = Property->GetFName();
		if (propName == GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial90))
		{
			fillOrigin = (uint8)fillOriginType_Radial90;
			fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial180))
		{
			fillOrigin = (uint8)fillOriginType_Radial180;
			fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)fillOrigin;
			fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial360))
		{
			fillOrigin = (uint8)fillOriginType_Radial360;
			fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)fillOrigin;
		}
	}
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
	if (IsValid(texture))
	{
		spriteData.width = texture->GetSurfaceWidth();
		spriteData.height = texture->GetSurfaceHeight();
		if (type != UITextureType::Tiled)
		{
			spriteData.ApplyUV(0, 0, spriteData.width, spriteData.height, 1.0f / spriteData.width, 1.0f / spriteData.height, uvRect);
			spriteData.ApplyBorderUV(1.0f / spriteData.width, 1.0f / spriteData.height);
		}
	}
}

void UUITexture::OnCreateGeometry()
{
	CheckSpriteData();
	switch (type)
	{
	case UITextureType::Normal:
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, spriteData, RenderCanvas.Get(), this);
		break;
	case UITextureType::Sliced:
	case UITextureType::SlicedFrame:
	{
		if (spriteData.HasBorder())
			UIGeometry::FromUIRectBorder(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, spriteData, RenderCanvas.Get(), this, type == UITextureType::Sliced);
		else
			UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, spriteData, RenderCanvas.Get(), this);
	}
	break;
	case UITextureType::Tiled:
	{
		FLGUISpriteInfo tempSpriteInfo;
		tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / texture->GetSurfaceWidth(), 1.0f / texture->GetSurfaceHeight(), uvRect);
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, tempSpriteInfo, RenderCanvas.Get(), this);
	}
	break;
	case UITextureType::Filled:
	{
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
			UIGeometry::FromUIRectFillHorizontalVertical(widget.width, widget.height, widget.pivot, widget.color, geometry, spriteData, fillDirectionFlip, fillAmount, true, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Vertical:
			UIGeometry::FromUIRectFillHorizontalVertical(widget.width, widget.height, widget.pivot, widget.color, geometry, spriteData, fillDirectionFlip, fillAmount, false, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial90:
			UIGeometry::FromUIRectFillRadial90(widget.width, widget.height, widget.pivot, widget.color, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial180:
			UIGeometry::FromUIRectFillRadial180(widget.width, widget.height, widget.pivot, widget.color, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial360:
			UIGeometry::FromUIRectFillRadial360(widget.width, widget.height, widget.pivot, widget.color, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas.Get(), this);
			break;
		}
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
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, spriteData, RenderCanvas.Get(), this);
			break;
		case UITextureType::Sliced:
		case UITextureType::SlicedFrame:
		{
			if (spriteData.HasBorder())
				UIGeometry::UpdateUIRectBorderVertex(geometry, widget.width, widget.height, widget.pivot, spriteData, RenderCanvas.Get(), this);
			else
				UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, spriteData, RenderCanvas.Get(), this);
		}
		break;
		case UITextureType::Tiled:
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, spriteData, RenderCanvas.Get(), this);
			break;
		case UITextureType::Filled:
		{
			switch (fillMethod)
			{
			case UISpriteFillMethod::Horizontal:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, true, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Vertical:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, false, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial90:
				UIGeometry::UpdateUIRectFillRadial90Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial180:
				UIGeometry::UpdateUIRectFillRadial180Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial360:
				UIGeometry::UpdateUIRectFillRadial360Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			}
		}
		break;
		}
	}
	if (InVertexUVChanged)
	{
		switch (type)
		{
		case UITextureType::Normal:
			UIGeometry::UpdateUIRectSimpleUV(geometry, spriteData);
			break;
		case UITextureType::Sliced:
		case UITextureType::SlicedFrame:
		{
			if (spriteData.HasBorder())
				UIGeometry::UpdateUIRectBorderUV(geometry, spriteData);
			else
				UIGeometry::UpdateUIRectSimpleUV(geometry, spriteData);
		}
		break;
		case UITextureType::Tiled:
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / texture->GetSurfaceWidth(), 1.0f / texture->GetSurfaceHeight(), uvRect);
			UIGeometry::UpdateUIRectSimpleUV(geometry, tempSpriteInfo);
		}
		break;
		case UITextureType::Filled:
		{
			switch (fillMethod)
			{
			case UISpriteFillMethod::Horizontal:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, true, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Vertical:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, false, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial90:
				UIGeometry::UpdateUIRectFillRadial90Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial180:
				UIGeometry::UpdateUIRectFillRadial180Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial360:
				UIGeometry::UpdateUIRectFillRadial360Vertex(widget.width, widget.height, widget.pivot, geometry, spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			}
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
	if (!IsValid(texture))return;
	if (type == UITextureType::Tiled)
	{
		spriteData.ApplyUV(0, 0, widget.width, widget.height, 1.0f / spriteData.width, 1.0f / spriteData.height);
		MarkVertexPositionDirty();
		MarkUVDirty();
	}
	else
	{
		Super::HeightChanged();
	}
}
void UUITexture::HeightChanged()
{
	if (!IsValid(texture))return;
	if (type == UITextureType::Tiled)
	{
		spriteData.ApplyUV(0, 0, widget.width, widget.height, 1.0f / spriteData.width, 1.0f / spriteData.height);
		MarkVertexPositionDirty();
		MarkUVDirty();
	}
	else
	{
		Super::HeightChanged();
	}
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
		Super::SetTexture(newTexture);
		CheckSpriteData();
	}
}

void UUITexture::SetFillMethod(UISpriteFillMethod newValue)
{
	if (fillMethod != newValue)
	{
		fillMethod = newValue;
		if (type == UITextureType::Filled)
		{
			MarkTriangleDirty();
		}
	}
}
void UUITexture::SetFillOrigin(uint8 newValue)
{
	if (fillOrigin != newValue)
	{
		fillOrigin = newValue;
		if (type == UITextureType::Filled)
		{
			if (fillMethod == UISpriteFillMethod::Radial90)
			{
				MarkVertexPositionDirty();
				MarkUVDirty();
			}
			else if (fillMethod == UISpriteFillMethod::Radial180 || fillMethod == UISpriteFillMethod::Radial360)
			{
				MarkTriangleDirty();
			}
		}
	}
}
void UUITexture::SetFillDirectionFlip(bool newValue)
{
	if (fillDirectionFlip != newValue)
	{
		fillDirectionFlip = newValue;
		if (type == UITextureType::Filled)
		{
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
}
void UUITexture::SetFillAmount(float newValue)
{
	if (fillAmount != newValue)
	{
		fillAmount = newValue;
		if (type == UITextureType::Filled)
		{
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
}