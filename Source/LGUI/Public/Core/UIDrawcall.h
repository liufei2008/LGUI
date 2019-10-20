// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
//#include "UIDrawcall.generated.h"

class LGUI_API UUIDrawcall
{
public:
	TArray<TSharedPtr<UIGeometry>> geometryList;//UI geometries that construct this drawcall
	UTexture* texture = nullptr;//drawcall used this texture to render
	UMaterialInterface* material = nullptr;//drawcall use this material to render, can be null to use default material
	UMaterialInstanceDynamic* materialInstanceDynamic = nullptr;//created MaterialInstanceDynamic that render this drawcall
	int depthMin = 0;//min depth of all geometries
	int depthMax = 0;//max depth of all geometries
	bool needToBeRebuild = false;//rebuild only if need to
	bool needToUpdateVertex = false;//update only if need to
	bool vertexPositionChanged = false;//if vertex position changed? use for update bounds
	bool isFontTexture = false;//the texture of this drawcall is font texture or not
public:
	void GetCombined(TArray<FVector>& vertices, TArray<FVector2D>& uvs, TArray<FColor>& colors, TArray<uint16>& triangles
		, TArray<FVector>& normals
		, TArray<FVector>& tangents
		, TArray<FVector2D>& uvs1
		, TArray<FVector2D>& uvs2
		, TArray<FVector2D>& uvs3)const
	{
		int count = geometryList.Num();
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		for (int i = 0; i < count; i++)
		{
			auto& geometry = geometryList[i];
			auto& geomTriangles = geometry->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;
			vertices.Append(geometry->GetTransformedVertices());
			uvs.Append(geometry->uvs);
			colors.Append(geometry->colors);
			normals.Append(geometry->GetTransformedNormals());
			tangents.Append(geometry->GetTransformedTangents());
			uvs1.Append(geometry->uvs1);
			uvs2.Append(geometry->uvs2);
			uvs3.Append(geometry->uvs3);
			
			triangles.AddUninitialized(triangleCount);
			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				triangles[triangleIndicesIndex++] = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
			}

			prevVertexCount += geometry->GetTransformedVertices().Num();
		}
	}
	void UpdateData(TArray<FVector>& vertices, TArray<FVector2D>& uvs, TArray<FColor>& colors, TArray<uint16>& triangles
		, TArray<FVector>& normals
		, TArray<FVector>& tangents
		, TArray<FVector2D>& uvs1
		, TArray<FVector2D>& uvs2
		, TArray<FVector2D>& uvs3)
	{
		int count = geometryList.Num();
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		int vertPosMemOffset = 0, vertUVMemOffset = 0, vertColorMemOffset = 0, vertNormalMemOffset = 0, vertTangentMemOffset = 0, vertUV1MemOffset = 0, vertUV2MemOffset = 0, vertUV3MemOffset = 0;
		for (int i = 0; i < count; i++)
		{
			auto& geometry = geometryList[i];
			auto& geomTriangles = geometry->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;

			int vertCount = geometry->vertices.Num();

			int vertPosDataCount = vertCount * sizeof(FVector);
			FMemory::Memcpy((uint8*)vertices.GetData() + vertPosMemOffset, (uint8*)geometry->GetTransformedVertices().GetData(), vertPosDataCount);
			vertPosMemOffset += vertPosDataCount;

			int vertUVDataCount = vertCount * sizeof(FVector2D);
			FMemory::Memcpy((uint8*)uvs.GetData() + vertUVMemOffset, (uint8*)geometry->uvs.GetData(), vertUVDataCount);
			vertUVMemOffset += vertUVDataCount;

			int vertColorDataCount = vertCount * sizeof(FColor);
			FMemory::Memcpy((uint8*)colors.GetData() + vertColorMemOffset, (uint8*)geometry->colors.GetData(), vertColorDataCount);
			vertColorMemOffset += vertColorDataCount;

			int vertNormalDataCount = vertCount * sizeof(FVector);
			FMemory::Memcpy((uint8*)normals.GetData() + vertNormalMemOffset, (uint8*)geometry->GetTransformedNormals().GetData(), vertNormalDataCount);
			vertNormalMemOffset += vertNormalDataCount;

			int vertTangentDataCount = vertCount * sizeof(FVector);
			FMemory::Memcpy((uint8*)tangents.GetData() + vertTangentMemOffset, (uint8*)geometry->GetTransformedTangents().GetData(), vertTangentDataCount);
			vertTangentMemOffset += vertTangentDataCount;

			int vertUV1DataCount = vertCount * sizeof(FVector2D);
			FMemory::Memcpy((uint8*)uvs1.GetData() + vertUV1MemOffset, (uint8*)geometry->uvs1.GetData(), vertUV1DataCount);
			vertUV1MemOffset += vertUV1DataCount;

			int vertUV2DataCount = vertCount * sizeof(FVector2D);
			FMemory::Memcpy((uint8*)uvs2.GetData() + vertUV2MemOffset, (uint8*)geometry->uvs2.GetData(), vertUV2DataCount);
			vertUV2MemOffset += vertUV2DataCount;

			int vertUV3DataCount = vertCount * sizeof(FVector2D);
			FMemory::Memcpy((uint8*)uvs3.GetData() + vertUV3MemOffset, (uint8*)geometry->uvs3.GetData(), vertUV3DataCount);
			vertUV3MemOffset += vertUV3DataCount;

			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				triangles[triangleIndicesIndex++] = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
			}

			prevVertexCount += geometry->GetTransformedVertices().Num();
		}
	}
	//update the max and min depth
	void UpdateDepthRange()
	{
		depthMin = INT32_MAX;
		depthMax = INT32_MIN;
		for (auto item : geometryList)
		{
			auto itemDepth = item->depth;
			if (depthMin > itemDepth)
			{
				depthMin = itemDepth;
			}
			if (depthMax < itemDepth)
			{
				depthMax = itemDepth;
			}
		}
	}
	
	bool IsDepthInsideDrawcall(int depth)
	{
		return depth >= depthMin && depth < depthMax;
	}
	void Clear()
	{
		geometryList.Empty();
		texture = nullptr;
		material = nullptr;
	}
};
