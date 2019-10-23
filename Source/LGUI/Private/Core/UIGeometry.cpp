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
		triangles.AddUninitialized(6);
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}
	//vertices
	UpdateUIRectSimpleVertex(uiGeo, width, height, pivot);
	//uvs
	UpdateUIRectSimpleUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.Reserve(4);
			uvs1.Add(FVector2D(0, 1));
			uvs1.Add(FVector2D(1, 1));
			uvs1.Add(FVector2D(0, 0));
			uvs1.Add(FVector2D(1, 0));
		}
	}
}

void UIGeometry::UpdateUIRectSimpleUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo)
{
	auto& uvs = uiGeo->uvs;
	if (uvs.Num() == 0)
	{
		uvs.AddUninitialized(4);
	}
	if (spriteInfo == nullptr)
	{
		uvs[0] = FVector2D(0, 1);
		uvs[1] = FVector2D(1, 1);
		uvs[2] = FVector2D(0, 0);
		uvs[3] = FVector2D(1, 0);
	}
	else
	{
		uvs[0] = spriteInfo->GetUV0();
		uvs[1] = spriteInfo->GetUV1();
		uvs[2] = spriteInfo->GetUV2();
		uvs[3] = spriteInfo->GetUV3();
	}
}
void UIGeometry::UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4;
		vertices.AddUninitialized(4);
	}
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
	//vertices;
	UpdateUIRectBorderVertex(uiGeo, width, height, pivot, spriteInfo);
	//uvs
	UpdateUIRectBorderUV(uiGeo, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.AddUninitialized(16);
			float widthReciprocal = 1.0f / spriteInfo->width;
			float heightReciprocal = 1.0f / spriteInfo->height;
			float buv0X = spriteInfo->borderLeft * widthReciprocal;
			float buv3X = 1.0f - spriteInfo->borderRight * widthReciprocal;
			float buv0Y = 1.0f - spriteInfo->borderBottom * heightReciprocal;
			float buv3Y = spriteInfo->borderTop * heightReciprocal;

			uvs1[0] = FVector2D(0, 1);
			uvs1[1] = FVector2D(buv0X, 1);
			uvs1[2] = FVector2D(buv3X, 1);
			uvs1[3] = FVector2D(1, 1);

			uvs1[4] = FVector2D(0, buv0Y);
			uvs1[5] = FVector2D(buv0X, buv0Y);
			uvs1[6] = FVector2D(buv3X, buv0Y);
			uvs1[7] = FVector2D(1, buv0Y);

			uvs1[8] = FVector2D(0, buv3Y);
			uvs1[9] = FVector2D(buv0X, buv3Y);
			uvs1[10] = FVector2D(buv3X, buv3Y);
			uvs1[11] = FVector2D(1, buv3Y);

			uvs1[12] = FVector2D(0, 0);
			uvs1[13] = FVector2D(buv0X, 0);
			uvs1[14] = FVector2D(buv3X, 0);
			uvs1[15] = FVector2D(1, 0);
		}
	}
}

