// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIFontData.h"
#include "RichTextParser.h"


FORCEINLINE float RoundToFloat(float value)
{
	return value > 0.0 ? FMath::FloorToFloat(value + 0.5f) : FMath::CeilToFloat(value - 0.5f);
}

//adjust pixel snap in world position, because world canvas is placed in world origin, so that 1 unit is 1 pixel
//only position offset is take considerred
void AdjustPixelPerfectPos(TArray<FVector>& verticesArray, int startIndex, int count, const FVector& worldLocation, float rootCanvasScale)
{
	auto refVertPos = verticesArray[startIndex];
	auto worldVertPos = worldLocation + refVertPos;
	worldVertPos.X = RoundToFloat(worldVertPos.X);
	worldVertPos.Y = RoundToFloat(worldVertPos.Y);
	auto localVertPos = worldVertPos - worldLocation;
	auto vertOffset = localVertPos - refVertPos;
	verticesArray[startIndex] = localVertPos;
	for (int i = 1; i < count; i++)
	{
		verticesArray[startIndex + i] += vertOffset;
	}
}

#pragma region UISprite_UITexture_Simple
void UIGeometry::FromUIRectSimple(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp)
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
	UpdateUIRectSimpleVertex(uiGeo, width, height, pivot, renderCanvas, uiComp);
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
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
		}
	}
	//tangents
	if (renderCanvas->GetRequireTangent())
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(4);
			tangents.Add(FVector(1, 0, 0));
			tangents.Add(FVector(1, 0, 0));
			tangents.Add(FVector(1, 0, 0));
			tangents.Add(FVector(1, 0, 0));
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
void UIGeometry::UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& originPositions = uiGeo->originPositions;
	float x, y;
	x = (-halfW + pivotOffsetX);
	y = (-halfH + pivotOffsetY);
	originPositions[0] = FVector(x, y, 0);
	x = (halfW + pivotOffsetX);
	originPositions[1] = FVector(x, y, 0);
	x = (-halfW + pivotOffsetX);
	y = (halfH + pivotOffsetY);
	originPositions[2] = FVector(x, y, 0);
	x = (halfW + pivotOffsetX);
	originPositions[3] = FVector(x, y, 0);
	//snap pixel
	if (renderCanvas->GetPixelPerfect())
	{
		float rootCanvasScale = renderCanvas->GetRootCanvas()->CheckAndGetUIItem()->RelativeScale3D.X;
		AdjustPixelPerfectPos(originPositions, 0, originPositions.Num(), uiComp->GetComponentLocation(), rootCanvasScale);
	}
}
#pragma endregion
#pragma region UISprite_UITexture_Border
void UIGeometry::FromUIRectBorder(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp
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
				normals.Add(FVector(0, 0, -1));
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
				tangents.Add(FVector(1, 0, 0));
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
void UIGeometry::UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	float x0, x1, x2, x3, y0, y1, y2, y3;
	int widthBorder = spriteInfo.borderLeft + spriteInfo.borderRight;
	int heightBorder = spriteInfo.borderTop + spriteInfo.borderBottom;
	float widthScale = width < widthBorder ? width / widthBorder : 1.0f;
	float heightScale = height < heightBorder ? height / heightBorder : 1.0f;
	x0 = (-halfW + pivotOffsetX);
	x1 = (x0 + spriteInfo.borderLeft * widthScale);
	x3 = (halfW + pivotOffsetX);
	x2 = (x3 - spriteInfo.borderRight * widthScale);
	y0 = (-halfH + pivotOffsetY);
	y1 = (y0 + spriteInfo.borderBottom * heightScale);
	y3 = (halfH + pivotOffsetY);
	y2 = (y3 - spriteInfo.borderTop * heightScale);

	auto& originPositions = uiGeo->originPositions;
	originPositions[0] = FVector(x0, y0, 0);
	originPositions[1] = FVector(x1, y0, 0);
	originPositions[2] = FVector(x2, y0, 0);
	originPositions[3] = FVector(x3, y0, 0);

	originPositions[4] = FVector(x0, y1, 0);
	originPositions[5] = FVector(x1, y1, 0);
	originPositions[6] = FVector(x2, y1, 0);
	originPositions[7] = FVector(x3, y1, 0);

	originPositions[8] = FVector(x0, y2, 0);
	originPositions[9] = FVector(x1, y2, 0);
	originPositions[10] = FVector(x2, y2, 0);
	originPositions[11] = FVector(x3, y2, 0);

	originPositions[12] = FVector(x0, y3, 0);
	originPositions[13] = FVector(x1, y3, 0);
	originPositions[14] = FVector(x2, y3, 0);
	originPositions[15] = FVector(x3, y3, 0);

	//snap pixel
	if (renderCanvas->GetPixelPerfect())
	{
		float rootCanvasScale = renderCanvas->GetRootCanvas()->CheckAndGetUIItem()->RelativeScale3D.X;
		AdjustPixelPerfectPos(originPositions, 0, originPositions.Num(), uiComp->GetComponentLocation(), rootCanvasScale);
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
				normals.Add(FVector(0, 0, -1));
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
				tangents.Add(FVector(1, 0, 0));
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
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
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
			originPositions[vertIndex++] = FVector(x, y, 0);
			originPositions[vertIndex++] = FVector(x + realWidth, y, 0);
			originPositions[vertIndex++] = FVector(x, y + realHeight, 0);
			originPositions[vertIndex++] = FVector(x + realWidth, y + realHeight, 0);
			
			x += spriteInfo.width;
		}
		x = startX;
		y += spriteInfo.height;
	}
	//snap pixel
	if (renderCanvas->GetPixelPerfect())
	{
		float rootCanvasScale = renderCanvas->GetRootCanvas()->CheckAndGetUIItem()->RelativeScale3D.X;
		AdjustPixelPerfectPos(originPositions, 0, originPositions.Num(), uiComp->GetComponentLocation(), rootCanvasScale);
	}
}
#pragma endregion

