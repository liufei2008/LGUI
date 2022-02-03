﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "RichTextParser.h"


FORCEINLINE float RoundToFloat(float value)
{
	return FMath::FloorToFloat(value + 0.5f);
}

DECLARE_CYCLE_STAT(TEXT("UIGeometry TransformPixelPerfectVertices"), STAT_TransformPixelPerfectVertices, STATGROUP_LGUI);

void AdjustPixelPerfectPos(TArray<FVector>& originPositions, int startIndex, int count, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	for (int i = startIndex; i < count; i++)
	{
		auto item = originPositions[i];

		auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(item);
		canvasSpaceLocation.Y -= halfCanvasWidth;
		canvasSpaceLocation.Z -= halfCanvasHeight;
		float screenSpaceLocationY = canvasSpaceLocation.Y * rootCanvasScale;
		float screenSpaceLocationZ = canvasSpaceLocation.Z * rootCanvasScale;
		item.Y = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
		item.Z = RoundToFloat(screenSpaceLocationZ) * inv_RootCanvasScale;
		item.Y += halfCanvasWidth;
		item.Z += halfCanvasHeight;

		originPositions[i] = canvasToComponentTransform.TransformPosition(item);
	}
}
void AdjustPixelPerfectPos_For_UIRectFillRadial360(TArray<FVector>& originPositions, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	static TArray<int> vertArray = { 0, 2, 6, 8 };
	for (int i = 0; i < vertArray.Num(); i++)
	{
		int vertIndex = vertArray[i];
		auto originPos = originPositions[vertIndex];

		auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(originPos);
		canvasSpaceLocation.Y -= halfCanvasWidth;
		canvasSpaceLocation.Z -= halfCanvasHeight;
		float screenSpaceLocationY = canvasSpaceLocation.Y * rootCanvasScale;
		float screenSpaceLocationZ = canvasSpaceLocation.Z * rootCanvasScale;
		canvasSpaceLocation.Y = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
		canvasSpaceLocation.Z = RoundToFloat(screenSpaceLocationZ) * inv_RootCanvasScale;
		canvasSpaceLocation.Y += halfCanvasWidth;
		canvasSpaceLocation.Z += halfCanvasHeight;

		originPositions[vertIndex] = canvasToComponentTransform.TransformPosition(canvasSpaceLocation);
	}
}
void AdjustPixelPerfectPos_For_UIText(TArray<FVector>& originPositions, const TArray<FUITextCharProperty>& cacheCharPropertyArray, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	if (cacheCharPropertyArray.Num() <= 0)return;
	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	for (int i = 0; i < cacheCharPropertyArray.Num(); i++)
	{
		auto charProperty = cacheCharPropertyArray[i];
		int vertStartIndex = charProperty.StartVertIndex;
		int vertEndIndex = charProperty.StartVertIndex + charProperty.VertCount;

		//calculate first vert
		float offsetY, offsetZ;
		{
			auto originPos = originPositions[vertStartIndex];

			auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(originPos);
			canvasSpaceLocation.Y -= halfCanvasWidth;
			canvasSpaceLocation.Z -= halfCanvasHeight;
			float screenSpaceLocationX = canvasSpaceLocation.Y * rootCanvasScale;
			float screenSpaceLocationY = canvasSpaceLocation.Z * rootCanvasScale;
			canvasSpaceLocation.Y = RoundToFloat(screenSpaceLocationX) * inv_RootCanvasScale;
			canvasSpaceLocation.Z = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
			canvasSpaceLocation.Y += halfCanvasWidth;
			canvasSpaceLocation.Z += halfCanvasHeight;

			auto newPos = canvasToComponentTransform.TransformPosition(canvasSpaceLocation);
			originPositions[vertStartIndex] = newPos;
			offsetY = newPos.Y - originPos.Y;
			offsetZ = newPos.Z - originPos.Z;
		}

		for (int vertIndex = vertStartIndex + 1; vertIndex < vertEndIndex; vertIndex++)
		{
			auto& originPos = originPositions[vertIndex];
			originPos.Y += offsetY;
			originPos.Z += offsetZ;
		}
	}
}

#pragma region UISprite_UITexture_Simple
void UIGeometry::FromUIRectSimple(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 6;
		triangles.SetNumUninitialized(6);
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4;
		vertices.SetNumUninitialized(4);
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectSimpleVertex(uiGeo, width, height, pivot, spriteInfo, renderCanvas, uiComp);
	//uvs
	UpdateUIRectSimpleUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(4);
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(4);
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
	}
}

void UIGeometry::UpdateUIRectSimpleUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	vertices[0].TextureCoordinate[0] = spriteInfo.GetUV0();
	vertices[1].TextureCoordinate[0] = spriteInfo.GetUV1();
	vertices[2].TextureCoordinate[0] = spriteInfo.GetUV2();
	vertices[3].TextureCoordinate[0] = spriteInfo.GetUV3();
}
void UIGeometry::UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//offset and size
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float minX = -halfW + pivotOffsetX;
	float minY = -halfH + pivotOffsetY;
	float maxX = halfW + pivotOffsetX;
	float maxY = halfH + pivotOffsetY;
	originPositions[0] = FVector(0, minX, minY);
	originPositions[1] = FVector(0, maxX, minY);
	originPositions[2] = FVector(0, minX, maxY);
	originPositions[3] = FVector(0, maxX, maxY);
	//snap pixel
	if (renderCanvas->GetActualPixelPerfect())
	{
		AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount, renderCanvas, uiComp);
	}
}
#pragma endregion
#pragma region UISprite_UITexture_Border
void UIGeometry::FromUIRectBorder(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, bool fillCenter)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		if (fillCenter)
		{
			uiGeo->originTriangleCount = 54;
			triangles.Reserve(54);
		}
		else
		{
			uiGeo->originTriangleCount = 48;
			triangles.Reserve(48);
		}
		int wSeg = 3, hSeg = 3;
		int vStartIndex = 0;
		for (int h = 0; h < hSeg; h++)
		{
			for (int w = 0; w < wSeg; w++)
			{
				if (!fillCenter)
					if (h == 1 && w == 1)continue;
				int vIndex = vStartIndex + w;
				triangles.Add(vIndex);
				triangles.Add(vIndex + wSeg + 2);
				triangles.Add(vIndex + wSeg + 1);

				triangles.Add(vIndex);
				triangles.Add(vIndex + 1);
				triangles.Add(vIndex + wSeg + 2);
			}
			vStartIndex += wSeg + 1;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 16;
		vertices.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectBorderVertex(uiGeo, width, height, pivot, spriteInfo, renderCanvas, uiComp);
	//uvs
	UpdateUIRectBorderUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(16);
			for (int i = 0; i < 16; i++)
			{
				normals.Add(FVector(-1, 0, 0));
			}
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(16);
			for (int i = 0; i < 16; i++)
			{
				tangents.Add(FVector(0, 1, 0));
			}
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		float widthReciprocal = 1.0f / spriteInfo.width;
		float heightReciprocal = 1.0f / spriteInfo.height;
		float buv0X = spriteInfo.borderLeft * widthReciprocal;
		float buv3X = 1.0f - spriteInfo.borderRight * widthReciprocal;
		float buv0Y = 1.0f - spriteInfo.borderBottom * heightReciprocal;
		float buv3Y = spriteInfo.borderTop * heightReciprocal;

		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(buv0X, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(buv3X, 1);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 1);

		vertices[4].TextureCoordinate[1] = FVector2D(0, buv0Y);
		vertices[5].TextureCoordinate[1] = FVector2D(buv0X, buv0Y);
		vertices[6].TextureCoordinate[1] = FVector2D(buv3X, buv0Y);
		vertices[7].TextureCoordinate[1] = FVector2D(1, buv0Y);

		vertices[8].TextureCoordinate[1] = FVector2D(0, buv3Y);
		vertices[9].TextureCoordinate[1] = FVector2D(buv0X, buv3Y);
		vertices[10].TextureCoordinate[1] = FVector2D(buv3X, buv3Y);
		vertices[11].TextureCoordinate[1] = FVector2D(1, buv3Y);

		vertices[12].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[13].TextureCoordinate[1] = FVector2D(buv0X, 0);
		vertices[14].TextureCoordinate[1] = FVector2D(buv3X, 0);
		vertices[15].TextureCoordinate[1] = FVector2D(1, 0);
	}
}

void UIGeometry::UpdateUIRectBorderUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, spriteInfo.uv0Y);
	vertices[1].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, spriteInfo.uv0Y);
	vertices[2].TextureCoordinate[0] = FVector2D(spriteInfo.buv3X, spriteInfo.uv0Y);
	vertices[3].TextureCoordinate[0] = FVector2D(spriteInfo.uv3X, spriteInfo.uv0Y);

	vertices[4].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, spriteInfo.buv0Y);
	vertices[5].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, spriteInfo.buv0Y);
	vertices[6].TextureCoordinate[0] = FVector2D(spriteInfo.buv3X, spriteInfo.buv0Y);
	vertices[7].TextureCoordinate[0] = FVector2D(spriteInfo.uv3X, spriteInfo.buv0Y);

	vertices[8].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, spriteInfo.buv3Y);
	vertices[9].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, spriteInfo.buv3Y);
	vertices[10].TextureCoordinate[0] = FVector2D(spriteInfo.buv3X, spriteInfo.buv3Y);
	vertices[11].TextureCoordinate[0] = FVector2D(spriteInfo.uv3X, spriteInfo.buv3Y);

	vertices[12].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, spriteInfo.uv3Y);
	vertices[13].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, spriteInfo.uv3Y);
	vertices[14].TextureCoordinate[0] = FVector2D(spriteInfo.buv3X, spriteInfo.uv3Y);
	vertices[15].TextureCoordinate[0] = FVector2D(spriteInfo.uv3X, spriteInfo.uv3Y);
}
void UIGeometry::UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	float geoWidth = halfW * 2;
	float geoHeight = halfH * 2;
	//vertices
	float x0, x1, x2, x3, y0, y1, y2, y3;
	int widthBorder = spriteInfo.borderLeft + spriteInfo.borderRight;
	int heightBorder = spriteInfo.borderTop + spriteInfo.borderBottom;
	float widthScale = geoWidth < widthBorder ? geoWidth / widthBorder : 1.0f;
	float heightScale = geoHeight < heightBorder ? geoHeight / heightBorder : 1.0f;
	x0 = (-halfW + pivotOffsetX);
	x1 = (x0 + spriteInfo.borderLeft * widthScale);
	x3 = (halfW + pivotOffsetX);
	x2 = (x3 - spriteInfo.borderRight * widthScale);
	y0 = (-halfH + pivotOffsetY);
	y1 = (y0 + spriteInfo.borderBottom * heightScale);
	y3 = (halfH + pivotOffsetY);
	y2 = (y3 - spriteInfo.borderTop * heightScale);

	auto& originPositions = uiGeo->originPositions;
	originPositions[0] = FVector(0, x0, y0);
	originPositions[1] = FVector(0, x1, y0);
	originPositions[2] = FVector(0, x2, y0);
	originPositions[3] = FVector(0, x3, y0);

	originPositions[4] = FVector(0, x0, y1);
	originPositions[5] = FVector(0, x1, y1);
	originPositions[6] = FVector(0, x2, y1);
	originPositions[7] = FVector(0, x3, y1);

	originPositions[8] = FVector(0, x0, y2);
	originPositions[9] = FVector(0, x1, y2);
	originPositions[10] = FVector(0, x2, y2);
	originPositions[11] = FVector(0, x3, y2);

	originPositions[12] = FVector(0, x0, y3);
	originPositions[13] = FVector(0, x1, y3);
	originPositions[14] = FVector(0, x2, y3);
	originPositions[15] = FVector(0, x3, y3);

	//snap pixel
	if (renderCanvas->GetActualPixelPerfect())
	{
		AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount, renderCanvas, uiComp);
	}
}
#pragma endregion

