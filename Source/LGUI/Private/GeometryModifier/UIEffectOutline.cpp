﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "GeometryModifier/UIEffectOutline.h"
#include "LGUI.h"


UUIEffectOutline::UUIEffectOutline()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIEffectOutline::ApplyColorAndAlpha(FColor& InOutColor, uint8 InSourceAlpha)
{
	if (multiplySourceAlpha)
	{
		InOutColor.A = (uint8)(UUIBaseRenderable::Color255To1_Table[InSourceAlpha] * outlineColor.A);
		InOutColor.R = outlineColor.R;
		InOutColor.G = outlineColor.G;
		InOutColor.B = outlineColor.B;
	}
	else
	{
		InOutColor = outlineColor;
	}
}
void UUIEffectOutline::ModifyUIGeometry(
	UIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto& triangles = InGeometry.triangles;
	auto& originPositions = InGeometry.originPositions;
	auto& vertices = InGeometry.vertices;

	auto vertexCount = originPositions.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;

	const int32 singleChannelTriangleIndicesCount = triangleCount;
	const int32 singleChannelVerticesCount = vertexCount;
	const int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * (use8Direction ? 8 : 4);

	triangles.AddUninitialized(additionalTriangleIndicesCount);
	//put orgin triangles on last pass, this will make the origin triangle render at top
	for (int triangleIndex = additionalTriangleIndicesCount, originTriangleIndex = 0; originTriangleIndex < singleChannelTriangleIndicesCount; triangleIndex++, originTriangleIndex++)
	{
		auto index = triangles[originTriangleIndex];
		triangles[triangleIndex] = index;
	}

	//calculate other pass
	{
		int channelTriangleIndex1 = 0
			, channelTriangleIndex2 = channelTriangleIndex1 + singleChannelTriangleIndicesCount
			, channelTriangleIndex3 = channelTriangleIndex2 + singleChannelTriangleIndicesCount
			, channelTriangleIndex4 = channelTriangleIndex3 + singleChannelTriangleIndicesCount
			, channelTriangleIndex5 = channelTriangleIndex4 + singleChannelTriangleIndicesCount
			, channelTriangleIndex6 = channelTriangleIndex5 + singleChannelTriangleIndicesCount
			, channelTriangleIndex7 = channelTriangleIndex6 + singleChannelTriangleIndicesCount
			, channelTriangleIndex8 = channelTriangleIndex7 + singleChannelTriangleIndicesCount
			;
		int channelIndicesOffset1 = singleChannelVerticesCount
			, channelIndicesOffset2 = channelIndicesOffset1 + singleChannelVerticesCount
			, channelIndicesOffset3 = channelIndicesOffset2 + singleChannelVerticesCount
			, channelIndicesOffset4 = channelIndicesOffset3 + singleChannelVerticesCount
			, channelIndicesOffset5 = channelIndicesOffset4 + singleChannelVerticesCount
			, channelIndicesOffset6 = channelIndicesOffset5 + singleChannelVerticesCount
			, channelIndicesOffset7 = channelIndicesOffset6 + singleChannelVerticesCount
			, channelIndicesOffset8 = channelIndicesOffset7 + singleChannelVerticesCount
			;
		int triangleIndicesCount = additionalTriangleIndicesCount + singleChannelTriangleIndicesCount;
		for (int channelIndexOrigin = additionalTriangleIndicesCount; channelIndexOrigin < triangleIndicesCount; channelIndexOrigin++)
		{
			auto originTriangleIndex = triangles[channelIndexOrigin];
			triangles[channelTriangleIndex1] = originTriangleIndex + channelIndicesOffset1;
			triangles[channelTriangleIndex2] = originTriangleIndex + channelIndicesOffset2;
			triangles[channelTriangleIndex3] = originTriangleIndex + channelIndicesOffset3;
			triangles[channelTriangleIndex4] = originTriangleIndex + channelIndicesOffset4;
			if (use8Direction)
			{
				triangles[channelTriangleIndex5] = originTriangleIndex + channelIndicesOffset5;
				triangles[channelTriangleIndex6] = originTriangleIndex + channelIndicesOffset6;
				triangles[channelTriangleIndex7] = originTriangleIndex + channelIndicesOffset7;
				triangles[channelTriangleIndex8] = originTriangleIndex + channelIndicesOffset8;
			}

			channelTriangleIndex1++, channelTriangleIndex2++, channelTriangleIndex3++, channelTriangleIndex4++;
			channelTriangleIndex5++, channelTriangleIndex6++, channelTriangleIndex7++, channelTriangleIndex8++;
		}
	}

	vertexCount = singleChannelVerticesCount * (use8Direction ? 9 : 5);
	originPositions.Reserve(vertexCount);
	vertices.Reserve(vertexCount);
	for (int i = singleChannelVerticesCount; i < vertexCount; i++)
	{
		originPositions.Add(FVector());
		vertices.Add(FVector());
	}

	//vertices
	{
		int channelVertIndex1 = singleChannelVerticesCount
			, channelVertIndex2 = channelVertIndex1 + singleChannelVerticesCount
			, channelVertIndex3 = channelVertIndex2 + singleChannelVerticesCount
			, channelVertIndex4 = channelVertIndex3 + singleChannelVerticesCount
			, channelVertIndex5 = channelVertIndex4 + singleChannelVerticesCount
			, channelVertIndex6 = channelVertIndex5 + singleChannelVerticesCount
			, channelVertIndex7 = channelVertIndex6 + singleChannelVerticesCount
			, channelVertIndex8 = channelVertIndex7 + singleChannelVerticesCount
			;

		for (int channelOriginVertIndex = 0; channelOriginVertIndex < singleChannelVerticesCount; channelOriginVertIndex++)
		{
			auto originUV = vertices[channelOriginVertIndex].TextureCoordinate[0];
			vertices[channelVertIndex1].TextureCoordinate[0] = originUV;
			vertices[channelVertIndex2].TextureCoordinate[0] = originUV;
			vertices[channelVertIndex3].TextureCoordinate[0] = originUV;
			vertices[channelVertIndex4].TextureCoordinate[0] = originUV;
			if (use8Direction)
			{
				vertices[channelVertIndex5].TextureCoordinate[0] = originUV;
				vertices[channelVertIndex6].TextureCoordinate[0] = originUV;
				vertices[channelVertIndex7].TextureCoordinate[0] = originUV;
				vertices[channelVertIndex8].TextureCoordinate[0] = originUV;
			}

			auto originAlpha = vertices[channelOriginVertIndex].Color.A;
			ApplyColorAndAlpha(vertices[channelVertIndex1].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex2].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex3].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex4].Color, originAlpha);
			if (use8Direction)
			{
				ApplyColorAndAlpha(vertices[channelVertIndex5].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex6].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex7].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex8].Color, originAlpha);
			}

			auto originVert = originPositions[channelOriginVertIndex];
			auto& channel1Vert = originPositions[channelVertIndex1];
			channel1Vert = originVert;
			channel1Vert.Y += outlineSize.X;
			channel1Vert.Z += outlineSize.Y;
			auto& channel2Vert = originPositions[channelVertIndex2];
			channel2Vert = originVert;
			channel2Vert.Y -= outlineSize.X;
			channel2Vert.Z += outlineSize.Y;
			auto& channel3Vert = originPositions[channelVertIndex3];
			channel3Vert = originVert;
			channel3Vert.Y += outlineSize.X;
			channel3Vert.Z -= outlineSize.Y;
			auto& channel4Vert = originPositions[channelVertIndex4];
			channel4Vert = originVert;
			channel4Vert.Y -= outlineSize.X;
			channel4Vert.Z -= outlineSize.Y;
			if (use8Direction)
			{
				auto& channel5Vert = originPositions[channelVertIndex5];
				channel5Vert = originVert;
				channel5Vert.Y -= outlineSize.X;
				channel5Vert.Z += 0;
				auto& channel6Vert = originPositions[channelVertIndex6];
				channel6Vert = originVert;
				channel6Vert.Y += outlineSize.X;
				channel6Vert.Z += 0;
				auto& channel7Vert = originPositions[channelVertIndex7];
				channel7Vert = originVert;
				channel7Vert.Y += 0;
				channel7Vert.Z += outlineSize.Y;
				auto& channel8Vert = originPositions[channelVertIndex8];
				channel8Vert = originVert;
				channel8Vert.Y += 0;
				channel8Vert.Z -= outlineSize.Y;
			}

			channelVertIndex1++, channelVertIndex2++, channelVertIndex3++, channelVertIndex4++;
			channelVertIndex5++, channelVertIndex6++, channelVertIndex7++, channelVertIndex8++;
		}
	}
}

void UUIEffectOutline::SetOutlineColor(FColor newColor)
{
	if (outlineColor != newColor)
	{
		outlineColor = newColor;
		if (GetUIRenderable())GetUIRenderable()->MarkColorDirty();
	}
}
void UUIEffectOutline::SetOutlineSize(FVector2D newSize)
{
	if (outlineSize != newSize)
	{
		outlineSize = newSize;
		if (GetUIRenderable())GetUIRenderable()->MarkVertexPositionDirty();
	}
}
void UUIEffectOutline::SetUse8Direction(bool newValue)
{
	if (use8Direction != newValue)
	{
		use8Direction = newValue;
		if (GetUIRenderable())GetUIRenderable()->MarkTriangleDirty();
	}
}