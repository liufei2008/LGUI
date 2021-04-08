// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectShadow.h"
#include "LGUI.h"


UUIEffectShadow::UUIEffectShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIEffectShadow::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
	bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
	)
{
	auto& triangles = InGeometry->triangles;
	auto& originPositions = InGeometry->originPositions;
	auto& vertices = InGeometry->vertices;

	auto vertexCount = originPositions.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;
	
	const int32 singleChannelTriangleIndicesCount = InOutOriginTriangleIndicesCount;
	const int32 singleChannelVerticesCount = InOutOriginVerticesCount;
	InOutOriginTriangleIndicesCount = singleChannelTriangleIndicesCount + singleChannelTriangleIndicesCount;
	if (singleChannelTriangleIndicesCount == triangleCount)//trangle count should have changed after apply this modifier
	{
		//create additional triangle pass
		OutTriangleChanged = true;
		triangles.AddUninitialized(singleChannelTriangleIndicesCount);
		//put orgin triangles on last pass, this will make the origin triangle render at top
		for (int i = singleChannelTriangleIndicesCount, j = 0; j < singleChannelTriangleIndicesCount; i++, j++)
		{
			auto index = triangles[j];
			triangles[i] = index;
			triangles[j] = index + singleChannelVerticesCount;
		}
	}
	else
	{
		//update triangle is slightly different from craete triangle data
		for (int i = singleChannelTriangleIndicesCount, j = 0; j < singleChannelTriangleIndicesCount; i++, j++)
		{
			triangles[j] = triangles[i] + singleChannelVerticesCount;
		}
	}
	
	if (singleChannelVerticesCount == vertexCount)
	{
		vertexCount = singleChannelVerticesCount + singleChannelVerticesCount;
		originPositions.Reserve(vertexCount);
		vertices.Reserve(vertexCount);
		for (int i = singleChannelVerticesCount; i < vertexCount; i++)
		{
			originPositions.Add(FVector());
			vertices.Add(FVector());
		}
	}
	InOutOriginVerticesCount = singleChannelVerticesCount + singleChannelVerticesCount;

	for (int channelIndex1 = singleChannelVerticesCount, channelIndexOrigin = 0; channelIndex1 < vertexCount; channelIndex1++, channelIndexOrigin++)
	{
		auto originVertPos = originPositions[channelIndexOrigin];
		originVertPos.X += shadowOffset.X;
		originVertPos.Y += shadowOffset.Y;
		originPositions[channelIndex1] = originVertPos;

		if (multiplySourceAlpha)
		{
			auto& vertColor = vertices[channelIndex1].Color;
			vertColor.A = (uint8)(UUIItem::Color255To1_Table[vertices[channelIndexOrigin].Color.A] * shadowColor.A);
			vertColor.R = shadowColor.R;
			vertColor.G = shadowColor.G;
			vertColor.B = shadowColor.B;
		}
		else
		{
			vertices[channelIndex1].Color = shadowColor;
		}

		vertices[channelIndex1].TextureCoordinate[0] = vertices[channelIndexOrigin].TextureCoordinate[0];
	}
}

void UUIEffectShadow::SetShadowColor(FColor newColor)
{
	if (shadowColor != newColor)
	{
		shadowColor = newColor;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
	}
}
void UUIEffectShadow::SetShadowOffset(FVector2D newOffset)
{
	if (shadowOffset != newOffset)
	{
		shadowOffset = newOffset;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkVertexPositionDirty();
	}
}