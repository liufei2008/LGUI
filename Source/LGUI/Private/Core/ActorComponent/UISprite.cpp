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
	}
}
void UUISprite::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexPositionChanged)
	{
		switch (type)
		{
		case UISpriteType::Normal:
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, RenderCanvas, this);
			break;
		case UISpriteType::Sliced:
		case UISpriteType::SlicedFrame:
		{
			UIGeometry::UpdateUIRectBorderVertex(geometry, widget.width, widget.height, widget.pivot, sprite->InitAndGetSpriteInfo(), RenderCanvas, this);
		}
		break;
		case UISpriteType::Tiled:
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
			break;
		}
	}
	if (InVertexUVChanged)
	{
		switch (type)
		{
		case UISpriteType::Normal:
			UIGeometry::UpdateUIRectSimpleUV(geometry, sprite->InitAndGetSpriteInfo());
			break;
		case UISpriteType::Sliced:
		case UISpriteType::SlicedFrame:
		{
			UIGeometry::UpdateUIRectBorderUV(geometry, sprite->InitAndGetSpriteInfo());
		}
		break;
		case UISpriteType::Tiled:
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
			break;
		}
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
