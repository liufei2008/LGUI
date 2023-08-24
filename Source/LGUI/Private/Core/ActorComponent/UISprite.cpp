// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISprite.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData_BaseObject.h"


UUISprite::UUISprite(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISprite::BeginPlay()
{
	Super::BeginPlay();
}
#if WITH_EDITOR
void UUISprite::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto propName = Property->GetFName();
		if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial90))
		{
			fillOrigin = (uint8)fillOriginType_Radial90;
			fillOriginType_Radial180 = (EUISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial360 = (EUISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial180))
		{
			fillOrigin = (uint8)fillOriginType_Radial180;
			fillOriginType_Radial90 = (EUISpriteFillOriginType_Radial90)fillOrigin;
			fillOriginType_Radial360 = (EUISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial360))
		{
			fillOrigin = (uint8)fillOriginType_Radial360;
			fillOriginType_Radial180 = (EUISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial90 = (EUISpriteFillOriginType_Radial90)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, sprite))
		{
			if (IsValid(sprite))
			{
				if (sprite->GetSpriteInfo().HasBorder())
				{
					if (this->type == EUISpriteType::Normal)
					{
						this->SetSpriteType(EUISpriteType::Sliced);
					}
				}
			}
		}
	}
}
#endif

void UUISprite::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	switch (type)
	{
	case EUISpriteType::Normal:
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo, 
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), sprite->GetSpriteInfo(), RenderCanvas.Get(), this, GetFinalColor(), 
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
		break;
	case EUISpriteType::Sliced:
	case EUISpriteType::SlicedFrame:
		if (sprite->GetSpriteInfo().HasBorder())
		{
			UIGeometry::UpdateUIRectBorderVertex(&InGeo, type == EUISpriteType::Sliced, this->GetWidth(), this->GetHeight(), FVector2D(this->GetPivot()), sprite->GetSpriteInfo(), RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		else
		{
			UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
				this->GetWidth(), this->GetHeight(), this->GetPivot(), sprite->GetSpriteInfo(), RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
	break;
	case EUISpriteType::Tiled:
		if (!sprite->IsIndividual())
		{
			UIGeometry::UpdateUIRectTiledVertex(&InGeo, sprite->GetSpriteInfo(), RenderCanvas.Get(), this, this->GetWidth(), this->GetHeight(), this->GetPivot(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, GetFinalColor(), 
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		else
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, this->GetWidth(), this->GetHeight(), 1.0f / sprite->GetSpriteInfo().width, 1.0f / sprite->GetSpriteInfo().height);
			UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
				this->GetWidth(), this->GetHeight(), this->GetPivot(), tempSpriteInfo, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
		}
		break;
	case EUISpriteType::Filled:
	{
		switch (fillMethod)
		{
		case EUISpriteFillMethod::Horizontal:
		case EUISpriteFillMethod::Vertical:
			UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2D(this->GetPivot()), sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, fillMethod == UISpriteFillMethod::Horizontal, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial90:
			UIGeometry::UpdateUIRectFillRadial90Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2D(this->GetPivot()), sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial180:
			UIGeometry::UpdateUIRectFillRadial180Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2D(this->GetPivot()), sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		case EUISpriteFillMethod::Radial360:
			UIGeometry::UpdateUIRectFillRadial360Vertex(&InGeo, this->GetWidth(), this->GetHeight(), FVector2D(this->GetPivot()), sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas.Get(), this, GetFinalColor(),
				InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
			);
			break;
		}
	}
	break;
	}
}

void UUISprite::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
	if (!IsValid(sprite))return;
	if (type == EUISpriteType::Tiled)
	{
        if (InWidthChange)
        {
			CalculateTiledWidth();
        }
		if (InHeightChange)
		{
			CalculateTiledHeight();
		}
	}
    else
    {
        if (InPivotChange || InWidthChange || InHeightChange)
        {
			MarkVertexPositionDirty();
		}
    }
}

void UUISprite::CalculateTiledWidth()
{
	if (!sprite->IsIndividual())
	{
		if (this->GetWidth() <= 0)
		{
			if (Tiled_WidthRectCount != 0)
			{
				Tiled_WidthRectCount = 0;
				Tiled_WidthRemainedRectSize = 0;
				MarkVerticesDirty(true, true, true, false);
			}
			return;
		}
		float widthCountFloat = this->GetWidth() / sprite->GetSpriteInfo().width;
		int widthCount = (int)widthCountFloat + 1;//rect count of width-direction, +1 means not-full-size rect
		if (widthCount != Tiled_WidthRectCount)
		{
			Tiled_WidthRectCount = widthCount;
			MarkVerticesDirty(true, true, true, false);
		}
		float remainedWidth = (widthCountFloat - (widthCount - 1)) * sprite->GetSpriteInfo().width;//not-full-size rect's width
		if (remainedWidth != Tiled_WidthRemainedRectSize)
		{
			Tiled_WidthRemainedRectSize = remainedWidth;
			MarkVerticesDirty(false, true, true, false);
		}
	}
	else
	{
		MarkVerticesDirty(false, true, true, false);
	}
}
void UUISprite::CalculateTiledHeight()
{
	if (!sprite->IsIndividual())
	{
		if (this->GetHeight() <= 0)
		{
			if (Tiled_HeightRectCount != 0)
			{
				Tiled_HeightRectCount = 0;
				Tiled_HeightRemainedRectSize = 0;
				MarkVerticesDirty(true, true, true, false);
			}
			return;
		}
		float heightCountFloat = this->GetHeight() / sprite->GetSpriteInfo().height;
		int heightCount = (int)heightCountFloat + 1;//rect count of height-direction, +1 means not-full-size rect
		if (heightCount != Tiled_HeightRectCount)
		{
			Tiled_HeightRectCount = heightCount;
			MarkVerticesDirty(true, true, true, false);
		}
		float remainedHeight = (heightCountFloat - (heightCount - 1)) * sprite->GetSpriteInfo().height;//not-full-size rect's height
		if (remainedHeight != Tiled_HeightRemainedRectSize)
		{
			Tiled_HeightRemainedRectSize = remainedHeight;
			MarkVerticesDirty(false, true, true, false);
		}
	}
	else
	{
		MarkVerticesDirty(false, true, true, false);
	}
}

void UUISprite::SetSpriteType(EUISpriteType newType) {
	if (type != newType)
	{
		type = newType;
		MarkVerticesDirty(true, true, true, true);
		if (type == EUISpriteType::Tiled)
		{
			CalculateTiledWidth();
			CalculateTiledHeight();
		}
	}
}
void UUISprite::SetFillMethod(EUISpriteFillMethod newValue)
{
	if (fillMethod != newValue)
	{
		fillMethod = newValue;
		if (type == EUISpriteType::Filled)
		{
			MarkVerticesDirty(true, true, true, true);
		}
	}
}
void UUISprite::SetFillOrigin(uint8 newValue)
{
	if (fillOrigin != newValue)
	{
		fillOrigin = newValue;
		if (type == EUISpriteType::Filled)
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
void UUISprite::SetFillDirectionFlip(bool newValue)
{
	if (fillDirectionFlip != newValue)
	{
		fillDirectionFlip = newValue;
		if (type == EUISpriteType::Filled)
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}
void UUISprite::SetFillAmount(float newValue)
{
	if (fillAmount != newValue)
	{
		fillAmount = newValue;
		if (type == EUISpriteType::Filled)
		{
			MarkVerticesDirty(false, true, true, false);
		}
	}
}
