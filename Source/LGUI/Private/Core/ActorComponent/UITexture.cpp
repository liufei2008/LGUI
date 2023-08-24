// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UITexture.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif
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
			fillOriginType_Radial180 = (EUISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial360 = (EUISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial180))
		{
			fillOrigin = (uint8)fillOriginType_Radial180;
			fillOriginType_Radial90 = (EUISpriteFillOriginType_Radial90)fillOrigin;
			fillOriginType_Radial360 = (EUISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUITexture, fillOriginType_Radial360))
		{
			fillOrigin = (uint8)fillOriginType_Radial360;
			fillOriginType_Radial180 = (EUISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial90 = (EUISpriteFillOriginType_Radial90)fillOrigin;
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
		if (type != EUITextureType::Tiled)
		{
			ApplyUVRect();
			spriteData.ApplyUV(0, 0, spriteData.width, spriteData.height, 1.0f / spriteData.width, 1.0f / spriteData.height, uvRect);
			spriteData.ApplyBorderUV(1.0f / spriteData.width, 1.0f / spriteData.height);
		}
	}
}

void UUITexture::ApplyUVRect()
{
	switch (UVRectControlMode)
	{
	default:
	case EUITextureUVRectControlMode::None:
		break;
	case EUITextureUVRectControlMode::KeepAspectRatio_FitIn:
	{
		auto TextureWidth = texture->GetSurfaceWidth();
		auto TextureHeight = texture->GetSurfaceHeight();
		auto TextureAspect = TextureWidth / TextureHeight;
		auto ThisWidth = this->GetWidth();
		auto ThisHeight = this->GetHeight();
		auto ThisAspect = ThisWidth / ThisHeight;
		if (TextureAspect > ThisAspect)
		{
			auto VerticalScale = TextureAspect / ThisAspect;
			auto VerticalOffset = (1.0f - VerticalScale) * 0.5f;
			uvRect = FVector4(0, VerticalOffset, 1, VerticalScale);
		}
		else
		{
			auto HorizontalScale = ThisAspect / TextureAspect;
			auto HorizontalOffset = (1.0f - HorizontalScale) * 0.5f;
			uvRect = FVector4(HorizontalOffset, 0, HorizontalScale, 1);
		}
	}
	break;
	case EUITextureUVRectControlMode::KeepAspectRatio_Envelope:
	{
		auto TextureWidth = texture->GetSurfaceWidth();
		auto TextureHeight = texture->GetSurfaceHeight();
		auto TextureAspect = TextureWidth / TextureHeight;
		auto ThisWidth = this->GetWidth();
		auto ThisHeight = this->GetHeight();
		auto ThisAspect = ThisWidth / ThisHeight;
		if (TextureAspect > ThisAspect)
		{
			auto HorizontalScale = ThisAspect / TextureAspect;
			auto HorizontalOffset = (1.0f - HorizontalScale) * 0.5f;
			uvRect = FVector4(HorizontalOffset, 0, HorizontalScale, 1);
		}
		else
		{
			auto VerticalScale = TextureAspect / ThisAspect;
			auto VerticalOffset = (1.0f - VerticalScale) * 0.5f;
			uvRect = FVector4(0, VerticalOffset, 1, VerticalScale);
		}
	}
	break;
	}
}

void UUITexture::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	switch (type)
	{
	case EUITextureType::Normal:
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case EUITextureType::Sliced:
	case EUITextureType::SlicedFrame:
		if (spriteData.HasBorder())
		{
			UIGeometry::UpdateUIRectBorderVertex(&InGeo, type == EUITextureType::Sliced, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
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
	case EUITextureType::Tiled:
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, RenderCanvas.Get(), this, GetFinalColor(),
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case EUITextureType::Filled:
	{
		switch (fillMethod)
		{
		case EUISpriteFillMethod::Horizontal:
		case EUISpriteFillMethod::Vertical:
			UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, fillMethod == EUISpriteFillMethod::Horizontal, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial90:
			UIGeometry::UpdateUIRectFillRadial90Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (EUISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial180:
			UIGeometry::UpdateUIRectFillRadial180Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (EUISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial360:
			UIGeometry::UpdateUIRectFillRadial360Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), spriteData, fillDirectionFlip, fillAmount, (EUISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		}
	}
	break;
	}
}

void UUITexture::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
	if (!IsValid(texture))return;
	if (type == EUITextureType::Tiled)
	{
        if (InWidthChange || InHeightChange)
        {
            spriteData.ApplyUV(0, 0, this->GetWidth(), this->GetHeight(), 1.0f / spriteData.width, 1.0f / spriteData.height);
            MarkUVDirty();
        }
	}
	if (UVRectControlMode != EUITextureUVRectControlMode::None)
	{
		if (InWidthChange || InHeightChange)
		{
			CheckSpriteData();
			MarkUVDirty();
		}
	}
    if (InPivotChange || InWidthChange || InHeightChange)
    {
        MarkVertexPositionDirty();
    }
}


void UUITexture::SetTextureType(EUITextureType newType)
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
		if (UVRectControlMode == EUITextureUVRectControlMode::KeepAspectRatio_FitIn
			|| UVRectControlMode == EUITextureUVRectControlMode::KeepAspectRatio_Envelope
			)
		{
			MarkUVDirty();
		}
		CheckSpriteData();
	}
}

void UUITexture::SetFillMethod(EUISpriteFillMethod newValue)
{
	if (fillMethod != newValue)
	{
		fillMethod = newValue;
		if (type == EUITextureType::Filled)
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
		if (type == EUITextureType::Filled)
		{
			if (fillMethod == EUISpriteFillMethod::Radial90)
			{
				MarkVerticesDirty(false, true, true, false);
			}
			else if (fillMethod == EUISpriteFillMethod::Radial180 || fillMethod == EUISpriteFillMethod::Radial360)
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
		if (type == EUITextureType::Filled)
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
		if (type == EUITextureType::Filled)
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}
void UUITexture::SetUVRectControlMode(EUITextureUVRectControlMode newValue)
{
	if (UVRectControlMode != newValue)
	{
		UVRectControlMode = newValue;
		MarkUVDirty();
		CheckSpriteData();
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