#pragma region UIText
void UIGeometry::FromUIText(FString& content, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
	, FColor color, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, float fontSize
	, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
	, bool adjustWidth, bool adjustHeight, UITextFontStyle fontStyle, FVector2D& textRealSize
	, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, TArray<FUITextLineProperty>& cacheTextPropertyList, ULGUIFontData* font, bool richText)
{
	//vertex and triangle
	UpdateUIText(content, visibleCharCount, width, height, pivot, color, fontSpace, uiGeo, fontSize, paragraphHAlign, paragraphVAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, renderCanvas, uiComp, cacheTextPropertyList, font, richText);

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
				normals.Add(FVector(0, 0, -1));
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
				tangents.Add(FVector(1, 0, 0));
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

void UIGeometry::UpdateUIText(FString& content, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
	, FColor color, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, float fontSize
	, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
	, bool adjustWidth, bool adjustHeight, UITextFontStyle fontStyle, FVector2D& textRealSize
	, ULGUICanvas* renderCanvas, UUIItem* uiComp
	, TArray<FUITextLineProperty>& cacheTextPropertyList, ULGUIFontData* font, bool richText)
{
	bool pixelPerfect = renderCanvas->GetPixelPerfect();
	float dynamicPixelsPerUnit = renderCanvas->GetDynamicPixelsPerUnit();
	bool dynamicPixelsPerUnitIsNot1 = dynamicPixelsPerUnit != 1;//use dynamicPixelsPerUnit or not
	float rootCanvasScale = renderCanvas->GetRootCanvas()->CheckAndGetUIItem()->RelativeScale3D.X;

	auto GetAdjustedFontSize = [&](float inFontSize)
	{
		float adjustedFontSize = inFontSize;
		if (pixelPerfect && rootCanvasScale != -1.0f)
		{
			adjustedFontSize = adjustedFontSize * rootCanvasScale;
			adjustedFontSize = FMath::Min(adjustedFontSize, 200.0f);//limit font size to 200. too large font size will result in large texture
		}
		else if (dynamicPixelsPerUnitIsNot1)
		{
			adjustedFontSize = adjustedFontSize * dynamicPixelsPerUnit;
			adjustedFontSize = FMath::Min(adjustedFontSize, 200.0f);//limit font size to 200. too large font size will result in large texture
		}
		return adjustedFontSize;
	};

	FVector worldLocation;
	if (pixelPerfect)
	{
		worldLocation = uiComp->GetComponentLocation();
	}
	static TArray<int> firstVertIndexOfChar_Array;//for pixel perfect adjust, first vertex_position_index of a char, in originPosition's array
	firstVertIndexOfChar_Array.Reset();

	bool bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
	float boldSize = fontSize * font->boldRatio;
	bool italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
	float italicSlop = FMath::Tan(FMath::DegreesToRadians(font->italicAngle));

	//rich text
	using namespace LGUIRichTextParser;
	static RichTextParser richTextParser;
	RichTextParseResult richTextParseResult;
	if (richText)
	{
		richTextParser.Clear();
		richTextParser.Prepare(fontSize, color, bold, italic, richTextParseResult);
	}

	auto GetCharFixedOffset = [font](float fontSize)
	{
		return fontSize * (font->fixedVerticalOffset - 0.25f);
	};
	float charFixedOffset = GetCharFixedOffset(fontSize);//some font may not render at vertical center, use this to mofidy it. 0.25 * size is tested value for most fonts

	cacheTextPropertyList.Reset();
	int contentLength = content.Len();
	FVector2D currentLineOffset(0, 0);
	float currentLineWidth = 0, currentLineHeight = fontSize, paragraphHeight = 0;//single line width, height, all line height
	float firstLineHeight = fontSize;//first line height
	float maxLineWidth = 0;//if have multiple line
	int lineUIGeoVertStart = 0;//vertex index in originPositions of current line
	int visibleCharIndex = 0;//visible char index, skip invisible char(\n,\t)
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
			AlignUITextLineVertexForRichText(paragraphHAlign, currentLineWidth, lineUIGeoVertStart, originPositions);
		}
		else
		{
			FUITextCaretProperty charProperty;
			charProperty.caretPosition = caretPosition;
			charProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(charProperty);

			AlignUITextLineVertex(paragraphHAlign, currentLineWidth, lineUIGeoVertStart, originPositions, sentenceProperty);

			cacheTextPropertyList.Add(sentenceProperty);
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
		//set line height to origin
		firstLineHeight = richText ? currentLineHeight : fontSize;
		currentLineHeight = fontSize;
	};

	auto GetCharGeo = [&](TCHAR charCode, int overrideFontSize)
	{
		FUITextCharGeometry charGeo;
		if (charCode == ' ')
		{
			charGeo.xadvance = overrideFontSize * 0.5f;
			charGeo.geoWidth = charGeo.geoHeight = 0;
			charGeo.xoffset = charGeo.yoffset = 0;
			charGeo.uv0 = charGeo.uv1 = charGeo.uv2 = charGeo.uv3 = FVector2D(1, 1);
		}
		else if (charCode == '\t')
		{
			charGeo.xadvance = overrideFontSize + overrideFontSize;
			charGeo.geoWidth = charGeo.geoHeight = 0;
			charGeo.xoffset = charGeo.yoffset = 0;
			charGeo.uv0 = charGeo.uv1 = charGeo.uv2 = charGeo.uv3 = FVector2D(1, 1);
		}
		else
		{
			auto charData = font->GetCharData(charCode, (uint16)overrideFontSize);
			charGeo.geoWidth = charData->width;
			charGeo.geoHeight = charData->height;
			charGeo.xadvance = charData->xadvance;
			charGeo.xoffset = charData->xoffset;
			charGeo.yoffset = charData->yoffset + (richText ? GetCharFixedOffset(overrideFontSize) : charFixedOffset);

			auto overrideCharData = charData;
			if ((pixelPerfect && rootCanvasScale != 1.0f) || dynamicPixelsPerUnitIsNot1)
			{
				overrideCharData = font->GetCharData(charCode, (uint16)GetAdjustedFontSize(overrideFontSize));
			}

			charGeo.uv0 = overrideCharData->GetUV0();
			charGeo.uv1 = overrideCharData->GetUV1();
			charGeo.uv2 = overrideCharData->GetUV2();
			charGeo.uv3 = overrideCharData->GetUV3();
		}
		return charGeo;
	};
	auto GetCharGeoXAdv = [&](TCHAR charCode, int overrideFontSize)
	{
		if (charCode == ' ')
		{
			return (uint16)(overrideFontSize * 0.5f);
		}
		else if (charCode == '\t')
		{
			return (uint16)(overrideFontSize + overrideFontSize);
		}
		else
		{
			auto charData = font->GetCharData(charCode, overrideFontSize);
			return charData->xadvance;
		}
	};
	auto GetUnderlineOrStrikethroughCharGeo = [&](TCHAR charCode, int overrideFontSize)
	{
		FUITextCharGeometry charGeo;
		{
			auto charData = font->GetCharData(charCode, (uint16)overrideFontSize);
			charGeo.geoHeight = charData->height;
			charGeo.xoffset = charData->xoffset;
			charGeo.yoffset = charData->yoffset + GetCharFixedOffset(overrideFontSize);

			float uvX = (charData->uv3X - charData->uv0X) * 0.5f + charData->uv0X;
			charGeo.uv0 = charGeo.uv1 = FVector2D(uvX, charData->uv0Y);
			charGeo.uv2 = charGeo.uv3 = FVector2D(uvX, charData->uv3Y);
		}
		return charGeo;
	};

	for (int charIndex = 0; charIndex < contentLength; charIndex++)
	{
		auto charCode = content[charIndex];
		//rich text
		if (richText)
		{
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
			if (charIndex >= contentLength)break;
		}

		if (charCode == '\n' || charCode == '\r')//10 -- \n, 13 -- \r
		{
			NewLine(charIndex);
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
			FUITextCaretProperty charProperty;
			charProperty.caretPosition = caretPosition;
			charProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(charProperty);

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
						auto vert0 = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						auto vert1 = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						if (richTextParseResult.italic)x += charGeo.geoHeight * italicSlop;
						auto vert2 = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						if (richTextParseResult.italic)x += charGeo.geoHeight * italicSlop;
						auto vert3 = FVector(x, y, 0);
						//bold left
						originPositions[verticesCount] = vert0 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 1] = vert1 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 2] = vert2 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 3] = vert3 + FVector(-boldSize, 0, 0);
						//bold right
						originPositions[verticesCount + 4] = vert0 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 5] = vert1 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 6] = vert2 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 7] = vert3 + FVector(boldSize, 0, 0);
						//bold top
						originPositions[verticesCount + 8] = vert0 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 9] = vert1 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 10] = vert2 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 11] = vert3 + FVector(0, boldSize, 0);
						//bold bottom
						originPositions[verticesCount + 12] = vert0 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 13] = vert1 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 14] = vert2 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 15] = vert3 + FVector(0, -boldSize, 0);

						addVertCount = 16;
					}
					else
					{
						float x = offsetX;
						float y = offsetY - charGeo.geoHeight;
						originPositions[verticesCount] = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						originPositions[verticesCount + 1] = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						if (richTextParseResult.italic)x += charGeo.geoHeight * italicSlop;
						originPositions[verticesCount + 2] = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						if (richTextParseResult.italic)x += charGeo.geoHeight * italicSlop;
						originPositions[verticesCount + 3] = FVector(x, y, 0);

						addVertCount = 4;
					}
					
					if (richTextParseResult.underline)
					{
						offsetX = lineOffset.X;
						offsetY = lineOffset.Y + underlineCharGeo.yoffset;
						x = offsetX;
						y = offsetY - underlineCharGeo.geoHeight;
						originPositions[verticesCount + addVertCount] = FVector(x, y, 0);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 1] = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						originPositions[verticesCount + addVertCount + 2] = FVector(x, y, 0);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 3] = FVector(x, y, 0);

						addVertCount += 4;
					}
					if (richTextParseResult.strikethrough)
					{
						offsetX = lineOffset.X;
						offsetY = lineOffset.Y + strikethroughCharGeo.yoffset;
						x = offsetX;
						y = offsetY - strikethroughCharGeo.geoHeight;
						originPositions[verticesCount + addVertCount] = FVector(x, y, 0);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 1] = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						originPositions[verticesCount + addVertCount + 2] = FVector(x, y, 0);
						x = charWidth + offsetX;
						originPositions[verticesCount + addVertCount + 3] = FVector(x, y, 0);

						addVertCount += 4;
					}
					//snap pixel
					if (pixelPerfect)
					{
						firstVertIndexOfChar_Array.Add(verticesCount);
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
						auto vert0 = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						auto vert1 = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						if (italic)x += charGeo.geoHeight * italicSlop;
						auto vert2 = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						if (italic)x += charGeo.geoHeight * italicSlop;
						auto vert3 = FVector(x, y, 0);
						//bold left
						originPositions[verticesCount] = vert0 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 1] = vert1 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 2] = vert2 + FVector(-boldSize, 0, 0);
						originPositions[verticesCount + 3] = vert3 + FVector(-boldSize, 0, 0);
						//bold right
						originPositions[verticesCount + 4] = vert0 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 5] = vert1 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 6] = vert2 + FVector(boldSize, 0, 0);
						originPositions[verticesCount + 7] = vert3 + FVector(boldSize, 0, 0);
						//bold top
						originPositions[verticesCount + 8] = vert0 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 9] = vert1 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 10] = vert2 + FVector(0, boldSize, 0);
						originPositions[verticesCount + 11] = vert3 + FVector(0, boldSize, 0);
						//bold bottom
						originPositions[verticesCount + 12] = vert0 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 13] = vert1 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 14] = vert2 + FVector(0, -boldSize, 0);
						originPositions[verticesCount + 15] = vert3 + FVector(0, -boldSize, 0);
					}
					else
					{
						float x = offsetX;
						float y = offsetY - charGeo.geoHeight;
						originPositions[verticesCount] = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						originPositions[verticesCount + 1] = FVector(x, y, 0);
						x = offsetX;
						y = offsetY;
						if (italic)x += charGeo.geoHeight * italicSlop;
						originPositions[verticesCount + 2] = FVector(x, y, 0);
						x = charGeo.geoWidth + offsetX;
						if (italic)x += charGeo.geoHeight * italicSlop;
						originPositions[verticesCount + 3] = FVector(x, y, 0);
					}
					//snap pixel
					if (pixelPerfect)
					{
						firstVertIndexOfChar_Array.Add(verticesCount);
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

				verticesCount += additionalVerticesCount;
				indicesCount += additionalIndicesCount;
			}
			visibleCharIndex++;
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
				int nextCharXAdv = richText
					? GetCharGeoXAdv(content[charIndex + 1], richTextParseResult.size)
					: GetCharGeoXAdv(content[charIndex + 1], fontSize);
				if (currentLineOffset.X + nextCharXAdv > width)//horizontal cannot fit next char
				{
					//@todo: ClampContent mode may cause triangle change if UIText's width change
					//set rest char's position to invisible
					int allVerticesCount = originPositions.Num();
					for (int vertIndex = verticesCount; vertIndex < allVerticesCount; vertIndex++)
					{
						originPositions[vertIndex] = FVector::ZeroVector;
					}
					//break the main loop
					charIndex = contentLength;
				}
			}
			break;
			}
		}
	}

	//last line
	NewLine(contentLength);

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
		for (auto& sentenceItem : cacheTextPropertyList)
		{
			for (auto& charItem : sentenceItem.charPropertyList)
			{
				charItem.caretPosition.X += xOffset;
				charItem.caretPosition.Y += yOffset;
			}
		}
	}

	UIGeometry::OffsetVertices(originPositions, xOffset, yOffset);

	//snap pixel
	if (pixelPerfect)
	{
		if (firstVertIndexOfChar_Array.Num() > 0)
		{
			int indexOf_FirstVertIndexOfChar = 0;
			for (int vertStartIndex = 0, count = originPositions.Num(), vertEndIndex = firstVertIndexOfChar_Array[indexOf_FirstVertIndexOfChar]
				; vertStartIndex < count;)
			{
				indexOf_FirstVertIndexOfChar++;
				if (indexOf_FirstVertIndexOfChar >= firstVertIndexOfChar_Array.Num())
				{
					vertEndIndex = count;
				}
				else
				{
					vertEndIndex = firstVertIndexOfChar_Array[indexOf_FirstVertIndexOfChar];
				}
				AdjustPixelPerfectPos(originPositions, vertStartIndex, vertEndIndex - vertStartIndex, worldLocation, rootCanvasScale);
				vertStartIndex = vertEndIndex;
			}
		}
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
		vertex.X += xOffset;
	}
	
	auto& charList = sentenceProperty.charPropertyList;
	for (auto& item : charList)
	{
		item.caretPosition.X += xOffset;
	}
}
void UIGeometry::AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart, TArray<FVector>& vertices)
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
		vertex.X += xOffset;
	}
}

