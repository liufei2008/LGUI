// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/LGUICanvas.h"


#pragma region UISprite_UITexture_Simple
void UIGeometry::FromUIRectSimple(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
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
	UpdateUIRectSimpleVertex(uiGeo, width, height, pivot);
	//uvs
	UpdateUIRectSimpleUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
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
	if (requireTangent)
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
	if (requireUV1)
	{
		auto& vertices = uiGeo->vertices;
		vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
	}
}

void UIGeometry::UpdateUIRectSimpleUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	if (spriteInfo == nullptr)
	{
		vertices[0].TextureCoordinate[0] = FVector2D(0, 1);
		vertices[1].TextureCoordinate[0] = FVector2D(1, 1);
		vertices[2].TextureCoordinate[0] = FVector2D(0, 0);
		vertices[3].TextureCoordinate[0] = FVector2D(1, 0);
	}
	else
	{
		vertices[0].TextureCoordinate[0] = spriteInfo->GetUV0();
		vertices[1].TextureCoordinate[0] = spriteInfo->GetUV1();
		vertices[2].TextureCoordinate[0] = spriteInfo->GetUV2();
		vertices[3].TextureCoordinate[0] = spriteInfo->GetUV3();
	}
}
void UIGeometry::UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//positions
	auto& vertices = uiGeo->originPositions;
	float x, y;
	x = (-halfW + pivotOffsetX);
	y = (-halfH + pivotOffsetY);
	vertices[0] = FVector(x, y, 0);
	x = (halfW + pivotOffsetX);
	vertices[1] = FVector(x, y, 0);
	x = (-halfW + pivotOffsetX);
	y = (halfH + pivotOffsetY);
	vertices[2] = FVector(x, y, 0);
	x = (halfW + pivotOffsetX);
	vertices[3] = FVector(x, y, 0);
}
#pragma endregion
#pragma region UISprite_UITexture_Border
void UIGeometry::FromUIRectBorder(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool fillCenter, bool requireNormal, bool requireTangent, bool requireUV1)
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
	UpdateUIRectBorderVertex(uiGeo, width, height, pivot, spriteInfo);
	//uvs
	UpdateUIRectBorderUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
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
	if (requireTangent)
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
	if (requireUV1)
	{
		auto& vertices = uiGeo->vertices;
		float widthReciprocal = 1.0f / spriteInfo->width;
		float heightReciprocal = 1.0f / spriteInfo->height;
		float buv0X = spriteInfo->borderLeft * widthReciprocal;
		float buv3X = 1.0f - spriteInfo->borderRight * widthReciprocal;
		float buv0Y = 1.0f - spriteInfo->borderBottom * heightReciprocal;
		float buv3Y = spriteInfo->borderTop * heightReciprocal;

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

void UIGeometry::UpdateUIRectBorderUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, spriteInfo->uv0Y);
	vertices[1].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, spriteInfo->uv0Y);
	vertices[2].TextureCoordinate[0] = FVector2D(spriteInfo->buv3X, spriteInfo->uv0Y);
	vertices[3].TextureCoordinate[0] = FVector2D(spriteInfo->uv3X, spriteInfo->uv0Y);

	vertices[4].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, spriteInfo->buv0Y);
	vertices[5].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, spriteInfo->buv0Y);
	vertices[6].TextureCoordinate[0] = FVector2D(spriteInfo->buv3X, spriteInfo->buv0Y);
	vertices[7].TextureCoordinate[0] = FVector2D(spriteInfo->uv3X, spriteInfo->buv0Y);

	vertices[8].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, spriteInfo->buv3Y);
	vertices[9].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, spriteInfo->buv3Y);
	vertices[10].TextureCoordinate[0] = FVector2D(spriteInfo->buv3X, spriteInfo->buv3Y);
	vertices[11].TextureCoordinate[0] = FVector2D(spriteInfo->uv3X, spriteInfo->buv3Y);

	vertices[12].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, spriteInfo->uv3Y);
	vertices[13].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, spriteInfo->uv3Y);
	vertices[14].TextureCoordinate[0] = FVector2D(spriteInfo->buv3X, spriteInfo->uv3Y);
	vertices[15].TextureCoordinate[0] = FVector2D(spriteInfo->uv3X, spriteInfo->uv3Y);
}
void UIGeometry::UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, const FLGUISpriteInfo* spriteInfo)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	float x0, x1, x2, x3, y0, y1, y2, y3;
	int widthBorder = spriteInfo->borderLeft + spriteInfo->borderRight;
	int heightBorder = spriteInfo->borderTop + spriteInfo->borderBottom;
	float widthScale = width < widthBorder ? width / widthBorder : 1.0f;
	float heightScale = height < heightBorder ? height / heightBorder : 1.0f;
	x0 = (-halfW + pivotOffsetX);
	x1 = (x0 + spriteInfo->borderLeft * widthScale);
	x3 = (halfW + pivotOffsetX);
	x2 = (x3 - spriteInfo->borderRight * widthScale);
	y0 = (-halfH + pivotOffsetY);
	y1 = (y0 + spriteInfo->borderBottom * heightScale);
	y3 = (halfH + pivotOffsetY);
	y2 = (y3 - spriteInfo->borderTop * heightScale);

	auto& vertices = uiGeo->originPositions;
	vertices[0] = FVector(x0, y0, 0);
	vertices[1] = FVector(x1, y0, 0);
	vertices[2] = FVector(x2, y0, 0);
	vertices[3] = FVector(x3, y0, 0);

	vertices[4] = FVector(x0, y1, 0);
	vertices[5] = FVector(x1, y1, 0);
	vertices[6] = FVector(x2, y1, 0);
	vertices[7] = FVector(x3, y1, 0);

	vertices[8] = FVector(x0, y2, 0);
	vertices[9] = FVector(x1, y2, 0);
	vertices[10] = FVector(x2, y2, 0);
	vertices[11] = FVector(x3, y2, 0);

	vertices[12] = FVector(x0, y3, 0);
	vertices[13] = FVector(x1, y3, 0);
	vertices[14] = FVector(x2, y3, 0);
	vertices[15] = FVector(x3, y3, 0);
}
#pragma endregion

