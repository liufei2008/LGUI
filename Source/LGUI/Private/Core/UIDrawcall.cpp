// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/UIDrawcall.h"
#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "DynamicMeshBuilder.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/LGUISettings.h"

void UUIDrawcall::GetCombined(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)const
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
				auto triangleIndex = geomTriangles[geomTriangleIndicesIndex] + prevVertexCount;
				triangles[triangleIndicesIndex++] = triangleIndex;
			}

			prevVertexCount += geometry->vertices.Num();
		}
	}
}
void UUIDrawcall::UpdateData(TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangles)
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
void UUIDrawcall::UpdateDepthRange()
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


void UUIDrawcall::Clear()
{
	geometryList.Empty();
	texture = nullptr;
	material = nullptr;
	materialInstanceDynamic = nullptr;

	postProcessObject = nullptr;

	renderObjectList.Empty();
	is3DDrawcall = false;
}
bool UUIDrawcall::Equals(UUIDrawcall* Other)
{
	return
		this->type == Other->type
		&& this->texture == Other->texture
		&& this->material == Other->material
		&& this->postProcessObject == Other->postProcessObject
		&& this->geometryList == Other->geometryList
		;
}

bool UUIDrawcall::CompareDrawcallList(const TArray<TSharedPtr<UUIDrawcall>>& A, const TArray<TSharedPtr<UUIDrawcall>>& B)
{
	if (A.Num() == B.Num())
	{
		for (int i = 0; i < A.Num(); i++)
		{
			if (!A[i]->Equals(B[i].Get()))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}
void UUIDrawcall::CopyDrawcallList(const TArray<TSharedPtr<UUIDrawcall>>& From, TArray<TSharedPtr<UUIDrawcall>>& To)
{
	auto prevDrawcallListCount = To.Num();
	int drawcallCount = 0;
	for (int i = 0; i < From.Num(); i++)
	{
		auto drawcall = GetAvailableDrawcall(To, prevDrawcallListCount, drawcallCount);
		auto fromDrawcall = From[i];


		drawcall->type = fromDrawcall->type;
		drawcall->geometryList = fromDrawcall->geometryList;
		drawcall->texture = fromDrawcall->texture;
		drawcall->material = fromDrawcall->material;
		drawcall->materialInstanceDynamic = fromDrawcall->materialInstanceDynamic;

		drawcall->postProcessObject = fromDrawcall->postProcessObject;

		drawcall->renderObjectList = fromDrawcall->renderObjectList;
		drawcall->is3DDrawcall = fromDrawcall->is3DDrawcall;
	}
	while (prevDrawcallListCount > drawcallCount)//delete needless drawcall
	{
		To.RemoveAt(prevDrawcallListCount - 1);
		prevDrawcallListCount--;
	}
}


void UUIDrawcall::CreateDrawcall(TArray<TWeakObjectPtr<UUIBaseRenderable>>& sortedList, TArray<TSharedPtr<UUIDrawcall>>& drawcallList)
{
	UTexture* prevTex = nullptr;
	TSharedPtr<UUIDrawcall> prevUIDrawcall = nullptr;
	int drawcallCount = 0;
	int prevDrawcallListCount = drawcallList.Num();
	for (int i = 0; i < sortedList.Num(); i++)
	{
		switch (sortedList[i]->GetUIRenderableType())
		{
		default:
		case EUIRenderableType::UIBatchGeometryRenderable:
		{
			auto sortedItem = (UUIBatchGeometryRenderable*)sortedList[i].Get();
			auto itemGeo = sortedItem->GetGeometry();
			if (itemGeo.IsValid() == false)continue;
			if (itemGeo->vertices.Num() == 0)continue;

			if (itemGeo->material.IsValid())//consider every custom material as a drawcall
			{
				prevUIDrawcall = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
				prevUIDrawcall->texture = itemGeo->texture;
				prevUIDrawcall->material = itemGeo->material;
				prevUIDrawcall->geometryList.Add(itemGeo);
				prevUIDrawcall->type = EUIDrawcallType::BatchGeometry;
				prevTex = nullptr;
			}
			else//batch elements into drawcall by comparing their texture
			{
				auto itemTex = itemGeo->texture;
				if (itemTex.Get() != prevTex)//this ui element's texture is different from previous one
				{
					prevUIDrawcall = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
					prevUIDrawcall->texture = itemTex;
					prevUIDrawcall->geometryList.Add(itemGeo);
					prevUIDrawcall->type = EUIDrawcallType::BatchGeometry;
				}
				else//same texture means same drawcall
				{
					if (!prevUIDrawcall.IsValid())//if first is null
					{
						prevUIDrawcall = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
						prevUIDrawcall->texture = itemTex;
						prevUIDrawcall->geometryList.Add(itemGeo);
					}
					else
					{
						prevUIDrawcall->geometryList.Add(itemGeo);
					}
					prevUIDrawcall->type = EUIDrawcallType::BatchGeometry;
				}
				prevTex = itemTex.Get();
			}
			itemGeo->drawcallIndex = drawcallCount - 1;
		}
		break;
		case EUIRenderableType::UIPostProcessRenderable:
		{
			auto sortedItem = (UUIPostProcessRenderable*)sortedList[i].Get();
			auto itemGeo = sortedItem->GetGeometry();
			if (itemGeo.IsValid() == false)continue;
			if (itemGeo->vertices.Num() == 0)continue;
			//every postprocess is a drawcall
			prevUIDrawcall = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
			prevUIDrawcall->postProcessObject = sortedItem;
			prevUIDrawcall->type = EUIDrawcallType::PostProcess;
			prevUIDrawcall = nullptr;
			prevTex = nullptr;
		}
		break;
		}
	}
	while (prevDrawcallListCount > drawcallCount)//delete needless drawcall
	{
		drawcallList.RemoveAt(prevDrawcallListCount - 1);
		prevDrawcallListCount--;
	}
}

void UUIDrawcall::CreateDrawcallForAutoManageDepth(TArray<TWeakObjectPtr<UUIBaseRenderable>>& sortedList, TArray<TSharedPtr<UUIDrawcall>>& drawcallList)
{
	auto IntersectBounds = [](FVector2D aMin, FVector2D aMax, FVector2D bMin, FVector2D bMax) {
		return !(bMin.X >= aMax.X
			|| bMax.X <= aMin.X
			|| bMax.Y <= aMin.Y
			|| bMin.Y >= aMax.Y
			);
	};
	auto OverlapWithOtherDrawcall = [IntersectBounds](UUIBatchGeometryRenderable* item, const FLGUICacheTransformContainer& itemToCanvasTf, TSharedPtr<UUIDrawcall> drawcallItem) {
		//calculate drawcall item's bounds
		for (auto otherItem : drawcallItem->renderObjectList)
		{
			FLGUICacheTransformContainer otherItemToCanvasTf;
			otherItem->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(otherItem.Get(), true, otherItemToCanvasTf);
			//check bounds overlap
			if (IntersectBounds(itemToCanvasTf.BoundsMin2D, itemToCanvasTf.BoundsMax2D, otherItemToCanvasTf.BoundsMin2D, otherItemToCanvasTf.BoundsMax2D))
			{
				return true;
			}
		}

		return false;
	};
	auto CanFitInDrawcall = [OverlapWithOtherDrawcall](UUIBatchGeometryRenderable* item, TArray<TSharedPtr<UUIDrawcall>>& drawcallList, int drawcallCount, int& resultDrawcallIndex, bool& is3DUIItem) {
		for (int i = drawcallCount - 1; i >= 0; i--)
		{
			auto drawcallItem = drawcallList[i];
			if (drawcallItem->is3DDrawcall)//auto-manage-depth only for 2d items
			{
				continue;
			}
			FLGUICacheTransformContainer itemToCanvasTf;
			item->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(item, true, itemToCanvasTf);
			if (!Is2DUITransform(itemToCanvasTf.Transform))//auto-manage-depth only for 2d items
			{
				is3DUIItem = true;
				return false;
			}
			if (drawcallItem->material.IsValid()
				|| drawcallItem->type == EUIDrawcallType::PostProcess
				|| drawcallItem->texture != item->GetGeometry()->texture
				)
			{
				if (OverlapWithOtherDrawcall(item, itemToCanvasTf, drawcallItem))//overlap with other drawcall
				{
					return false;
				}
				continue;//keep searching
			}
			resultDrawcallIndex = i;
			return true;//can fit in drawcall
		}
		return false;
	};

	int drawcallCount = 0;
	int prevDrawcallListCount = drawcallList.Num();
	for (int i = 0; i < sortedList.Num(); i++)
	{
		switch (sortedList[i]->GetUIRenderableType())
		{
		default:
		case EUIRenderableType::UIBatchGeometryRenderable:
		{
			auto sortedItem = (UUIBatchGeometryRenderable*)sortedList[i].Get();
			auto itemGeo = sortedItem->GetGeometry();
			if (itemGeo.IsValid() == false)continue;
			if (itemGeo->vertices.Num() == 0)continue;

			if (itemGeo->material.IsValid())//consider every custom material as a drawcall
			{
				auto drawcallItem = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
				drawcallItem->texture = itemGeo->texture;
				drawcallItem->material = itemGeo->material;
				drawcallItem->type = EUIDrawcallType::BatchGeometry;
				drawcallItem->geometryList.Add(itemGeo);
				drawcallItem->renderObjectList.Add(sortedItem);
				itemGeo->drawcallIndex = drawcallCount - 1;
			}
			else//batch elements into drawcall
			{
				int drawcallIndexToFitin; bool is3DUIItem = false;
				if (CanFitInDrawcall(sortedItem, drawcallList, drawcallCount, drawcallIndexToFitin, is3DUIItem))
				{
					auto drawcallItem = drawcallList[drawcallIndexToFitin];

					drawcallItem->geometryList.Add(itemGeo);
					drawcallItem->is3DDrawcall = false;
					drawcallItem->renderObjectList.Add(sortedItem);
					itemGeo->drawcallIndex = drawcallIndexToFitin;
				}
				else
				{
					auto drawcallItem = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
					drawcallItem->geometryList.Add(itemGeo);
					drawcallItem->is3DDrawcall = is3DUIItem;
					drawcallItem->renderObjectList.Add(sortedItem);

					drawcallItem->texture = itemGeo->texture;
					drawcallItem->type = EUIDrawcallType::BatchGeometry;
					itemGeo->drawcallIndex = drawcallCount - 1;
				}
			}
		}
		break;
		case EUIRenderableType::UIPostProcessRenderable:
		{
			auto sortedItem = (UUIPostProcessRenderable*)sortedList[i].Get();
			auto itemGeo = sortedItem->GetGeometry();
			if (itemGeo.IsValid() == false)continue;
			if (itemGeo->vertices.Num() == 0)continue;
			//every postprocess is a drawcall
			auto drawcall = GetAvailableDrawcall(drawcallList, prevDrawcallListCount, drawcallCount);
			drawcall->postProcessObject = sortedItem;
			drawcall->type = EUIDrawcallType::PostProcess;
		}
		break;
		}
	}
	while (prevDrawcallListCount > drawcallCount)//delete needless drawcall
	{
		drawcallList.RemoveAt(prevDrawcallListCount - 1);
		prevDrawcallListCount--;
	}
}

TSharedPtr<class UUIDrawcall> UUIDrawcall::GetAvailableDrawcall(TArray<TSharedPtr<UUIDrawcall>>& drawcallList, int& prevDrawcallListCount, int& drawcallCount)
{
	TSharedPtr<UUIDrawcall> result;
	drawcallCount++;
	if (drawcallCount > prevDrawcallListCount)
	{
		result = TSharedPtr<UUIDrawcall>(new UUIDrawcall());
		drawcallList.Add(result);
		prevDrawcallListCount++;
	}
	else
	{
		result = drawcallList[drawcallCount - 1];
		result->Clear();
	}
	return result;
}

bool UUIDrawcall::Is2DUITransform(const FTransform& Transform)
{
#if WITH_EDITOR
	float threshold = ULGUISettings::GetAutoManageDepthThreshold();
#else
	static float threshold = ULGUISettings::GetAutoManageDepthThreshold();
#endif
	if (FMath::Abs(Transform.GetLocation().Z) > threshold)//location Z moved
	{
		return false;
	}
	auto rotation = Transform.GetRotation().Rotator();
	if (FMath::Abs(rotation.Roll) > threshold || FMath::Abs(rotation.Pitch) > threshold)//rotate
	{
		return false;
	}
	return true;
}
