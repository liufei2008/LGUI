// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectLongShadow.h"
#include "LGUI.h"


UUIEffectLongShadow::UUIEffectLongShadow()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectLongShadow::ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha)
{
	if (multiplySourceAlpha)
	{
		InOutColor.A = (uint8)(UUIBaseRenderable::Color255To1_Table[InOriginAlpha] * InTintColor.A);
		InOutColor.R = InTintColor.R;
		InOutColor.G = InTintColor.G;
		InOutColor.B = InTintColor.B;
	}
	else
	{
		InOutColor = InTintColor;
	}
}
void UUIEffectLongShadow::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
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
	int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * (shadowSegment + 1);
	InOutOriginTriangleIndicesCount = singleChannelTriangleIndicesCount * (shadowSegment + 2);
	if (singleChannelTriangleIndicesCount == triangleCount)
	{
		OutTriangleChanged = true;
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
	else
	{
		//update triangle data is slightly different from create triangle data. the first pass may changed or not, all we need to do is set origin data (which can be calculated from other pass)
		int lastPassVerticesCount = singleChannelVerticesCount * (shadowSegment + 1);
		for (int lastPassTriangleIndex = additionalTriangleIndicesCount, firstPassTriangleIndex = 0; firstPassTriangleIndex < singleChannelTriangleIndicesCount; lastPassTriangleIndex++, firstPassTriangleIndex++)
		{
			triangles[firstPassTriangleIndex] = triangles[lastPassTriangleIndex] + lastPassVerticesCount;
		}
	}

	if (singleChannelVerticesCount == vertexCount)
	{
		vertexCount = singleChannelVerticesCount * (shadowSegment + 2);
		originPositions.Reserve(vertexCount);
		vertices.Reserve(vertexCount);
		for (int i = singleChannelVerticesCount; i < vertexCount; i++)
		{
			originPositions.Add(FVector());
			vertices.Add(FVector());
		}
	}
	InOutOriginVerticesCount = singleChannelVerticesCount * (shadowSegment + 2);

	//verticies
	{
		FVector shadowSizeInterval = shadowSize / (shadowSegment + 1);
		int32 shadowChannelCount = shadowSegment + 1;
		for (int channelOriginVertIndex = 0; channelOriginVertIndex < singleChannelVerticesCount; channelOriginVertIndex++)
		{
			auto originVert = originPositions[channelOriginVertIndex];
			auto originUV = vertices[channelOriginVertIndex].TextureCoordinate[0];
			auto originAlpha = vertices[channelOriginVertIndex].Color.A;
			for (int channelIndex = 0; channelIndex < shadowChannelCount; channelIndex++)
			{
				int channelVertIndex = (channelIndex + 1) * singleChannelVerticesCount + channelOriginVertIndex;
				vertices[channelVertIndex].TextureCoordinate[0] = originUV;
				auto& vert = originPositions[channelVertIndex];
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
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
	}
}
void UUIEffectLongShadow::SetShadowSize(FVector newSize)
{
	if (shadowSize != newSize)
	{
		shadowSize = newSize;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkVertexPositionDirty();
	}
}
void UUIEffectLongShadow::SetShadowSegment(uint8 newSegment)
{
	if (shadowSegment != newSegment)
	{
		shadowSegment = newSegment;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkTriangleDirty();
	}
}
void UUIEffectLongShadow::SetUseGradientColor(bool newBool)
{
	if (useGradientColor != newBool)
	{
		useGradientColor = newBool;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
	}
}
void UUIEffectLongShadow::SetGradientColor(FColor newColor)
{
	if (shadowColor != newColor)
	{
		shadowColor = newColor;
		if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
	}
}