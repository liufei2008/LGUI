// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/UIDrawcall.h"
#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "DynamicMeshBuilder.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "Core/LGUISettings.h"

void UUIDrawcall::GetCombined(TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIMeshIndexBufferType>& triangles)const
{
	int count = RenderObjectList.Num();
	if (count == 1)
	{
		auto uiGeo = RenderObjectList[0]->GetGeometry();
		vertices = uiGeo->vertices;
		triangles = uiGeo->triangles;
	}
	else
	{
		int prevVertexCount = 0;
		int triangleIndicesIndex = 0;
		vertices.Reserve(this->VerticesCount);
		triangles.SetNumUninitialized(this->IndicesCount);
		for (int geoIndex = 0; geoIndex < count; geoIndex++)
		{
			auto uiGeo = RenderObjectList[geoIndex]->GetGeometry();
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
	if (bMaterialChanged)Target->bMaterialChanged = true;
	if (bTextureChanged)Target->bTextureChanged = true;
	if (bNeedToUpdateVertex)Target->bNeedToUpdateVertex = true;
	if (bVertexPositionChanged)Target->bVertexPositionChanged = true;
	if (bNeedToAddPostProcessRenderProxyToRender)Target->bNeedToAddPostProcessRenderProxyToRender = true;
}

bool UUIDrawcall::CanConsumeUIBatchGeometryRenderable(UIGeometry* geo, int32 itemVertCount)
{
	return this->Type == EUIDrawcallType::BatchGeometry
		&& this->Material == geo->material
		&& this->Texture == geo->texture
		&& this->VerticesCount + itemVertCount < LGUI_MAX_VERTEX_COUNT;
}