#pragma region UISprite_Tiled
void UIGeometry::FromUIRectTiled(const float& width, const float& height, const FVector2D& pivot, const FColor& color, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
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
	UpdateUIRectTiledVertex(uiGeo, spriteInfo, width, height, pivot, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//uvs
	UpdateUIRectTiledUV(uiGeo, spriteInfo, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
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
	if (requireTangent)
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
	if (requireUV1)
	{
		auto& vertices = uiGeo->vertices;
		for (int i = 0; i < uiGeo->originVerticesCount; i += 4)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 0);
			vertices[i].TextureCoordinate[1] = FVector2D(1, 0);
			vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
			vertices[i].TextureCoordinate[1] = FVector2D(1, 1);
		}
	}
}
void UIGeometry::UpdateUIRectTiledUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize)
{
	auto& vertices = uiGeo->vertices;

	int vertIndex = 0;
	float remainedUV3X = spriteInfo->buv0X + (spriteInfo->buv3X - spriteInfo->buv0X) * widthRemainedRectSize / spriteInfo->width;
	float remainedUV3Y = spriteInfo->buv0Y + (spriteInfo->buv3Y - spriteInfo->buv0Y) * heightRemainedRectSize / spriteInfo->height;
	for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
	{
		float realUV3Y = heightRectIndex == heightRectCount ? remainedUV3Y : spriteInfo->buv3Y;
		for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
		{
			float realUV3X = widthRectIndex == widthRectCount ? remainedUV3X : spriteInfo->buv3X;
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, spriteInfo->buv0Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(realUV3X, spriteInfo->buv0Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(spriteInfo->buv0X, realUV3Y);
			vertices[vertIndex++].TextureCoordinate[0] = FVector2D(realUV3X, realUV3Y);
		}
	}
}
void UIGeometry::UpdateUIRectTiledVertex(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	int rectangleCount = widthRectCount * heightRectCount;
	auto& vertices = uiGeo->originPositions;
	int vertIndex = 0;
	float startX = (-halfW + pivotOffsetX);
	float startY = (-halfH + pivotOffsetY);
	float x = startX, y = startY;
	for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
	{
		float realHeight = heightRectIndex == heightRectCount ? heightRemainedRectSize : spriteInfo->height;
		for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
		{
			float realWidth = widthRectIndex == widthRectCount ? (widthRemainedRectSize) : spriteInfo->width;
			vertices[vertIndex++] = FVector(x, y, 0);
			vertices[vertIndex++] = FVector(x + realWidth, y, 0);
			vertices[vertIndex++] = FVector(x, y + realHeight, 0);
			vertices[vertIndex++] = FVector(x + realWidth, y + realHeight, 0);
			
			x += spriteInfo->width;
		}
		x = startX;
		y += spriteInfo->height;
	}
}
#pragma endregion

#pragma region UIText
void UIGeometry::FromUIText(FString& content, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot, FColor color, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, int fontSize, uint8 paragraphHAlign, uint8 paragraphVAlign, uint8 overflowType, bool adjustWidth, bool adjustHeight, uint8 fontStyle, FVector2D& textRealSize, float dynamicPixelsPerUnit, TArray<FUITextLineProperty>& cacheTextPropertyList, const TArray<FUITextCharGeometry>& cacheTextGeometryList, bool requireNormal, bool requireTangent, bool requireUV1)
{
	//generate vertex/triangle/uv
	int vertexCount = visibleCharCount * 4;
	int triangleCount = visibleCharCount * 6;
	auto& vertices = uiGeo->vertices;
	auto& triangles = uiGeo->triangles;
	uiGeo->originVerticesCount = vertexCount;
	vertices.SetNumUninitialized(vertexCount);

	uiGeo->originTriangleCount = triangleCount;
	triangles.Reserve(triangleCount);
	int vertIndex = 0;
	for (int i = 0; i < visibleCharCount; i++)
	{
		triangles.Add(vertIndex);
		triangles.Add(vertIndex + 3);
		triangles.Add(vertIndex + 2);
		triangles.Add(vertIndex);
		triangles.Add(vertIndex + 1);
		triangles.Add(vertIndex + 3);
		vertIndex += 4;
	}

	//position and uv
	auto& originPositions = uiGeo->originPositions;
	if (originPositions.Num() == 0)
	{
		originPositions.SetNumUninitialized(uiGeo->originVerticesCount);
	}
	UpdateUITextVertexOrUV(content, width, height, pivot, fontSpace, uiGeo, fontSize, paragraphHAlign, paragraphVAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, dynamicPixelsPerUnit, true, true, cacheTextPropertyList, cacheTextGeometryList);
	//colors
	UpdateUIColor(uiGeo, color);

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
		auto& vertices = uiGeo->vertices;
		for (int i = 0; i < visibleCharCount; i++)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
			vertices[i].TextureCoordinate[1] = FVector2D(1, 1);
			vertices[i].TextureCoordinate[1] = FVector2D(0, 0);
			vertices[i].TextureCoordinate[1] = FVector2D(1, 0);
		}
	}
}