void UIGeometry::UpdateUIRectBorderUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo)
{
	auto& uvs = uiGeo->uvs;
	if (uvs.Num() == 0)
	{
		uvs.AddUninitialized(16);
	}

	uvs[0] = FVector2D(spriteInfo->uv0X, spriteInfo->uv0Y);
	uvs[1] = FVector2D(spriteInfo->buv0X, spriteInfo->uv0Y);
	uvs[2] = FVector2D(spriteInfo->buv3X, spriteInfo->uv0Y);
	uvs[3] = FVector2D(spriteInfo->uv3X, spriteInfo->uv0Y);

	uvs[4] = FVector2D(spriteInfo->uv0X, spriteInfo->buv0Y);
	uvs[5] = FVector2D(spriteInfo->buv0X, spriteInfo->buv0Y);
	uvs[6] = FVector2D(spriteInfo->buv3X, spriteInfo->buv0Y);
	uvs[7] = FVector2D(spriteInfo->uv3X, spriteInfo->buv0Y);

	uvs[8] = FVector2D(spriteInfo->uv0X, spriteInfo->buv3Y);
	uvs[9] = FVector2D(spriteInfo->buv0X, spriteInfo->buv3Y);
	uvs[10] = FVector2D(spriteInfo->buv3X, spriteInfo->buv3Y);
	uvs[11] = FVector2D(spriteInfo->uv3X, spriteInfo->buv3Y);

	uvs[12] = FVector2D(spriteInfo->uv0X, spriteInfo->uv3Y);
	uvs[13] = FVector2D(spriteInfo->buv0X, spriteInfo->uv3Y);
	uvs[14] = FVector2D(spriteInfo->buv3X, spriteInfo->uv3Y);
	uvs[15] = FVector2D(spriteInfo->uv3X, spriteInfo->uv3Y);
}
void UIGeometry::UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, const FLGUISpriteInfo* spriteInfo)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices;
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

	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 16;
		vertices.Reserve(16);
		for (int i = 0; i < 16; i++)
		{
			vertices.Add(FVector());
		}
	}
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
	UpdateUIRectTiledVertex(uiGeo, spriteInfo, width, height, pivot, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//uvs
	UpdateUIRectTiledUV(uiGeo, spriteInfo, widthRectCount, heightRectCount, widthRemainedRectSize, heightRemainedRectSize);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.Reserve(uiGeo->originVerticesCount);
			for (int i = 0; i < uiGeo->originVerticesCount; i+=4)
			{
				uvs1.Add(FVector2D(0, 0));
				uvs1.Add(FVector2D(1, 0));
				uvs1.Add(FVector2D(0, 1));
				uvs1.Add(FVector2D(1, 1));
			}
		}
	}
}
void UIGeometry::UpdateUIRectTiledUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize)
{
	auto& uvs = uiGeo->uvs;
	if (uvs.Num() == 0)
	{
		uvs.AddUninitialized(uiGeo->originVerticesCount);
	}

	int vertIndex = 0;
	float remainedUV3X = spriteInfo->buv0X + (spriteInfo->buv3X - spriteInfo->buv0X) * widthRemainedRectSize / spriteInfo->width;
	float remainedUV3Y = spriteInfo->buv0Y + (spriteInfo->buv3Y - spriteInfo->buv0Y) * heightRemainedRectSize / spriteInfo->height;
	for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
	{
		float realUV3Y = heightRectIndex == heightRectCount ? remainedUV3Y : spriteInfo->buv3Y;
		for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
		{
			float realUV3X = widthRectIndex == widthRectCount ? remainedUV3X : spriteInfo->buv3X;
			uvs[vertIndex++] = FVector2D(spriteInfo->buv0X, spriteInfo->buv0Y);
			uvs[vertIndex++] = FVector2D(realUV3X, spriteInfo->buv0Y);
			uvs[vertIndex++] = FVector2D(spriteInfo->buv0X, realUV3Y);
			uvs[vertIndex++] = FVector2D(realUV3X, realUV3Y);
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
	auto& vertices = uiGeo->vertices;
	if (vertices.Num() == 0)
	{
		uiGeo->originVerticesCount = 4 * rectangleCount;
		vertices.AddUninitialized(uiGeo->originVerticesCount);
	}
	//
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
	auto& uvs = uiGeo->uvs;
	auto& triangles = uiGeo->triangles;
	uiGeo->originVerticesCount = vertexCount;
	vertices.AddUninitialized(vertexCount);
	uvs.AddUninitialized(vertexCount);
	
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

	UpdateUITextVertexOrUV(content, width, height, pivot, fontSpace, uiGeo, fontSize, paragraphHAlign, paragraphVAlign, overflowType, adjustWidth, adjustHeight, fontStyle, textRealSize, dynamicPixelsPerUnit, true, true, cacheTextPropertyList, cacheTextGeometryList);
	//colors
	UpdateUIColor(uiGeo, color);

	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.Reserve(vertexCount);
			for (int i = 0; i < visibleCharCount; i++)
			{
				uvs1.Add(FVector2D(0, 1));
				uvs1.Add(FVector2D(1, 1));
				uvs1.Add(FVector2D(0, 0));
				uvs1.Add(FVector2D(1, 0));
			}
		}
	}
}

