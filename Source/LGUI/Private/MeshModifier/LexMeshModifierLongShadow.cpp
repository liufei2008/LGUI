// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierLongShadow.h"
#include "LGUI.h"
#include "Utils/LexUIUtils.h"


ULexMeshModifierLongShadow::ULexMeshModifierLongShadow()
{
}

void ULexMeshModifierLongShadow::ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha)
{
	if (bMultiplySourceAlpha)
	{
		InOutColor.A = (uint8)(FLexUIUtils::ByteToFloat01(InOriginAlpha) * InTintColor.A);
		InOutColor.R = InTintColor.R;
		InOutColor.G = InTintColor.G;
		InOutColor.B = InTintColor.B;
	}
	else
	{
		InOutColor = InTintColor;
	}
}
void ULexMeshModifierLongShadow::ModifyUIGeometry(
	FLexUIGeometry& InGeometry, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
)
{
	auto& triangles = InGeometry.Triangles;
	auto& originVertices = InGeometry.OriginVertices;
	auto& vertices = InGeometry.Vertices;

	auto vertexCount = originVertices.Num();
	int32 triangleCount = triangles.Num();
	if (triangleCount == 0 || vertexCount == 0)return;

	const int32 singleChannelTriangleIndicesCount = triangleCount;
	const int32 singleChannelVerticesCount = vertexCount;
	int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * (ShadowSegment + 1);

	triangles.AddUninitialized(additionalTriangleIndicesCount);
	//put orgin triangles on last pass, this will make the origin triangle render at top
	for (int triangleIndex = additionalTriangleIndicesCount, originTriangleIndex = 0; originTriangleIndex < singleChannelTriangleIndicesCount; triangleIndex++, originTriangleIndex++)
	{
		auto index = triangles[originTriangleIndex];
		triangles[triangleIndex] = index;
	}
	//calculate other pass
	int32 prevChannelVerticesCount = singleChannelVerticesCount;
	int32 shadowChannelCount = ShadowSegment + 1;
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

	int additionalVertCount = singleChannelVerticesCount * (ShadowSegment + 1);
	vertexCount = singleChannelVerticesCount + additionalVertCount;
	originVertices.AddDefaulted(additionalVertCount);
	vertices.AddDefaulted(additionalVertCount);

	//verticies
	{
		auto shadowSizeInterval = ShadowSize / (ShadowSegment + 1);
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
				
				if (bUseGradientColor)
				{
					float colorRatio = ((float)(channelIndex) / (shadowChannelCount));
					float colorRatio_INV = 1.0f - colorRatio;
					FColor color;
					color.R = ShadowColor.R * colorRatio + GradientColor.R * colorRatio_INV;
					color.G = ShadowColor.G * colorRatio + GradientColor.G * colorRatio_INV;
					color.B = ShadowColor.B * colorRatio + GradientColor.B * colorRatio_INV;
					color.A = ShadowColor.A * colorRatio + GradientColor.A * colorRatio_INV;
					ApplyColorAndAlpha(vertices[channelVertIndex].Color, color, originAlpha);
				}
				else
				{
					ApplyColorAndAlpha(vertices[channelVertIndex].Color, ShadowColor, originAlpha);
				}
			}
		}
	}
}

void ULexMeshModifierLongShadow::SetShadowColor(FColor Value)
{
	if (ShadowColor != Value)
	{
		ShadowColor = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkColorDirty();
	}
}
void ULexMeshModifierLongShadow::SetShadowSize(FVector3f Value)
{
	if (ShadowSize != Value)
	{
		ShadowSize = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkVertexPositionDirty();
	}
}
void ULexMeshModifierLongShadow::SetShadowSegment(uint8 Value)
{
	if (ShadowSegment != Value)
	{
		ShadowSegment = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkVerticesDirty(true, true, true, true);
	}
}
void ULexMeshModifierLongShadow::SetUseGradientColor(bool Value)
{
	if (bUseGradientColor != Value)
	{
		bUseGradientColor = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkColorDirty();
	}
}
void ULexMeshModifierLongShadow::SetGradientColor(FColor Value)
{
	if (ShadowColor != Value)
	{
		ShadowColor = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkColorDirty();
	}
}