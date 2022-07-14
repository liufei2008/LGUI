// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectLongShadow.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"


UUIEffectLongShadow::UUIEffectLongShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectLongShadow::ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha)
{
	if (multiplySourceAlpha)
	{
		InOutColor.A = (uint8)(LGUIUtils::Color255To1_Table[InOriginAlpha] * InTintColor.A);
		InOutColor.R = InTintColor.R;
		InOutColor.G = InTintColor.G;
		InOutColor.B = InTintColor.B;
	}
	else
	{
		InOutColor = InTintColor;
	}
}
void UUIEffectLongShadow::ModifyUIGeometry(
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

	int additionalVertCount = singleChannelVerticesCount * (shadowSegment + 1);
	vertexCount = singleChannelVerticesCount + additionalVertCount;
	originVertices.AddDefaulted(additionalVertCount);
	vertices.AddDefaulted(additionalVertCount);

	//verticies
	{
		FVector shadowSizeInterval = shadowSize / (shadowSegment + 1);
		for (int channelOriginVertIndex = 0; channelOriginVertIndex < singleChannelVerticesCount; channelOriginVertIndex++)
		{
			auto originVert = originVertices[channelOriginVertIndex].Position;
			auto originUV0 = vertices[channelOriginVertIndex].TextureCoordinate[0];
			auto originUV1 = vertices[channelOriginVertIndex].TextureCoordinate[1];
			auto originUV2 = vertices[channelOriginVertIndex].TextureCoordinate[2];
			auto originUV3 = vertices[channelOriginVertIndex].TextureCoordinate[3];
			auto originAlpha = vertices[channelOriginVertIndex].Color.A;
			for (int channelIndex = 0; channelIndex < shadowChannelCount; channelIndex++)
			{
				int channelVertIndex = (channelIndex + 1) * singleChannelVerticesCount + channelOriginVertIndex;
				vertices[channelVertIndex].TextureCoordinate[0] = originUV0;
				vertices[channelVertIndex].TextureCoordinate[1] = originUV1;
				vertices[channelVertIndex].TextureCoordinate[2] = originUV2;
				vertices[channelVertIndex].TextureCoordinate[3] = originUV3;
				auto& vert = originVertices[channelVertIndex].Position;
				vert = originVert;
				vert.X += shadowSizeInterval.X * (shadowChannelCount - channelIndex);
				vert.Y += shadowSizeInterval.Y * (shadowChannelCount - channelIndex);
				vert.Z += shadowSizeInterval.Z * (shadowChannelCount - channelIndex);
				
				if (useGradientColor)
				{
					float colorRatio = ((float)(channelIndex) / (shadowChannelCount));
					float colorRatio_INV = 1.0f - colorRatio;
					FColor color;
					color.R = shadowColor.R * colorRatio + gradientColor.R * colorRatio_INV;
					color.G = shadowColor.G * colorRatio + gradientColor.G * colorRatio_INV;
					color.B = shadowColor.B * colorRatio + gradientColor.B * colorRatio_INV;
					color.A = shadowColor.A * colorRatio + gradientColor.A * colorRatio_INV;
					ApplyColorAndAlpha(vertices[channelVertIndex].Color, color, originAlpha);
				}
				else
				{
					ApplyColorAndAlpha(vertices[channelVertIndex].Color, shadowColor, originAlpha);
				}
			}
		}
	}
}

void UUIEffectLongShadow::SetShadowColor(FColor newColor)
{
	if (shadowColor != newColor)
	{
		shadowColor = newColor;
		if (GetUIRenderable())GetUIRenderable()->MarkColorDirty();
	}
}
void UUIEffectLongShadow::SetShadowSize(FVector newSize)
{
	if (shadowSize != newSize)
	{
		shadowSize = newSize;
		if (GetUIRenderable())GetUIRenderable()->MarkVertexPositionDirty();
	}
}
void UUIEffectLongShadow::SetShadowSegment(uint8 newSegment)
{
	if (shadowSegment != newSegment)
	{
		shadowSegment = newSegment;
		if (GetUIRenderable())GetUIRenderable()->MarkVerticesDirty(true, true, true, true);
	}
}
void UUIEffectLongShadow::SetUseGradientColor(bool newBool)
{
	if (useGradientColor != newBool)
	{
		useGradientColor = newBool;
		if (GetUIRenderable())GetUIRenderable()->MarkColorDirty();
	}
}
void UUIEffectLongShadow::SetGradientColor(FColor newColor)
{
	if (shadowColor != newColor)
	{
		shadowColor = newColor;
		if (GetUIRenderable())GetUIRenderable()->MarkColorDirty();
	}
}