#pragma endregion

#pragma region UISector
void UIGeometry::FromUISector(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = (segment + 1) * 3;
		triangles.AddUninitialized(uiGeo->originTriangleCount);
		int index = 0;
		for (int i = 0; i < segment + 1; i++)
		{
			triangles[index++] = 0;
			triangles[index++] = i + 1;
			triangles[index++] = i + 2;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		int vertexCount = 2 + segment + 1;
		uiGeo->originVerticesCount = vertexCount;
		vertices.SetNumUninitialized(vertexCount);
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUISectorVertex(uiGeo, width, height, pivot, startAngle, endAngle, segment);
	//uvs
	UpdateUISectorUV(uiGeo, uvType, startAngle, endAngle, segment, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	int vertexCount = 2 + segment + 1;
	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				normals.Add(FVector(0, 0, -1));
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
				tangents.Add(FVector(1, 0, 0));
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
void UIGeometry::UpdateUISectorUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, const FLGUISpriteInfo& spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	int vertexCount = uiGeo->originVerticesCount;
	if(uvType == 0)
	{
		float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
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

		for (int i = 0; i < segment + 2; i++)
		{
			sin = FMath::Sin(angle);
			cos = FMath::Cos(angle);
			x = cos * halfUVWidth + centerUVX;
			y = sin * halfUVHeight + centerUVY;
			vertices[i + 1].TextureCoordinate[0] = FVector2D(x, y);
			angle += singleAngle;
		}
	}
	else if(uvType == 1)
	{
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		FVector2D otherUV(spriteInfo.uv3X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
		}
	}
	else if (uvType == 2)
	{
		vertices[0].TextureCoordinate[0] = FVector2D((spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f, spriteInfo.uv0Y);
		FVector2D otherUV((spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f, spriteInfo.uv3Y);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
		}
	}
	else if (uvType == 3)
	{
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo.uv0X, (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f);
		float uvYInterval = (spriteInfo.uv3Y - spriteInfo.uv0Y) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo.uv3X, spriteInfo.uv0Y);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
			otherUV.Y += uvYInterval;
		}
	}
	else if (uvType == 4)
	{
		vertices[0].TextureCoordinate[0] = FVector2D((spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f, spriteInfo.uv0Y);
		float uvXInterval = (spriteInfo.uv3X - spriteInfo.uv0X) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo.uv0X, spriteInfo.uv3Y);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
			otherUV.X += uvXInterval;
		}
	}
}
void UIGeometry::UpdateUISectorVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		int vertexCount = 2 + segment + 1;
		uiGeo->originVerticesCount = vertexCount;
		originPositions.AddUninitialized(vertexCount);
	}

	float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
	float angle = FMath::DegreesToRadians(startAngle);

	float sin = FMath::Sin(angle);
	float cos = FMath::Cos(angle);

	float x = pivotOffsetX;
	float y = pivotOffsetY;
	originPositions[0] = FVector(x, y, 0);
	x = cos * halfW + pivotOffsetX;
	y = sin * halfH + pivotOffsetY;
	originPositions[1] = FVector(x, y, 0);

	for (int i = 0; i < segment + 1; i++)
	{
		angle += singleAngle;
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * halfW + pivotOffsetX;
		y = sin * halfH + pivotOffsetY;
		originPositions[i + 2] = FVector(x, y, 0);
	}
}
#pragma endregion

