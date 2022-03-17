// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		vertices.Reserve(this->verticesCount);
		triangles.SetNumUninitialized(this->indicesCount);
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

void UUIDrawcall::CopyUpdateState(UUIDrawcall* Target)
{
	if (materialChanged)Target->materialChanged = true;
	if (textureChanged)Target->textureChanged = true;
	if (needToUpdateVertex)Target->needToUpdateVertex = true;
	if (vertexPositionChanged)Target->vertexPositionChanged = true;
	if (needToAddPostProcessRenderProxyToRender)Target->needToAddPostProcessRenderProxyToRender = true;
	if (shouldSortRenderObjectList)Target->shouldSortRenderObjectList = true;
}

bool UUIDrawcall::CanConsumeUIBatchGeometryRenderable(UIGeometry* geo, int32 itemVertCount)
{
	return this->type == EUIDrawcallType::BatchGeometry
		&& this->material == geo->material
		&& this->texture == geo->texture
		&& this->verticesCount + itemVertCount < LGUI_MAX_VERTEX_COUNT;
}
