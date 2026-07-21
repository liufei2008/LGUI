// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUI/Public/MeshModifier/LexMeshModifierOutline.h"
#include "LGUI.h"
#include "Utils/LexUIUtils.h"


ULexMeshModifierOutline::ULexMeshModifierOutline()
{
}

void ULexMeshModifierOutline::ApplyColorAndAlpha(FColor& InOutColor, uint8 InSourceAlpha)
{
	if (bMultiplySourceAlpha)
	{
		InOutColor.A = (uint8)(FLexUIUtils::ByteToFloat01(InSourceAlpha) * OutlineColor.A);
		InOutColor.R = OutlineColor.R;
		InOutColor.G = OutlineColor.G;
		InOutColor.B = OutlineColor.B;
	}
	else
	{
		InOutColor = OutlineColor;
	}
}
void ULexMeshModifierOutline::ModifyUIGeometry(
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
	const int32 additionalTriangleIndicesCount = singleChannelTriangleIndicesCount * (bUse8Direction ? 8 : 4);

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
			if (bUse8Direction)
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

	int additionalVertCount = singleChannelVerticesCount * (bUse8Direction ? 8 : 4);
	vertexCount = singleChannelVerticesCount + additionalVertCount;
	originVertices.AddDefaulted(additionalVertCount);
	vertices.AddDefaulted(additionalVertCount);

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
			for (int i = 0; i < LEXUI_VERTEX_TEXCOORDINATE_COUNT; i++)
			{
				auto originUV = vertices[channelOriginVertIndex].TextureCoordinate[i];
				vertices[channelVertIndex1].TextureCoordinate[i] = originUV;
				vertices[channelVertIndex2].TextureCoordinate[i] = originUV;
				vertices[channelVertIndex3].TextureCoordinate[i] = originUV;
				vertices[channelVertIndex4].TextureCoordinate[i] = originUV;
				if (bUse8Direction)
				{
					vertices[channelVertIndex5].TextureCoordinate[i] = originUV;
					vertices[channelVertIndex6].TextureCoordinate[i] = originUV;
					vertices[channelVertIndex7].TextureCoordinate[i] = originUV;
					vertices[channelVertIndex8].TextureCoordinate[i] = originUV;
				}
			}

			auto originAlpha = vertices[channelOriginVertIndex].Color.A;
			ApplyColorAndAlpha(vertices[channelVertIndex1].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex2].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex3].Color, originAlpha);
			ApplyColorAndAlpha(vertices[channelVertIndex4].Color, originAlpha);
			if (bUse8Direction)
			{
				ApplyColorAndAlpha(vertices[channelVertIndex5].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex6].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex7].Color, originAlpha);
				ApplyColorAndAlpha(vertices[channelVertIndex8].Color, originAlpha);
			}

			auto originVert = originVertices[channelOriginVertIndex].Position;
			auto& channel1Vert = originVertices[channelVertIndex1].Position;
			channel1Vert = originVert;
			channel1Vert.Y += OutlineSize.X;
			channel1Vert.Z += OutlineSize.Y;
			auto& channel2Vert = originVertices[channelVertIndex2].Position;
			channel2Vert = originVert;
			channel2Vert.Y -= OutlineSize.X;
			channel2Vert.Z += OutlineSize.Y;
			auto& channel3Vert = originVertices[channelVertIndex3].Position;
			channel3Vert = originVert;
			channel3Vert.Y += OutlineSize.X;
			channel3Vert.Z -= OutlineSize.Y;
			auto& channel4Vert = originVertices[channelVertIndex4].Position;
			channel4Vert = originVert;
			channel4Vert.Y -= OutlineSize.X;
			channel4Vert.Z -= OutlineSize.Y;
			if (bUse8Direction)
			{
				auto& channel5Vert = originVertices[channelVertIndex5].Position;
				channel5Vert = originVert;
				channel5Vert.Y -= OutlineSize.X;
				channel5Vert.Z += 0;
				auto& channel6Vert = originVertices[channelVertIndex6].Position;
				channel6Vert = originVert;
				channel6Vert.Y += OutlineSize.X;
				channel6Vert.Z += 0;
				auto& channel7Vert = originVertices[channelVertIndex7].Position;
				channel7Vert = originVert;
				channel7Vert.Y += 0;
				channel7Vert.Z += OutlineSize.Y;
				auto& channel8Vert = originVertices[channelVertIndex8].Position;
				channel8Vert = originVert;
				channel8Vert.Y += 0;
				channel8Vert.Z -= OutlineSize.Y;
			}

			channelVertIndex1++, channelVertIndex2++, channelVertIndex3++, channelVertIndex4++;
			channelVertIndex5++, channelVertIndex6++, channelVertIndex7++, channelVertIndex8++;
		}
	}
}

void ULexMeshModifierOutline::SetOutlineColor(FColor Value)
{
	if (OutlineColor != Value)
	{
		OutlineColor = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkColorDirty();
	}
}
void ULexMeshModifierOutline::SetOutlineSize(FVector2f Value)
{
	if (OutlineSize != Value)
	{
		OutlineSize = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkVertexPositionDirty();
	}
}
void ULexMeshModifierOutline::SetUse8Direction(bool Value)
{
	if (bUse8Direction != Value)
	{
		bUse8Direction = Value;
		if (auto Visual = GetVisualBatchMesh())Visual->MarkVerticesDirty(true, true, true, true);
	}
}