#pragma region UIRing
void UIGeometry::FromUIRing(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, float ringWidth, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
{
	//triangles
	auto& triangles = uiGeo->triangles;
	if (triangles.Num() == 0)
	{
		uiGeo->originTriangleCount = (segment + 1) * 2 * 3;
		triangles.AddUninitialized(uiGeo->originTriangleCount);
		for (int i = 0; i < segment + 1; i++)
		{
			int k = i * 6;
			int j = i * 2;
			triangles[k] = j;
			triangles[k + 1] = j + 3;
			triangles[k + 2] = j + 2;

			triangles[k + 3] = j;
			triangles[k + 4] = j + 1;
			triangles[k + 5] = j + 3;
		}
	}
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		int vertexCount = 2 + (segment + 1) * 2;
		uiGeo->originVerticesCount = vertexCount;
		vertices.AddUninitialized(vertexCount);
	}
	//positions
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUIRingVertex(uiGeo, width, height, pivot, startAngle, endAngle, segment, ringWidth);
	//uvs
	UpdateUIRingUV(uiGeo, uvType, startAngle, endAngle, segment, ringWidth, width, height, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	int vertexCount = 2 + (segment + 1) * 2;
	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->originNormals;
		if (normals.Num() == 0)
		{
			normals.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				normals.Add(FVector(0, 0, -1));
			}
		}
	}
	//tangents
	if (requireTangent)
	{
		auto& tangents = uiGeo->originTangents;
		if (tangents.Num() == 0)
		{
			tangents.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				tangents.Add(FVector(1, 0, 0));
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
void UIGeometry::UpdateUIRingUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, float ringWidth, float& width, float& height, const FLGUISpriteInfo& spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	int vertexCount = uiGeo->originVerticesCount;
	if (uvType == 0)
	{
		float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
		float angle = FMath::DegreesToRadians(startAngle);

		float sin = FMath::Sin(angle);
		float cos = FMath::Cos(angle);

		float halfUVWidth = (spriteInfo.uv3X - spriteInfo.uv0X) * 0.5f;
		float halfUVHeight = (spriteInfo.uv3Y - spriteInfo.uv0Y) * 0.5f;
		float centerUVX = (spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f;
		float centerUVY = (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f;

		float halfW = width * 0.5f;
		float halfH = height * 0.5f;
		float minHalfWRatio = (halfW - ringWidth) / halfW;
		float minHalfHRatio = (halfH - ringWidth) / halfH;
		float x = cos * halfUVWidth * minHalfWRatio + centerUVX;
		float y = sin * halfUVHeight * minHalfHRatio + centerUVY;
		vertices[0].TextureCoordinate[0] = FVector2D(x, y);
		x = cos * halfUVWidth + centerUVX;
		y = sin * halfUVHeight + centerUVY;
		vertices[1].TextureCoordinate[0] = FVector2D(x, y);

		for (int i = 0; i < segment + 1; i++)
		{
			angle += singleAngle;
			sin = FMath::Sin(angle);
			cos = FMath::Cos(angle);
			x = cos * halfUVWidth * minHalfWRatio + centerUVX;
			y = sin * halfUVHeight * minHalfHRatio + centerUVY;
			vertices[i * 2 + 2].TextureCoordinate[0] = FVector2D(x, y);
			x = cos * halfUVWidth + centerUVX;
			y = sin * halfUVHeight + centerUVY;
			vertices[i * 2 + 3].TextureCoordinate[0] = FVector2D(x, y);
		}
	}
	else if (uvType == 1)
	{
		float centerUVX = (spriteInfo.uv0X + spriteInfo.uv3X) * 0.5f;

		float uvYInterval = (spriteInfo.uv3Y - spriteInfo.uv0Y) / (segment + 1);
		float uvY = spriteInfo.uv0Y;
		vertices[0].TextureCoordinate[0] = FVector2D(centerUVX, uvY);
		vertices[1].TextureCoordinate[0] = FVector2D(centerUVX, uvY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvY += uvYInterval;
			vertices[i * 2 + 2].TextureCoordinate[0] = FVector2D(centerUVX, uvY);
			vertices[i * 2 + 3].TextureCoordinate[0] = FVector2D(centerUVX, uvY);
		}
	}
	else if (uvType == 2)
	{
		float centerUVY = (spriteInfo.uv0Y + spriteInfo.uv3Y) * 0.5f;

		float uvXInterval = (spriteInfo.uv3X - spriteInfo.uv0X) / (segment + 1);
		float uvX = spriteInfo.uv0X;
		vertices[0].TextureCoordinate[0] = FVector2D(uvX, centerUVY);
		vertices[1].TextureCoordinate[0] = FVector2D(uvX, centerUVY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvX += uvXInterval;
			vertices[i * 2 + 2].TextureCoordinate[0] = FVector2D(uvX, centerUVY);
			vertices[i * 2 + 3].TextureCoordinate[0] = FVector2D(uvX, centerUVY);
		}
	}
	else if (uvType == 3)
	{
		float innerUVX = spriteInfo.uv0X;
		float outerUVX = spriteInfo.uv3X;

		float uvYInterval = (spriteInfo.uv3Y - spriteInfo.uv0Y) / (segment + 1);
		float uvY = spriteInfo.uv0Y;

		vertices[0].TextureCoordinate[0] = FVector2D(innerUVX, uvY);
		vertices[1].TextureCoordinate[0] = FVector2D(outerUVX, uvY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvY += uvYInterval;
			vertices[i * 2 + 2].TextureCoordinate[0] = FVector2D(innerUVX, uvY);
			vertices[i * 2 + 3].TextureCoordinate[0] = FVector2D(outerUVX, uvY);
		}
	}
	else if (uvType == 4)
	{
		float innerUVY = spriteInfo.uv0Y;
		float outerUVY = spriteInfo.uv3Y;

		float uvXInterval = (spriteInfo.uv3X - spriteInfo.uv0X) / (segment + 1);
		float uvX = spriteInfo.uv0X;

		vertices[0].TextureCoordinate[0] = FVector2D(uvX, innerUVY);
		vertices[1].TextureCoordinate[0] = FVector2D(uvX, outerUVY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvX += uvXInterval;
			vertices[i * 2 + 2].TextureCoordinate[0] = FVector2D(uvX, innerUVY);
			vertices[i * 2 + 3].TextureCoordinate[0] = FVector2D(uvX, outerUVY);
		}
	}
}
void UIGeometry::UpdateUIRingVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, float ringWidth)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		int vertexCount = 2 + (segment + 1) * 2;
		uiGeo->originVerticesCount = vertexCount;
		originPositions.AddUninitialized(vertexCount);
	}

	float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
	float angle = FMath::DegreesToRadians(startAngle);
	float minHalfW = halfW - ringWidth;
	float minHalfH = halfH - ringWidth;

	float sin = FMath::Sin(angle);
	float cos = FMath::Cos(angle);

	float x = cos * minHalfW + pivotOffsetX;
	float y = sin * minHalfH + pivotOffsetY;
	originPositions[0] = FVector(x, y, 0);
	x = cos * halfW + pivotOffsetX;
	y = sin * halfH + pivotOffsetY;
	originPositions[1] = FVector(x, y, 0);

	for (int i = 0; i < (segment + 1); i++)
	{
		angle += singleAngle;
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * minHalfW + pivotOffsetX;
		y = sin * minHalfH + pivotOffsetY;
		originPositions[i * 2 + 2] = FVector(x, y, 0);
		x = cos * halfW + pivotOffsetX;
		y = sin * halfH + pivotOffsetY;
		originPositions[i * 2 + 3] = FVector(x, y, 0);
	}
}
#pragma endregion

