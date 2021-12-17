// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
			fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial180))
		{
			fillOrigin = (uint8)fillOriginType_Radial180;
			fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)fillOrigin;
			fillOriginType_Radial360 = (UISpriteFillOriginType_Radial360)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, fillOriginType_Radial360))
		{
			fillOrigin = (uint8)fillOriginType_Radial360;
			fillOriginType_Radial180 = (UISpriteFillOriginType_Radial180)fillOrigin;
			fillOriginType_Radial90 = (UISpriteFillOriginType_Radial90)fillOrigin;
		}
		else if (propName == GET_MEMBER_NAME_CHECKED(UUISprite, sprite))
		{
			if (IsValid(sprite))
			{
				if (sprite->GetSpriteInfo().HasBorder())
				{
					if (this->type == UISpriteType::Normal)
					{
						this->SetSpriteType(UISpriteType::Sliced);
					}
				}
			}
		}
	}
}
void UUISprite::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
}
#endif
void UUISprite::OnCreateGeometry()
{
	switch (type)
	{
	case UISpriteType::Normal:
		UIGeometry::FromUIRectSimple(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), RenderCanvas.Get(), this);
		break;
	case UISpriteType::Sliced:
	case UISpriteType::SlicedFrame:
	{
		UIGeometry::FromUIRectBorder(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), RenderCanvas.Get(), this, type == UISpriteType::Sliced);
	}
	break;
	case UISpriteType::Tiled:
	{
		if (!sprite->IsIndividual())
		{
			CalculateTiledWidth();
			CalculateTiledHeight();
			UIGeometry::FromUIRectTiled(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, geometry, sprite->GetSpriteInfo(), RenderCanvas.Get(), this);
		}
		else
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, this->GetWidth(), this->GetHeight(), 1.0f / sprite->GetSpriteInfo().width, 1.0f / sprite->GetSpriteInfo().height);
			UIGeometry::FromUIRectSimple(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, tempSpriteInfo, RenderCanvas.Get(), this);
		}
	}
	break;
	case UISpriteType::Filled:
	{
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
			UIGeometry::FromUIRectFillHorizontalVertical(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, true, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Vertical:
			UIGeometry::FromUIRectFillHorizontalVertical(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, false, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial90:
			UIGeometry::FromUIRectFillRadial90(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial180:
			UIGeometry::FromUIRectFillRadial180(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas.Get(), this);
			break;
		case UISpriteFillMethod::Radial360:
			UIGeometry::FromUIRectFillRadial360(this->GetWidth(), this->GetHeight(), this->GetPivot(), GetFinalColor(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas.Get(), this);
			break;
		}
	}
	break;
	}
}
void UUISprite::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	switch (type)
	{
	case UISpriteType::Normal:
	{
		if (InVertexPositionChanged)
		{
			UIGeometry::UpdateUIRectSimpleVertex(geometry, this->GetWidth(), this->GetHeight(), this->GetPivot(), sprite->GetSpriteInfo(), RenderCanvas.Get(), this);
		}
		if (InVertexUVChanged)
		{
			UIGeometry::UpdateUIRectSimpleUV(geometry, sprite->GetSpriteInfo());
		}
	}
	break;
	case UISpriteType::Sliced:
	case UISpriteType::SlicedFrame:
	{
		if (InVertexPositionChanged)
		{
			UIGeometry::UpdateUIRectBorderVertex(geometry, this->GetWidth(), this->GetHeight(), this->GetPivot(), sprite->GetSpriteInfo(), RenderCanvas.Get(), this);
		}
		if (InVertexUVChanged)
		{
			UIGeometry::UpdateUIRectBorderUV(geometry, sprite->GetSpriteInfo());
		}
	}
	break;
	case UISpriteType::Tiled:
	{
		if (InVertexPositionChanged)
		{
			if (!sprite->IsIndividual())
			{
				UIGeometry::UpdateUIRectTiledVertex(geometry, sprite->GetSpriteInfo(), RenderCanvas.Get(), this, this->GetWidth(), this->GetHeight(), this->GetPivot(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				UIGeometry::UpdateUIRectSimpleVertex(geometry, this->GetWidth(), this->GetHeight(), this->GetPivot(), sprite->GetSpriteInfo(), RenderCanvas.Get(), this);
			}
		}
		if (InVertexUVChanged)
		{
			if (!sprite->IsIndividual())
			{
				UIGeometry::UpdateUIRectTiledUV(geometry, sprite->GetSpriteInfo(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				FLGUISpriteInfo tempSpriteInfo;
				tempSpriteInfo.ApplyUV(0, 0, this->GetWidth(), this->GetHeight(), 1.0f / sprite->GetSpriteInfo().width, 1.0f / sprite->GetSpriteInfo().height);
				UIGeometry::UpdateUIRectSimpleUV(geometry, tempSpriteInfo);
			}
		}
	}
	break;
	case UISpriteType::Filled:
	{
		if (InVertexPositionChanged || InVertexUVChanged)
		{
			switch (fillMethod)
			{
			case UISpriteFillMethod::Horizontal:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(this->GetWidth(), this->GetHeight(), this->GetPivot(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, true, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Vertical:
				UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(this->GetWidth(), this->GetHeight(), this->GetPivot(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, false, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial90:
				UIGeometry::UpdateUIRectFillRadial90Vertex(this->GetWidth(), this->GetHeight(), this->GetPivot(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial180:
				UIGeometry::UpdateUIRectFillRadial180Vertex(this->GetWidth(), this->GetHeight(), this->GetPivot(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			case UISpriteFillMethod::Radial360:
				UIGeometry::UpdateUIRectFillRadial360Vertex(this->GetWidth(), this->GetHeight(), this->GetPivot(), geometry, sprite->GetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas.Get(), this);
				break;
			}
		}
	}
	break;
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
}

void UUISprite::MarkLayoutDirty(bool InTransformChange, bool InPivotChange, bool InSizeChange, bool DoPropergateLayoutChange)
{
    Super::MarkLayoutDirty(InTransformChange, InPivotChange, InSizeChange, DoPropergateLayoutChange);
	if (!IsValid(sprite))return;
	if (type == UISpriteType::Tiled)
	{
        if (InSizeChange)
        {
			CalculateTiledWidth();
			CalculateTiledHeight();
        }
	}
    else
    {
        if (InPivotChange || InSizeChange)
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
				MarkTriangleDirty();
				MarkVertexPositionDirty();
				MarkUVDirty();
			}
			return;
		}
		float widthCountFloat = this->GetWidth() / sprite->GetSpriteInfo().width;
		int widthCount = (int)widthCountFloat + 1;//rect count of width-direction, +1 means not-full-size rect
		if (widthCount != Tiled_WidthRectCount)
		{
			Tiled_WidthRectCount = widthCount;
			MarkTriangleDirty();
		}
		float remainedWidth = (widthCountFloat - (widthCount - 1)) * sprite->GetSpriteInfo().width;//not-full-size rect's width
		if (remainedWidth != Tiled_WidthRemainedRectSize)
		{
			Tiled_WidthRemainedRectSize = remainedWidth;
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
	else
	{
		MarkVertexPositionDirty();
		MarkUVDirty();
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
				MarkTriangleDirty();
				MarkVertexPositionDirty();
				MarkUVDirty();
			}
			return;
		}
		float heightCountFloat = this->GetHeight() / sprite->GetSpriteInfo().height;
		int heightCount = (int)heightCountFloat + 1;//rect count of height-direction, +1 means not-full-size rect
		if (heightCount != Tiled_HeightRectCount)
		{
			Tiled_HeightRectCount = heightCount;
			MarkTriangleDirty();
		}
		float remainedHeight = (heightCountFloat - (heightCount - 1)) * sprite->GetSpriteInfo().height;//not-full-size rect's height
		if (remainedHeight != Tiled_HeightRemainedRectSize)
		{
			Tiled_HeightRemainedRectSize = remainedHeight;
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
	else
	{
		MarkVertexPositionDirty();
		MarkUVDirty();
	}
}

void UUISprite::SetSpriteType(UISpriteType newType) {
	if (type != newType)
	{
		type = newType;
		MarkTriangleDirty();
		if (type == UISpriteType::Tiled)
		{
			CalculateTiledWidth();
			CalculateTiledHeight();
		}
	}
}
void UUISprite::SetFillMethod(UISpriteFillMethod newValue)
{
	if (fillMethod != newValue)
	{
		fillMethod = newValue;
		if (type == UISpriteType::Filled)
		{
			MarkTriangleDirty();
		}
	}
}
void UUISprite::SetFillOrigin(uint8 newValue)
{
	if (fillOrigin != newValue)
	{
		fillOrigin = newValue;
		if (type == UISpriteType::Filled)
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
void UUISprite::SetFillDirectionFlip(bool newValue)
{
	if (fillDirectionFlip != newValue)
	{
		fillDirectionFlip = newValue;
		if (type == UISpriteType::Filled)
		{
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
}
void UUISprite::SetFillAmount(float newValue)
{
	if (fillAmount != newValue)
	{
		fillAmount = newValue;
		if (type == UISpriteType::Filled)
		{
			MarkVertexPositionDirty();
			MarkUVDirty();
		}
	}
}
