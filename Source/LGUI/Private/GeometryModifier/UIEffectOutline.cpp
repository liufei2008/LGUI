// Copyright 2019 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectOutline.h"
#include "LGUI.h"


UUIEffectOutline::UUIEffectOutline()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectOutline::ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)
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
		const int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * 4;
		triangles.AddUninitialized(additionalTriangleIndicesCount);
		//put orgin triangles on last pass, this will make the origin triangle render at top
		for (int triangleIndex = additionalTriangleIndicesCount, originTriangleIndex = 0; originTriangleIndex < singleChannelTriangleIndicesCount; triangleIndex++, originTriangleIndex++)
		{
			auto index = triangles[originTriangleIndex];
			triangles[triangleIndex] = index;
		}
		//calculate other pass
		int channelTriangleIndex1 = 0
			, channelTriangleIndex2 = channelTriangleIndex1 + singleChannelTriangleIndicesCount
			, channelTriangleIndex3 = channelTriangleIndex2 + singleChannelTriangleIndicesCount
			, channelTriangleIndex4 = channelTriangleIndex3 + singleChannelTriangleIndicesCount;
		int channelIndicesOffset1 = singleChannelVerticesCount
			, channelIndicesOffset2 = channelIndicesOffset1 + singleChannelVerticesCount
			, channelIndicesOffset3 = channelIndicesOffset2 + singleChannelVerticesCount
			, channelIndicesOffset4 = channelIndicesOffset3 + singleChannelVerticesCount;
		int triangleIndicesCount = additionalTriangleIndicesCount + singleChannelTriangleIndicesCount;
		for (int channelIndexOrigin = additionalTriangleIndicesCount; channelIndexOrigin < triangleIndicesCount; channelIndexOrigin++)
		{
			auto originTriangleIndex = triangles[channelIndexOrigin];
			triangles[channelTriangleIndex1] = originTriangleIndex + channelIndicesOffset1;
			triangles[channelTriangleIndex2] = originTriangleIndex + channelIndicesOffset2;
			triangles[channelTriangleIndex3] = originTriangleIndex + channelIndicesOffset3;
			triangles[channelTriangleIndex4] = originTriangleIndex + channelIndicesOffset4;

			channelTriangleIndex1++, channelTriangleIndex2++, channelTriangleIndex3++, channelTriangleIndex4++;
		}
	}
	InOutOriginTriangleIndicesCount = singleChannelTriangleIndicesCount * 5;
	

	if (singleChannelVerticesCount == vertexCount)
	{
		vertexCount = singleChannelVerticesCount * 5;
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
	InOutOriginVerticesCount = singleChannelVerticesCount * 5;
	int channelVertIndex1 = singleChannelVerticesCount
		, channelVertIndex2 = channelVertIndex1 + singleChannelVerticesCount
		, channelVertIndex3 = channelVertIndex2 + singleChannelVerticesCount
		, channelVertIndex4 = channelVertIndex3 + singleChannelVerticesCount;
	auto color = outlineColor;
	if (multiplySourceAlpha)
	{
		color.A = (uint8)(GetRenderableUIItem()->GetFinalAlpha() * color.A * 0.003921568627f/* 1 / 255 */);
	}
	for (int channelOriginVertIndex = 0; channelOriginVertIndex < singleChannelVerticesCount; channelOriginVertIndex++)
	{
		auto originUV = uvs[channelOriginVertIndex];
		uvs[channelVertIndex1] = originUV;
		uvs[channelVertIndex2] = originUV;
		uvs[channelVertIndex3] = originUV;
		uvs[channelVertIndex4] = originUV;

		colors[channelVertIndex1] = color;
		colors[channelVertIndex2] = color;
		colors[channelVertIndex3] = color;
		colors[channelVertIndex4] = color;

		auto originVert = vertices[channelOriginVertIndex];
		auto& channel1Vert = vertices[channelVertIndex1];
		channel1Vert = originVert;
		channel1Vert.X += outlineSize.X;
		channel1Vert.Y += outlineSize.Y;
		auto& channel2Vert = vertices[channelVertIndex2];
		channel2Vert = originVert;
		channel2Vert.X -= outlineSize.X;
		channel2Vert.Y += outlineSize.Y;
		auto& channel3Vert = vertices[channelVertIndex3];
		channel3Vert = originVert;
		channel3Vert.X += outlineSize.X;
		channel3Vert.Y -= outlineSize.Y;
		auto& channel4Vert = vertices[channelVertIndex4];
		channel4Vert = originVert;
		channel4Vert.X -= outlineSize.X;
		channel4Vert.Y -= outlineSize.Y;

		channelVertIndex1++, channelVertIndex2++, channelVertIndex3++, channelVertIndex4++;
	}

}

void UUIEffectOutline::SetOutlineColor(FColor newColor)
{
	outlineColor = newColor;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkColorDirty();
}
void UUIEffectOutline::SetOutlineSize(FVector2D newSize)
{
	outlineSize = newSize;
	if (GetRenderableUIItem())GetRenderableUIItem()->MarkVertexPositionDirty();
}