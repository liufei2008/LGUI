// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
//#include "UIDrawcall.generated.h"
class UUIPostProcess;

class LGUI_API UUIDrawcall
{
public:
	TArray<TSharedPtr<UIGeometry>> geometryList;//UI geometries that construct this drawcall
	TWeakObjectPtr<UTexture> texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInstanceDynamic> materialInstanceDynamic = nullptr;//created MaterialInstanceDynamic that render this drawcall
	int depthMin = 0;//min depth of all geometries
	int depthMax = 0;//max depth of all geometries
	bool needToBeRebuild = false;//rebuild only if need to
	bool needToUpdateVertex = false;//update only if need to
	bool vertexPositionChanged = false;//if vertex position changed? use for update bounds
	TWeakObjectPtr<UUIPostProcess> postProcessObject;
public:
	void GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<uint16>& triangles)const
	{
		int count = geometryList.Num();
		int totalVertCount = 0;
		int totalTriangleIndicesCount = 0;
		for (int i = 0; i < count; i++)
		{
			totalVertCount += geometryList[i]->vertices.Num();
			totalTriangleIndicesCount += geometryList[i]->triangles.Num();
		}
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		vertices.Reserve(totalVertCount);
		triangles.SetNumUninitialized(totalTriangleIndicesCount);
		for (int geoIndex = 0; geoIndex < count; geoIndex++)
		{
			auto& geometry = geometryList[geoIndex];
			auto& geomTriangles = geometry->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;
			vertices.Append(geometry->vertices);
			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				triangles[triangleIndicesIndex++] = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
			}

			prevVertexCount += geometry->vertices.Num();
		}
	}
	void UpdateData(TArray<FDynamicMeshVertex>& vertices, TArray<uint16>& triangles)
	{
		int count = geometryList.Num();
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		int vertBufferOffset = 0;
		for (int geoIndex = 0; geoIndex < count; geoIndex++)
		{
			auto& geometry = geometryList[geoIndex];
			auto& geomTriangles = geometry->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;
			int vertCount = geometry->vertices.Num();
			int bufferSize = vertCount * sizeof(FDynamicMeshVertex);
			FMemory::Memcpy((uint8*)vertices.GetData() + vertBufferOffset, (uint8*)geometry->vertices.GetData(), bufferSize);
			vertBufferOffset += bufferSize;

			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				triangles[triangleIndicesIndex++] = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
			}

			prevVertexCount += vertCount;
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
