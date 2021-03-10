// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
//#include "UIDrawcall.generated.h"
class UUIPostProcess;
class UUISelfRenderable;

enum class EUIDrawcallType :uint8
{
	/** Build UIGeometry and send to canvas for rendering. */
	Geometry = 1,
	/** PostProcess item, not send geometry to canvas, but need canvas to render. */
	PostProcess,
};
class LGUI_API UUIDrawcall
{
public:
	EUIDrawcallType type;

	TArray<TSharedPtr<UIGeometry>> geometryList;//UI geometries that construct this drawcall
	TWeakObjectPtr<UTexture> texture = nullptr;//drawcall used this texture to render
	TWeakObjectPtr<UMaterialInterface> material = nullptr;//drawcall use this material to render, can be null to use default material
	TWeakObjectPtr<UMaterialInstanceDynamic> materialInstanceDynamic = nullptr;//created MaterialInstanceDynamic that render this drawcall

	int depthMin = 0;//min depth of all geometries
	int depthMax = 0;//max depth of all geometries
	bool needToBeRebuild = false;//rebuild only if need to
	bool needToUpdateVertex = false;//update only if need to
	bool vertexPositionChanged = false;//if vertex position changed? use for update bounds

	TWeakObjectPtr<UUIPostProcess> postProcessObject;//post process object
public:
	void GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<uint16>& triangles)const
	{
		int count = geometryList.Num();
		if (count == 1)
		{
			vertices = geometryList[0]->vertices;
			triangles = geometryList[0]->triangles;
		}
		else
		{
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
	}
	void UpdateData(TArray<FDynamicMeshVertex>& vertices, TArray<uint16>& triangles)
	{
		int count = geometryList.Num();
		if (count == 1)
		{
			FMemory::Memcpy((uint8*)vertices.GetData(), geometryList[0]->vertices.GetData(), vertices.Num() * sizeof(FDynamicMeshVertex));
			FMemory::Memcpy((uint8*)triangles.GetData(), geometryList[0]->triangles.GetData(), triangles.Num() * sizeof(uint16));
		}
		else
		{
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
		materialInstanceDynamic = nullptr;

		postProcessObject = nullptr;
	}
};
