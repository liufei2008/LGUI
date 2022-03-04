// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
}
#if WITH_EDITOR
void UUITexture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
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
void UUITexture::EditorForceUpdate()
{
	Super::EditorForceUpdate();
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

void UUITexture::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	switch (type)
	{
	case UITextureType::Normal:
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case UITextureType::Sliced:
	case UITextureType::SlicedFrame:
		if (spriteData.HasBorder())
		{
			UIGeometry::UpdateUIRectBorderVertex(&InGeo, type == UITextureType::Sliced, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		else
		{
			UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
				this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		break;
	case UITextureType::Tiled:
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case UITextureType::Filled:
	{
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
		case UISpriteFillMethod::Vertical:
			UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, fillMethod == UISpriteFillMethod::Horizontal, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case UISpriteFillMethod::Radial90:
			UIGeometry::UpdateUIRectFillRadial90Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case UISpriteFillMethod::Radial180:
			UIGeometry::UpdateUIRectFillRadial180Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case UISpriteFillMethod::Radial360:
			UIGeometry::UpdateUIRectFillRadial360Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		}
	}
	break;
	}
}

void UUITexture::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
	if (!IsValid(texture))return;
	if (type == UITextureType::Tiled)
	{
        if (InSizeChange)
        {
            spriteData.ApplyUV(0, 0, this->GetWidth(), this->GetHeight(), 1.0f / spriteData.width, 1.0f / spriteData.height);
            MarkUVDirty();
        }
	}
    if (InPivotChange || InSizeChange)
    {
        MarkVertexPositionDirty();
    }
}


void UUITexture::SetTextureType(UITextureType newType)
{
	if (type != newType)
	{
		type = newType;
		MarkVerticesDirty(true, true, true, true);
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
			MarkVerticesDirty(true, true, true, true);
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
				MarkVerticesDirty(false, true, true, false);
			}
			else if (fillMethod == UISpriteFillMethod::Radial180 || fillMethod == UISpriteFillMethod::Radial360)
			{
				MarkVerticesDirty(true, true, true, true);
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
			MarkVerticesDirty(false, true, true, false);
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
			MarkVerticesDirty(false, true, true, false);
		}
	}
}