void UIGeometry::UpdateUITextVertexOrUV(FString& content, float& width, float& height, const FVector2D& pivot, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, int fontSize, uint8 paragraphHAlign, uint8 paragraphVAlign, uint8 overflowType, bool adjustWidth, bool adjustHeight, uint8 fontStyle, FVector2D& textRealSize, float dynamicPixelsPerUnit, bool updateVertex, bool updateUV, TArray<FUITextLineProperty>& cacheTextPropertyList, const TArray<FUITextCharGeometry>& cacheTextGeometryList)
{
	cacheTextPropertyList.Reset();
	int contentLength = content.Len();
	FVector2D currentLineOffset(0, 0);
	float currentLineWidth = 0, paragraphHeight = 0;//single line width, all line height
	float maxLineWidth = 0;//if have multiple line
	TArray<FVector*> currentLineUIGeoVertexList;//single line's vertex collection
	int visibleCharIndex = 0;//visible char index, skip invisible char(\n,\t)
	auto& vertices = uiGeo->originPositions;
	auto& uvs = uiGeo->vertices;
	FUITextLineProperty sentenceProperty;
	FVector2D caretPosition(0, 0);
	float charGeoWidth = 0, charGeoHeight = 0, halfFontSize = fontSize * 0.5f;
	bool haveMultipleSentence = false;
	bool isNewLineFirstChar = false;
	FUITextCharGeometry charGeo = FUITextCharGeometry();

	for (int charIndex = 0; charIndex < contentLength; charIndex++)
	{
		auto charCode = content[charIndex];
		FVector2D charOffset(0, 0);//single char offset

		if (charCode == '\n' || charCode == '\r')//10 -- \n, 13 -- \r
		{
MANUAL_NEWLINE://new line
			currentLineWidth -= fontSpace.X;//last char of a line don't need space
			maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
			FUITextCharProperty charProperty;
			charProperty.caretPosition = caretPosition;
			charProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(charProperty);

			UpdateUITextLineVertex(paragraphHAlign, currentLineWidth, currentLineUIGeoVertexList, sentenceProperty);
			currentLineUIGeoVertexList.Empty();

			cacheTextPropertyList.Add(sentenceProperty);

			sentenceProperty = FUITextLineProperty();
			if (charIndex + 1 == contentLength)//if \n is also last char, then we need to add a caret position to the next line
			{
				currentLineOffset.X = 0;
				currentLineOffset.Y -= fontSize + fontSpace.Y;
				caretPosition.X = currentLineOffset.X - fontSpace.X;
				caretPosition.Y = currentLineOffset.Y;
				FUITextCharProperty nextLineCharProperty;
				nextLineCharProperty.caretPosition = caretPosition;
				nextLineCharProperty.charIndex = charIndex + 1;
				sentenceProperty.charPropertyList.Add(nextLineCharProperty);
				cacheTextPropertyList.Add(sentenceProperty);
			}
			currentLineOffset.X = 0;
			goto NEW_LINE;
		}
		
		charGeo = cacheTextGeometryList[visibleCharIndex];

		charGeoWidth = charGeo.geoWidth;
		charGeoHeight = charGeo.geoHeight;
		charOffset = FVector2D(charGeo.xoffset, charGeo.yoffset);
		//caret property
		{
			if (isNewLineFirstChar)
			{
				caretPosition.X = currentLineOffset.X;
			}
			else
			{
				caretPosition.X = currentLineOffset.X + charOffset.X - fontSpace.X;
			}
			caretPosition.Y = currentLineOffset.Y;
			FUITextCharProperty charProperty;
			charProperty.caretPosition = caretPosition;
			charProperty.charIndex = charIndex;
			sentenceProperty.charPropertyList.Add(charProperty);

			caretPosition.X += fontSpace.X + charGeoWidth;//next caret position x
		}
		
		//char geometry
		{
			if (updateVertex)
			{
				FVector2D offset = currentLineOffset + charOffset;
				float x, y;
				x = (offset.X);
				y = (offset.Y - charGeoHeight);
				int index = visibleCharIndex * 4;
				vertices[index] = FVector(x, y, 0);
				x = (charGeoWidth + offset.X);
				vertices[index + 1] = FVector(x, y, 0);
				x = (offset.X);
				y = (offset.Y);
				vertices[index + 2] = FVector(x, y, 0);
				x = (charGeoWidth + offset.X);
				vertices[index + 3] = FVector(x, y, 0);

				currentLineUIGeoVertexList.Add(&vertices[index]);
				currentLineUIGeoVertexList.Add(&vertices[index + 1]);
				currentLineUIGeoVertexList.Add(&vertices[index + 2]);
				currentLineUIGeoVertexList.Add(&vertices[index + 3]);
			}
			if (updateUV)
			{
				int index = visibleCharIndex * 4;
				uvs[index].TextureCoordinate[0] = charGeo.uv0;
				uvs[index + 1].TextureCoordinate[0] = charGeo.uv1;
				uvs[index + 2].TextureCoordinate[0] = charGeo.uv2;
				uvs[index + 3].TextureCoordinate[0] = charGeo.uv3;
			}
			visibleCharIndex++;
		}

		if (isNewLineFirstChar)
		{
			isNewLineFirstChar = false;
			if (charCode == ' ')//if is new line, and first char is space, then skip that space
			{
				continue;
			}
		}

		if (charCode == ' ')//char is space
		{
			if (overflowType == 1)//char is space and UIText can have multi line, then we need to calculate if the followed word can fit the rest space, if not means new line
			{
				float spaceNeeded = halfFontSize;//space
				spaceNeeded += fontSpace.X;
				for (int j = charIndex + 1, forwardCharGoeIndex = visibleCharIndex; j < contentLength && forwardCharGoeIndex < cacheTextGeometryList.Num(); j++)
				{
					auto charCodeOfJ = content[j];
					if (charCodeOfJ == ' ')//space
					{
						break;
					}
					if (charCodeOfJ == '\n' || charCodeOfJ == '\r' || charCodeOfJ == '\t')//\n\r\t
					{
						break;
					}
					spaceNeeded += cacheTextGeometryList[forwardCharGoeIndex].xadvance;
					spaceNeeded += fontSpace.X;
					forwardCharGoeIndex++;
				}

				if (currentLineOffset.X + spaceNeeded > width)
				{
					goto MANUAL_NEWLINE;
				}
			}
		}
		

		float charWidth = charGeo.xadvance + fontSpace.X;
		currentLineOffset.X += charWidth;
		currentLineWidth += charWidth;

		if (charIndex + 1 == contentLength)//if is last char
		{
			currentLineWidth -= fontSpace.X;//last char don't need space
			maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
			FUITextCharProperty charProperty;
			charProperty.caretPosition = caretPosition;
			charProperty.charIndex = charIndex + 1;
			sentenceProperty.charPropertyList.Add(charProperty);
			UpdateUITextLineVertex(paragraphHAlign, currentLineWidth, currentLineUIGeoVertexList, sentenceProperty);
			paragraphHeight += fontSize;
			cacheTextPropertyList.Add(sentenceProperty);
			break;//end loop
		}
		else//not last char
		{
			switch (overflowType)
			{
			case 0://horizontal overflow
			{
				//no need to do anything
			}
			break;
			case 1://vertical overflow
			{
				int nextCharXAdv = 0;
				if (UUIText::IsVisibleChar(content[charIndex + 1]))//next char is visible
				{
					nextCharXAdv = cacheTextGeometryList[visibleCharIndex].xadvance;
				}
				else
				{
					nextCharXAdv = halfFontSize;
				}
				if (currentLineOffset.X + nextCharXAdv > width)//if next char cannot fit this line, then add new line
				{
					auto nextChar = content[charIndex + 1];
					if (nextChar == '\r' || nextChar == '\n')
					{
						//next char is new line, no need to add new line
					}
					else
					{
						//add end caret position
						FUITextCharProperty charProperty;
						charProperty.caretPosition = caretPosition;
						charProperty.charIndex = charIndex + 1;
						sentenceProperty.charPropertyList.Add(charProperty);

						currentLineWidth -= fontSpace.X;//last char don't need space
						maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
						UpdateUITextLineVertex(paragraphHAlign, currentLineWidth, currentLineUIGeoVertexList, sentenceProperty);
						currentLineUIGeoVertexList.Empty();
						cacheTextPropertyList.Add(sentenceProperty);
						sentenceProperty = FUITextLineProperty();

						currentLineOffset.X = 0;
						goto NEW_LINE;
					}
				}
			}
			break;
			case 2://clamp content
			{
				int nextCharXAdv = 0;
				if (UUIText::IsVisibleChar(content[charIndex + 1]))//next char is visible
				{
					nextCharXAdv = cacheTextGeometryList[visibleCharIndex].xadvance;
				}
				else
				{
					nextCharXAdv = halfFontSize;
				}
				if (currentLineOffset.X + nextCharXAdv > width)//discard out-of-range chars
				{
					currentLineWidth -= fontSpace.X;//last char don't need space
					maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
					UpdateUITextLineVertex(paragraphHAlign, currentLineWidth, currentLineUIGeoVertexList, sentenceProperty);
					paragraphHeight += fontSize;
					cacheTextPropertyList.Add(sentenceProperty);
					//set rest char's position to invisible
					int verticesCount = vertices.Num();
					for (int vertIndex = visibleCharIndex * 4; vertIndex < verticesCount; vertIndex++)
					{
						vertices[vertIndex] = FVector::ZeroVector;
					}

					charIndex = contentLength;//break the main loop
				}
			}
			break;
			}
		}
		continue;//skip NEW_LINE
	NEW_LINE://jump from auto-new-line or \n
		currentLineWidth = 0;
		currentLineOffset.Y -= fontSize + fontSpace.Y;
		paragraphHeight += fontSize + fontSpace.Y;
		haveMultipleSentence = true;
		isNewLineFirstChar = true;
	}


	switch (overflowType)
	{
	case 0://horizontal overflow
	{
		textRealSize.X = maxLineWidth;
		textRealSize.Y = paragraphHeight;
		if (adjustWidth)
		{
			width = textRealSize.X;
		}
	}
	break;
	case 1://vertical overflow
	{
		if (haveMultipleSentence)
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
	case 2://clamp content
	{
		textRealSize.X = maxLineWidth;
		textRealSize.Y = paragraphHeight;
	}
	break;
	}



	float pivotOffsetX = width * (0.5f - pivot.X);
	float pivotOffsetY = height * (0.5f - pivot.Y);
	float xOffset = pivotOffsetX;
	float yOffset = pivotOffsetY - halfFontSize;
	switch (paragraphHAlign)
	{
	case 0:
		xOffset += -width * 0.5f;
		break;
	case 1:

		break;
	case 2:
		xOffset += width * 0.5f;
		break;
	}
	switch (paragraphVAlign)
	{
	case 0:
		yOffset += height * 0.5f;
		break;
	case 1:
		yOffset += paragraphHeight * 0.5f;
		break;
	case 2:
		yOffset += paragraphHeight - height * 0.5f;
		break;
	}
	//cache property
	for (auto& sentenceItem : cacheTextPropertyList)
	{
		for (auto& charItem : sentenceItem.charPropertyList)
		{
			charItem.caretPosition.X += xOffset;
			charItem.caretPosition.Y += yOffset;
		}
	}
	if (updateVertex)
	{
		UIGeometry::OffsetVertices(vertices, xOffset, yOffset);
	}
}

void UIGeometry::UpdateUITextLineVertex(int pivotHAlign, float sentenceWidth, TArray<FVector*>& sentenceUIGeoVertexList, FUITextLineProperty& sentenceProperty)
{
	float xOffset = 0;
	switch (pivotHAlign)
	{
	case 1:
		xOffset = -sentenceWidth * 0.5f;
		break;
	case 2:
		xOffset = -sentenceWidth;
		break;
	}
	UIGeometry::OffsetVertices(sentenceUIGeoVertexList, xOffset, 0);
	
	auto& charList = sentenceProperty.charPropertyList;
	for (auto& item : charList)
	{
		item.caretPosition.X += xOffset;
	}
}

#pragma endregion

#pragma region UISector
void UIGeometry::FromUISector(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
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
		auto& vertices = uiGeo->vertices;
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
		}
	}
}
void UIGeometry::UpdateUISectorUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, const FLGUISpriteInfo* spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	int vertexCount = uiGeo->originVerticesCount;
	if(uvType == 0)
	{
		float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
		float angle = FMath::DegreesToRadians(startAngle);

		float sin = FMath::Sin(angle);
		float cos = FMath::Cos(angle);

		float halfUVWidth = (spriteInfo->uv3X - spriteInfo->uv0X) * 0.5f;
		float halfUVHeight = (spriteInfo->uv3Y - spriteInfo->uv0Y) * 0.5f;
		float centerUVX = (spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f;
		float centerUVY = (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f;

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
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		FVector2D otherUV(spriteInfo->uv3X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
		}
	}
	else if (uvType == 2)
	{
		vertices[0].TextureCoordinate[0] = FVector2D((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv0Y);
		FVector2D otherUV((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv3Y);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
		}
	}
	else if (uvType == 3)
	{
		vertices[0].TextureCoordinate[0] = FVector2D(spriteInfo->uv0X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo->uv3X, spriteInfo->uv0Y);
		for (int i = 1; i < vertexCount; i++)
		{
			vertices[i].TextureCoordinate[0] = otherUV;
			otherUV.Y += uvYInterval;
		}
	}
	else if (uvType == 4)
	{
		vertices[0].TextureCoordinate[0] = FVector2D((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv0Y);
		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo->uv0X, spriteInfo->uv3Y);
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
	auto& vertices = uiGeo->originPositions;
	if (vertices.Num() == 0)
	{
		int vertexCount = 2 + segment + 1;
		uiGeo->originVerticesCount = vertexCount;
		vertices.AddUninitialized(vertexCount);
	}

	float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
	float angle = FMath::DegreesToRadians(startAngle);

	float sin = FMath::Sin(angle);
	float cos = FMath::Cos(angle);

	float x = pivotOffsetX;
	float y = pivotOffsetY;
	vertices[0] = FVector(x, y, 0);
	x = cos * halfW + pivotOffsetX;
	y = sin * halfH + pivotOffsetY;
	vertices[1] = FVector(x, y, 0);

	for (int i = 0; i < segment + 1; i++)
	{
		angle += singleAngle;
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * halfW + pivotOffsetX;
		y = sin * halfH + pivotOffsetY;
		vertices[i + 2] = FVector(x, y, 0);
	}
}
#pragma endregion

#pragma region UIRing
void UIGeometry::FromUIRing(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, float ringWidth, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1)
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
		auto& vertices = uiGeo->vertices;
		if (vertices.Num() == 0)
		{
			vertices.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				vertices[i].TextureCoordinate[1] = FVector2D(0, 1);
			}
		}
	}
}
void UIGeometry::UpdateUIRingUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, float ringWidth, float& width, float& height, const FLGUISpriteInfo* spriteInfo)
{
	auto& vertices = uiGeo->vertices;
	int vertexCount = uiGeo->originVerticesCount;
	if (uvType == 0)
	{
		float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
		float angle = FMath::DegreesToRadians(startAngle);

		float sin = FMath::Sin(angle);
		float cos = FMath::Cos(angle);

		float halfUVWidth = (spriteInfo->uv3X - spriteInfo->uv0X) * 0.5f;
		float halfUVHeight = (spriteInfo->uv3Y - spriteInfo->uv0Y) * 0.5f;
		float centerUVX = (spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f;
		float centerUVY = (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f;

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
		float centerUVX = (spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f;

		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (segment + 1);
		float uvY = spriteInfo->uv0Y;
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
		float centerUVY = (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f;

		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (segment + 1);
		float uvX = spriteInfo->uv0X;
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
		float innerUVX = spriteInfo->uv0X;
		float outerUVX = spriteInfo->uv3X;

		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (segment + 1);
		float uvY = spriteInfo->uv0Y;

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
		float innerUVY = spriteInfo->uv0Y;
		float outerUVY = spriteInfo->uv3Y;

		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (segment + 1);
		float uvX = spriteInfo->uv0X;

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
	auto& vertices = uiGeo->originPositions;
	if (vertices.Num() == 0)
	{
		int vertexCount = 2 + (segment + 1) * 2;
		uiGeo->originVerticesCount = vertexCount;
		vertices.AddUninitialized(vertexCount);
	}

	float singleAngle = FMath::DegreesToRadians((endAngle - startAngle) / (segment + 1));
	float angle = FMath::DegreesToRadians(startAngle);
	float minHalfW = halfW - ringWidth;
	float minHalfH = halfH - ringWidth;

	float sin = FMath::Sin(angle);
	float cos = FMath::Cos(angle);

	float x = cos * minHalfW + pivotOffsetX;
	float y = sin * minHalfH + pivotOffsetY;
	vertices[0] = FVector(x, y, 0);
	x = cos * halfW + pivotOffsetX;
	y = sin * halfH + pivotOffsetY;
	vertices[1] = FVector(x, y, 0);

	for (int i = 0; i < (segment + 1); i++)
	{
		angle += singleAngle;
		sin = FMath::Sin(angle);
		cos = FMath::Cos(angle);
		x = cos * minHalfW + pivotOffsetX;
		y = sin * minHalfH + pivotOffsetY;
		vertices[i * 2 + 2] = FVector(x, y, 0);
		x = cos * halfW + pivotOffsetX;
		y = sin * halfH + pivotOffsetY;
		vertices[i * 2 + 3] = FVector(x, y, 0);
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

FORCEINLINE float RoundToFloat(float value)
{
	return value > 0.0 ? FMath::FloorToFloat(value + 0.5f) : FMath::CeilToFloat(value - 0.5f);
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
	
	auto rootCanvasUIItem = canvas->GetRootCanvas()->CheckAndGetUIItem();

	FTransform itemToCanvasTf;
	FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);
	FVector tempV3;
	if (canvas->GetPixelPerfect() && rootCanvasUIItem != nullptr)
	{
		if (canvasUIItem == rootCanvasUIItem)
		{
			auto canvasLeftBottom = FVector2D(-canvasUIItem->GetWidth() * canvasUIItem->GetPivot().X, -canvasUIItem->GetHeight() * canvasUIItem->GetPivot().Y);
			for (int i = 0; i < vertexCount; i++)
			{
				tempV3 = itemToCanvasTf.TransformPosition(originPositions[i]);
				tempV3.X -= canvasLeftBottom.X;
				tempV3.Y -= canvasLeftBottom.Y;
				tempV3.X = RoundToFloat(tempV3.X);
				tempV3.Y = RoundToFloat(tempV3.Y);
				tempV3.X += canvasLeftBottom.X;
				tempV3.Y += canvasLeftBottom.Y;

				vertices[i].Position = tempV3;
			}
		}
		else
		{
			auto rootCanvasToWorld = rootCanvasUIItem->GetComponentTransform();
			auto inverseRootCanvasTf = rootCanvasToWorld.Inverse();
			FTransform vertToRootCanvasTf;
			FTransform::Multiply(&vertToRootCanvasTf, &itemTf, &inverseRootCanvasTf);

			FTransform vertToCanvas;
			FTransform::Multiply(&vertToCanvas, &rootCanvasToWorld, &inverseCanvasTf);

			auto canvasLeftBottom = FVector2D(-rootCanvasUIItem->GetWidth() * rootCanvasUIItem->GetPivot().X, -rootCanvasUIItem->GetHeight() * rootCanvasUIItem->GetPivot().Y);
			for (int i = 0; i < vertexCount; i++)
			{
				tempV3 = vertToRootCanvasTf.TransformPosition(originPositions[i]);
				tempV3.X -= canvasLeftBottom.X;
				tempV3.Y -= canvasLeftBottom.Y;
				tempV3.X = RoundToFloat(tempV3.X);
				tempV3.Y = RoundToFloat(tempV3.Y);
				tempV3.X += canvasLeftBottom.X;
				tempV3.Y += canvasLeftBottom.Y;

				tempV3 = vertToCanvas.TransformPosition(tempV3);

				vertices[i].Position = tempV3;
			}
		}
	}
	else
	{
		for (int i = 0; i < vertexCount; i++)
		{
			tempV3 = itemToCanvasTf.TransformPosition(originPositions[i]);
			vertices[i].Position = tempV3;
		}
	}

	if (requireNormal)
	{
		auto& vertices = uiGeo->vertices;
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
			vertices[i].TangentZ.Vector.W = 127;
		}
	}
	if (requireTangent)
	{
		auto& vertices = uiGeo->vertices;
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