void UIGeometry::OffsetVertices(TArray<FVector>& vertices, float offsetX, float offsetY)
{
	int count = vertices.Num();
	for (int i = 0; i < count; i++)
	{
		auto& vertex = vertices[i];
		vertex.X += offsetX;
		vertex.Y += offsetY;
	}
}
void UIGeometry::OffsetVertices(TArray<FVector*>& vertices, float offsetX, float offsetY)
{
	int count = vertices.Num();
	for (int i = 0; i < count; i++)
	{
		auto& vertex = vertices[i];
		vertex->X += offsetX;
		vertex->Y += offsetY;
	}
}
void UIGeometry::UpdateUIColor(TSharedPtr<UIGeometry> uiGeo, FColor color)
{
	auto& vertices = uiGeo->vertices;
	for (int i = 0; i < uiGeo->originVerticesCount; i++)
	{
		vertices[i].Color = color;
	}
}

void UIGeometry::CalculatePivotOffset(const float& width, const float& height, const FVector2D& pivot, float& pivotOffsetX, float& pivotOffsetY, float& halfW, float& halfH)
{
	halfW = width * 0.5f;
	halfH = height * 0.5f;
	pivotOffsetX = width * (0.5f - pivot.X);//width * 0.5f *(1 - pivot.X * 2)
	pivotOffsetY = height * (0.5f - pivot.Y);//height * 0.5f *(1 - pivot.Y * 2)
}


