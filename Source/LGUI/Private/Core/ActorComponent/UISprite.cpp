// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISprite.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"


UUISprite::UUISprite(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISprite::BeginPlay()
{
	Super::BeginPlay();
	WidthChanged();
	HeightChanged();
}
#if WITH_EDITOR
void UUISprite::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	WidthChanged();
	HeightChanged();
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
	}
}
void UUISprite::EditorForceUpdateImmediately()
{
	Super::EditorForceUpdateImmediately();
	WidthChanged();
	HeightChanged();
}
#endif
void UUISprite::OnCreateGeometry()
{
	switch (type)
	{
	case UISpriteType::Normal:
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas, this);
		break;
	case UISpriteType::Sliced:
	case UISpriteType::SlicedFrame:
	{
		UIGeometry::FromUIRectBorder(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas, this, type == UISpriteType::Sliced);
	}
	break;
	case UISpriteType::Tiled:
	{
		if (sprite->HavePackingTag())
		{
			WidthChanged();
			HeightChanged();
			UIGeometry::FromUIRectTiled(widget.width, widget.height, widget.pivot, GetFinalColor(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas, this);
		}
		else
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / sprite->InitAndGetSpriteInfo().width, 1.0f / sprite->InitAndGetSpriteInfo().height);
			UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, tempSpriteInfo, RenderCanvas, this);
		}
	}
	break;
	case UISpriteType::Filled:
	{
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
			UIGeometry::FromUIRectFillHorizontalVertical(widget.width, widget.height, widget.pivot, widget.color, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, true, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Vertical:
			UIGeometry::FromUIRectFillHorizontalVertical(widget.width, widget.height, widget.pivot, widget.color, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, false, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial90:
			UIGeometry::FromUIRectFillRadial90(widget.width, widget.height, widget.pivot, widget.color, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial180:
			UIGeometry::FromUIRectFillRadial180(widget.width, widget.height, widget.pivot, widget.color, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial360:
			UIGeometry::FromUIRectFillRadial360(widget.width, widget.height, widget.pivot, widget.color, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, RenderCanvas, this);
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
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, RenderCanvas, this);
		}
		if (InVertexUVChanged)
		{
			UIGeometry::UpdateUIRectSimpleUV(geometry, sprite->InitAndGetSpriteInfo());
		}
	}
	break;
	case UISpriteType::Sliced:
	case UISpriteType::SlicedFrame:
	{
		if (InVertexPositionChanged)
		{
			UIGeometry::UpdateUIRectBorderVertex(geometry, widget.width, widget.height, widget.pivot, sprite->InitAndGetSpriteInfo(), RenderCanvas, this);
		}
		if (InVertexUVChanged)
		{
			UIGeometry::UpdateUIRectBorderUV(geometry, sprite->InitAndGetSpriteInfo());
		}
	}
	break;
	case UISpriteType::Tiled:
	{
		if (InVertexPositionChanged)
		{
			if (sprite->HavePackingTag())
			{
				UIGeometry::UpdateUIRectTiledVertex(geometry, sprite->InitAndGetSpriteInfo(), RenderCanvas, this, widget.width, widget.height, widget.pivot, Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, RenderCanvas, this);
			}
		}
		if (InVertexUVChanged)
		{
			if (sprite->HavePackingTag())
			{
				UIGeometry::UpdateUIRectTiledUV(geometry, sprite->InitAndGetSpriteInfo(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				FLGUISpriteInfo tempSpriteInfo;
				tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / sprite->InitAndGetSpriteInfo().width, 1.0f / sprite->InitAndGetSpriteInfo().height);
				UIGeometry::UpdateUIRectSimpleUV(geometry, tempSpriteInfo);
			}
		}
	}
	break;
	case UISpriteType::Filled:
	{
		switch (fillMethod)
		{
		case UISpriteFillMethod::Horizontal:
			UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, true, InVertexPositionChanged, InVertexUVChanged, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Vertical:
			UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(widget.width, widget.height, widget.pivot, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, false, InVertexPositionChanged, InVertexUVChanged, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial90:
			UIGeometry::UpdateUIRectFillRadial90Vertex(widget.width, widget.height, widget.pivot, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial90)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial180:
			UIGeometry::UpdateUIRectFillRadial180Vertex(widget.width, widget.height, widget.pivot, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial180)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas, this);
			break;
		case UISpriteFillMethod::Radial360:
			UIGeometry::UpdateUIRectFillRadial360Vertex(widget.width, widget.height, widget.pivot, geometry, sprite->InitAndGetSpriteInfo(), fillDirectionFlip, fillAmount, (UISpriteFillOriginType_Radial360)fillOrigin, InVertexPositionChanged, InVertexUVChanged, RenderCanvas, this);
			break;
		}
	}
	break;
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
}

void UUISprite::WidthChanged()
{
	if (!IsValid(sprite))return;
	if (type != UISpriteType::Tiled)return;
	if (sprite->HavePackingTag())
	{
		if (widget.width <= 0)
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
		float widthCountFloat = widget.width / sprite->InitAndGetSpriteInfo().width;
		int widthCount = (int)widthCountFloat + 1;//rect count of width-direction, +1 means not-full-size rect
		if (widthCount != Tiled_WidthRectCount)
		{
			Tiled_WidthRectCount = widthCount;
			MarkTriangleDirty();
		}
		float remainedWidth = (widthCountFloat - (widthCount - 1)) * sprite->InitAndGetSpriteInfo().width;//not-full-size rect's width
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
void UUISprite::HeightChanged()
{
	if (!IsValid(sprite))return;
	if (type != UISpriteType::Tiled)return;
	if (sprite->HavePackingTag())
	{
		if (widget.height <= 0)
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
		float heightCountFloat = widget.height / sprite->InitAndGetSpriteInfo().height;
		int heightCount = (int)heightCountFloat + 1;//rect count of height-direction, +1 means not-full-size rect
		if (heightCount != Tiled_HeightRectCount)
		{
			Tiled_HeightRectCount = heightCount;
			MarkTriangleDirty();
		}
		float remainedHeight = (heightCountFloat - (heightCount - 1)) * sprite->InitAndGetSpriteInfo().height;//not-full-size rect's height
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
			WidthChanged();
			HeightChanged();
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