void UIGeometry::UpdateUITextVertexOrUV(FString& content, float& width, float& height, const FVector2D& pivot, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, int fontSize, uint8 paragraphHAlign, uint8 paragraphVAlign, uint8 overflowType, bool adjustWidth, bool adjustHeight, uint8 fontStyle, FVector2D& textRealSize, float dynamicPixelsPerUnit, bool updateVertex, bool updateUV, TArray<FUITextLineProperty>& cacheTextPropertyList, const TArray<FUITextCharGeometry>& cacheTextGeometryList)
{
	cacheTextPropertyList.Reset();
	int contentLength = content.Len();
	FVector2D currentLineOffset(0, 0);
	float currentLineWidth = 0, paragraphHeight = 0;//single line width, all line height
	TArray<FVector*> currentLineUIGeoVertexList;//single line's vertex collection
	int visibleCharIndex = 0;//visible char index, skip invisible char(\n,\t)
	auto& vertices = uiGeo->vertices;
	auto& uvs = uiGeo->uvs;
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
				uvs[index] = charGeo.uv0;
				uvs[index + 1] = charGeo.uv1;
				uvs[index + 2] = charGeo.uv2;
				uvs[index + 3] = charGeo.uv3;
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
		textRealSize.X = currentLineWidth;
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
			textRealSize.X = currentLineWidth;
		textRealSize.Y = paragraphHeight;
		if (adjustHeight)
		{
			height = textRealSize.Y;
		}
	}
	break;
	case 2://clamp content
	{
		textRealSize.X = currentLineWidth;
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
		UIGeometry::OffsetVertices(uiGeo->vertices, xOffset, yOffset);
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
	UpdateUISectorVertex(uiGeo, width, height, pivot, startAngle, endAngle, segment);
	//uvs
	UpdateUISectorUV(uiGeo, uvType, startAngle, endAngle, segment, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	int vertexCount = 2 + segment + 1;
	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				uvs1.Add(FVector2D(0, 1));
			}
		}
	}
}
void UIGeometry::UpdateUISectorUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, const FLGUISpriteInfo* spriteInfo)
{
	auto& uvs = uiGeo->uvs;
	int vertexCount = uiGeo->originVerticesCount;
	if (uvs.Num() == 0)
	{
		uvs.AddUninitialized(vertexCount);
	}
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
		uvs[0] = FVector2D(x, y);

		for (int i = 0; i < segment + 2; i++)
		{
			sin = FMath::Sin(angle);
			cos = FMath::Cos(angle);
			x = cos * halfUVWidth + centerUVX;
			y = sin * halfUVHeight + centerUVY;
			uvs[i + 1] = FVector2D(x, y);
			angle += singleAngle;
		}
	}
	else if(uvType == 1)
	{
		uvs[0] = FVector2D(spriteInfo->uv0X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		FVector2D otherUV(spriteInfo->uv3X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		for (int i = 1; i < vertexCount; i++)
		{
			uvs[i] = otherUV;
		}
	}
	else if (uvType == 2)
	{
		uvs[0] = FVector2D((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv0Y);
		FVector2D otherUV((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv3Y);
		for (int i = 1; i < vertexCount; i++)
		{
			uvs[i] = otherUV;
		}
	}
	else if (uvType == 3)
	{
		uvs[0] = FVector2D(spriteInfo->uv0X, (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f);
		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo->uv3X, spriteInfo->uv0Y);
		for (int i = 1; i < vertexCount; i++)
		{
			uvs[i] = otherUV;
			otherUV.Y += uvYInterval;
		}
	}
	else if (uvType == 4)
	{
		uvs[0] = FVector2D((spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f, spriteInfo->uv0Y);
		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (vertexCount - 1);
		FVector2D otherUV(spriteInfo->uv0X, spriteInfo->uv3Y);
		for (int i = 1; i < vertexCount; i++)
		{
			uvs[i] = otherUV;
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
	auto& vertices = uiGeo->vertices;
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
	UpdateUIRingVertex(uiGeo, width, height, pivot, startAngle, endAngle, segment, ringWidth);
	//uvs
	UpdateUIRingUV(uiGeo, uvType, startAngle, endAngle, segment, ringWidth, width, height, spriteInfo);
	//colors
	UpdateUIColor(uiGeo, color);

	int vertexCount = 2 + (segment + 1) * 2;
	//normals
	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
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
		auto& tangents = uiGeo->tangents;
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
		auto& uvs1 = uiGeo->uvs1;
		if (uvs1.Num() == 0)
		{
			uvs1.Reserve(vertexCount);
			for (int i = 0; i < vertexCount; i++)
			{
				uvs1.Add(FVector2D(0, 1));
			}
		}
	}
}
void UIGeometry::UpdateUIRingUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, float ringWidth, float& width, float& height, const FLGUISpriteInfo* spriteInfo)
{
	auto& uvs = uiGeo->uvs;
	int vertexCount = uiGeo->originVerticesCount;
	if (uvs.Num() == 0)
	{
		uvs.AddUninitialized(vertexCount);
	}
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
		uvs[0] = FVector2D(x, y);
		x = cos * halfUVWidth + centerUVX;
		y = sin * halfUVHeight + centerUVY;
		uvs[1] = FVector2D(x, y);

		for (int i = 0; i < segment + 1; i++)
		{
			angle += singleAngle;
			sin = FMath::Sin(angle);
			cos = FMath::Cos(angle);
			x = cos * halfUVWidth * minHalfWRatio + centerUVX;
			y = sin * halfUVHeight * minHalfHRatio + centerUVY;
			uvs[i * 2 + 2] = FVector2D(x, y);
			x = cos * halfUVWidth + centerUVX;
			y = sin * halfUVHeight + centerUVY;
			uvs[i * 2 + 3] = FVector2D(x, y);
		}
	}
	else if (uvType == 1)
	{
		float centerUVX = (spriteInfo->uv0X + spriteInfo->uv3X) * 0.5f;

		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (segment + 1);
		float uvY = spriteInfo->uv0Y;
		uvs[0] = FVector2D(centerUVX, uvY);
		uvs[1] = FVector2D(centerUVX, uvY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvY += uvYInterval;
			uvs[i * 2 + 2] = FVector2D(centerUVX, uvY);
			uvs[i * 2 + 3] = FVector2D(centerUVX, uvY);
		}
	}
	else if (uvType == 2)
	{
		float centerUVY = (spriteInfo->uv0Y + spriteInfo->uv3Y) * 0.5f;

		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (segment + 1);
		float uvX = spriteInfo->uv0X;
		uvs[0] = FVector2D(uvX, centerUVY);
		uvs[1] = FVector2D(uvX, centerUVY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvX += uvXInterval;
			uvs[i * 2 + 2] = FVector2D(uvX, centerUVY);
			uvs[i * 2 + 3] = FVector2D(uvX, centerUVY);
		}
	}
	else if (uvType == 3)
	{
		float innerUVX = spriteInfo->uv0X;
		float outerUVX = spriteInfo->uv3X;

		float uvYInterval = (spriteInfo->uv3Y - spriteInfo->uv0Y) / (segment + 1);
		float uvY = spriteInfo->uv0Y;

		uvs[0] = FVector2D(innerUVX, uvY);
		uvs[1] = FVector2D(outerUVX, uvY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvY += uvYInterval;
			uvs[i * 2 + 2] = FVector2D(innerUVX, uvY);
			uvs[i * 2 + 3] = FVector2D(outerUVX, uvY);
		}
	}
	else if (uvType == 4)
	{
		float innerUVY = spriteInfo->uv0Y;
		float outerUVY = spriteInfo->uv3Y;

		float uvXInterval = (spriteInfo->uv3X - spriteInfo->uv0X) / (segment + 1);
		float uvX = spriteInfo->uv0X;

		uvs[0] = FVector2D(uvX, innerUVY);
		uvs[1] = FVector2D(uvX, outerUVY);
		for (int i = 0; i < segment + 1; i++)
		{
			uvX += uvXInterval;
			uvs[i * 2 + 2] = FVector2D(uvX, innerUVY);
			uvs[i * 2 + 3] = FVector2D(uvX, outerUVY);
		}
	}
}
void UIGeometry::UpdateUIRingVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, float ringWidth)
{
	//pivot offset
	float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY, halfW, halfH);
	//vertices
	auto& vertices = uiGeo->vertices;
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
	auto& colors = uiGeo->colors;
	auto count = colors.Num();
	if (count == 0)
	{
		for (int i = 0; i < uiGeo->originVerticesCount; i++)
		{
			colors.Add(color);
		}
	}
	else
	{
		for (int i = 0; i < uiGeo->originVerticesCount; i++)
		{
			colors[i] = color;
		}
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
	auto& transformedVertices = uiGeo->transformedVertices;
	auto vertexCount = vertices.Num();
	auto transformedVertexCount = transformedVertices.Num();
	if (transformedVertexCount > vertexCount)
	{
		transformedVertices.RemoveAt(vertexCount, transformedVertexCount - vertexCount);
	}
	else if (transformedVertexCount < vertexCount)
	{
		transformedVertices.AddDefaulted(vertexCount - transformedVertexCount);
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
				tempV3 = itemToCanvasTf.TransformPosition(vertices[i]);
				tempV3.X -= canvasLeftBottom.X;
				tempV3.Y -= canvasLeftBottom.Y;
				tempV3.X = RoundToFloat(tempV3.X);
				tempV3.Y = RoundToFloat(tempV3.Y);
				tempV3.X += canvasLeftBottom.X;
				tempV3.Y += canvasLeftBottom.Y;

				transformedVertices[i] = tempV3;
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
				tempV3 = vertToRootCanvasTf.TransformPosition(vertices[i]);
				tempV3.X -= canvasLeftBottom.X;
				tempV3.Y -= canvasLeftBottom.Y;
				tempV3.X = RoundToFloat(tempV3.X);
				tempV3.Y = RoundToFloat(tempV3.Y);
				tempV3.X += canvasLeftBottom.X;
				tempV3.Y += canvasLeftBottom.Y;

				tempV3 = vertToCanvas.TransformPosition(tempV3);

				transformedVertices[i] = tempV3;
			}
		}
	}
	else
	{
		for (int i = 0; i < vertexCount; i++)
		{
			tempV3 = itemToCanvasTf.TransformPosition(vertices[i]);
			transformedVertices[i] = tempV3;
		}
	}

	if (requireNormal)
	{
		auto& normals = uiGeo->normals;
		auto& transformedNormals = uiGeo->transformedNormals;
		auto transformedNormalCount = transformedNormals.Num();
		if (transformedNormalCount < vertexCount)
		{
			transformedNormals.AddDefaulted(vertexCount - transformedNormalCount);
		}
		else if (transformedNormalCount > vertexCount)
		{
			transformedNormals.RemoveAt(vertexCount, transformedNormalCount - vertexCount);
		}

		for (int i = 0; i < vertexCount; i++)
		{
			transformedNormals[i] = itemToCanvasTf.TransformVector(normals[i]);
		}
	}
	else
	{
		uiGeo->transformedNormals = uiGeo->normals;
	}
	if (requireTangent)
	{
		auto& tangents = uiGeo->tangents;
		auto& transformedTangents = uiGeo->transformedTangents;
		auto transformedTangentCount = transformedTangents.Num();;
		if (transformedTangentCount < vertexCount)
		{
			transformedTangents.AddDefaulted(vertexCount - transformedTangentCount);
		}
		else if (transformedTangentCount > vertexCount)
		{
			transformedTangents.RemoveAt(vertexCount, transformedTangentCount - vertexCount);
		}

		for (int i = 0; i < vertexCount; i++)
		{
			transformedTangents[i] = itemToCanvasTf.TransformVector(tangents[i]);
		}
	}
	else
	{
		uiGeo->transformedTangents = uiGeo->tangents;
	}
}