#pragma region UISprite_Tiled
void UIGeometry::FromUIRectTiled(const float& width, const float& height, const FVector2D& pivot, const FColor& color, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	int rectangleCount = widthRectCount * heightRectCount;
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 6 * rectangleCount;
		triangles.AddUninitialized(uiGeo->originTriangleCount);
		for (int i = 0, j = 0, triangleIndicesIndex = 0; i < rectangleCount; i++, j+=4)
		{
			triangles[triangleIndicesIndex++] = j;
			triangles[triangleIndicesIndex++] = j + 3;
			triangles[triangleIndicesIndex++] = j + 2;
			triangles[triangleIndicesIndex++] = j;
			triangles[triangleIndicesIndex++] = j + 1;
			triangles[triangleIndicesIndex++] = j + 3;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4 * rectangleCount;
		vertices.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectTiledVertex(uiGeo, spriteInfo, renderCanvas, uiComp, width, height, pivot, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//uvs
	UpdateUIRectTiledUV(uiGeo, spriteInfo, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(uiGeo->originVerticesCount);
			for (int i = 0; i < uiGeo->originVerticesCount; i++)
			{
				normals.Add(FVector(-1, 0, 0));
			}
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(uiGeo->originVerticesCount);
			for (int i = 0; i < uiGeo->originVerticesCount; i++)
			{
				tangents.Add(FVector(0, 1, 0));
			}
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		for (int i = 0; i < uiGeo->originVerticesCount; i += 4)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 0);
			vertices[i + 1].TextureCoordinate[1] = FVector2D(1, 0);
			vertices[i + 2].TextureCoordinate[1] = FVector2D(0, 1);
			vertices[i + 3].TextureCoordinate[1] = FVector2D(1, 1);
		}
	}
}
void UIGeometry::UpdateUIRectTiledUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize)
{
	auto& vertices = uiGeo->vertices;

	int vertIndex = 0;
	float remainedUV3X = spriteInfo.buv0X + (spriteInfo.buv3X - spriteInfo.buv0X) * widthRemainedRectSize / spriteInfo.width;
	float remainedUV3Y = spriteInfo.buv0Y + (spriteInfo.buv3Y - spriteInfo.buv0Y) * heightRemainedRectSize / spriteInfo.height;
	for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
	{
		float realUV3Y = heightRectIndex == heightRectCount ? remainedUV3Y : spriteInfo.buv3Y;
		for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
		{
			float realUV3X = widthRectIndex == widthRectCount ? remainedUV3X : spriteInfo.buv3X;
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, spriteInfo.buv0Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(realUV3X, spriteInfo.buv0Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(spriteInfo.buv0X, realUV3Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(realUV3X, realUV3Y);
		}
	}
}
void UIGeometry::UpdateUIRectTiledVertex(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	int rectangleCount = widthRectCount * heightRectCount;
	auto& originPositions = uiGeo->originPositions;
	int vertIndex = 0;
	float startX = (-halfW + pivotOffsetX);
	float startY = (-halfH + pivotOffsetY);
	float x = startX, y = startY;
	for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
	{
		float realHeight = heightRectIndex == heightRectCount ? heightRemainedRectSize : spriteInfo.height;
		for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
		{
			float realWidth = widthRectIndex == widthRectCount ? (widthRemainedRectSize) : spriteInfo.width;
			originPositions[vertIndex++] = FVector(0, x, y);
			originPositions[vertIndex++] = FVector(0, x + realWidth, y);
			originPositions[vertIndex++] = FVector(0, x, y + realHeight);
			originPositions[vertIndex++] = FVector(0, x + realWidth, y + realHeight);
			
			x += spriteInfo.width;
		}
		x = startX;
		y += spriteInfo.height;
	}
	//snap pixel
	if (renderCanvas->GetActualPixelPerfect())
	{
		AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount, renderCanvas, uiComp);
	}
}
#pragma endregion

#pragma region UISprite_Fill_Horizontal_Vertial
void UIGeometry::FromUIRectFillHorizontalVertical(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 6;
		triangles.SetNumUninitialized(6);
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4;
		vertices.SetNumUninitialized(4);
	}
	//positions and uvs
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectFillHorizontalVerticalVertex(width, height, pivot, uiGeo, spriteInfo, flipDirection, fillAmount, horizontalOrVertical, true, true, renderCanvas, uiComp);
	//colors
	UpdateUIColor(uiGeo, color);

	//@todo: additional shader channels
	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(4);
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(4);
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
	}
}
void UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(const float& width, const float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount
	, bool horizontalOrVertical, bool updatePosition, bool updateUV
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float posMinX = -halfW + pivotOffsetX;
	float posMinY = -halfH + pivotOffsetY;
	float posMaxX = halfW + pivotOffsetX;
	float posMaxY = halfH + pivotOffsetY;
	//uvs
	auto& vertices = uiGeo->vertices;
	float uvMinX = spriteInfo.uv0X;
	float uvMinY = spriteInfo.uv0Y;
	float uvMaxX = spriteInfo.uv3X;
	float uvMaxY = spriteInfo.uv3Y;

	if (updatePosition)
	{
		originPositions[0] = FVector(0, posMinX, posMinY);
		originPositions[1] = FVector(0, posMaxX, posMinY);
		originPositions[2] = FVector(0, posMinX, posMaxY);
		originPositions[3] = FVector(0, posMaxX, posMaxY);

		//snap pixel
		if (renderCanvas->GetActualPixelPerfect())
		{
			AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount, renderCanvas, uiComp);

			posMinX = originPositions[0].X;
			posMinY = originPositions[0].Y;
			posMaxX = originPositions[3].X;
			posMaxY = originPositions[3].Y;
		}
	}
	if (horizontalOrVertical)
	{
		if (flipDirection)
		{
			if (updatePosition)
			{
				float value = FMath::Lerp(posMinX, posMaxX, fillAmount);
				originPositions[1].Y = originPositions[3].Y = value;
			}
			if (updateUV)
			{
				float value = FMath::Lerp(uvMinX, uvMaxX, fillAmount);
				vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
				vertices[1].TextureCoordinate[0] = FVector2D(value, uvMinY);
				vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
				vertices[3].TextureCoordinate[0] = FVector2D(value, uvMaxY);
			}
		}
		else
		{
			if (updatePosition)
			{
				float value = FMath::Lerp(posMaxX, posMinX, fillAmount);
				originPositions[0].Y = originPositions[2].Y = value;
			}
			if (updateUV)
			{
				float value = FMath::Lerp(uvMaxX, uvMinX, fillAmount);
				vertices[0].TextureCoordinate[0] = FVector2D(value, uvMinY);
				vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
				vertices[2].TextureCoordinate[0] = FVector2D(value, uvMaxY);
				vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
			}
		}
	}
	else
	{
		if (flipDirection)
		{
			if (updatePosition)
			{
				float value = FMath::Lerp(posMinY, posMaxY, fillAmount);
				originPositions[2].Z = originPositions[3].Z = value;
			}
			if (updateUV)
			{
				float value = FMath::Lerp(uvMinY, uvMaxY, fillAmount);
				vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
				vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
				vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, value);
				vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, value);
			}
		}
		else
		{
			if (updatePosition)
			{
				float value = FMath::Lerp(posMaxY, posMinY, fillAmount);
				originPositions[0].Z = originPositions[1].Z = value;
			}
			if (updateUV)
			{
				float value = FMath::Lerp(uvMaxY, uvMinY, fillAmount);
				vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, value);
				vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, value);
				vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
				vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
			}			
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial90
void UIGeometry::FromUIRectFillRadial90(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 6;
		triangles.SetNumUninitialized(6);
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4;
		vertices.SetNumUninitialized(4);
	}
	//positions and uvs
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectFillRadial90Vertex(width, height, pivot, uiGeo, spriteInfo, flipDirection, fillAmount, originType, true, true, renderCanvas, uiComp);
	//colors
	UpdateUIColor(uiGeo, color);

	//@todo: additional shader channels
	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(4);
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(4);
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
	}
}
void UIGeometry::UpdateUIRectFillRadial90Vertex(const float& width, const float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
	, bool updatePosition, bool updateUV
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float posMinX = -halfW + pivotOffsetX;
	float posMinY = -halfH + pivotOffsetY;
	float posMaxX = halfW + pivotOffsetX;
	float posMaxY = halfH + pivotOffsetY;
	//uvs
	auto& vertices = uiGeo->vertices;
	float uvMinX = spriteInfo.uv0X;
	float uvMinY = spriteInfo.uv0Y;
	float uvMaxX = spriteInfo.uv3X;
	float uvMaxY = spriteInfo.uv3Y;

	if (updatePosition)
	{
		originPositions[0] = FVector(0, posMinX, posMinY);
		originPositions[1] = FVector(0, posMaxX, posMinY);
		originPositions[2] = FVector(0, posMinX, posMaxY);
		originPositions[3] = FVector(0, posMaxX, posMaxY);
		//snap pixel
		if (renderCanvas->GetActualPixelPerfect())
		{
			AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount, renderCanvas, uiComp);

			posMinX = originPositions[0].X;
			posMinY = originPositions[0].Y;
			posMaxX = originPositions[3].X;
			posMaxY = originPositions[3].Y;
		}
	}
	switch (originType)
	{
	case UISpriteFillOriginType_Radial90::BottomLeft:
	{
		if (flipDirection)
		{	
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[1] = FVector(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[3] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[2] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[3] = FVector(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
				}
			}			
		}
	}
	break;
	case UISpriteFillOriginType_Radial90::TopLeft:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[0] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[1] = FVector(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[3] = FVector(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
				}
			}
			else
			{
				float lerpVaue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[1] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpVaue), posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpVaue), uvMinY);
				}
			}			
		}
	}
	break;
	case UISpriteFillOriginType_Radial90::TopRight:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[2] = FVector(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[0] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[1] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[0] = FVector(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
				}
				if (updateUV)
				{
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
				}
			}			
		}
	}
	break;
	case UISpriteFillOriginType_Radial90::BottomRight:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[3] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[2] = FVector(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
				}
			}
		}
		else
		{
			if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 2.0f;
				if (updatePosition)
				{
					originPositions[0] = FVector(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 2.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[2] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
				}
			}			
		}
	}
	break;
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial180
void UIGeometry::FromUIRectFillRadial180(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 9;
		triangles.SetNumUninitialized(9);
		switch (originType)
		{
		case UISpriteFillOriginType_Radial180::Bottom:
		{
			triangles[0] = 4;
			triangles[1] = 2;
			triangles[2] = 0;

			triangles[3] = 4;
			triangles[4] = 3;
			triangles[5] = 2;

			triangles[6] = 4;
			triangles[7] = 1;
			triangles[8] = 3;
		}
		break;
		case UISpriteFillOriginType_Radial180::Left:
		{
			triangles[0] = 4;
			triangles[1] = 3;
			triangles[2] = 2;

			triangles[3] = 4;
			triangles[4] = 1;
			triangles[5] = 3;

			triangles[6] = 4;
			triangles[7] = 0;
			triangles[8] = 1;
		}
		break;
		case UISpriteFillOriginType_Radial180::Top:
		{
			triangles[0] = 4;
			triangles[1] = 1;
			triangles[2] = 3;

			triangles[3] = 4;
			triangles[4] = 0;
			triangles[5] = 1;

			triangles[6] = 4;
			triangles[7] = 2;
			triangles[8] = 0;
		}
		break;
		case UISpriteFillOriginType_Radial180::Right:
		{
			triangles[0] = 4;
			triangles[1] = 0;
			triangles[2] = 1;

			triangles[3] = 4;
			triangles[4] = 2;
			triangles[5] = 0;

			triangles[6] = 4;
			triangles[7] = 3;
			triangles[8] = 2;
		}
		break;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 5;
		vertices.SetNumUninitialized(5);
	}
	//positions and uvs
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectFillRadial180Vertex(width, height, pivot, uiGeo, spriteInfo, flipDirection, fillAmount, originType, true, true, renderCanvas, uiComp);
	//colors
	UpdateUIColor(uiGeo, color);

	//@todo: additional shader channels
	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(5);
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(5);
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
		vertices[4].TextureCoordinate[1] = FVector2D(1, 0);
	}
}
void UIGeometry::UpdateUIRectFillRadial180Vertex(const float& width, const float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
	, bool updatePosition, bool updateUV
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float posMinX = -halfW + pivotOffsetX;
	float posMinY = -halfH + pivotOffsetY;
	float posMaxX = halfW + pivotOffsetX;
	float posMaxY = halfH + pivotOffsetY;
	//uvs
	auto& vertices = uiGeo->vertices;
	float uvMinX = spriteInfo.uv0X;
	float uvMinY = spriteInfo.uv0Y;
	float uvMaxX = spriteInfo.uv3X;
	float uvMaxY = spriteInfo.uv3Y;

	if (updatePosition)
	{
		originPositions[0] = FVector(0, posMinX, posMinY);
		originPositions[1] = FVector(0, posMaxX, posMinY);
		originPositions[2] = FVector(0, posMinX, posMaxY);
		originPositions[3] = FVector(0, posMaxX, posMaxY);
		//snap pixel
		if (renderCanvas->GetActualPixelPerfect())
		{
			AdjustPixelPerfectPos(originPositions, 0, uiGeo->originVerticesCount - 1, renderCanvas, uiComp);

			posMinX = originPositions[0].X;
			posMinY = originPositions[0].Y;
			posMaxX = originPositions[3].X;
			posMaxY = originPositions[3].Y;
		}
	}
	switch (originType)
	{
	case UISpriteFillOriginType_Radial180::Bottom:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = FVector(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[3] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[3] = originPositions[2] = FVector(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = FVector(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[2] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[2] = originPositions[3] = FVector(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMinY);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMinY);
				}
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial180::Left:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[1] = FVector(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[0] = originPositions[1] = originPositions[3] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[3] = FVector(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[3] = originPositions[1] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
					originPositions[4] = FVector(0, posMinX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMinX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial180::Top:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = FVector(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[0] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[2] = originPositions[0] = originPositions[1] = FVector(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = FVector(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[1] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[1] = originPositions[0] = FVector(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[2] = FVector(0, posMinX, posMaxY);
					originPositions[4] = FVector(0, (posMinX + posMaxX) * 0.5f, posMaxY);
				}
				if (updateUV)
				{
					vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D((uvMinX + uvMaxX) * 0.5f, uvMaxY);
				}
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial180::Right:
	{
		if (flipDirection)
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[2] = FVector(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[3] = originPositions[2] = originPositions[0] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
		}
		else
		{
			if (fillAmount >= 0.666666666f)
			{
				float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = FVector(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
					vertices[1].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else if (fillAmount >= 0.33333333f)
			{
				float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[0] = FVector(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					vertices[2].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
			else
			{
				float lerpValue = fillAmount * 3.0f;
				if (updatePosition)
				{
					originPositions[1] = originPositions[0] = originPositions[2] = FVector(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
					originPositions[4] = FVector(0, posMaxX, (posMinY + posMaxY) * 0.5f);
				}
				if (updateUV)
				{
					vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2D(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
					vertices[3].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
					vertices[4].TextureCoordinate[0] = FVector2D(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
				}
			}
		}
	}
	break;
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial360
void UIGeometry::FromUIRectFillRadial360(const float& width, const float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = 24;
		triangles.SetNumUninitialized(24);

		triangles[0] = 4;
		triangles[1] = 1;
		triangles[2] = 2;

		triangles[3] = 4;
		triangles[4] = 0;
		triangles[5] = 1;

		triangles[6] = 4;
		triangles[7] = 3;
		triangles[8] = 0;

		triangles[9] = 4;
		triangles[10] = 6;
		triangles[11] = 3;

		triangles[12] = 4;
		triangles[13] = 7;
		triangles[14] = 6;

		triangles[15] = 4;
		triangles[16] = 8;
		triangles[17] = 7;

		triangles[18] = 4;
		triangles[19] = 5;
		triangles[20] = 8;

		triangles[21] = 4;
		triangles[22] = 2;
		triangles[23] = 5;

		switch (originType)
		{
		case UISpriteFillOriginType_Radial360::Bottom:
			triangles[1] = 9;
			break;
		case UISpriteFillOriginType_Radial360::Right:
			triangles[19] = 9;
			break;
		case UISpriteFillOriginType_Radial360::Top:
			triangles[13] = 9;
			break;
		case UISpriteFillOriginType_Radial360::Left:
			triangles[7] = 9;
			break;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 10;
		vertices.SetNumUninitialized(10);
	}
	//positions and uvs
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRectFillRadial360Vertex(width, height, pivot, uiGeo, spriteInfo, flipDirection, fillAmount, originType, true, true, renderCanvas, uiComp);
	//colors
	UpdateUIColor(uiGeo, color);

	//@todo: additional shader channels
	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(4);
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(4);
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
			tangents.Add(FVector(0, 1, 0));
		}
	}
	//uvs1
	if (renderCanvas->GetRequireUV1())
	{
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
	}
}
void UIGeometry::UpdateUIRectFillRadial360Vertex(const float& width, const float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
	, bool updatePosition, bool updateUV
	, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float posMinX = -halfW + pivotOffsetX;
	float posMinY = -halfH + pivotOffsetY;
	float posMaxX = halfW + pivotOffsetX;
	float posMaxY = halfH + pivotOffsetY;
	float posHalfX = (posMinX + posMaxX) * 0.5f;
	float posHalfY = (posMinY + posMaxY) * 0.5f;
	//uvs
	auto& vertices = uiGeo->vertices;
	float uvMinX = spriteInfo.uv0X;
	float uvMinY = spriteInfo.uv0Y;
	float uvMaxX = spriteInfo.uv3X;
	float uvMaxY = spriteInfo.uv3Y;
	float uvHalfX = (uvMinX + uvMaxX) * 0.5f;
	float uvHalfY = (uvMinY + uvMaxY) * 0.5f;

	//reset position
	{
		originPositions[0] = FVector(0, posMinX, posMinY);
		originPositions[2] = FVector(0, posMaxX, posMinY);
		originPositions[6] = FVector(0, posMinX, posMaxY);
		originPositions[8] = FVector(0, posMaxX, posMaxY);
		//snap pixel
		if (renderCanvas->GetActualPixelPerfect())
		{
			AdjustPixelPerfectPos_For_UIRectFillRadial360(originPositions, renderCanvas, uiComp);

			posMinX = originPositions[0].Y;
			posMaxX = originPositions[2].Y;
			posMinY = originPositions[0].Z;
			posMaxY = originPositions[6].Z;
			posHalfX = (posMinX + posMaxX) * 0.5f;
			posHalfY = (posMinY + posMaxY) * 0.5f;
		}

		originPositions[1] = FVector(0, posHalfX, posMinY);
		originPositions[3] = FVector(0, posMinX, posHalfY);
		originPositions[4] = FVector(0, posHalfX, posHalfY);
		originPositions[5] = FVector(0, posMaxX, posHalfY);
		originPositions[7] = FVector(0, posHalfX, posMaxY);
	}
	//reset uv
	{
		vertices[0].TextureCoordinate[0] = FVector2D(uvMinX, uvMinY);
		vertices[1].TextureCoordinate[0] = FVector2D(uvHalfX, uvMinY);
		vertices[2].TextureCoordinate[0] = FVector2D(uvMaxX, uvMinY);
		vertices[3].TextureCoordinate[0] = FVector2D(uvMinX, uvHalfY);
		vertices[4].TextureCoordinate[0] = FVector2D(uvHalfX, uvHalfY);
		vertices[5].TextureCoordinate[0] = FVector2D(uvMaxX, uvHalfY);
		vertices[6].TextureCoordinate[0] = FVector2D(uvMinX, uvMaxY);
		vertices[7].TextureCoordinate[0] = FVector2D(uvHalfX, uvMaxY);
		vertices[8].TextureCoordinate[0] = FVector2D(uvMaxX, uvMaxY);
	}

	auto setPosAndUv = [&](int changeIndex, bool xory, float posFrom, float uvFrom, float lerpValue, const TArray<int>& inVertIndexArray) {
		auto& pos = originPositions[changeIndex];
		auto& uv = vertices[changeIndex].TextureCoordinate[0];
		if (xory)
		{
			pos.Y = FMath::Lerp(posFrom, pos.Y, lerpValue);
			uv.X = FMath::Lerp(uvFrom, uv.X, lerpValue);
		}
		else
		{
			pos.Z = FMath::Lerp(posFrom, pos.Z, lerpValue);
			uv.Y = FMath::Lerp(uvFrom, uv.Y, lerpValue);
		}
		for (int i : inVertIndexArray)
		{
			originPositions[i] = pos;
			vertices[i].TextureCoordinate[0] = uv;
		}
	};
	switch (originType)
	{
	case UISpriteFillOriginType_Radial360::Bottom:
	{
		originPositions[9] = originPositions[1];
		vertices[9].TextureCoordinate[0] = vertices[1].TextureCoordinate[0];
		if (flipDirection)
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(9, true, posMaxX, uvMaxX, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(2, false, posHalfY, uvHalfY, lerpValue, {9});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(5, false, posMaxY, uvMaxY, lerpValue, { 9, 2 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, { 9, 2, 5});
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(7, true, posMinX, uvMinX, lerpValue, { 9, 2, 5, 8 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(6, false, posHalfY, uvHalfY, lerpValue, { 9, 2, 5, 8, 7 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(3, false, posMinY, uvMinY, lerpValue, { 9, 2, 5, 8, 7, 6 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(0, true, posHalfX, uvHalfX, lerpValue, { 9, 2, 5, 8, 7, 6, 3 });
			}
		}
		else
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(1, true, posMinX, uvMinX, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(0, false, posHalfY, uvHalfY, lerpValue, {1});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(3, false, posMaxY, uvMaxY, lerpValue, { 1, 0 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(6, true, posHalfX, uvHalfX, lerpValue, { 1, 0, 3 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(7, true, posMaxX, uvMaxX, lerpValue, { 1, 0, 3, 6 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(8, false, posHalfY, uvHalfY, lerpValue, { 1, 0, 3, 6, 7 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(5, false, posMinY, uvMinY, lerpValue, { 1, 0, 3, 6, 7, 8 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(2, true, posHalfX, uvHalfX, lerpValue, { 1, 0, 3, 6, 7, 8, 5 });
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial360::Right:
	{
		originPositions[9] = originPositions[5];
		vertices[9].TextureCoordinate[0] = vertices[5].TextureCoordinate[0];
		if (flipDirection)
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(9, false, posMaxY, uvMaxY, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, {9});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(7, true, posMinX, uvMinX, lerpValue, { 9, 8 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(6, false, posHalfY, uvHalfY, lerpValue, { 9, 8, 7 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(3, false, posMinY, uvMinY, lerpValue, { 9, 8, 7, 6 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(0, true, posHalfX, uvHalfX, lerpValue, { 9, 8, 7, 6, 3 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(1, true, posMaxX, uvMaxX, lerpValue, { 9, 8, 7, 6, 3, 0 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(2, false, posHalfY, uvHalfY, lerpValue, { 9, 8, 7, 6, 3, 0, 1 });
			}
		}
		else
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(5, false, posMinY, uvMinY, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(2, true, posHalfX, uvHalfX, lerpValue, {5});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(1, true, posMinX, uvMinX, lerpValue, { 5, 2 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(0, false, posHalfY, uvHalfY, lerpValue, { 5, 2, 1 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(3, false, posMaxY, uvMaxY, lerpValue, { 5, 2, 1, 0 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(6, true, posHalfX, uvHalfX, lerpValue, { 5, 2, 1, 0, 3 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(7, true, posMaxX, uvMaxX, lerpValue, { 5, 2, 1, 0, 3, 6 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(8, false, posHalfY, uvHalfY, lerpValue, { 5, 2, 1, 0, 3, 6, 7 });
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial360::Top:
	{
		originPositions[9] = originPositions[7];
		vertices[9].TextureCoordinate[0] = vertices[7].TextureCoordinate[0];
		if (flipDirection)
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(9, true, posMinX, uvMinX, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(6, false, posHalfY, uvHalfY, lerpValue, {9});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(3, false, posMinY, uvMinY, lerpValue, { 9, 6 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(0, true, posHalfX, uvHalfX, lerpValue, { 9, 6, 3 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(1, true, posMaxX, uvMaxX, lerpValue, { 9, 6, 3, 0 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(2, false, posHalfY, uvHalfY, lerpValue, { 9, 6, 3, 0, 1 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(5, false, posMaxY, uvMaxY, lerpValue, { 9, 6, 3, 0, 1, 2 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, { 9, 6, 3, 0, 1, 2, 5 });
			}
		}
		else
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(7, true, posMaxX, uvMaxX, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(8, false, posHalfY, uvHalfY, lerpValue, {7});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(5, false, posMinY, uvMinY, lerpValue, { 7, 8 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(2, true, posHalfX, uvHalfX, lerpValue, { 7, 8, 5 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(1, true, posMinX, uvMinX, lerpValue, { 7, 8, 5, 2 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(0, false, posHalfY, uvHalfY, lerpValue, { 7, 8, 5, 2, 1 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(3, false, posMaxY, uvMaxY, lerpValue, { 7, 8, 5, 2, 1, 0 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(6, true, posHalfX, uvHalfX, lerpValue, { 7, 8, 5, 2, 1, 0, 3 });
			}
		}
	}
	break;
	case UISpriteFillOriginType_Radial360::Left:
	{
		originPositions[9] = originPositions[3];
		vertices[9].TextureCoordinate[0] = vertices[3].TextureCoordinate[0];
		if (flipDirection)
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(9, false, posMinY, uvMinY, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(0, true, posHalfX, uvHalfX, lerpValue, {9});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(1, true, posMaxX, uvMaxX, lerpValue, { 9, 0 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(2, false, posHalfY, uvHalfY, lerpValue, { 9, 0, 1 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(5, false, posMaxY, uvMaxY, lerpValue, { 9, 0, 1, 2 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, { 9, 0, 1, 2, 5 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(7, true, posMinX, uvMinX, lerpValue, { 9, 0, 1, 2, 5, 8 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(6, false, posHalfY, uvHalfY, lerpValue, { 9, 0, 1, 2, 5, 8, 7 });
			}
		}
		else
		{
			if (fillAmount >= 0.875f)
			{
				float lerpValue = (fillAmount - 0.875f) * 8.0f;
				setPosAndUv(3, false, posMaxY, uvMaxY, lerpValue, {});
			}
			else if (fillAmount >= 0.75f)
			{
				float lerpValue = (fillAmount - 0.75f) * 8.0f;
				setPosAndUv(6, true, posHalfX, uvHalfX, lerpValue, {3});
			}
			else if (fillAmount >= 0.625f)
			{
				float lerpValue = (fillAmount - 0.625f) * 8.0f;
				setPosAndUv(7, true, posMaxX, uvMaxX, lerpValue, { 3, 6 });
			}
			else if (fillAmount >= 0.5f)
			{
				float lerpValue = (fillAmount - 0.5f) * 8.0f;
				setPosAndUv(8, false, posHalfY, uvHalfY, lerpValue, { 3, 6, 7 });
			}
			else if (fillAmount >= 0.375f)
			{
				float lerpValue = (fillAmount - 0.375f) * 8.0f;
				setPosAndUv(5, false, posMinY, uvMinY, lerpValue, { 3, 6, 7, 8 });
			}
			else if (fillAmount >= 0.25f)
			{
				float lerpValue = (fillAmount - 0.25f) * 8.0f;
				setPosAndUv(2, true, posHalfX, uvHalfX, lerpValue, { 3, 6, 7, 8, 5 });
			}
			else if (fillAmount >= 0.125f)
			{
				float lerpValue = (fillAmount - 0.125f) * 8.0f;
				setPosAndUv(1, true, posMinX, uvMinX, lerpValue, { 3, 6, 7, 8, 5, 2 });
			}
			else
			{
				float lerpValue = fillAmount * 8.0f;
				setPosAndUv(0, false, posHalfY, uvHalfY, lerpValue, { 3, 6, 7, 8, 5, 2, 1 });
			}
		}
	}
	break;
	}
}
#pragma endregion

#pragma region UIText
void UIGeometry::FromUIText(const FString& content, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
	, const FColor& color, const FVector2D& fontSpace, TSharedPtr<UIGeometry> uiGeo, const float& fontSize
	, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
	, bool adjustWidth, bool adjustHeight
	, UITextFontStyle fontStyle, FVector2D& textRealSize
	, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, TArray<FUITextLineProperty>& cacheTextPropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
	, ULGUIFontData_BaseObject* font, bool richText)
{
	//vertex and triangle
	UpdateUIText(content, visibleCharCount, width, height, pivot, color, fontSpace, uiGeo, fontSize, paragraphHAlign, paragraphVAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, renderCanvas, uiComp, cacheTextPropertyArray, cacheCharPropertyArray, cacheRichTextCustomTagArray, font, richText);

	int32 vertexCount = uiGeo->originVerticesCount;
	//normals
	if (renderCanvas->GetRequireNormal())
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				normals.Add(FVector(-1, 0, 0));
			}
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				tangents.Add(FVector(0, 1, 0));
			}
		}
	}
	//uv1
	if (renderCanvas->GetRequireUV1())
	{
		auto& vertices = uiGeo->vertices;
		for (int i = 0; i < vertexCount; i+=4)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
			vertices[i + 1].TextureCoordinate[1] = FVector2D(1, 1);
			vertices[i + 2].TextureCoordinate[1] = FVector2D(0, 0);
			vertices[i + 3].TextureCoordinate[1] = FVector2D(1, 0);
		}
	}
}

//single char geometry
struct FUITextCharGeometry
{
	float geoWidth = 0;
	float geoHeight = 0;
	float xadvance = 0;
	float xoffset = 0;
	float yoffset = 0;

	FVector2D uv0 = FVector2D(0, 0);
	FVector2D uv1 = FVector2D(0, 0);
	FVector2D uv2 = FVector2D(0, 0);
	FVector2D uv3 = FVector2D(0, 0);
};

void UIGeometry::UpdateUIText(const FString& text, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
	, const FColor& color, const FVector2D& fontSpace, TSharedPtr<UIGeometry> uiGeo, const float& fontSize
	, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
	, bool adjustWidth, bool adjustHeight
	, UITextFontStyle fontStyle, FVector2D& textRealSize
	, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, TArray<FUITextLineProperty>& cacheTextPropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
	, ULGUIFontData_BaseObject* font, bool richText)
{
	FString content = text;

	bool pixelPerfect = renderCanvas->GetActualPixelPerfect();
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float dynamicPixelsPerUnit = renderCanvas->GetActualDynamicPixelsPerUnit() * rootCanvasScale;
	float oneDivideRootCanvasScale = 1.0f / rootCanvasScale;
	float oneDivideDynamicPixelsPerUnit = 1.0f / dynamicPixelsPerUnit;
	bool shouldScaleFontSizeWithRootCanvas = false;
	bool isWorldSpace = renderCanvas->GetRootCanvas()->IsRenderToWorldSpace();
	if (isWorldSpace)
	{
		pixelPerfect = false;
		if (dynamicPixelsPerUnit != 1.0f)
		{
			shouldScaleFontSizeWithRootCanvas = true;
		}
	}
	else
	{
		if (rootCanvasScale != 1.0f)
		{
			shouldScaleFontSizeWithRootCanvas = true;
		}
		else
		{
			if (dynamicPixelsPerUnit != 1.0f)
			{
				shouldScaleFontSizeWithRootCanvas = true;
			}
		}
	}

	bool bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
	float boldSize = fontSize * font->GetBoldRatio();
	bool italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
	float italicSlop = FMath::Tan(FMath::DegreesToRadians(font->GetItalicAngle()));

	//rich text
	using namespace LGUIRichTextParser;
	static RichTextParser richTextParser;
	RichTextParseResult richTextParseResult;
	if (richText)
	{
		richTextParser.Clear();
		richTextParser.Prepare(fontSize, color, bold, italic, richTextParseResult);
	}

	auto GetCharFixedOffset = [font](float inFontSize)
	{
		return inFontSize * (font->GetFixedVerticalOffset());
	};
	float charFixedOffset = GetCharFixedOffset(fontSize);//some font may not render at vertical center, use this to mofidy it. 0.25 * size is tested value for most fonts

	cacheTextPropertyArray.Reset();
	cacheCharPropertyArray.Reset();
	cacheRichTextCustomTagArray.Reset();
	int contentLength = content.Len();
	FVector2D currentLineOffset(0, 0);
	float currentLineWidth = 0, currentLineHeight = fontSize, paragraphHeight = 0;//single line width, height, all line height
	float firstLineHeight = fontSize;//first line height
	float maxLineWidth = 0;//if have multiple line
	int lineUIGeoVertStart = 0;//vertex index in originPositions of current line
	int visibleCharIndex = 0;//visible char index, skip invisible char(\r,\n,\t)
	FUITextLineProperty sentenceProperty;
	FVector2D caretPosition(0, 0);
	float halfFontSize = fontSize * 0.5f;
	float halfFontSpaceX = fontSpace.X * 0.5f;
	int linesCount = 0;//how many lines, default is 1

	int verticesCount = 0;
	auto& originPositions = uiGeo->originPositions;
	auto& vertices = uiGeo->vertices;
	int indicesCount = 0;
	auto& triangles = uiGeo->triangles;

	auto CheckVertices = [&](int additionalVerticesCount)
	{
		int32 newCount = verticesCount + additionalVerticesCount;
		if (originPositions.Num() < newCount)
		{
			originPositions.AddUninitialized(newCount - originPositions.Num());
		}
		if (vertices.Num() < newCount)
		{
			vertices.AddUninitialized(newCount - vertices.Num());
		}
	};

	auto CheckIndices = [&](int additionalIndicesCount)
	{
		int32 newCount = indicesCount + additionalIndicesCount;
		if (triangles.Num() < newCount)
		{
			triangles.AddUninitialized(newCount - triangles.Num());
		}
	};
	 
	enum class NewLineMode
	{
		None,//not new line
		LineBreak,//this new line come from line break
		Space,//this new line come from space char
		Overflow,//this new line come from overflow
	};
	NewLineMode newLineMode = NewLineMode::None;

	auto NewLine = [&](int32 charIndex)
	{
		//add end caret position
		currentLineWidth -= fontSpace.X;//last char of a line don't need space
		maxLineWidth = FMath::Max(maxLineWidth, currentLineWidth);

		if (richText)
		{
			AlignUITextLineVertexForRichText(paragraphHAlign, currentLineWidth, currentLineHeight, fontSize, lineUIGeoVertStart, originPositions);
		}
		else
		{
			FUITextCaretProperty caretProperty;
			caretProperty.caretPosition = caretPosition;
			caretProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(caretProperty);

			AlignUITextLineVertex(paragraphHAlign, currentLineWidth, lineUIGeoVertStart, originPositions, sentenceProperty);

			cacheTextPropertyArray.Add(sentenceProperty);
			sentenceProperty = FUITextLineProperty();
		}
		lineUIGeoVertStart = verticesCount;

		currentLineWidth = 0;
		currentLineOffset.X = 0;
		currentLineOffset.Y -= (richText ? currentLineHeight : fontSize) + fontSpace.Y;
		paragraphHeight += (richText ? currentLineHeight : fontSize) + fontSpace.Y;
		linesCount++;

		//set caret position for empty newline
		if (!richText)
		{
			caretPosition.X = currentLineOffset.X - halfFontSpaceX;
			caretPosition.Y = currentLineOffset.Y;
		}
		//store first line height for paragraph align
		if (linesCount == 1)
		{
			firstLineHeight = richText ? currentLineHeight : fontSize;
		}
		//set line height to origin
		currentLineHeight = fontSize;
	};

	auto GetCharGeo = [&](TCHAR charCode, int inFontSize)
	{
		FUITextCharGeometry charGeo;
		if (charCode == ' ')
		{
			charGeo.xadvance = inFontSize * 0.5f;
			charGeo.geoWidth = charGeo.geoHeight = 0;
			charGeo.xoffset = charGeo.yoffset = 0;
			charGeo.uv0 = charGeo.uv1 = charGeo.uv2 = charGeo.uv3 = FVector2D(1, 1);
		}
		else if (charCode == '\t')
		{
			charGeo.xadvance = inFontSize + inFontSize;
			charGeo.geoWidth = charGeo.geoHeight = 0;
			charGeo.xoffset = charGeo.yoffset = 0;
			charGeo.uv0 = charGeo.uv1 = charGeo.uv2 = charGeo.uv3 = FVector2D(1, 1);
		}
		else
		{
			auto charData = font->GetCharData(charCode, (uint16)inFontSize);
			float calculatedCharFixedOffset = richText ? GetCharFixedOffset(inFontSize) : charFixedOffset;

			auto overrideCharData = charData;
			if (shouldScaleFontSizeWithRootCanvas)
			{
				if (pixelPerfect)
				{
					inFontSize = inFontSize * rootCanvasScale;
					inFontSize = FMath::Clamp(inFontSize, 0, 200);//limit font size to 200. too large font size will result in extream large texture
					overrideCharData = font->GetCharData(charCode, inFontSize);

					charGeo.geoWidth = overrideCharData.width * oneDivideRootCanvasScale;
					charGeo.geoHeight = overrideCharData.height * oneDivideRootCanvasScale;
					charGeo.xadvance = overrideCharData.xadvance * oneDivideRootCanvasScale;
					charGeo.xoffset = overrideCharData.xoffset * oneDivideRootCanvasScale;
					charGeo.yoffset = overrideCharData.yoffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
				}
				else if (dynamicPixelsPerUnit != 1.0f)
				{
					inFontSize = inFontSize * dynamicPixelsPerUnit;
					inFontSize = FMath::Clamp(inFontSize, 0, 200);//limit font size to 200. too large font size will result in extream large texture
					overrideCharData = font->GetCharData(charCode, inFontSize);

					charGeo.geoWidth = overrideCharData.width * oneDivideDynamicPixelsPerUnit;
					charGeo.geoHeight = overrideCharData.height * oneDivideDynamicPixelsPerUnit;
					charGeo.xadvance = overrideCharData.xadvance * oneDivideDynamicPixelsPerUnit;
					charGeo.xoffset = overrideCharData.xoffset * oneDivideDynamicPixelsPerUnit;
					charGeo.yoffset = overrideCharData.yoffset * oneDivideDynamicPixelsPerUnit + calculatedCharFixedOffset;
				}
				else
				{
					inFontSize = inFontSize * rootCanvasScale;
					inFontSize = FMath::Clamp(inFontSize, 0, 200);//limit font size to 200. too large font size will result in large texture
					overrideCharData = font->GetCharData(charCode, inFontSize);

					charGeo.geoWidth = overrideCharData.width * oneDivideRootCanvasScale;
					charGeo.geoHeight = overrideCharData.height * oneDivideRootCanvasScale;
					charGeo.xadvance = overrideCharData.xadvance * oneDivideRootCanvasScale;
					charGeo.xoffset = overrideCharData.xoffset * oneDivideRootCanvasScale;
					charGeo.yoffset = overrideCharData.yoffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
				}
			}
			else
			{
				charGeo.geoWidth = charData.width;
				charGeo.geoHeight = charData.height;
				charGeo.xadvance = charData.xadvance;
				charGeo.xoffset = charData.xoffset;
				charGeo.yoffset = charData.yoffset + calculatedCharFixedOffset;
			}

			charGeo.uv0 = overrideCharData.GetUV0();
			charGeo.uv1 = overrideCharData.GetUV1();
			charGeo.uv2 = overrideCharData.GetUV2();
			charGeo.uv3 = overrideCharData.GetUV3();
		}
		return charGeo;
	};
	auto GetCharGeoXAdv = [&](TCHAR charCode, int overrideFontSize)
	{
		if (charCode == ' ')
		{
			return overrideFontSize * 0.5f;
		}
		else if (charCode == '\t')
		{
			return (float)(overrideFontSize + overrideFontSize);
		}
		else
		{
			auto charData = font->GetCharData(charCode, overrideFontSize);
			return charData.xadvance;
		}
	};
	auto GetUnderlineOrStrikethroughCharGeo = [&](TCHAR charCode, int overrideFontSize)
	{
		FUITextCharGeometry charGeo;
		{
			auto charData = font->GetCharData(charCode, (uint16)overrideFontSize);
			charGeo.geoHeight = charData.height;
			charGeo.xoffset = charData.xoffset;
			charGeo.yoffset = charData.yoffset + GetCharFixedOffset(overrideFontSize);

			float uvX = (charData.uv3X - charData.uv0X) * 0.5f + charData.uv0X;
			charGeo.uv0 = charGeo.uv1 = FVector2D(uvX, charData.uv0Y);
			charGeo.uv2 = charGeo.uv3 = FVector2D(uvX, charData.uv3Y);
		}
		return charGeo;
	};

	//pre parse rich text
	FString richTextContent;
	richTextContent.Reserve(content.Len());
	static TArray<RichTextParseResult> richTextPropertyArray;
	richTextPropertyArray.Reset();
	if (richText)
	{
		for (int charIndex = 0; charIndex < contentLength; charIndex++)
		{
			auto charCode = content[charIndex];
			richTextParseResult.customTag = NAME_None;
			richTextParseResult.customTagMode = CustomTagMode::None;
			while (richTextParser.Parse(content, contentLength, charIndex, richTextParseResult))
			{
				if (charIndex < contentLength)
				{
					charCode = content[charIndex];
				}
				else
				{
					break;
				}
			}
			//if find end symbol, then mark the prev one as end
			if (richTextParseResult.customTagMode == LGUIRichTextParser::CustomTagMode::End)
			{
				auto& last = richTextPropertyArray[richTextPropertyArray.Num() - 1];
				last.customTag = richTextParseResult.customTag;
				last.customTagMode = richTextParseResult.customTagMode;
				richTextParseResult.customTag = NAME_None;
				richTextParseResult.customTagMode = LGUIRichTextParser::CustomTagMode::None;
			}

			if (charIndex >= contentLength)break;
			richTextContent.AppendChar(charCode);
			richTextPropertyArray.Add(richTextParseResult);
		}
		//replace text content with parsed rich text content
		content = richTextContent;
		contentLength = richTextContent.Len();
	}

	bool haveClampContent = false;
	int clamp_RestVerticesCount = 0;
	float clamp_CurrentLineWidth = 0;
	float clamp_ParagraphHeight = 0;
	for (int charIndex = 0; charIndex < contentLength; charIndex++)
	{
		auto charCode = content[charIndex];
		if (richText)
		{
			richTextParseResult = richTextPropertyArray[charIndex];
		}

		if (charCode == '\n' || charCode == '\r')//10 -- \n, 13 -- \r
		{
			NewLine(charIndex);
			if (charIndex + 1 < contentLength)
			{
				auto nextCharCode = content[charIndex + 1];
				if ((charCode == '\r' && nextCharCode == '\n') || (charCode == '\n' && nextCharCode == '\r'))
				{
					charIndex++;//\n\r or \r\n
				}
			}
			newLineMode = NewLineMode::LineBreak;
			continue;
		}

		if (newLineMode == NewLineMode::Space || newLineMode == NewLineMode::Overflow)
		{
			if (charCode == ' ')
			{
				if (newLineMode == NewLineMode::Overflow)
				{
					newLineMode = NewLineMode::None;
				}
				continue;
			}
			else
			{
				newLineMode = NewLineMode::None;
			}
		}
		
		auto charGeo = richText 
			? GetCharGeo(charCode, richTextParseResult.size)
			: GetCharGeo(charCode, fontSize);
		//caret property
		if (!richText)
		{
			caretPosition.X = currentLineOffset.X - halfFontSpaceX;
			caretPosition.Y = currentLineOffset.Y;
			FUITextCaretProperty caretProperty;
			caretProperty.caretPosition = caretPosition;
			caretProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(caretProperty);

			caretPosition.X += fontSpace.X + charGeo.xadvance;//for line's last char's caret position
		}

		if (charCode == ' ')//char is space
		{
			if (overflowType == UITextOverflowType::VerticalOverflow)//char is space and UIText can have multi line, then we need to calculate if the followed word can fit the rest space, if not means new line
			{
				float spaceNeeded = richText ? richTextParseResult.size * 0.5f : halfFontSize;//space
				spaceNeeded += fontSpace.X;
				for (int forwardCharIndex = charIndex + 1, forwardVisibleCharIndex = visibleCharIndex + 1; forwardCharIndex < contentLength && forwardVisibleCharIndex < visibleCharCount; forwardCharIndex++)
				{
					auto charCodeOfForwardChar = content[forwardCharIndex];
					if (charCodeOfForwardChar == ' ')//space
					{
						break;
					}
					if (charCodeOfForwardChar == '\n' || charCodeOfForwardChar == '\r' || charCodeOfForwardChar == '\t')//\n\r\t
					{
						break;
					}
					spaceNeeded += richText
						? GetCharGeoXAdv(charCodeOfForwardChar, richTextParseResult.size)
						: GetCharGeoXAdv(charCodeOfForwardChar, fontSize);
					spaceNeeded += fontSpace.X;
					forwardVisibleCharIndex++;
				}

				if (currentLineOffset.X + spaceNeeded > width)
				{
					NewLine(charIndex);
					newLineMode = NewLineMode::Space;
					continue;
				}
			}
		}

		float charWidth = charGeo.xadvance + fontSpace.X;
		//char geometry
		if (charCode != ' ' && charCode != '\t')
		{
			if (richText)
			{
				currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.size);

				int additionalVerticesCount = 4;
				int additionalIndicesCount = 6;
				FUITextCharGeometry underlineCharGeo;
				FUITextCharGeometry strikethroughCharGeo;
				//underline and strikethrough should not exist at same char
				if (richTextParseResult.underline)
				{
					additionalVerticesCount += 4;
					additionalIndicesCount += 6;
					underlineCharGeo = GetUnderlineOrStrikethroughCharGeo('_', richTextParseResult.size);
				}
				if (richTextParseResult.strikethrough)
				{
					additionalVerticesCount += 4;
					additionalIndicesCount += 6;
					strikethroughCharGeo = GetUnderlineOrStrikethroughCharGeo('-', richTextParseResult.size);
				}
				if (richTextParseResult.bold)
				{
					additionalVerticesCount += 12;
					additionalIndicesCount += 18;
				}
				CheckVertices(additionalVerticesCount);
				CheckIndices(additionalIndicesCount);

				//position
				{
					auto lineOffset = currentLineOffset;
					if (richTextParseResult.supOrSubMode == LGUIRichTextParser::SupOrSubMode::Sup)
					{
						lineOffset.Y += richTextParseResult.size * 0.5f;
					}
					else if (richTextParseResult.supOrSubMode == LGUIRichTextParser::SupOrSubMode::Sub)
					{
						lineOffset.Y -= richTextParseResult.size * 0.5f;
					}
					float offsetX = lineOffset.X + charGeo.xoffset;
					float offsetY = lineOffset.Y + charGeo.yoffset;
					float x, y;

					int addVertCount = 0;
					if (richTextParseResult.bold)
					{
						x = offsetX;
						y = offsetY - charGeo.geoHeight;
						auto vert0 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto vert1 = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						auto vert2 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto vert3 = FVector(0, x, y);
						if (richTextParseResult.italic)
						{
							auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
							vert0.Y -= vert01ItalicOffset;
							vert1.Y -= vert01ItalicOffset;
							auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
							vert2.Y += vert23ItalicOffset;
							vert3.Y += vert23ItalicOffset;
						}
						//bold left
						originPositions[verticesCount] = vert0 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 1] = vert1 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 2] = vert2 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 3] = vert3 + FVector(0, -boldSize, 0);
						//bold right
						originPositions[verticesCount + 4] = vert0 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 5] = vert1 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 6] = vert2 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 7] = vert3 + FVector(0, boldSize, 0);
						//bold top
						originPositions[verticesCount + 8] = vert0 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 9] = vert1 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 10] = vert2 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 11] = vert3 + FVector(0, 0, boldSize);
						//bold bottom
						originPositions[verticesCount + 12] = vert0 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 13] = vert1 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 14] = vert2 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 15] = vert3 + FVector(0, 0, -boldSize);

						addVertCount = 16;
					}
					else
					{
						x = offsetX;
						y = offsetY - charGeo.geoHeight;
						auto& vert0 = originPositions[verticesCount];
						vert0 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto& vert1 = originPositions[verticesCount + 1];
						vert1 = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						auto& vert2 = originPositions[verticesCount + 2];
						vert2 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto& vert3 = originPositions[verticesCount + 3];
						vert3 = FVector(0, x, y);
						if (richTextParseResult.italic)
						{
							auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
							vert0.Y -= vert01ItalicOffset;
							vert1.Y -= vert01ItalicOffset;
							auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
							vert2.Y += vert23ItalicOffset;
							vert3.Y += vert23ItalicOffset;
						}

						addVertCount = 4;
					}

					if (richTextParseResult.underline)
					{
						offsetX = lineOffset.X;
						offsetY = lineOffset.Y + underlineCharGeo.yoffset;
						x = offsetX;
						y = offsetY - underlineCharGeo.geoHeight;
						originPositions[verticesCount + addVertCount] = FVector(0, x, y);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 1] = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						originPositions[verticesCount + addVertCount + 2] = FVector(0, x, y);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 3] = FVector(0, x, y);

						addVertCount += 4;
					}
					if (richTextParseResult.strikethrough)
					{
						offsetX = lineOffset.X;
						offsetY = lineOffset.Y + strikethroughCharGeo.yoffset;
						x = offsetX;
						y = offsetY - strikethroughCharGeo.geoHeight;
						originPositions[verticesCount + addVertCount] = FVector(0, x, y);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 1] = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						originPositions[verticesCount + addVertCount + 2] = FVector(0, x, y);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 3] = FVector(0, x, y);

						addVertCount += 4;
					}
				}
				//uv
				{
					int addVertCount = 0;
					if(richTextParseResult.bold)
					{
						//bold left
						vertices[verticesCount].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 1].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 2].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 3].TextureCoordinate[0] = charGeo.uv3;
						//bold right
						vertices[verticesCount + 4].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 5].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 6].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 7].TextureCoordinate[0] = charGeo.uv3;
						//bold top
						vertices[verticesCount + 8].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 9].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 10].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 11].TextureCoordinate[0] = charGeo.uv3;
						//bold bottom
						vertices[verticesCount + 12].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 13].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 14].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 15].TextureCoordinate[0] = charGeo.uv3;

						addVertCount = 16;
					}
					else
					{
						vertices[verticesCount].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 1].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 2].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 3].TextureCoordinate[0] = charGeo.uv3;

						addVertCount = 4;
					}
					if (richTextParseResult.underline)
					{
						vertices[verticesCount + addVertCount].TextureCoordinate[0] = underlineCharGeo.uv0;
						vertices[verticesCount + addVertCount + 1].TextureCoordinate[0] = underlineCharGeo.uv1;
						vertices[verticesCount + addVertCount + 2].TextureCoordinate[0] = underlineCharGeo.uv2;
						vertices[verticesCount + addVertCount + 3].TextureCoordinate[0] = underlineCharGeo.uv3;

						addVertCount += 4;
					}
					if (richTextParseResult.strikethrough)
					{
						vertices[verticesCount + addVertCount].TextureCoordinate[0] = strikethroughCharGeo.uv0;
						vertices[verticesCount + addVertCount + 1].TextureCoordinate[0] = strikethroughCharGeo.uv1;
						vertices[verticesCount + addVertCount + 2].TextureCoordinate[0] = strikethroughCharGeo.uv2;
						vertices[verticesCount + addVertCount + 3].TextureCoordinate[0] = strikethroughCharGeo.uv3;

						addVertCount += 4;
					}
				}
				//color
				{
					int addVertCount = 0;
					if (richTextParseResult.bold)
					{
						//bold left
						vertices[verticesCount].Color = richTextParseResult.color;
						vertices[verticesCount + 1].Color = richTextParseResult.color;
						vertices[verticesCount + 2].Color = richTextParseResult.color;
						vertices[verticesCount + 3].Color = richTextParseResult.color;
						//bold right
						vertices[verticesCount + 4].Color = richTextParseResult.color;
						vertices[verticesCount + 5].Color = richTextParseResult.color;
						vertices[verticesCount + 6].Color = richTextParseResult.color;
						vertices[verticesCount + 7].Color = richTextParseResult.color;
						//bold top
						vertices[verticesCount + 8].Color = richTextParseResult.color;
						vertices[verticesCount + 9].Color = richTextParseResult.color;
						vertices[verticesCount + 10].Color = richTextParseResult.color;
						vertices[verticesCount + 11].Color = richTextParseResult.color;
						//bold bottom
						vertices[verticesCount + 12].Color = richTextParseResult.color;
						vertices[verticesCount + 13].Color = richTextParseResult.color;
						vertices[verticesCount + 14].Color = richTextParseResult.color;
						vertices[verticesCount + 15].Color = richTextParseResult.color;

						addVertCount = 16;
					}
					else
					{
						vertices[verticesCount].Color = richTextParseResult.color;
						vertices[verticesCount + 1].Color = richTextParseResult.color;
						vertices[verticesCount + 2].Color = richTextParseResult.color;
						vertices[verticesCount + 3].Color = richTextParseResult.color;

						addVertCount = 4;
					}
					if (richTextParseResult.underline)
					{
						vertices[verticesCount + addVertCount].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 1].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 2].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 3].Color = richTextParseResult.color;

						addVertCount += 4;
					}
					if (richTextParseResult.strikethrough)
					{
						vertices[verticesCount + addVertCount].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 1].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 2].Color = richTextParseResult.color;
						vertices[verticesCount + addVertCount + 3].Color = richTextParseResult.color;

						addVertCount += 4;
					}
				}
				//triangle
				{
					int addVertCount = 0;
					int addIndCount = 0;
					if (richTextParseResult.bold)
					{
						//bold left
						triangles[indicesCount] = verticesCount;
						triangles[indicesCount + 1] = verticesCount + 3;
						triangles[indicesCount + 2] = verticesCount + 2;
						triangles[indicesCount + 3] = verticesCount;
						triangles[indicesCount + 4] = verticesCount + 1;
						triangles[indicesCount + 5] = verticesCount + 3;
						//bold right
						triangles[indicesCount + 6] = verticesCount + 4;
						triangles[indicesCount + 7] = verticesCount + 7;
						triangles[indicesCount + 8] = verticesCount + 6;
						triangles[indicesCount + 9] = verticesCount + 4;
						triangles[indicesCount + 10] = verticesCount + 5;
						triangles[indicesCount + 11] = verticesCount + 7;
						//bold top
						triangles[indicesCount + 12] = verticesCount + 8;
						triangles[indicesCount + 13] = verticesCount + 11;
						triangles[indicesCount + 14] = verticesCount + 10;
						triangles[indicesCount + 15] = verticesCount + 8;
						triangles[indicesCount + 16] = verticesCount + 9;
						triangles[indicesCount + 17] = verticesCount + 11;
						//bold bottom
						triangles[indicesCount + 18] = verticesCount + 12;
						triangles[indicesCount + 19] = verticesCount + 15;
						triangles[indicesCount + 20] = verticesCount + 14;
						triangles[indicesCount + 21] = verticesCount + 12;
						triangles[indicesCount + 22] = verticesCount + 13;
						triangles[indicesCount + 23] = verticesCount + 15;

						addVertCount = 16;
						addIndCount = 24;
					}
					else
					{
						triangles[indicesCount] = verticesCount;
						triangles[indicesCount + 1] = verticesCount + 3;
						triangles[indicesCount + 2] = verticesCount + 2;
						triangles[indicesCount + 3] = verticesCount;
						triangles[indicesCount + 4] = verticesCount + 1;
						triangles[indicesCount + 5] = verticesCount + 3;

						addVertCount = 4;
						addIndCount = 6;
					}
					if (richTextParseResult.underline)
					{
						triangles[indicesCount + addIndCount] = verticesCount + addVertCount;
						triangles[indicesCount + addIndCount + 1] = verticesCount + addVertCount + 3;
						triangles[indicesCount + addIndCount + 2] = verticesCount + addVertCount + 2;
						triangles[indicesCount + addIndCount + 3] = verticesCount + addVertCount;
						triangles[indicesCount + addIndCount + 4] = verticesCount + addVertCount + 1;
						triangles[indicesCount + addIndCount + 5] = verticesCount + addVertCount + 3;

						addVertCount += 4;
						addIndCount += 6;
					}
					if (richTextParseResult.strikethrough)
					{
						triangles[indicesCount + addIndCount] = verticesCount + addVertCount;
						triangles[indicesCount + addIndCount + 1] = verticesCount + addVertCount + 3;
						triangles[indicesCount + addIndCount + 2] = verticesCount + addVertCount + 2;
						triangles[indicesCount + addIndCount + 3] = verticesCount + addVertCount;
						triangles[indicesCount + addIndCount + 4] = verticesCount + addVertCount + 1;
						triangles[indicesCount + addIndCount + 5] = verticesCount + addVertCount + 3;

						addVertCount += 4;
						addIndCount += 6;
					}
				}

				//collect char property
				{
					FUITextCharProperty charProperty;
					charProperty.CharIndex = charIndex;
					charProperty.StartVertIndex = verticesCount;
					charProperty.VertCount = additionalVerticesCount;
					charProperty.StartTriangleIndex = indicesCount;
					charProperty.IndicesCount = indicesCount + additionalIndicesCount;
					cacheCharPropertyArray.Add(charProperty);
				}

				verticesCount += additionalVerticesCount;
				indicesCount += additionalIndicesCount;
			}
			else
			{
				int additionalVerticesCount = 4;
				int additionalIndicesCount = 6;
				if (bold)
				{
					additionalVerticesCount = 16;
					additionalIndicesCount = 24;
				}
				CheckVertices(additionalVerticesCount);
				CheckIndices(additionalIndicesCount);

				//position
				{
					float offsetX = currentLineOffset.X + charGeo.xoffset;
					float offsetY = currentLineOffset.Y + charGeo.yoffset;
					if (bold)
					{
						float x = offsetX;
						float y = offsetY - charGeo.geoHeight;
						auto vert0 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto vert1 = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						auto vert2 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto vert3 = FVector(0, x, y);
						if (italic)
						{
							auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
							vert0.Y -= vert01ItalicOffset;
							vert1.Y -= vert01ItalicOffset;
							auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
							vert2.Y += vert23ItalicOffset;
							vert3.Y += vert23ItalicOffset;
						}
						//bold left
						originPositions[verticesCount] = vert0 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 1] = vert1 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 2] = vert2 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 3] = vert3 + FVector(0, -boldSize, 0);
						//bold right
						originPositions[verticesCount + 4] = vert0 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 5] = vert1 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 6] = vert2 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 7] = vert3 + FVector(0, boldSize, 0);
						//bold top
						originPositions[verticesCount + 8] = vert0 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 9] = vert1 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 10] = vert2 + FVector(0, 0, boldSize);
						originPositions[verticesCount + 11] = vert3 + FVector(0, 0, boldSize);
						//bold bottom
						originPositions[verticesCount + 12] = vert0 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 13] = vert1 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 14] = vert2 + FVector(0, 0, -boldSize);
						originPositions[verticesCount + 15] = vert3 + FVector(0, 0, -boldSize);
					}
					else
					{
						float x = offsetX;
						float y = offsetY - charGeo.geoHeight;
						auto& vert0 = originPositions[verticesCount];
						vert0 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto& vert1 = originPositions[verticesCount + 1];
						vert1 = FVector(0, x, y);
						x = offsetX;
						y = offsetY;
						auto& vert2 = originPositions[verticesCount + 2];
						vert2 = FVector(0, x, y);
						x = charGeo.geoWidth + offsetX;
						auto& vert3 = originPositions[verticesCount + 3];
						vert3 = FVector(0, x, y);
						if (italic)
						{
							auto vert01ItalicOffset = (charGeo.geoHeight - charGeo.yoffset) * italicSlop;
							vert0.Y -= vert01ItalicOffset;
							vert1.Y -= vert01ItalicOffset;
							auto vert23ItalicOffset = charGeo.yoffset * italicSlop;
							vert2.Y += vert23ItalicOffset;
							vert3.Y += vert23ItalicOffset;
						}
					}
				}
				//uv
				{
					if (bold)
					{
						//bold left
						vertices[verticesCount].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 1].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 2].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 3].TextureCoordinate[0] = charGeo.uv3;
						//bold right
						vertices[verticesCount + 4].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 5].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 6].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 7].TextureCoordinate[0] = charGeo.uv3;
						//bold top
						vertices[verticesCount + 8].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 9].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 10].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 11].TextureCoordinate[0] = charGeo.uv3;
						//bold bottom
						vertices[verticesCount + 12].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 13].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 14].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 15].TextureCoordinate[0] = charGeo.uv3;
					}
					else
					{
						vertices[verticesCount].TextureCoordinate[0] = charGeo.uv0;
						vertices[verticesCount + 1].TextureCoordinate[0] = charGeo.uv1;
						vertices[verticesCount + 2].TextureCoordinate[0] = charGeo.uv2;
						vertices[verticesCount + 3].TextureCoordinate[0] = charGeo.uv3;
					}
				}
				//color
				{
					if (bold)
					{
						//bold left
						vertices[verticesCount].Color = color;
						vertices[verticesCount + 1].Color = color;
						vertices[verticesCount + 2].Color = color;
						vertices[verticesCount + 3].Color = color;
						//bold right
						vertices[verticesCount + 4].Color = color;
						vertices[verticesCount + 5].Color = color;
						vertices[verticesCount + 6].Color = color;
						vertices[verticesCount + 7].Color = color;
						//bold top
						vertices[verticesCount + 8].Color = color;
						vertices[verticesCount + 9].Color = color;
						vertices[verticesCount + 10].Color = color;
						vertices[verticesCount + 11].Color = color;
						//bold bottom
						vertices[verticesCount + 12].Color = color;
						vertices[verticesCount + 13].Color = color;
						vertices[verticesCount + 14].Color = color;
						vertices[verticesCount + 15].Color = color;
					}
					else
					{
						vertices[verticesCount].Color = color;
						vertices[verticesCount + 1].Color = color;
						vertices[verticesCount + 2].Color = color;
						vertices[verticesCount + 3].Color = color;
					}
				}
				//triangle
				{
					if (bold)
					{
						//bold left
						triangles[indicesCount] = verticesCount;
						triangles[indicesCount + 1] = verticesCount + 3;
						triangles[indicesCount + 2] = verticesCount + 2;
						triangles[indicesCount + 3] = verticesCount;
						triangles[indicesCount + 4] = verticesCount + 1;
						triangles[indicesCount + 5] = verticesCount + 3;
						//bold right
						triangles[indicesCount + 6] = verticesCount + 4;
						triangles[indicesCount + 7] = verticesCount + 7;
						triangles[indicesCount + 8] = verticesCount + 6;
						triangles[indicesCount + 9] = verticesCount + 4;
						triangles[indicesCount + 10] = verticesCount + 5;
						triangles[indicesCount + 11] = verticesCount + 7;
						//bold top
						triangles[indicesCount + 12] = verticesCount + 8;
						triangles[indicesCount + 13] = verticesCount + 11;
						triangles[indicesCount + 14] = verticesCount + 10;
						triangles[indicesCount + 15] = verticesCount + 8;
						triangles[indicesCount + 16] = verticesCount + 9;
						triangles[indicesCount + 17] = verticesCount + 11;
						//bold bottom
						triangles[indicesCount + 18] = verticesCount + 12;
						triangles[indicesCount + 19] = verticesCount + 15;
						triangles[indicesCount + 20] = verticesCount + 14;
						triangles[indicesCount + 21] = verticesCount + 12;
						triangles[indicesCount + 22] = verticesCount + 13;
						triangles[indicesCount + 23] = verticesCount + 15;
					}
					else
					{
						triangles[indicesCount] = verticesCount;
						triangles[indicesCount + 1] = verticesCount + 3;
						triangles[indicesCount + 2] = verticesCount + 2;
						triangles[indicesCount + 3] = verticesCount;
						triangles[indicesCount + 4] = verticesCount + 1;
						triangles[indicesCount + 5] = verticesCount + 3;
					}
				}

				//collect char property
				{
					FUITextCharProperty charProperty;
					charProperty.StartVertIndex = verticesCount;
					charProperty.VertCount = additionalVerticesCount;
					charProperty.StartTriangleIndex = indicesCount;
					charProperty.IndicesCount = indicesCount + additionalIndicesCount;
					cacheCharPropertyArray.Add(charProperty);
				}

				verticesCount += additionalVerticesCount;
				indicesCount += additionalIndicesCount;
			}
			visibleCharIndex++;
		}

		//collect rich text custom tag. custom tag use start/end mark, so put these code outside of visible-char-check.
		if (richText)
		{
			switch (richTextParseResult.customTagMode)
			{
			case LGUIRichTextParser::CustomTagMode::Start:
			{
				FUIText_RichTextCustomTag customTag;
				customTag.TagName = richTextParseResult.customTag;
				customTag.CharIndexStart = visibleCharIndex - 1;//-1 because visibleCharIndex++
				customTag.CharIndexStart = FMath::Max(0, customTag.CharIndexStart);//incase first char is invisible char, that makes index == -1
				customTag.CharIndexEnd = -1;
				cacheRichTextCustomTagArray.Add(customTag);
			}
			break;
			case LGUIRichTextParser::CustomTagMode::End:
			{
				int foundIndex = cacheRichTextCustomTagArray.IndexOfByPredicate([richTextParseResult](const FUIText_RichTextCustomTag& A) {
					return A.TagName == richTextParseResult.customTag;
					});
				if (foundIndex != -1)
				{
					cacheRichTextCustomTagArray[foundIndex].CharIndexEnd = visibleCharIndex - 1;//-1 because visibleCharIndex++
				}
			}
			break;
			}
		}

		currentLineOffset.X += charWidth;
		currentLineWidth += charWidth;

		//overflow
		{
			switch (overflowType)
			{
			case UITextOverflowType::HorizontalOverflow:
			{
				//no need to do anything
			}
			break;
			case UITextOverflowType::VerticalOverflow:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				int nextCharXAdv = nextCharXAdv = richText
					? GetCharGeoXAdv(content[charIndex + 1], richTextParseResult.size)
					: GetCharGeoXAdv(content[charIndex + 1], fontSize);
				if (currentLineOffset.X + nextCharXAdv > width)//if next char cannot fit this line, then add new line
				{
					auto nextChar = content[charIndex + 1];
					if (nextChar == '\r' || nextChar == '\n')
					{
						//next char is new line, no need to add new line
					}
					else
					{
						NewLine(charIndex + 1);
						newLineMode = NewLineMode::Overflow;
						continue;
					}
				}
			}
			break;
			case UITextOverflowType::ClampContent:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				if (haveClampContent)continue;

				int nextCharXAdv = richText
					? GetCharGeoXAdv(content[charIndex + 1], richTextParseResult.size)
					: GetCharGeoXAdv(content[charIndex + 1], fontSize);
				if (currentLineOffset.X + nextCharXAdv > width)//horizontal cannot fit next char
				{
					haveClampContent = true;
					clamp_RestVerticesCount = verticesCount;
					clamp_CurrentLineWidth = currentLineWidth;
					clamp_ParagraphHeight = paragraphHeight;
				}
			}
			break;
			}
		}
	}

	//verify custom tag
	if (richText)
	{
		for (int i = 0; i < cacheRichTextCustomTagArray.Num(); i++)
		{
			auto& item = cacheRichTextCustomTagArray[i];
			if (item.CharIndexEnd == -1)
			{
				item.CharIndexEnd = visibleCharIndex - 1;
			}
		}
	}

	//clamp content
	if (haveClampContent)
	{
		//set rest char's position to invisible
		int allVerticesCount = originPositions.Num();
		for (int vertIndex = clamp_RestVerticesCount; vertIndex < allVerticesCount; vertIndex++)
		{
			originPositions[vertIndex] = FVector::ZeroVector;
		}

		currentLineWidth = clamp_CurrentLineWidth;
		paragraphHeight = clamp_ParagraphHeight;
	}

	//last line
	NewLine(contentLength);
	//remove last line's space Y
	paragraphHeight -= fontSpace.Y;

	uiGeo->originVerticesCount = verticesCount;
	uiGeo->originTriangleCount = indicesCount;
	
	switch (overflowType)
	{
	case UITextOverflowType::HorizontalOverflow:
	{
		textRealSize.X = maxLineWidth;
		textRealSize.Y = paragraphHeight;
		if (adjustWidth)
		{
			width = textRealSize.X;
		}
	}
	break;
	case UITextOverflowType::VerticalOverflow:
	{
		if (linesCount > 1)
			textRealSize.X = width;
		else
			textRealSize.X = maxLineWidth;
		textRealSize.Y = paragraphHeight;
		if (adjustHeight)
		{
			height = textRealSize.Y;
		}
	}
	break;
	case UITextOverflowType::ClampContent:
	{
		textRealSize.X = maxLineWidth;
		textRealSize.Y = paragraphHeight;
	}
	break;
	}


	float pivotOffsetX = width * (0.5f - pivot.X);
	float pivotOffsetY = height * (0.5f - pivot.Y);
	float xOffset = pivotOffsetX;
	switch (paragraphHAlign)
	{
	case UITextParagraphHorizontalAlign::Left:
		xOffset += -width * 0.5f;
		break;
	case UITextParagraphHorizontalAlign::Center:

		break;
	case UITextParagraphHorizontalAlign::Right:
		xOffset += width * 0.5f;
		break;
	}
	float yOffset = pivotOffsetY - firstLineHeight * 0.5f;
	switch (paragraphVAlign)
	{
	case UITextParagraphVerticalAlign::Top:
		yOffset += height * 0.5f;
		break;
	case UITextParagraphVerticalAlign::Middle:
		yOffset += paragraphHeight * 0.5f;
		break;
	case UITextParagraphVerticalAlign::Bottom:
		yOffset += paragraphHeight - height * 0.5f;
		break;
	}
	//caret property
	if (!richText)
	{
		for (auto& sentenceItem : cacheTextPropertyArray)
		{
			for (auto& charItem : sentenceItem.charPropertyList)
			{
				charItem.caretPosition.X += xOffset;
				charItem.caretPosition.Y += yOffset;
			}
		}
	}

	UIGeometry::OffsetVertices(originPositions, uiGeo->originVerticesCount, xOffset, yOffset);

	//snap pixel
	if (pixelPerfect)
	{
		AdjustPixelPerfectPos_For_UIText(originPositions, cacheCharPropertyArray, renderCanvas, uiComp);
	}
}

void UIGeometry::AlignUITextLineVertex(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart, TArray<FVector>& vertices, FUITextLineProperty& sentenceProperty)
{
	float xOffset = 0;
	switch (pivotHAlign)
	{
	case UITextParagraphHorizontalAlign::Center:
		xOffset = -lineWidth * 0.5f;
		break;
	case UITextParagraphHorizontalAlign::Right:
		xOffset = -lineWidth;
		break;
	}

	for (int i = lineUIGeoVertStart; i < vertices.Num(); i++)
	{
		auto& vertex = vertices[i];
		vertex.Y += xOffset;
	}
	
	auto& charList = sentenceProperty.charPropertyList;
	for (auto& item : charList)
	{
		item.caretPosition.X += xOffset;
	}
}
void UIGeometry::AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineHeight, float fontSize, int lineUIGeoVertStart, TArray<FVector>& vertices)
{
	float xOffset = 0;
	switch (pivotHAlign)
	{
	case UITextParagraphHorizontalAlign::Center:
		xOffset = -lineWidth * 0.5f;
	break;
	case UITextParagraphHorizontalAlign::Right:
		xOffset = -lineWidth;
	break;
	}
	float yOffset = -(lineHeight - fontSize) * 0.5f;

	for (int i = lineUIGeoVertStart; i < vertices.Num(); i++)
	{
		auto& vertex = vertices[i];
		vertex.Y += xOffset;
		vertex.Z += yOffset;
	}
}

#pragma endregion

#pragma region UIPolygon
void UIGeometry::FromUIPolygon(const float& width, const float& height, const FVector2D& pivot
	, float startAngle, float endAngle, int sides, UIPolygonUVType uvType
	, TArray<float>& vertexOffsetArray, bool fullCycle
	, const FColor& color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo
	, bool requireNormal, bool requireTangent, bool requireUV1)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = sides * 3;
		triangles.AddUninitialized(uiGeo->originTriangleCount);
		int index = 0;
		if (fullCycle)
		{
			for (int i = 0; i < sides - 1; i++)
			{
				triangles[index++] = 0;
				triangles[index++] = i + 1;
				triangles[index++] = i + 2;
			}
			triangles[index++] = 0;
			triangles[index++] = sides;
			triangles[index++] = 1;
		}
		else
		{
			for (int i = 0; i < sides; i++)
			{
				triangles[index++] = 0;
				triangles[index++] = i + 1;
				triangles[index++] = i + 2;
			}
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		int vertexCount = (fullCycle ? 1 : 2) + sides;
		uiGeo->originVerticesCount = vertexCount;
		vertices.SetNumUninitialized(vertexCount);
	}
	//vert offset
	int vertexOffsetCount = fullCycle ? sides : (sides + 1);
	if (vertexOffsetArray.Num() != vertexOffsetCount)
	{
		if (vertexOffsetArray.Num() > vertexOffsetCount)
		{
			vertexOffsetArray.SetNumUninitialized(vertexOffsetCount);
		}
		else
		{
			for (int i = vertexOffsetArray.Num(); i < vertexOffsetCount; i++)
			{
				vertexOffsetArray.Add(1.0f);
			}
		}
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIPolygonVertex(width, height, pivot, startAngle, endAngle, sides, vertexOffsetArray, fullCycle, uiGeo);
	//uvs
	UpdateUIPolygonUV(startAngle, endAngle, sides, uvType, fullCycle, uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	int vertexCount = uiGeo->originVerticesCount;
	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				normals.Add(FVector(-1, 0, 0));
			}
		}
	}
	//tangents
	if (requireTangent)
	{
		auto& tangents = uiGeo->originNormals;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				tangents.Add(FVector(0, 1, 0));
			}
		}
	}
	//uv1
	if (requireUV1)
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
		}
	}
}
void UIGeometry::UpdateUIPolygonUV(float startAngle, float endAngle, int sides, UIPolygonUVType uvType
	, bool fullCycle
	, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	int vertexCount = uiGeo->originVerticesCount;
	switch (uvType)
	{
	case UIPolygonUVType::SpriteRect:
	{
		if (fullCycle)endAngle = startAngle + 360.0f;
		float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / sides);
		float angle = FMath::DegreesToRadians(startAngle);

		float sin = FMath::Sin(angle);
		float cos = FMath::Cos(angle);

		float halfUVWidth = (spriteInfo.uv3X - spriteInfo.uv0X) * 0.5f;
		float halfUVHeight = (spriteInfo.uv3Y - spriteInfo.uv0Y) * 0.5f;
		float centerUVX = (spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f;
		float centerUVY = (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f;

		float x = centerUVX;
		float y = centerUVY;
		vertices[0].TextureCoordinate[0] = FVector2D(x, y);

		int count = fullCycle ? sides : (sides + 1);
		for (int i = 0; i < count; i++)
		{
			sin = FMath::Sin(angle);
			cos = FMath::Cos(angle);
			x = cos * halfUVWidth + centerUVX;
			y = sin * halfUVHeight + centerUVY;
			vertices[i + 1].TextureCoordinate[0] = FVector2D(x, y);
			angle += singleAngle;
		}
	}
	break;
	case UIPolygonUVType::HeightCenter:
	{
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		FVector2D otherUV(spriteInfo.uv3X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
		}
	}
	break;
	case UIPolygonUVType::StretchSpriteHeight:
	{
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		float uvX = spriteInfo.uv3X;
		float uvY = spriteInfo.uv0Y;
		float uvYInterval = (spriteInfo.uv3Y - spriteInfo.uv0Y) / (vertexCount - 2);
		for (int i = 1; i < vertexCount; i++)
		{
			auto& uv = vertices[i].TextureCoordinate[0];
			uv.X = uvX;
			uv.Y = uvY;
			uvY += uvYInterval;
		}
	}
	break;
	}
}
void UIGeometry::UpdateUIPolygonVertex(const float& width, const float& height, const FVector2D& pivot
	, float startAngle, float endAngle, int sides
	, const TArray<float>& vertexOffsetArray, bool fullCycle
	, TSharedPtr<UIGeometry> uiGeo)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY);
	float halfW = width * 0.5f;
	float halfH = height * 0.5f;
	//vertices
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		int vertexCount = (fullCycle ? 1 : 2) + sides;
		uiGeo->originVerticesCount = vertexCount;
		originPositions.AddUninitialized(vertexCount);
	}

	if (fullCycle)endAngle = startAngle + 360.0f;
	float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / sides);
	float angle = FMath::DegreesToRadians(startAngle);

	float sin = FMath::Sin(angle);
	float cos = FMath::Cos(angle);

	float x = pivotOffsetX;
	float y = pivotOffsetY;
	originPositions[0] = FVector(0, x, y);

	for (int i = 0, count = sides; i < count; i++)
	{
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * halfW * vertexOffsetArray[i] + pivotOffsetX;
		y = sin * halfH * vertexOffsetArray[i] + pivotOffsetY;
		originPositions[i + 1] = FVector(0, x, y);
		angle += singleAngle;
	}
	if (!fullCycle)
	{
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * halfW * vertexOffsetArray[sides] + pivotOffsetX;
		y = sin * halfH * vertexOffsetArray[sides] + pivotOffsetY;
		originPositions[sides + 1] = FVector(0, x, y);
	}
}
#pragma endregion

void UIGeometry::OffsetVertices(TArray<FVector>& vertices, int count, float offsetX, float offsetY)
{
	for (int i = 0; i < count; i++)
	{
		auto& vertex = vertices[i];
		vertex.Y += offsetX;
		vertex.Z += offsetY;
	}
}
void UIGeometry::UpdateUIColor(TSharedPtr<UIGeometry> uiGeo, const FColor& color)
{
	auto& vertices = uiGeo->vertices;
	for (int i = 0; i < uiGeo->originVerticesCount; i++)
	{
		vertices[i].Color = color;
	}
}

void UIGeometry::CalculatePivotOffset(
	const float& width, const float& height, const FVector2D& pivot
	, float& pivotOffsetX, float& pivotOffsetY
)
{
	pivotOffsetX = width * (0.5f - pivot.X);//width * 0.5f *(1 - pivot.X * 2)
	pivotOffsetY = height * (0.5f - pivot.Y);//height * 0.5f *(1 - pivot.Y * 2)
}

void UIGeometry::CalculateOffsetAndSize(
	const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo
	, float& pivotOffsetX, float& pivotOffsetY, float& halfWidth, float& halfHeight
)
{
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY);

	if (spriteInfo.HasPadding())
	{
		float widthScale = width / spriteInfo.GetSourceWidth();
		float heightScale = height / spriteInfo.GetSourceHeight();
		float geoWidth = spriteInfo.width * widthScale;
		float geoHeight = spriteInfo.height * heightScale;
		pivotOffsetX += (-width + geoWidth) * 0.5f + spriteInfo.paddingLeft * widthScale;
		pivotOffsetY += (-height + geoHeight) * 0.5f + spriteInfo.paddingBottom * heightScale;
		halfWidth = geoWidth * 0.5f;
		halfHeight = geoHeight * 0.5f;
	}
	else
	{
		halfWidth = width * 0.5f;
		halfHeight = height * 0.5f;
	}
}


