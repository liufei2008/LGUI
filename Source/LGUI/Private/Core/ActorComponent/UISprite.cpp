// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UISprite.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"


UUISprite::UUISprite()
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
		UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, sprite->GetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
		break;
	case UISpriteType::Sliced:
	case UISpriteType::SlicedFrame:
	{
		UIGeometry::FromUIRectBorder(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, sprite->GetSpriteInfo(), type == UISpriteType::Sliced, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
	}
	break;
	case UISpriteType::Tiled:
	{
		if (sprite->HavePackingTag())
		{
			WidthChanged();
			HeightChanged();
			UIGeometry::FromUIRectTiled(widget.width, widget.height, widget.pivot, GetFinalColor(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize, geometry, sprite->GetSpriteInfo(), RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
		}
		else
		{
			FLGUISpriteInfo tempSpriteInfo;
			tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / sprite->GetSpriteTexture()->GetSurfaceWidth(), 1.0f / sprite->GetSpriteTexture()->GetSurfaceHeight());
			UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, &tempSpriteInfo, RenderCanvas->GetRequireNormal(), RenderCanvas->GetRequireTangent(), RenderCanvas->GetRequireUV1());
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
			UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot);
			break;
		case UISpriteType::Sliced:
		case UISpriteType::SlicedFrame:
		{
			UIGeometry::UpdateUIRectBorderVertex(geometry, widget.width, widget.height, widget.pivot, sprite->GetSpriteInfo());
		}
		break;
		case UISpriteType::Tiled:
		{
			if (sprite->HavePackingTag())
			{
				UIGeometry::UpdateUIRectTiledVertex(geometry, sprite->GetSpriteInfo(), widget.width, widget.height, widget.pivot, Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot);
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
			UIGeometry::UpdateUIRectSimpleUV(geometry, sprite->GetSpriteInfo());
			break;
		case UISpriteType::Sliced:
		case UISpriteType::SlicedFrame:
		{
			UIGeometry::UpdateUIRectBorderUV(geometry, sprite->GetSpriteInfo());
		}
		break;
		case UISpriteType::Tiled:
		{
			if (sprite->HavePackingTag())
			{
				UIGeometry::UpdateUIRectTiledUV(geometry, sprite->GetSpriteInfo(), Tiled_WidthRectCount, Tiled_HeightRectCount, Tiled_WidthRemainedRectSize, Tiled_HeightRemainedRectSize);
			}
			else
			{
				FLGUISpriteInfo tempSpriteInfo;
				tempSpriteInfo.ApplyUV(0, 0, widget.width, widget.height, 1.0f / sprite->GetSpriteTexture()->GetSurfaceWidth(), 1.0f / sprite->GetSpriteTexture()->GetSurfaceHeight());
				UIGeometry::UpdateUIRectSimpleUV(geometry, &tempSpriteInfo);
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
	if (sprite == nullptr)return;
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
		float widthCountFloat = widget.width / sprite->GetSpriteInfo()->width;
		int widthCount = (int)widthCountFloat + 1;//rect count of width-direction, +1 means not-full-size rect
		if (widthCount != Tiled_WidthRectCount)
		{
			Tiled_WidthRectCount = widthCount;
			MarkTriangleDirty();
		}
		float remainedWidth = (widthCountFloat - (widthCount - 1)) * sprite->GetSpriteInfo()->width;//not-full-size rect's width
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
	if (sprite == nullptr)return;
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
		float heightCountFloat = widget.height / sprite->GetSpriteInfo()->height;
		int heightCount = (int)heightCountFloat + 1;//rect count of height-direction, +1 means not-full-size rect
		if (heightCount != Tiled_HeightRectCount)
		{
			Tiled_HeightRectCount = heightCount;
			MarkTriangleDirty();
		}
		float remainedHeight = (heightCountFloat - (heightCount - 1)) * sprite->GetSpriteInfo()->height;//not-full-size rect's height
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