void UIGeometry::CheckAndApplyAdditionalChannel(TSharedPtr<UIGeometry> uiGeo)
{
	auto vertCount = uiGeo->vertices.Num();

	auto& normals = uiGeo->normals;
	auto normalCount = normals.Num();
	if (normalCount < vertCount)
	{
		while (normalCount < vertCount)
		{
			normals.Add(FVector(0, 0, 0));
			normalCount++;
		}
	}
	else if (normalCount > vertCount)
	{
		normals.RemoveAt(vertCount, normalCount - vertCount);
	}

	auto& tangents = uiGeo->tangents;
	auto tangentCount = tangents.Num();
	if (tangentCount < vertCount)
	{
		while (tangentCount < vertCount)
		{
			tangents.Add(FVector(0, 0, 0));
			tangentCount++;
		}
	}
	else if (tangentCount > vertCount)
	{
		tangents.RemoveAt(vertCount, tangentCount - vertCount);
	}

	auto& uvs1 = uiGeo->uvs1;
	auto uv1Count = uvs1.Num();
	if (uv1Count < vertCount)
	{
		while (uv1Count < vertCount)
		{
			uvs1.Add(FVector2D(0, 0));
			uv1Count++;
		}
	}
	else if (uv1Count > vertCount)
	{
		uvs1.RemoveAt(vertCount, uv1Count - vertCount);
	}

	auto& uvs2 = uiGeo->uvs2;
	auto uv2Count = uvs2.Num();
	if (uv2Count < vertCount)
	{
		while (uv2Count < vertCount)
		{
			uvs2.Add(FVector2D(0, 0));
			uv2Count++;
		}
	}
	else if (uv2Count > vertCount)
	{
		uvs2.RemoveAt(vertCount, uv2Count - vertCount);
	}

	auto& uvs3 = uiGeo->uvs3;
	auto uv3Count = uvs3.Num();
	if (uv3Count < vertCount)
	{
		while (uv3Count < vertCount)
		{
			uvs3.Add(FVector2D(0, 0));
			uv3Count++;
		}
	}
	else if (uv3Count > vertCount)
	{
		uvs3.RemoveAt(vertCount, uv3Count - vertCount);
	}
}