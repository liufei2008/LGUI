// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierShadow.h"
#include "LGUI.h"
#include "Utils/LexUIUtils.h"


ULexMeshModifierShadow::ULexMeshModifierShadow()
{
}
void ULexMeshModifierShadow::ModifyUIGeometry(
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
	//create additional triangle pass
	triangles.AddUninitialized(singleChannelTriangleIndicesCount);
	//put origin triangles on last pass, this will make the origin triangle render at top
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
		originVertPos += ShadowOffset;
		originVertices[channelIndex1].Position = originVertPos;

		if (bMultiplySourceAlpha)
		{
			auto& vertColor = vertices[channelIndex1].Color;
			vertColor.A = (uint8)(FLexUIUtils::ByteToFloat01(vertices[channelIndexOrigin].Color.A) * ShadowColor.A);
			vertColor.R = ShadowColor.R;
			vertColor.G = ShadowColor.G;
			vertColor.B = ShadowColor.B;
		}
		else
		{
			vertices[channelIndex1].Color = ShadowColor;
		}

		for (int i = 0; i < MAX_STATIC_TEXCOORDS; i++)
		{
			vertices[channelIndex1].TextureCoordinate[i] = vertices[channelIndexOrigin].TextureCoordinate[i];
		}
	}
}

void ULexMeshModifierShadow::SetShadowColor(FColor Value)
{
	if (ShadowColor != Value)
	{
		ShadowColor = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkColorDirty();
	}
}
void ULexMeshModifierShadow::SetShadowOffset(FVector3f Value)
{
	if (ShadowOffset != Value)
	{
		ShadowOffset = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkVertexPositionDirty();
	}
}