DECLARE_CYCLE_STAT(TEXT("UIGeometry TransformVertices"), STAT_TransformVertices, STATGROUP_LGUI);
void UIGeometry::TransformVertices(ULGUICanvas* canvas, UUIBaseRenderable* item, TSharedPtr<UIGeometry> uiGeo)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformVertices);

	canvas = canvas->GetActualRenderCanvas();//Canvas could be render by other canvas

	auto& vertices = uiGeo->vertices;
	auto& originPositions = uiGeo->originPositions;
	auto vertexCount = vertices.Num();
	auto originVertexCount = originPositions.Num();
	if (originVertexCount > vertexCount)
	{
		originPositions.RemoveAt(vertexCount, originVertexCount - vertexCount);
	}
	else if (originVertexCount < vertexCount)
	{
		originPositions.AddDefaulted(vertexCount - originVertexCount);
	}
	
	FLGUICacheTransformContainer tempTf;
	canvas->GetCacheUIItemToCanvasTransform(item, true, tempTf);
	auto itemToCanvasTf = tempTf.Transform;


	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = itemToCanvasTf.TransformPosition(originPositions[i]);
	}

	if (canvas->GetRequireNormal())
	{
		auto& originNormals = uiGeo->originNormals;
		auto originNormalCount = originNormals.Num();
		if (originNormalCount < vertexCount)
		{
			originNormals.AddDefaulted(vertexCount - originNormalCount);
		}
		else if (originNormalCount > vertexCount)
		{
			originNormals.RemoveAt(vertexCount, originNormalCount - vertexCount);
		}

		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentZ = itemToCanvasTf.TransformVector(originNormals[i]);
			vertices[i].TangentZ.Vector.W = -127;
		}
	}
	if (canvas->GetRequireTangent())
	{
		auto& originTangents = uiGeo->originTangents;
		auto originTangentCount = originTangents.Num();;
		if (originTangentCount < vertexCount)
		{
			originTangents.AddDefaulted(vertexCount - originTangentCount);
		}
		else if (originTangentCount > vertexCount)
		{
			originTangents.RemoveAt(vertexCount, originTangentCount - vertexCount);
		}

		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentX = itemToCanvasTf.TransformVector(originTangents[i]);
		}
	}
}

