// Copyright 2019 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectShadow.h"
#include "LGUI.h"


UUIEffectShadow::UUIEffectShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UUIEffectShadow::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	if (!GetRenderableUIItem())return;
	auto& triangles = InGeometry->triangles;
	auto& originPositions = InGeometry->originPositions;
	auto& vertices = InGeometry->vertices;

	auto vertexCount = originPositions.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;
	
	const int32 singleChannelTriangleIndicesCount = InOutOriginTriangleIndicesCount;
	const int32 singleChannelVerticesCount = InOutOriginVerticesCount;
	if (singleChannelTriangleIndicesCount == triangleCount)//trangle count should have changed after apply this modifier
	{
		OutTriangleChanged = true;
		triangleCount = singleChannelTriangleIndicesCount + singleChannelTriangleIndicesCount;
		triangles.Reserve(triangleCount);
		//put orgin triangles on last pass, this will make the origin triangle render at top
		for (int i = singleChannelTriangleIndicesCount, j = 0; i < triangleCount; i++, j++)
		{
			auto index = triangles[j];
			triangles.Add(index);
			triangles[j] = triangles[j] + singleChannelVerticesCount;
		}
	}
	InOutOriginTriangleIndicesCount = singleChannelTriangleIndicesCount + singleChannelTriangleIndicesCount;
	
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
	FColor finalColor = shadowColor;
	if (multiplySourceAlpha)
	{
		finalColor.A = (uint8)(GetRenderableUIItem()->GetFinalAlpha() * shadowColor.A * 0.003921568627f/* 1 / 255 */);
	}
	for (int channelIndex1 = singleChannelVerticesCount, channelIndexOrigin = 0; channelIndex1 < vertexCount; channelIndex1++, channelIndexOrigin++)
	{
		auto originVertPos = originPositions[channelIndexOrigin];
		originVertPos.X += shadowOffset.X;
		originVertPos.Y += shadowOffset.Y;
		originPositions[channelIndex1] = originVertPos;

		vertices[channelIndex1].Color = finalColor;

		vertices[channelIndex1].TextureCoordinate[0] = vertices[channelIndexOrigin].TextureCoordinate[0];
	}
}

void UUIEffectShadow::SetShadowColor(FColor newColor)
{
	shadowColor = newColor;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
}
void UUIEffectShadow::SetShadowOffset(FVector2D newOffset)
{
	shadowOffset = newOffset;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkVertexPositionDirty();
}