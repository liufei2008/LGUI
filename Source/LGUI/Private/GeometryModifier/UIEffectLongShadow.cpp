// Copyright 2019 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectLongShadow.h"
#include "LGUI.h"


UUIEffectLongShadow::UUIEffectLongShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectLongShadow::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
{
	auto& triangles = InGeometry->triangles;
	auto& vertices = InGeometry->vertices;
	auto& colors = InGeometry->colors;
	auto& uvs = InGeometry->uvs;

	auto vertexCount = vertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;

	const int32 singleChannelTriangleIndicesCount = InOutOriginTriangleIndicesCount;
	const int32 singleChannelVerticesCount = InOutOriginVerticesCount;
	if (singleChannelTriangleIndicesCount == triangleCount)
	{
		OutTriangleChanged = true;
		int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * (shadowSegment + 1);
		triangles.AddUninitialized(additionalTriangleIndicesCount);
		//put orgin triangles on last pass, this will make the origin triangle render at top
		for (int triangleIndex = additionalTriangleIndicesCount, originTriangleIndex = 0; originTriangleIndex < singleChannelTriangleIndicesCount; triangleIndex++, originTriangleIndex++)
		{
			auto index = triangles[originTriangleIndex];
			triangles[triangleIndex] = index;
		}
		//calculate other pass
		int32 prevChannelVerticesCount = singleChannelVerticesCount;
		int32 shadowChannelCount = shadowSegment + 1;
		for (int channelIndex = 0, originTriangleIndex = 0, triangleIndex = 0; channelIndex < shadowChannelCount; triangleIndex++, originTriangleIndex++)
		{
			auto index = triangles[originTriangleIndex + additionalTriangleIndicesCount] + prevChannelVerticesCount;
			triangles[triangleIndex] = index;
			if (originTriangleIndex + 1 == singleChannelTriangleIndicesCount)
			{
				channelIndex += 1;
				originTriangleIndex = -1;
				prevChannelVerticesCount += singleChannelVerticesCount;
			}
		}
	}
	InOutOriginTriangleIndicesCount = singleChannelTriangleIndicesCount * (shadowSegment + 2);

	if (singleChannelVerticesCount == vertexCount)
	{
		vertexCount = singleChannelVerticesCount * (shadowSegment + 2);
		vertices.Reserve(vertexCount);
		uvs.Reserve(vertexCount);
		colors.Reserve(vertexCount);
		for (int i = singleChannelVerticesCount; i < vertexCount; i++)
		{
			vertices.Add(FVector());
			uvs.Add(FVector2D());
			colors.Add(FColor());
		}
	}
	InOutOriginVerticesCount = singleChannelVerticesCount * (shadowSegment + 2);

	FVector shadowSizeInterval = shadowSize / (shadowSegment + 1);
	int32 shadowChannelCount = shadowSegment + 1;
	float alphaMultiply = multiplySourceAlpha ? (GetRenderableUIItem()->GetFinalAlpha() * 0.003921568627f/* 1 / 255 */) : 1.0f;
	for (int channelOriginVertIndex = 0; channelOriginVertIndex < singleChannelVerticesCount; channelOriginVertIndex++)
	{
		auto originVert = vertices[channelOriginVertIndex];
		auto originUV = uvs[channelOriginVertIndex];
		for (int channelIndex = 0; channelIndex < shadowChannelCount; channelIndex++)
		{
			int channelVertIndex = (channelIndex + 1) * singleChannelVerticesCount + channelOriginVertIndex;
			uvs[channelVertIndex] = originUV;
			auto& vert = vertices[channelVertIndex];
			vert = originVert;
			vert.X += shadowSizeInterval.X * (shadowChannelCount - channelIndex);
			vert.Y += shadowSizeInterval.Y * (shadowChannelCount - channelIndex);
			vert.Z += shadowSizeInterval.Z * (shadowChannelCount - channelIndex);
			auto color = shadowColor;
			if (useGradientColor)
			{
				float colorRatio = ((float)(channelIndex) / (shadowChannelCount));
				float colorRatio_INV = 1.0f - colorRatio;
				color.R = shadowColor.R * colorRatio + gradientColor.R * colorRatio_INV;
				color.G = shadowColor.G * colorRatio + gradientColor.G * colorRatio_INV;
				color.B = shadowColor.B * colorRatio + gradientColor.B * colorRatio_INV;
				color.A = shadowColor.A * colorRatio + gradientColor.A * colorRatio_INV;
			}
			if (multiplySourceAlpha)
			{
				color.A = (uint8)(alphaMultiply * color.A);
			}
			colors[channelVertIndex] = color;
		}
	}
}

void UUIEffectLongShadow::SetShadowColor(FColor newColor)
{
	shadowColor = newColor;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
}
void UUIEffectLongShadow::SetShadowSize(FVector newSize)
{
	shadowSize = newSize;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkVertexPositionDirty();
}
void UUIEffectLongShadow::SetShadowSegment(uint8 newSegment)
{
	shadowSegment = newSegment;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkTriangleDirty();
}
void UUIEffectLongShadow::SetUseGradientColor(bool newBool)
{
	useGradientColor = newBool;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
}
void UUIEffectLongShadow::SetGradientColor(FColor newColor)
{
	shadowColor = newColor;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
}