DECLARE_CYCLE_STAT(TEXT("UIGeometry TransformVertices"), STAT_TransformVertices, STATGROUP_LGUI);
void UIGeometry::TransformVertices(ULGUICanvas* canvas, UUIRenderable* item, TSharedPtr<UIGeometry> uiGeo, bool requireNormal, bool requireTangent)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformVertices);
	auto canvasUIItem = canvas->CheckAndGetUIItem();
	auto inverseCanvasTf = canvasUIItem->GetComponentTransform().Inverse();
	const auto& itemTf = item->GetComponentTransform();
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
	
	FTransform itemToCanvasTf;
	FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);
	FVector tempV3;
#if WITH_EDITOR
	if (!canvas->GetWorld()->IsGameWorld())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			tempV3 = itemToCanvasTf.TransformPosition(originPositions[i]);
			vertices[i].Position = tempV3;
		}
	}
	else
#endif
	{
		if (canvas->IsScreenSpaceOverlayUI())//convert to world space
		{
			for (int i = 0; i < vertexCount; i++)
			{
				tempV3 = itemTf.TransformPosition(originPositions[i]);
				vertices[i].Position = tempV3;
			}
		}
		else//convert vertex to canvas's local space
		{
			for (int i = 0; i < vertexCount; i++)
			{
				tempV3 = itemToCanvasTf.TransformPosition(originPositions[i]);
				vertices[i].Position = tempV3;
			}
		}
	}

	if (requireNormal)
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
	if (requireTangent)
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
