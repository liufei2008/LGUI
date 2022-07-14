// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectShadow.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"


UUIEffectShadow::UUIEffectShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIEffectShadow::ModifyUIGeometry(
	UIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto& triangles = InGeometry.triangles;
	auto& originVertices = InGeometry.originVertices;
	auto& vertices = InGeometry.vertices;

	auto vertexCount = originVertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;
	
	const int32 singleChannelTriangleIndicesCount = triangleCount;
	const int32 singleChannelVerticesCount = vertexCount;
	//create additional triangle pass
	triangles.AddUninitialized(singleChannelTriangleIndicesCount);
	//put orgin triangles on last pass, this will make the origin triangle render at top
	for (int i = singleChannelTriangleIndicesCount, j = 0; j < singleChannelTriangleIndicesCount; i++, j++)
	{
		auto index = triangles[j];
		triangles[i] = index;
		triangles[j] = index + singleChannelVerticesCount;
	}
	
	vertexCount = singleChannelVerticesCount + singleChannelVerticesCount;
	originVertices.AddDefaulted(singleChannelVerticesCount);
	vertices.AddDefaulted(singleChannelVerticesCount);

	for (int channelIndex1 = singleChannelVerticesCount, channelIndexOrigin = 0; channelIndex1 < vertexCount; channelIndex1++, channelIndexOrigin++)
	{
		auto originVertPos = originVertices[channelIndexOrigin].Position;
		originVertPos.Y += shadowOffset.X;
		originVertPos.Z += shadowOffset.Y;
		originVertices[channelIndex1].Position = originVertPos;

		if (multiplySourceAlpha)
		{
			auto& vertColor = vertices[channelIndex1].Color;
			vertColor.A = (uint8)(LGUIUtils::Color255To1_Table[vertices[channelIndexOrigin].Color.A] * shadowColor.A);
			vertColor.R = shadowColor.R;
			vertColor.G = shadowColor.G;
			vertColor.B = shadowColor.B;
		}
		else
		{
			vertices[channelIndex1].Color = shadowColor;
		}

		for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
		{
			vertices[channelIndex1].TextureCoordinate[i] = vertices[channelIndexOrigin].TextureCoordinate[i];
		}
	}
}

void UUIEffectShadow::SetShadowColor(FColor newColor)
{
	if (shadowColor != newColor)
	{
		shadowColor = newColor;
		if (GetUIRenderable())GetUIRenderable()->MarkColorDirty();
	}
}
void UUIEffectShadow::SetShadowOffset(FVector2D newOffset)
{
	if (shadowOffset != newOffset)
	{
		shadowOffset = newOffset;
		if (GetUIRenderable())GetUIRenderable()->MarkVertexPositionDirty();
	}
}