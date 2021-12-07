// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/UIDrawcall.h"
#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "DynamicMeshBuilder.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "Core/LGUISettings.h"

void UUIDrawcall::GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)const
{
	int count = renderObjectList.Num();
	if (count == 1)
	{
		auto uiGeo = renderObjectList[0]->GetGeometry();
		vertices = uiGeo->vertices;
		triangles = uiGeo->triangles;
	}
	else
	{
		int totalVertCount = 0;
		int totalTriangleIndicesCount = 0;
		for (int i = 0; i < count; i++)
		{
			auto uiGeo = renderObjectList[i]->GetGeometry();
			totalVertCount += uiGeo->vertices.Num();
			totalTriangleIndicesCount += uiGeo->triangles.Num();
		}
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		vertices.Reserve(totalVertCount);
		triangles.SetNumUninitialized(totalTriangleIndicesCount);
		for (int geoIndex = 0; geoIndex < count; geoIndex++)
		{
			auto uiGeo = renderObjectList[geoIndex]->GetGeometry();
			auto& geomTriangles = uiGeo->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;
			vertices.Append(uiGeo->vertices);
			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				auto triangleIndex = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
				triangles[triangleIndicesIndex++] = triangleIndex;
			}

			prevVertexCount += uiGeo->vertices.Num();
		}
	}
}
void UUIDrawcall::UpdateData(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)
{
	int count = renderObjectList.Num();
	if (count == 1)
	{
		auto uiGeo = renderObjectList[0]->GetGeometry();
		FMemory::Memcpy((uint8*)vertices.GetData(), uiGeo->vertices.GetData(), vertices.Num() * sizeof(FDynamicMeshVertex));
		FMemory::Memcpy((uint8*)triangles.GetData(), uiGeo->triangles.GetData(), triangles.Num() * sizeof(uint16));
	}
	else
	{
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		int vertBufferOffset = 0;
		for (int geoIndex = 0; geoIndex < count; geoIndex++)
		{
			auto uiGeo = renderObjectList[geoIndex]->GetGeometry();
			auto& geomTriangles = uiGeo->triangles;
			int triangleCount = geomTriangles.Num();
			if (triangleCount <= 0)continue;
			int vertCount = uiGeo->vertices.Num();
			int bufferSize = vertCount * sizeof(FDynamicMeshVertex);
			FMemory::Memcpy((uint8*)vertices.GetData() + vertBufferOffset, (uint8*)uiGeo->vertices.GetData(), bufferSize);
			vertBufferOffset += bufferSize;

			for (int geomTriangleIndicesIndex = 0; geomTriangleIndicesIndex < triangleCount; geomTriangleIndicesIndex++)
			{
				triangles[triangleIndicesIndex++] = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
			}

			prevVertexCount += vertCount;
		}
	}
}

void UUIDrawcall::CopyUpdateState(UUIDrawcall* Target)
{
	if (materialChanged)Target->materialChanged = true;
	if (textureChanged)Target->textureChanged = true;
	if (needToRebuildMesh)Target->needToRebuildMesh = true;
	if (needToUpdateVertex)Target->needToUpdateVertex = true;
	if (vertexPositionChanged)Target->vertexPositionChanged = true;
	if (needToAddPostProcessRenderProxyToRender)Target->needToAddPostProcessRenderProxyToRender = true;
}

