// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIGeometry.h"
#include "LGUI.h"
#include "Core/Components/LexSprite.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexVisual.h"
#include "Core/LexUISpriteData.h"
#include "Core/LexUIFontData_BaseObject.h"
#include "Core/LexUIRichTextImageData_BaseObject.h"
#include "Core/FRichTextParser.h"
#include "Core/LexUIFontEmojiData.h"
#include "Core/Components/LexWidget.h"


FORCEINLINE float RoundToFloat(float value)
{
	return FMath::FloorToFloat(value + 0.5f);
}

DECLARE_CYCLE_STAT(TEXT("UIGeometry TransformPixelPerfectVertices"), STAT_TransformPixelPerfectVertices, STATGROUP_LGUI);

void FLexUIGeometry::AdjustPixelPerfectPos(TArray<FLexUIOriginVertexData>& originVertices, int startIndex, int count, ULexCanvas* RenderCanvas, ULexVisual* Visual)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto CanvasWidget = RenderCanvas->GetRootCanvas()->GetWidget();
	auto ComponentToCanvasTransform = Visual->GetWidget()->GetWorldTransform() * CanvasWidget->GetWorldTransform().Inverse();
	if (!ULexCanvas::Is2DUITransform(ComponentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = ComponentToCanvasTransform.Inverse();

	auto halfCanvasWidth = CanvasWidget->GetWidth() * 0.5f;
	auto halfCanvasHeight = CanvasWidget->GetHeight() * 0.5f;
	float rootCanvasScale = RenderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	for (int i = startIndex; i < count; i++)
	{
		auto item = originVertices[i].Position;

		auto canvasSpaceLocation = ComponentToCanvasTransform.TransformPosition(FVector(item));
		canvasSpaceLocation.Y -= halfCanvasWidth;
		canvasSpaceLocation.Z -= halfCanvasHeight;
		float screenSpaceLocationY = canvasSpaceLocation.Y * rootCanvasScale;
		float screenSpaceLocationZ = canvasSpaceLocation.Z * rootCanvasScale;
		item.Y = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
		item.Z = RoundToFloat(screenSpaceLocationZ) * inv_RootCanvasScale;
		item.Y += halfCanvasWidth;
		item.Z += halfCanvasHeight;

		originVertices[i].Position = FVector3f(canvasToComponentTransform.TransformPosition(FVector(item)));
	}
}
void AdjustPixelPerfectPos_For_UIRectFillRadial360(TArray<FLexUIOriginVertexData>& originVertices, ULexCanvas* RenderCanvas, ULexVisual* Visual)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto CanvasWidget = RenderCanvas->GetRootCanvas()->GetWidget();
	auto ComponentToCanvasTransform = Visual->GetWidget()->GetWorldTransform() * CanvasWidget->GetWorldTransform().Inverse();
	if (!ULexCanvas::Is2DUITransform(ComponentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = ComponentToCanvasTransform.Inverse();

	auto halfCanvasWidth = CanvasWidget->GetWidth() * 0.5f;
	auto halfCanvasHeight = CanvasWidget->GetHeight() * 0.5f;
	float rootCanvasScale = RenderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	static TArray<int> vertArray = { 0, 2, 6, 8 };
	for (int i = 0; i < vertArray.Num(); i++)
	{
		int vertIndex = vertArray[i];
		auto originPos = originVertices[vertIndex].Position;

		auto canvasSpaceLocation = ComponentToCanvasTransform.TransformPosition(FVector(originPos));
		canvasSpaceLocation.Y -= halfCanvasWidth;
		canvasSpaceLocation.Z -= halfCanvasHeight;
		float screenSpaceLocationY = canvasSpaceLocation.Y * rootCanvasScale;
		float screenSpaceLocationZ = canvasSpaceLocation.Z * rootCanvasScale;
		canvasSpaceLocation.Y = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
		canvasSpaceLocation.Z = RoundToFloat(screenSpaceLocationZ) * inv_RootCanvasScale;
		canvasSpaceLocation.Y += halfCanvasWidth;
		canvasSpaceLocation.Z += halfCanvasHeight;

		originVertices[vertIndex].Position = FVector3f(canvasToComponentTransform.TransformPosition(canvasSpaceLocation));
	}
}
void AdjustPixelPerfectPos_For_UIText(TArray<FLexUIOriginVertexData>& originVertices, const TArray<FLexUITextCharProperty>& cacheCharPropertyArray, ULexCanvas* RenderCanvas, ULexVisual* Visual)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	if (cacheCharPropertyArray.Num() <= 0)return;

	auto CanvasWidget = RenderCanvas->GetRootCanvas()->GetWidget();
	auto ComponentToCanvasTransform = Visual->GetWidget()->GetWorldTransform() * CanvasWidget->GetWorldTransform().Inverse();
	if (!ULexCanvas::Is2DUITransform(ComponentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = ComponentToCanvasTransform.Inverse();

	auto halfCanvasWidth = CanvasWidget->GetWidth() * 0.5f;
	auto halfCanvasHeight = CanvasWidget->GetHeight() * 0.5f;
	float rootCanvasScale = RenderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	for (int i = 0; i < cacheCharPropertyArray.Num(); i++)
	{
		auto charProperty = cacheCharPropertyArray[i];
		int vertStartIndex = charProperty.StartVertIndex;
		int vertEndIndex = charProperty.StartVertIndex + charProperty.VertCount;

		//calculate first vert
		float offsetY, offsetZ;
		{
			auto originPos = originVertices[vertStartIndex].Position;

			auto canvasSpaceLocation = ComponentToCanvasTransform.TransformPosition(FVector(originPos));
			canvasSpaceLocation.Y -= halfCanvasWidth;
			canvasSpaceLocation.Z -= halfCanvasHeight;
			float screenSpaceLocationX = canvasSpaceLocation.Y * rootCanvasScale;
			float screenSpaceLocationY = canvasSpaceLocation.Z * rootCanvasScale;
			canvasSpaceLocation.Y = RoundToFloat(screenSpaceLocationX) * inv_RootCanvasScale;
			canvasSpaceLocation.Z = RoundToFloat(screenSpaceLocationY) * inv_RootCanvasScale;
			canvasSpaceLocation.Y += halfCanvasWidth;
			canvasSpaceLocation.Z += halfCanvasHeight;

			auto newPos = canvasToComponentTransform.TransformPosition(canvasSpaceLocation);
			originVertices[vertStartIndex].Position = FVector3f(newPos);
			offsetY = newPos.Y - originPos.Y;
			offsetZ = newPos.Z - originPos.Z;
		}

		for (int vertIndex = vertStartIndex + 1; vertIndex < vertEndIndex; vertIndex++)
		{
			auto& originPos = originVertices[vertIndex].Position;
			originPos.Y += offsetY;
			originPos.Z += offsetZ;
		}
	}
}

#pragma region UISprite_UITexture_Simple
void FLexUIGeometry::UpdateUIRectSimpleVertex(FLexUIGeometry* uiGeo,
	float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	LexUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	LexUIGeometrySetArrayNum(vertices, 4);
	LexUIGeometrySetArrayNum(originVertices, 4);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			//offset and size
			float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
			CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
			//positions
			float minX = -halfW + pivotOffsetX;
			float minY = -halfH + pivotOffsetY;
			float maxX = halfW + pivotOffsetX;
			float maxY = halfH + pivotOffsetY;
			originVertices[0].Position = FVector3f(0, minX, minY);
			originVertices[1].Position = FVector3f(0, maxX, minY);
			originVertices[2].Position = FVector3f(0, minX, maxY);
			originVertices[3].Position = FVector3f(0, maxX, maxY);
			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos(originVertices, 0, 4, renderCanvas, uiComp);
			}
		}

		if (InVertexUVChanged)
		{
			vertices[0].TextureCoordinate[0] = spriteInfo.GetUV0();
			vertices[1].TextureCoordinate[0] = spriteInfo.GetUV1();
			vertices[2].TextureCoordinate[0] = spriteInfo.GetUV2();
			vertices[3].TextureCoordinate[0] = spriteInfo.GetUV3();
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for(int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
void FLexUIGeometry::UpdateRectBlockVertex(FLexUIGeometry* uiGeo,
	bool bEnableOuterShadow, FVector2f outerShadowOffset, float outerShadowSize, float outerShadowBlur, bool bSoftEdge,
	float width, float height, FVector2f pivot, 
	const FLexUISpriteInfo& uniformSpriteInfo, const FLexUISpriteInfo& spriteInfo,
	ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	LexUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	LexUIGeometrySetArrayNum(vertices, 4);
	LexUIGeometrySetArrayNum(originVertices, 4);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		//offset and size
		float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
		CalculateOffsetAndSize(width, height, pivot, uniformSpriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
		//positions
		float minX = -halfW + pivotOffsetX;
		float minY = -halfH + pivotOffsetY;
		float maxX = halfW + pivotOffsetX;
		float maxY = halfH + pivotOffsetY;
		
		if (InVertexPositionChanged)
		{
			if (bEnableOuterShadow)
			{
				float shadowMinX = minX + outerShadowOffset.X;
				float shadowMinY = minY + outerShadowOffset.Y;
				float shadowMaxX = maxX + outerShadowOffset.X;
				float shadowMaxY = maxY + outerShadowOffset.Y;
				float additionalShadowSize = outerShadowSize + outerShadowBlur * 0.5f;
				shadowMinX -= additionalShadowSize;
				shadowMaxX += additionalShadowSize;
				shadowMinY -= additionalShadowSize;
				shadowMaxY += additionalShadowSize;
				float PosMinX = FMath::Min(minX, shadowMinX);
				float PosMaxX = FMath::Max(maxX, shadowMaxX);
				float PosMinY = FMath::Min(minY, shadowMinY);
				float PosMaxY = FMath::Max(maxY, shadowMaxY);
				if (bSoftEdge)//offset 1 pixel to make edge smooth
				{
					PosMinX -= 1;
					PosMaxX += 1;
					PosMinY -= 1;
					PosMaxY += 1;
				}
				originVertices[0].Position = FVector3f(0, PosMinX, PosMinY);
				originVertices[1].Position = FVector3f(0, PosMaxX, PosMinY);
				originVertices[2].Position = FVector3f(0, PosMinX, PosMaxY);
				originVertices[3].Position = FVector3f(0, PosMaxX, PosMaxY);
			}
			else
			{
				float PosMinX = minX;
				float PosMaxX = maxX;
				float PosMinY = minY;
				float PosMaxY = maxY;
				if (bSoftEdge)//offset 1 pixel to make edge smooth
				{
					PosMinX -= 1;
					PosMaxX += 1;
					PosMinY -= 1;
					PosMaxY += 1;
				}
				originVertices[0].Position = FVector3f(0, PosMinX, PosMinY);
				originVertices[1].Position = FVector3f(0, PosMaxX, PosMinY);
				originVertices[2].Position = FVector3f(0, PosMinX, PosMaxY);
				originVertices[3].Position = FVector3f(0, PosMaxX, PosMaxY);
			}
			//snap pixel
			if (pixelPerfect)
			{
				int startIndex = 0;
				AdjustPixelPerfectPos(originVertices, startIndex, startIndex + 4, renderCanvas, uiComp);
			}
		}

		if (InVertexUVChanged || bSoftEdge)
		{
			auto& OriginVert0 = originVertices[0];
			auto& OriginVert1 = originVertices[1];
			auto& OriginVert2 = originVertices[2];
			auto& OriginVert3 = originVertices[3];
			auto& Vert0 = vertices[0];
			auto& Vert1 = vertices[1];
			auto& Vert2 = vertices[2];
			auto& Vert3 = vertices[3];
			
			float oneDivideWidth = 1.0f / width;
			float oneDivideHeight = 1.0f / height;
			
			Vert0.TextureCoordinate[0] = uniformSpriteInfo.GetUV0() + FVector2f((OriginVert0.Position.Y - minX) * oneDivideWidth, -(OriginVert0.Position.Z - minY) * oneDivideHeight);
			Vert1.TextureCoordinate[0] = uniformSpriteInfo.GetUV1() + FVector2f((OriginVert1.Position.Y - maxX) * oneDivideWidth, -(OriginVert1.Position.Z - minY) * oneDivideHeight);
			Vert2.TextureCoordinate[0] = uniformSpriteInfo.GetUV2() + FVector2f((OriginVert2.Position.Y - minX) * oneDivideWidth, -(OriginVert2.Position.Z - maxY) * oneDivideHeight);
			Vert3.TextureCoordinate[0] = uniformSpriteInfo.GetUV3() + FVector2f((OriginVert3.Position.Y - maxX) * oneDivideWidth, -(OriginVert3.Position.Z - maxY) * oneDivideHeight);
			
			//uv2 store the info for sampling texture and Sprite
			Vert0.TextureCoordinate[2] = spriteInfo.GetUV0();
			Vert1.TextureCoordinate[2] = spriteInfo.GetUV1();
			Vert2.TextureCoordinate[2] = spriteInfo.GetUV2();
			Vert3.TextureCoordinate[2] = spriteInfo.GetUV3();
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}
	}
}
#pragma endregion
#pragma region UISprite_UITexture_Border
void FLexUIGeometry::UpdateUIRectBorderVertex(FLexUIGeometry* uiGeo, bool fillCenter,
	float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	float pixelsPerUnitMultiplier,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	int triangleCount;
	if (fillCenter)
	{
		triangleCount = 54;
	}
	else
	{
		triangleCount = 48;
	}
	LexUIGeometrySetArrayNum(triangles, triangleCount);
	if (InTriangleChanged)
	{
		int wSeg = 3, hSeg = 3;
		int vStartIndex = 0;
		int triangleArrayIndex = 0;
		for (int h = 0; h < hSeg; h++)
		{
			for (int w = 0; w < wSeg; w++)
			{
				if (!fillCenter)
					if (h == 1 && w == 1)continue;
				int vIndex = vStartIndex + w;
				triangles[triangleArrayIndex++] = vIndex;
				triangles[triangleArrayIndex++] = vIndex + wSeg + 2;
				triangles[triangleArrayIndex++] = vIndex + wSeg + 1;

				triangles[triangleArrayIndex++] = vIndex;
				triangles[triangleArrayIndex++] = vIndex + 1;
				triangles[triangleArrayIndex++] = vIndex + wSeg + 2;
			}
			vStartIndex += wSeg + 1;
		}
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 16;
	LexUIGeometrySetArrayNum(vertices, verticesCount);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			//pivot offset
			float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
			CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
			float geoWidth = halfW * 2;
			float geoHeight = halfH * 2;
			//vertices
			float x0, x1, x2, x3, y0, y1, y2, y3;
			float widthBorder = (spriteInfo.Border.Left + spriteInfo.Border.Right) * pixelsPerUnitMultiplier;
			float heightBorder = (spriteInfo.Border.Top + spriteInfo.Border.Bottom) * pixelsPerUnitMultiplier;
			float widthScale = geoWidth < widthBorder ? geoWidth / widthBorder : 1.0f;
			float heightScale = geoHeight < heightBorder ? geoHeight / heightBorder : 1.0f;
			x0 = (-halfW + pivotOffsetX);
			x1 = (x0 + spriteInfo.Border.Left * widthScale * pixelsPerUnitMultiplier);
			x3 = (halfW + pivotOffsetX);
			x2 = (x3 - spriteInfo.Border.Right * widthScale * pixelsPerUnitMultiplier);
			y0 = (-halfH + pivotOffsetY);
			y1 = (y0 + spriteInfo.Border.Bottom * heightScale * pixelsPerUnitMultiplier);
			y3 = (halfH + pivotOffsetY);
			y2 = (y3 - spriteInfo.Border.Top * heightScale * pixelsPerUnitMultiplier);

			originVertices[0].Position = FVector3f(0, x0, y0);
			originVertices[1].Position = FVector3f(0, x1, y0);
			originVertices[2].Position = FVector3f(0, x2, y0);
			originVertices[3].Position = FVector3f(0, x3, y0);

			originVertices[4].Position = FVector3f(0, x0, y1);
			originVertices[5].Position = FVector3f(0, x1, y1);
			originVertices[6].Position = FVector3f(0, x2, y1);
			originVertices[7].Position = FVector3f(0, x3, y1);

			originVertices[8].Position = FVector3f(0, x0, y2);
			originVertices[9].Position = FVector3f(0, x1, y2);
			originVertices[10].Position = FVector3f(0, x2, y2);
			originVertices[11].Position = FVector3f(0, x3, y2);

			originVertices[12].Position = FVector3f(0, x0, y3);
			originVertices[13].Position = FVector3f(0, x1, y3);
			originVertices[14].Position = FVector3f(0, x2, y3);
			originVertices[15].Position = FVector3f(0, x3, y3);

			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos(originVertices, 0, verticesCount, renderCanvas, uiComp);
			}
		}

		if (InVertexUVChanged)
		{
			vertices[0].TextureCoordinate[0] = FVector2f(spriteInfo.MinUV.X, spriteInfo.MaxUV.Y);
			vertices[1].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, spriteInfo.MaxUV.Y);
			vertices[2].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMaxUV.X, spriteInfo.MaxUV.Y);
			vertices[3].TextureCoordinate[0] = FVector2f(spriteInfo.MaxUV.X, spriteInfo.MaxUV.Y);

			vertices[4].TextureCoordinate[0] = FVector2f(spriteInfo.MinUV.X, spriteInfo.BorderMaxUV.Y);
			vertices[5].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, spriteInfo.BorderMaxUV.Y);
			vertices[6].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMaxUV.X, spriteInfo.BorderMaxUV.Y);
			vertices[7].TextureCoordinate[0] = FVector2f(spriteInfo.MaxUV.X, spriteInfo.BorderMaxUV.Y);

			vertices[8].TextureCoordinate[0] = FVector2f(spriteInfo.MinUV.X, spriteInfo.BorderMinUV.Y);
			vertices[9].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, spriteInfo.BorderMinUV.Y);
			vertices[10].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMaxUV.X, spriteInfo.BorderMinUV.Y);
			vertices[11].TextureCoordinate[0] = FVector2f(spriteInfo.MaxUV.X, spriteInfo.BorderMinUV.Y);

			vertices[12].TextureCoordinate[0] = FVector2f(spriteInfo.MinUV.X, spriteInfo.MinUV.Y);
			vertices[13].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, spriteInfo.MinUV.Y);
			vertices[14].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMaxUV.X, spriteInfo.MinUV.Y);
			vertices[15].TextureCoordinate[0] = FVector2f(spriteInfo.MaxUV.X, spriteInfo.MinUV.Y);
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion

#pragma region UISprite_Tiled
void FLexUIGeometry::UpdateUIRectTiledVertex(FLexUIGeometry* uiGeo,
	const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, float width, float height, FVector2f pivot, const int& widthRectCount, const int& heightRectCount, float widthRemainedRectSize, float heightRemainedRectSize, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	int rectangleCount = widthRectCount * heightRectCount;
	auto& triangles = uiGeo->Triangles;
	auto triangleCount = 6 * rectangleCount;
	LexUIGeometrySetArrayNum(triangles, triangleCount);
	if (InTriangleChanged)
	{
		for (int i = 0, j = 0, triangleIndicesIndex = 0; i < rectangleCount; i++, j += 4)
		{
			triangles[triangleIndicesIndex++] = j;
			triangles[triangleIndicesIndex++] = j + 3;
			triangles[triangleIndicesIndex++] = j + 2;
			triangles[triangleIndicesIndex++] = j;
			triangles[triangleIndicesIndex++] = j + 1;
			triangles[triangleIndicesIndex++] = j + 3;
		}
	}
	
	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 4 * rectangleCount;
	LexUIGeometrySetArrayNum(vertices, verticesCount);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			//pivot offset
			float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
			CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
			//vertices
			int vertIndex = 0;
			float startX = (-halfW + pivotOffsetX);
			float startY = (-halfH + pivotOffsetY);
			float x = startX, y = startY;
			for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
			{
				float realHeight = heightRectIndex == heightRectCount ? heightRemainedRectSize : spriteInfo.Height;
				for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
				{
					float realWidth = widthRectIndex == widthRectCount ? (widthRemainedRectSize) : spriteInfo.Width;
					originVertices[vertIndex++].Position = FVector3f(0, x, y);
					originVertices[vertIndex++].Position = FVector3f(0, x + realWidth, y);
					originVertices[vertIndex++].Position = FVector3f(0, x, y + realHeight);
					originVertices[vertIndex++].Position = FVector3f(0, x + realWidth, y + realHeight);

					x += spriteInfo.Width;
				}
				x = startX;
				y += spriteInfo.Height;
			}
			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos(originVertices, 0, verticesCount, renderCanvas, uiComp);
			}
		}

		if (InVertexUVChanged)
		{
			int vertIndex = 0;
			float remainedUV3X = spriteInfo.BorderMinUV.X + (spriteInfo.BorderMaxUV.X - spriteInfo.BorderMinUV.X) * widthRemainedRectSize / spriteInfo.Width;
			float remainedUV3Y = spriteInfo.BorderMaxUV.Y + (spriteInfo.BorderMinUV.Y - spriteInfo.BorderMaxUV.Y) * heightRemainedRectSize / spriteInfo.Height;
			for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
			{
				float realUV3Y = heightRectIndex == heightRectCount ? remainedUV3Y : spriteInfo.BorderMaxUV.Y;
				for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
				{
					float realUV3X = widthRectIndex == widthRectCount ? remainedUV3X : spriteInfo.BorderMaxUV.X;
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, spriteInfo.BorderMaxUV.Y);
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(realUV3X, spriteInfo.BorderMaxUV.Y);
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(spriteInfo.BorderMinUV.X, realUV3Y);
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(realUV3X, realUV3Y);
				}
			}
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion

#pragma region UISprite_Fill_Horizontal_Vertial
void FLexUIGeometry::UpdateUIRectFillHorizontalVerticalVertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
	, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
	, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	auto triangleCount = 6;
	LexUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 4;
	LexUIGeometrySetArrayNum(vertices, 4);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged || InVertexUVChanged)
		{
			//pivot offset
			float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
			CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
			//positions
			float posMinX = -halfW + pivotOffsetX;
			float posMinY = -halfH + pivotOffsetY;
			float posMaxX = halfW + pivotOffsetX;
			float posMaxY = halfH + pivotOffsetY;
			//uvs
			float uvMinX = spriteInfo.MinUV.X;
			float uvMinY = spriteInfo.MaxUV.Y;
			float uvMaxX = spriteInfo.MaxUV.X;
			float uvMaxY = spriteInfo.MinUV.Y;

			if (InVertexPositionChanged)
			{
				originVertices[0].Position = FVector3f(0, posMinX, posMinY);
				originVertices[1].Position = FVector3f(0, posMaxX, posMinY);
				originVertices[2].Position = FVector3f(0, posMinX, posMaxY);
				originVertices[3].Position = FVector3f(0, posMaxX, posMaxY);

				//snap pixel
				if (pixelPerfect)
				{
					AdjustPixelPerfectPos(originVertices, 0, verticesCount, renderCanvas, uiComp);

					posMinX = originVertices[0].Position.Y;
					posMinY = originVertices[0].Position.Z;
					posMaxX = originVertices[3].Position.Y;
					posMaxY = originVertices[3].Position.Z;
				}
			}
			if (horizontalOrVertical)
			{
				if (flipDirection)
				{
					if (InVertexPositionChanged)
					{
						float value = FMath::Lerp(posMinX, posMaxX, fillAmount);
						originVertices[1].Position.Y = originVertices[3].Position.Y = value;
					}
					if (InVertexUVChanged)
					{
						float value = FMath::Lerp(uvMinX, uvMaxX, fillAmount);
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(value, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(value, uvMaxY);
					}
				}
				else
				{
					if (InVertexPositionChanged)
					{
						float value = FMath::Lerp(posMaxX, posMinX, fillAmount);
						originVertices[0].Position.Y = originVertices[2].Position.Y = value;
					}
					if (InVertexUVChanged)
					{
						float value = FMath::Lerp(uvMaxX, uvMinX, fillAmount);
						vertices[0].TextureCoordinate[0] = FVector2f(value, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(value, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
			}
			else
			{
				if (flipDirection)
				{
					if (InVertexPositionChanged)
					{
						float value = FMath::Lerp(posMinY, posMaxY, fillAmount);
						originVertices[2].Position.Z = originVertices[3].Position.Z = value;
					}
					if (InVertexUVChanged)
					{
						float value = FMath::Lerp(uvMinY, uvMaxY, fillAmount);
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, value);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, value);
					}
				}
				else
				{
					if (InVertexPositionChanged)
					{
						float value = FMath::Lerp(posMaxY, posMinY, fillAmount);
						originVertices[0].Position.Z = originVertices[1].Position.Z = value;
					}
					if (InVertexUVChanged)
					{
						float value = FMath::Lerp(uvMaxY, uvMinY, fillAmount);
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, value);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, value);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
			}
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial90
void FLexUIGeometry::UpdateUIRectFillRadial90Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
	, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial90 originType
	, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	auto triangleCount = 6;
	LexUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 4;
	LexUIGeometrySetArrayNum(vertices, 4);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		//pivot offset
		float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
		CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
		//positions
		float posMinX = -halfW + pivotOffsetX;
		float posMinY = -halfH + pivotOffsetY;
		float posMaxX = halfW + pivotOffsetX;
		float posMaxY = halfH + pivotOffsetY;
		//uvs
		float uvMinX = spriteInfo.MinUV.X;
		float uvMinY = spriteInfo.MaxUV.Y;
		float uvMaxX = spriteInfo.MaxUV.X;
		float uvMaxY = spriteInfo.MinUV.Y;

		if (InVertexPositionChanged)
		{
			originVertices[0].Position = FVector3f(0, posMinX, posMinY);
			originVertices[1].Position = FVector3f(0, posMaxX, posMinY);
			originVertices[2].Position = FVector3f(0, posMinX, posMaxY);
			originVertices[3].Position = FVector3f(0, posMaxX, posMaxY);
			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos(originVertices, 0, verticesCount, renderCanvas, uiComp);

				posMinX = originVertices[0].Position.Y;
				posMinY = originVertices[0].Position.Z;
				posMaxX = originVertices[3].Position.Y;
				posMaxY = originVertices[3].Position.Z;
			}
		}
		switch (originType)
		{
		case ELexUISpriteFillOriginType_Radial90::BottomLeft:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = FVector3f(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[3].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[3].Position = FVector3f(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial90::TopLeft:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[1].Position = FVector3f(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = FVector3f(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					}
				}
				else
				{
					float lerpVaue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[1].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpVaue), posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpVaue), uvMinY);
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial90::TopRight:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = FVector3f(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[0].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[0].Position = FVector3f(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial90::BottomRight:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[2].Position = FVector3f(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
					}
				}
			}
			else
			{
				if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = FVector3f(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 2.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[2].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
					}
				}
			}
		}
		break;
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial180
void FLexUIGeometry::UpdateUIRectFillRadial180Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
	, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial180 originType
	, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	auto triangleCount = 9;
	LexUIGeometrySetArrayNum(triangles, 9);
	if (InTriangleChanged)
	{
		switch (originType)
		{
		case ELexUISpriteFillOriginType_Radial180::Bottom:
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
		case ELexUISpriteFillOriginType_Radial180::Left:
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
		case ELexUISpriteFillOriginType_Radial180::Top:
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
		case ELexUISpriteFillOriginType_Radial180::Right:
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

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 5;
	LexUIGeometrySetArrayNum(vertices, 5);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		//pivot offset
		float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
		CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
		//positions
		float posMinX = -halfW + pivotOffsetX;
		float posMinY = -halfH + pivotOffsetY;
		float posMaxX = halfW + pivotOffsetX;
		float posMaxY = halfH + pivotOffsetY;
		//uvs
		float uvMinX = spriteInfo.MinUV.X;
		float uvMinY = spriteInfo.MaxUV.Y;
		float uvMaxX = spriteInfo.MaxUV.X;
		float uvMaxY = spriteInfo.MinUV.Y;

		if (InVertexPositionChanged)
		{
			originVertices[0].Position = FVector3f(0, posMinX, posMinY);
			originVertices[1].Position = FVector3f(0, posMaxX, posMinY);
			originVertices[2].Position = FVector3f(0, posMinX, posMaxY);
			originVertices[3].Position = FVector3f(0, posMaxX, posMaxY);
			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos(originVertices, 0, verticesCount - 1, renderCanvas, uiComp);

				posMinX = originVertices[0].Position.Y;
				posMinY = originVertices[0].Position.Z;
				posMaxX = originVertices[3].Position.Y;
				posMaxY = originVertices[3].Position.Z;
			}
		}
		switch (originType)
		{
		case ELexUISpriteFillOriginType_Radial180::Bottom:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = FVector3f(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[3].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[3].Position = originVertices[2].Position = FVector3f(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = FVector3f(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[2].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[2].Position = originVertices[3].Position = FVector3f(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMinY);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMinY);
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial180::Left:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[1].Position = FVector3f(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[0].Position = originVertices[1].Position = originVertices[3].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[3].Position = FVector3f(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[3].Position = originVertices[1].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, posMinX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[2].TextureCoordinate[0] = vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMinX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial180::Top:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = FVector3f(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[0].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[2].Position = originVertices[0].Position = originVertices[1].Position = FVector3f(0, posMaxX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = FVector3f(0, posMaxX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[1].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[1].Position = originVertices[0].Position = FVector3f(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[2].Position = FVector3f(0, posMinX, posMaxY);
						originVertices[4].Position = FVector3f(0, (posMinX + posMaxX) * 0.5f, posMaxY);
					}
					if (InVertexUVChanged)
					{
						vertices[3].TextureCoordinate[0] = vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f((uvMinX + uvMaxX) * 0.5f, uvMaxY);
					}
				}
			}
		}
		break;
		case ELexUISpriteFillOriginType_Radial180::Right:
		{
			if (flipDirection)
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[2].Position = FVector3f(0, posMinX, FMath::Lerp(posMinY, posMaxY, lerpValue));
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMinY, uvMaxY, lerpValue));
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[3].Position = originVertices[2].Position = originVertices[0].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[3].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
			}
			else
			{
				if (fillAmount >= 0.666666666f)
				{
					float lerpValue = (fillAmount - 0.666666666f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = FVector3f(0, FMath::Lerp(posMinX, posMaxX, lerpValue), posMinY);
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
						vertices[1].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMinX, uvMaxX, lerpValue), uvMinY);
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else if (fillAmount >= 0.33333333f)
				{
					float lerpValue = (fillAmount - 0.33333333f) * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[0].Position = FVector3f(0, posMinX, FMath::Lerp(posMaxY, posMinY, lerpValue));
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, FMath::Lerp(uvMaxY, uvMinY, lerpValue));
						vertices[2].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
				else
				{
					float lerpValue = fillAmount * 3.0f;
					if (InVertexPositionChanged)
					{
						originVertices[1].Position = originVertices[0].Position = originVertices[2].Position = FVector3f(0, FMath::Lerp(posMaxX, posMinX, lerpValue), posMaxY);
						originVertices[4].Position = FVector3f(0, posMaxX, (posMinY + posMaxY) * 0.5f);
					}
					if (InVertexUVChanged)
					{
						vertices[1].TextureCoordinate[0] = vertices[0].TextureCoordinate[0] = vertices[2].TextureCoordinate[0] = FVector2f(FMath::Lerp(uvMaxX, uvMinX, lerpValue), uvMaxY);
						vertices[3].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
						vertices[4].TextureCoordinate[0] = FVector2f(uvMaxX, (uvMinY + uvMaxY) * 0.5f);
					}
				}
			}
		}
		break;
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial360
void FLexUIGeometry::UpdateUIRectFillRadial360Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
	, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial360 originType
	, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->Triangles;
	auto triangleCount = 24;
	LexUIGeometrySetArrayNum(triangles, 24);
	if (InTriangleChanged)
	{
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
		case ELexUISpriteFillOriginType_Radial360::Bottom:
			triangles[1] = 9;
			break;
		case ELexUISpriteFillOriginType_Radial360::Right:
			triangles[19] = 9;
			break;
		case ELexUISpriteFillOriginType_Radial360::Top:
			triangles[13] = 9;
			break;
		case ELexUISpriteFillOriginType_Radial360::Left:
			triangles[7] = 9;
			break;
		}
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelSnapping() && uiComp->GetWidget()->GetPixelSnappingInHierarchy();
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto verticesCount = 10;
	LexUIGeometrySetArrayNum(vertices, verticesCount);
	LexUIGeometrySetArrayNum(originVertices, verticesCount);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		//pivot offset
		float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
		CalculateOffsetAndSize(width, height, pivot, spriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
		//positions
		float posMinX = -halfW + pivotOffsetX;
		float posMinY = -halfH + pivotOffsetY;
		float posMaxX = halfW + pivotOffsetX;
		float posMaxY = halfH + pivotOffsetY;
		float posHalfX = (posMinX + posMaxX) * 0.5f;
		float posHalfY = (posMinY + posMaxY) * 0.5f;
		//uvs
		float uvMinX = spriteInfo.MinUV.X;
		float uvMinY = spriteInfo.MaxUV.Y;
		float uvMaxX = spriteInfo.MaxUV.X;
		float uvMaxY = spriteInfo.MinUV.Y;
		float uvHalfX = (uvMinX + uvMaxX) * 0.5f;
		float uvHalfY = (uvMinY + uvMaxY) * 0.5f;

		//reset position
		{
			originVertices[0].Position = FVector3f(0, posMinX, posMinY);
			originVertices[2].Position = FVector3f(0, posMaxX, posMinY);
			originVertices[6].Position = FVector3f(0, posMinX, posMaxY);
			originVertices[8].Position = FVector3f(0, posMaxX, posMaxY);
			//snap pixel
			if (pixelPerfect)
			{
				AdjustPixelPerfectPos_For_UIRectFillRadial360(originVertices, renderCanvas, uiComp);

				posMinX = originVertices[0].Position.Y;
				posMaxX = originVertices[2].Position.Y;
				posMinY = originVertices[0].Position.Z;
				posMaxY = originVertices[6].Position.Z;
				posHalfX = (posMinX + posMaxX) * 0.5f;
				posHalfY = (posMinY + posMaxY) * 0.5f;
			}

			originVertices[1].Position = FVector3f(0, posHalfX, posMinY);
			originVertices[3].Position = FVector3f(0, posMinX, posHalfY);
			originVertices[4].Position = FVector3f(0, posHalfX, posHalfY);
			originVertices[5].Position = FVector3f(0, posMaxX, posHalfY);
			originVertices[7].Position = FVector3f(0, posHalfX, posMaxY);
		}
		//reset uv
		{
			vertices[0].TextureCoordinate[0] = FVector2f(uvMinX, uvMinY);
			vertices[1].TextureCoordinate[0] = FVector2f(uvHalfX, uvMinY);
			vertices[2].TextureCoordinate[0] = FVector2f(uvMaxX, uvMinY);
			vertices[3].TextureCoordinate[0] = FVector2f(uvMinX, uvHalfY);
			vertices[4].TextureCoordinate[0] = FVector2f(uvHalfX, uvHalfY);
			vertices[5].TextureCoordinate[0] = FVector2f(uvMaxX, uvHalfY);
			vertices[6].TextureCoordinate[0] = FVector2f(uvMinX, uvMaxY);
			vertices[7].TextureCoordinate[0] = FVector2f(uvHalfX, uvMaxY);
			vertices[8].TextureCoordinate[0] = FVector2f(uvMaxX, uvMaxY);
		}

		auto setPosAndUv = [&](int changeIndex, bool xory, float posFrom, float uvFrom, float lerpValue, const TArray<int>& inVertIndexArray) {
			auto& pos = originVertices[changeIndex].Position;
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
				originVertices[i].Position = pos;
				vertices[i].TextureCoordinate[0] = uv;
			}
		};
		switch (originType)
		{
		case ELexUISpriteFillOriginType_Radial360::Bottom:
		{
			originVertices[9].Position = originVertices[1].Position;
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
					setPosAndUv(2, false, posHalfY, uvHalfY, lerpValue, { 9 });
				}
				else if (fillAmount >= 0.625f)
				{
					float lerpValue = (fillAmount - 0.625f) * 8.0f;
					setPosAndUv(5, false, posMaxY, uvMaxY, lerpValue, { 9, 2 });
				}
				else if (fillAmount >= 0.5f)
				{
					float lerpValue = (fillAmount - 0.5f) * 8.0f;
					setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, { 9, 2, 5 });
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
					setPosAndUv(0, false, posHalfY, uvHalfY, lerpValue, { 1 });
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
		case ELexUISpriteFillOriginType_Radial360::Right:
		{
			originVertices[9].Position = originVertices[5].Position;
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
					setPosAndUv(8, true, posHalfX, uvHalfX, lerpValue, { 9 });
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
					setPosAndUv(2, true, posHalfX, uvHalfX, lerpValue, { 5 });
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
		case ELexUISpriteFillOriginType_Radial360::Top:
		{
			originVertices[9].Position = originVertices[7].Position;
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
					setPosAndUv(6, false, posHalfY, uvHalfY, lerpValue, { 9 });
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
					setPosAndUv(8, false, posHalfY, uvHalfY, lerpValue, { 7 });
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
		case ELexUISpriteFillOriginType_Radial360::Left:
		{
			originVertices[9].Position = originVertices[3].Position;
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
					setPosAndUv(0, true, posHalfX, uvHalfX, lerpValue, { 9 });
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
					setPosAndUv(6, true, posHalfX, uvHalfX, lerpValue, { 3 });
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

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetActualRequireNormalAndTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
		}
	}
}
#pragma endregion

#pragma region LexText
#include "Core/Components/LexText.h"
void UIGeometry_AlignUITextLineVertex(ELexUITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart
	, TArray<FLexUIOriginVertexData>& vertices, FLexUITextLineProperty& lineProperty
	, int lineEmojiStartIndex, TArray<FLexUIText_Emoji>& emojiArray
)
{
	float xOffset = 0;
	switch (pivotHAlign)
	{
	case ELexUITextParagraphHorizontalAlign::Center:
		xOffset = -lineWidth * 0.5f;
		break;
	case ELexUITextParagraphHorizontalAlign::Right:
		xOffset = -lineWidth;
		break;
	}

	for (int i = lineUIGeoVertStart; i < vertices.Num(); i++)
	{
		auto& vertex = vertices[i].Position;
		vertex.Y += xOffset;
	}

	auto& charList = lineProperty.CaretPropertyList;
	for (auto& item : charList)
	{
		item.CaretPosition.X += xOffset;
	}
	for (int i = lineEmojiStartIndex; i < emojiArray.Num(); i++)
	{
		auto& item = emojiArray[i];
		item.Position.X += xOffset;
	}
}
void UIGeometry_AlignUITextLineVertexForRichText(ELexUITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineMaxFontSize, float fontSize, int lineUIGeoVertStart
	, TArray<FLexUIOriginVertexData>& vertices
	, int lineImageStartIndex, TArray<FLexUIText_RichTextImageTag>& imageArray
	, int lineEmojiStartIndex, TArray<FLexUIText_Emoji>& emojiArray
)
{
	float xOffset = 0;
	switch (pivotHAlign)
	{
	case ELexUITextParagraphHorizontalAlign::Center:
		xOffset = -lineWidth * 0.5f;
		break;
	case ELexUITextParagraphHorizontalAlign::Right:
		xOffset = -lineWidth;
		break;
	}
	float yOffset = -(lineMaxFontSize - fontSize) * 0.5f;

	for (int i = lineUIGeoVertStart; i < vertices.Num(); i++)
	{
		auto& vertex = vertices[i].Position;
		vertex.Y += xOffset;
		vertex.Z += yOffset;
	}

	for (int i = lineImageStartIndex; i < imageArray.Num(); i++)
	{
		auto& item = imageArray[i];
		item.Position.X += xOffset;
		item.Position.Y += yOffset;
	}
	for (int i = lineEmojiStartIndex; i < emojiArray.Num(); i++)
	{
		auto& item = emojiArray[i];
		item.Position.X += xOffset;
		item.Position.Y += yOffset;
	}
}
#include "Core/LexUIRichTextCustomStyleData.h"
void FLexUIGeometry::UpdateUIText(const FString& Content
	, TArray<FLexUIText_TextProcessingElement>& TextProcessingArray
	, float width, float height, FVector2f pivot
	, FColor color, uint8 RenderOpacityForRichText, FVector2f fontSpace, FLexUIGeometry* uiGeo, float fontSize
	, ELexUITextParagraphHorizontalAlign paragraphHAlign, ELexUITextParagraphVerticalAlign paragraphVAlign, ELexUITextOverflowType overflowType
	, ETextWrappingPolicy wrappingPolicy, bool bUseKerning
	, ELexUITextFontStyle fontStyle, FVector2f& textPreferredSize, bool& outTruncated
	, ULexCanvas* renderCanvas, ULexText* LexText
	, TArray<FLexUITextLineProperty>& cacheLinePropertyArray, TArray<FLexUITextCharProperty>& cacheCharPropertyArray, TArray<FLexUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
	, TArray<FLexUIText_RichTextImageTag>& cacheRichTextImageTagArray
	, TArray<FLexUIText_Emoji>& cacheEmojiArray
	, ULexUIFontData_BaseObject* font, bool bRichText, int32 richTextFilterFlags)
{
	float maxFontSize = font->GetFontSizeLimit();
	fontSize = FMath::Clamp(fontSize, 0.0f, maxFontSize);
	bool pixelPerfect = LexText->GetShouldAffectByPixelSnapping() && LexText->GetWidget()->GetPixelSnappingInHierarchy();
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float dynamicPixelsPerUnit = LexText->GetDynamicPixelsPerUnit() * rootCanvasScale;
	float oneDivideRootCanvasScale = 1.0f / rootCanvasScale;
	float oneDivideDynamicPixelsPerUnit = 1.0f / dynamicPixelsPerUnit;
	bool shouldScaleFontSizeWithRootCanvas = false;

	auto richTextImageData = LexText->GetRichTextImageData();
	auto GetRichTextImageCharData = [&](FLexUICharData& overrideCharData, float inFontSize, FName imageTag)
	{
		//image use font size as default width & height & xadvance
		overrideCharData.Width = overrideCharData.Height = overrideCharData.XAdvance = inFontSize * oneDivideRootCanvasScale;
		
		FIntVector2 imageSize;
		if (IsValid(richTextImageData) && richTextImageData->GetImageSize(imageTag, imageSize))
		{
			float ratio = (float)imageSize.X / imageSize.Y;
			overrideCharData.Width = overrideCharData.Width * ratio;
			overrideCharData.XAdvance = overrideCharData.XAdvance * ratio;
		}
		else
		{
			//default use font size as width & height & xadvance
			overrideCharData.Width = overrideCharData.Height = overrideCharData.XAdvance = inFontSize * oneDivideRootCanvasScale;
		}
	};
	auto emojiData = font->GetEmojiData();
	auto GetEmojiCharData = [&](FLexUICharData& overrideCharData, float inFontSize, uint32 emojiCode)
	{
		//emoji use font size as default width & height & xadvance
		overrideCharData.Width = overrideCharData.Height = overrideCharData.XAdvance = inFontSize * oneDivideRootCanvasScale;
		
		FIntVector2 imageSize;
		if (IsValid(emojiData) && emojiData->GetImageSize(emojiCode, imageSize))
		{
			float ratio = (float)imageSize.X / imageSize.Y;
			overrideCharData.Width = overrideCharData.Width * ratio;
			overrideCharData.XAdvance = overrideCharData.XAdvance * ratio;
		}
		else
		{
			//default use font size as width & height & xadvance
			overrideCharData.Width = overrideCharData.Height = overrideCharData.XAdvance = inFontSize * oneDivideRootCanvasScale;
		}
	};

	if (renderCanvas->GetRootCanvas()->IsRenderToWorldSpace())
	{
		pixelPerfect = false;
		if (dynamicPixelsPerUnit != 1.0f && font->GetSupportDynamicPixelsPerUnit())
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
			if (dynamicPixelsPerUnit != 1.0f && font->GetSupportDynamicPixelsPerUnit())
			{
				shouldScaleFontSizeWithRootCanvas = true;
			}
		}
	}

	font->PrepareForPushCharData(LexText);
	bool useKerning = bUseKerning && font->HasKerning();

	bool bUseBold = fontStyle == ELexUITextFontStyle::Bold || fontStyle == ELexUITextFontStyle::BoldAndItalic;
	bool bUseItalic = fontStyle == ELexUITextFontStyle::Italic || fontStyle == ELexUITextFontStyle::BoldAndItalic;
	
	//rich text
	using namespace LexUIRichTextParser;
	static FRichTextParser richTextParser;
	FRichTextParseResult richTextParseResult;
	if (bRichText)
	{
		richTextParser.Clear();
		richTextParser.Prepare(fontSize, color, RenderOpacityForRichText, bUseBold, bUseItalic, richTextFilterFlags, richTextParseResult);
	}
	else
	{
		richTextParseResult.Color = color;
		richTextParseResult.Bold = bUseBold;
		richTextParseResult.Italic = bUseItalic;
		richTextParseResult.Size = fontSize;
	}

	float verticalOffset = font->GetVerticalOffset(fontSize);//some font may not render at vertical center, use this to mofidy it. 0.25 * size is tested value for most fonts

	cacheLinePropertyArray.Reset();
	cacheCharPropertyArray.Reset();
	cacheRichTextCustomTagArray.Reset();
	cacheRichTextImageTagArray.Reset();
	cacheEmojiArray.Reset();
	int contentLength = Content.Len();
	FVector2f currentLineOffset(0, 0);
	float originLineHeight = font->GetLineHeight(fontSize);
	float currentLineWidth = 0, currentLineHeight = originLineHeight, paragraphHeight = 0;//single line width, height, all line height
	float firstLineHeight = currentLineHeight;//first line height
	float currentLineMaxFontSize = fontSize;//for rich text, max font size of current line
	float maxLineWidth = 0;//if have multiple line
	float currentPreferredWidth = 0;//preferredWidth is the width that not wrapped width
	float maxPreferredWidth = 0;//preferredWidth of all (line-break) lines
	int lineUIGeoVertStart = 0;//vertex index in originVertices of current line
	int currentVisibleCharCount = 0;//visible char count, skip invisible char(\r,\n,\t)
	int imageStartIndexInCurrentLine = 0;//
	int emojiStartIndexInCurrentLine = 0;//
	FLexUITextLineProperty lineProperty;
	FVector2f caretPosition(0, 0);
	float halfFontSpaceX = fontSpace.X * 0.5f;
	int linesCount = 0;//how many lines, default is 1

	int verticesCount = 0;
	auto& originVertices = uiGeo->OriginVertices;
	auto& vertices = uiGeo->Vertices;
	int indicesCount = 0;
	auto& triangles = uiGeo->Triangles;

	bool hasClampContent = false;
	float currentLineWidth_ForClampContent = 0;
	float paragraphHeight_ForClampContent = 0;
	bool shouldSetParagraphHeightForClampContent = false;
	 
	enum class NewLineMode
	{
		None,//not new line
		LineBreak,//this new line come from line break
		Space,//this new line come from space char
		Overflow,//this new line come from overflow
	};
	NewLineMode newLineMode = NewLineMode::None;

	auto NewLine = [&](int32 charIndex, bool withCaret, NewLineMode inNewLineMode, float extraSizeForPreferredWidth)
	{
		//add end caret position
		currentLineWidth -= fontSpace.X;//last char of a line don't need space
		auto currentLineWidthWithClamp = hasClampContent ? currentLineWidth_ForClampContent : currentLineWidth;
		maxLineWidth = FMath::Max(maxLineWidth, currentLineWidth);
		if (inNewLineMode != NewLineMode::None)
		{
			if (inNewLineMode == NewLineMode::LineBreak)//if lineBreak then we should start a new preferredWidth
			{
				currentPreferredWidth += currentLineWidth;
				maxPreferredWidth = FMath::Max(maxPreferredWidth, currentPreferredWidth);
				currentPreferredWidth = 0;//lineBreak cause a newline and recalculation of preferredWidth
			}
			else
			{
				currentPreferredWidth += currentLineWidth + extraSizeForPreferredWidth;
			}
		}

		FLexUITextCaretProperty caretProperty;
		caretProperty.CaretPosition = caretPosition;
		caretProperty.CharIndex = withCaret ? charIndex : -1;
		lineProperty.CaretPropertyList.Add(caretProperty);
		if (bRichText)
		{
			UIGeometry_AlignUITextLineVertexForRichText(paragraphHAlign, currentLineWidthWithClamp, currentLineMaxFontSize, fontSize
				, lineUIGeoVertStart, originVertices
				, imageStartIndexInCurrentLine, cacheRichTextImageTagArray
				, emojiStartIndexInCurrentLine, cacheEmojiArray);
			imageStartIndexInCurrentLine = cacheRichTextImageTagArray.Num();
			emojiStartIndexInCurrentLine = cacheEmojiArray.Num();
		}
		else
		{
			UIGeometry_AlignUITextLineVertex(paragraphHAlign, currentLineWidthWithClamp, lineUIGeoVertStart, originVertices, lineProperty
				, emojiStartIndexInCurrentLine, cacheEmojiArray);
			emojiStartIndexInCurrentLine = cacheEmojiArray.Num();
		}
		cacheLinePropertyArray.Add(lineProperty);
		lineProperty = FLexUITextLineProperty();
		lineUIGeoVertStart = verticesCount;

		currentLineWidth = 0;
		currentLineOffset.X = 0;
		currentLineOffset.Y -= (bRichText ? currentLineHeight : originLineHeight) + fontSpace.Y;
		paragraphHeight += (bRichText ? currentLineHeight : originLineHeight) + fontSpace.Y;
		if (hasClampContent && shouldSetParagraphHeightForClampContent)
		{
			shouldSetParagraphHeightForClampContent = false;
			paragraphHeight_ForClampContent = paragraphHeight;
		}
		linesCount++;

		//set caret position for empty newline
		caretPosition.X = currentLineOffset.X - halfFontSpaceX;
		caretPosition.Y = currentLineOffset.Y;
		//store first line height for paragraph align
		if (linesCount == 1)
		{
			firstLineHeight = bRichText ? currentLineHeight : originLineHeight;
		}
		//set line height to origin
		currentLineHeight = originLineHeight;
		currentLineMaxFontSize = fontSize;

		newLineMode = inNewLineMode;
	};

	auto IsRichTextImageSpace = [&](uint32 charCode, const FRichTextParseResult& richTextResult)
	{
		if (charCode == ' ')
		{
			if (bRichText && !richTextResult.ImageTag.IsNone())
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	};
	auto IsSpace = [&](uint32 charCode, const FRichTextParseResult& richTextResult)
	{
		if (charCode == ' ')
		{
			if (bRichText && !richTextResult.ImageTag.IsNone())
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	};

	auto GetCharGeo = [&](uint32 prevCharCode, const FLexUIText_TextProcessingElement& charElement, float inFontSize, bool inBold)
	{
		auto charData = font->GetCharData(charElement.Unicode, inFontSize, inBold);
		float calculatedCharFixedOffset = bRichText ? font->GetVerticalOffset(inFontSize) : verticalOffset;

		auto overrideCharData = charData;
		if (shouldScaleFontSizeWithRootCanvas)
		{
			if (pixelPerfect)
			{
				inFontSize = inFontSize * rootCanvasScale;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charElement.Unicode, richTextParseResult))
				{
					GetRichTextImageCharData(overrideCharData, inFontSize, richTextParseResult.ImageTag);
				}
				else if (charElement.Type == ELexUIText_CodeType::Emoji)
				{
					GetEmojiCharData(overrideCharData, inFontSize, charElement.Unicode);
				}
				else
				{
					overrideCharData = font->GetCharData(charElement.Unicode, inFontSize, inBold);

					overrideCharData.Width = overrideCharData.Width * oneDivideRootCanvasScale;
					overrideCharData.Height = overrideCharData.Height * oneDivideRootCanvasScale;
					overrideCharData.XAdvance = overrideCharData.XAdvance * oneDivideRootCanvasScale;
				}
				overrideCharData.XOffset = overrideCharData.XOffset * oneDivideRootCanvasScale;
				overrideCharData.YOffset = overrideCharData.YOffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
			}
			else if (dynamicPixelsPerUnit != 1.0f)
			{
				inFontSize = inFontSize * dynamicPixelsPerUnit;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charElement.Unicode, richTextParseResult))
				{
					GetRichTextImageCharData(overrideCharData, inFontSize, richTextParseResult.ImageTag);
				}
				else if (charElement.Type == ELexUIText_CodeType::Emoji)
				{
					GetEmojiCharData(overrideCharData, inFontSize, charElement.Unicode);
				}
				else
				{
					overrideCharData = font->GetCharData(charElement.Unicode, inFontSize, inBold);

					overrideCharData.Width = overrideCharData.Width * oneDivideDynamicPixelsPerUnit;
					overrideCharData.Height = overrideCharData.Height * oneDivideDynamicPixelsPerUnit;
					overrideCharData.XAdvance = overrideCharData.XAdvance * oneDivideDynamicPixelsPerUnit;
				}
				overrideCharData.XOffset = overrideCharData.XOffset * oneDivideDynamicPixelsPerUnit;
				overrideCharData.YOffset = overrideCharData.YOffset * oneDivideDynamicPixelsPerUnit + calculatedCharFixedOffset;
			}
			else
			{
				inFontSize = inFontSize * rootCanvasScale;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charElement.Unicode, richTextParseResult))
				{
					GetRichTextImageCharData(overrideCharData, inFontSize, richTextParseResult.ImageTag);
				}
				else if (charElement.Type == ELexUIText_CodeType::Emoji)
				{
					GetEmojiCharData(overrideCharData, inFontSize, charElement.Unicode);
				}
				else
				{
					overrideCharData = font->GetCharData(charElement.Unicode, inFontSize, inBold);

					overrideCharData.Width = overrideCharData.Width * oneDivideRootCanvasScale;
					overrideCharData.Height = overrideCharData.Height * oneDivideRootCanvasScale;
					overrideCharData.XAdvance = overrideCharData.XAdvance * oneDivideRootCanvasScale;
				}
				overrideCharData.XOffset = overrideCharData.XOffset * oneDivideRootCanvasScale;
				overrideCharData.YOffset = overrideCharData.YOffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
			}
		}
		else
		{
			if (IsRichTextImageSpace(charElement.Unicode, richTextParseResult))
			{
				GetRichTextImageCharData(overrideCharData, inFontSize, richTextParseResult.ImageTag);
			}
			else if (charElement.Type == ELexUIText_CodeType::Emoji)
			{
				GetEmojiCharData(overrideCharData, inFontSize, charElement.Unicode);
			}
			overrideCharData.YOffset += calculatedCharFixedOffset;
		}
		if (useKerning && prevCharCode != charElement.Unicode)
		{
			auto KerningValue = font->GetKerning(prevCharCode, charElement.Unicode, inFontSize);
			overrideCharData.XAdvance += KerningValue;
			overrideCharData.XOffset += KerningValue;
		}

		return overrideCharData;
	};
	auto GetCharGeoXAdv = [&](uint32 prevCharCode, const FLexUIText_TextProcessingElement& charElement, const FRichTextParseResult& richTextResult)
	{
		if (IsRichTextImageSpace(charElement.Unicode, richTextResult))
		{
			return richTextResult.Size;//image use font size as width & height & xadvance
		}
		else if (charElement.Type == ELexUIText_CodeType::Emoji)
		{
			FIntVector2 Size;
			if (IsValid(emojiData) && emojiData->GetImageSize(charElement.Unicode, Size))
			{
				return (float)Size.X;
			}
			return 0.0f;
		}
		else
		{
			auto charData = font->GetCharData(charElement.Unicode, richTextResult.Size, richTextResult.Bold);
			if (useKerning && prevCharCode != charElement.Unicode)
			{
				auto KerningValue = font->GetKerning(prevCharCode, charElement.Unicode, richTextResult.Size);
				return charData.XAdvance + KerningValue;
			}
			else
			{
				return charData.XAdvance;
			}
		}
	};

	static TArray<FRichTextParseResult> richTextPropertyArray;
	richTextPropertyArray.Reset();
	TextProcessingArray.Reset(Content.Len());
	if (bRichText)
	{
		//pre parse rich text
		auto richTextCustomStyleData = LexText->GetRichTextCustomStyleData();
		bool useCustomStyle = IsValid(richTextCustomStyleData);
		for (int charIndex = 0; charIndex < contentLength; charIndex++)
		{
			auto charCode = Content[charIndex];
			richTextParseResult.CustomTag = NAME_None;
			richTextParseResult.CustomTagMode = ECustomTagMode::None;
			richTextParseResult.CharIndex = charIndex;
			richTextParser.ClearImageTag();
			while (richTextParser.Parse(Content, contentLength, charIndex, richTextParseResult))
			{
				if (!richTextParseResult.ImageTag.IsNone())//get image, append a blank placeholder
				{
					TextProcessingArray.Add(FLexUIText_TextProcessingElement{' ', charIndex, 1});
					richTextPropertyArray.Add(richTextParseResult);
					richTextParseResult.ImageTag = NAME_None;//clear it
					richTextParser.ClearImageTag();
				}
				if (charIndex < contentLength)
				{
					charCode = Content[charIndex];
				}
				else
				{
					break;
				}
			}
			//if find end symbol, then mark the prev one as end
			if (richTextParseResult.CustomTagMode == LexUIRichTextParser::ECustomTagMode::End)
			{
				auto& last = richTextPropertyArray[richTextPropertyArray.Num() - 1];
				last.CustomTag = richTextParseResult.CustomTag;
				last.CustomTagMode = richTextParseResult.CustomTagMode;
				richTextParseResult.CustomTag = NAME_None;
				richTextParseResult.CustomTagMode = LexUIRichTextParser::ECustomTagMode::None;
			}

			if (charIndex >= contentLength)break;
			
			richTextParseResult.CharIndex = charIndex;
			//convert custom tag to style
			if (useCustomStyle)
			{
				if (auto customStyleItemDataPtr = richTextCustomStyleData->GetDataMap().Find(richTextParseResult.CustomTag))
				{
					customStyleItemDataPtr->ApplyToRichTextParseResult(richTextParseResult);
				}
			}
			richTextPropertyArray.Add(richTextParseResult);

			TextProcessingArray.Add(FLexUIText_CodePoint::ReadCodePoint(Content, contentLength, charIndex));
		}
	}
	else
	{
		for (int32 charIndex = 0; charIndex < contentLength; charIndex++)
		{
			TextProcessingArray.Add(FLexUIText_CodePoint::ReadCodePoint(Content, contentLength, charIndex));
		}
	}
	contentLength = TextProcessingArray.Num();
	
	uint32 prevCharCode = '\0';//prev char code (not space or tab)
	for (int charIndex = 0; charIndex < contentLength; charIndex++)
	{
		auto charElement = TextProcessingArray[charIndex];
		auto charCode = charElement.Unicode;
		auto caretCharIndex = charIndex;
		if (bRichText)
		{
			richTextParseResult = richTextPropertyArray[charIndex];
			caretCharIndex = richTextParseResult.CharIndex;
		}

		if (charCode == '\n' || charCode == '\r')//10 -- \n, 13 -- \r
		{
			NewLine(bRichText ? richTextParseResult.CharIndex : charIndex, true, NewLineMode::LineBreak, 0);
			if (charIndex + 1 < contentLength)
			{
				auto nextCharCode = TextProcessingArray[charIndex + 1].Unicode;
				if ((charCode == '\r' && nextCharCode == '\n') || (charCode == '\n' && nextCharCode == '\r'))
				{
					charIndex++;//\n\r or \r\n
				}
			}
			continue;
		}

		if (newLineMode == NewLineMode::Space || newLineMode == NewLineMode::Overflow)
		{
			if (IsSpace(charCode, richTextParseResult))//skip empty space at start of newline
			{
				if (newLineMode == NewLineMode::Overflow)
				{
					auto TempCharGeo = GetCharGeo(charIndex == 0 ? charCode : prevCharCode, charElement
						, richTextParseResult.Size, richTextParseResult.Bold);
					currentPreferredWidth += TempCharGeo.XAdvance;//newline is caused by space, so the space size should add to preferredWidth, because preferredWidth should ignore auto wrapping
					newLineMode = NewLineMode::None;
				}
				continue;
			}
			else
			{
				newLineMode = NewLineMode::None;
			}
		}
		
		auto charGeo = GetCharGeo(charIndex == 0 ? charCode : prevCharCode, charElement
			, richTextParseResult.Size, richTextParseResult.Bold);
		//caret property
		caretPosition.X = currentLineOffset.X - halfFontSpaceX;
		caretPosition.Y = currentLineOffset.Y;
		FLexUITextCaretProperty caretProperty;
		caretProperty.CaretPosition = caretPosition;
		caretProperty.CharIndex = caretCharIndex;
		lineProperty.CaretPropertyList.Add(caretProperty);

		caretPosition.X += fontSpace.X + charGeo.XAdvance;//for line's last char's caret position
		currentLineMaxFontSize = FMath::Max(currentLineMaxFontSize, richTextParseResult.Size);

		if (IsSpace(charCode, richTextParseResult))//char is space
		{
			if (overflowType == ELexUITextOverflowType::VerticalOverflow//char is space and LexText can have overflow line, then we need to calculate if the following words can fit the rest space, if not means new line
				)
			{
				auto prevCharCodeOfForwardChar = prevCharCode;
				float spaceNeeded = GetCharGeoXAdv(prevCharCodeOfForwardChar, charElement, richTextParseResult);
				prevCharCodeOfForwardChar = charCode;
				spaceNeeded += fontSpace.X;
				bool needToRemoveLastFontSpace = false;
				for (int forwardCharIndex = charIndex + 1, forwardVisibleCharIndex = currentVisibleCharCount; forwardCharIndex < contentLength && forwardVisibleCharIndex < contentLength; forwardCharIndex++)
				{
					needToRemoveLastFontSpace = false;
					auto charElementOfForwardChar = TextProcessingArray[forwardCharIndex];
					auto charCodeOfForwardChar = charElementOfForwardChar.Unicode;
					auto richTextParseResultOfForwardChar = bRichText ? richTextPropertyArray[forwardCharIndex] : richTextParseResult;
					if (IsSpace(charCodeOfForwardChar, richTextParseResultOfForwardChar))//space
					{
						break;
					}
					if (charCodeOfForwardChar == '\n' || charCodeOfForwardChar == '\r' || charCodeOfForwardChar == '\t')//\n\r\t
					{
						break;
					}
					spaceNeeded += GetCharGeoXAdv(prevCharCodeOfForwardChar, charElementOfForwardChar, richTextParseResultOfForwardChar);
					spaceNeeded += fontSpace.X;
					needToRemoveLastFontSpace = true;
					forwardVisibleCharIndex++;
					prevCharCodeOfForwardChar = charCodeOfForwardChar;
				}
				if (needToRemoveLastFontSpace)
				{
					spaceNeeded -= fontSpace.X;
				}
				if (currentLineOffset.X + spaceNeeded > width + UE_KINDA_SMALL_NUMBER)
				{
					NewLine(caretCharIndex, false, NewLineMode::Space,
						charGeo.XAdvance
						+ fontSpace.X//this font-space is related to char
						+ fontSpace.X//because NewLine function remove font-space (currentLineWidth -= fontSpace.X to remove font-space), so we need add it back 
						);
					continue;
				}
			}
		}

		prevCharCode = charCode;
		//char geometry
		if (IsRichTextImageSpace(charCode, richTextParseResult))
		{
			FLexUIText_RichTextImageTag imageTagData;
			imageTagData.TagName = richTextParseResult.ImageTag;
			imageTagData.Position = FVector2D(currentLineOffset.X + charGeo.XAdvance * 0.5f, currentLineOffset.Y);
			imageTagData.Size = FVector2D(charGeo.Width, charGeo.Height);
			imageTagData.TintColor = richTextParseResult.HasColor ? richTextParseResult.Color : FColor::White;
			cacheRichTextImageTagArray.Add(imageTagData);
			currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.Size);
		}
		else if (charElement.Type == ELexUIText_CodeType::Emoji)
		{
			FLexUIText_Emoji Emoji;
			Emoji.EmojiCode = charElement.Unicode;
			Emoji.Position = FVector2D(currentLineOffset.X + charGeo.XAdvance * 0.5f, currentLineOffset.Y);
			Emoji.Size = FVector2D(charGeo.Width, charGeo.Height);
			cacheEmojiArray.Add(Emoji);
			currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.Size);
		}
		else
		{
			if (charCode != ' ' && charCode != '\t')//skip invisible char
			{
				if (bRichText)
				{
					currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.Size);
				}

				if (!hasClampContent)
				{
					int additionalVerticesCount = 0, additionalIndicesCount = 0;
					font->PushCharData(
						charCode, currentLineOffset, fontSpace, charGeo,
						richTextParseResult,
						verticesCount, indicesCount,
						additionalVerticesCount, additionalIndicesCount,
						originVertices, vertices, triangles
					);

					//collect char property
					{
						FLexUITextCharProperty charProperty;
						charProperty.CharIndex = charIndex;
						charProperty.StartVertIndex = verticesCount;
						charProperty.VertCount = additionalVerticesCount;
						charProperty.StartTriangleIndex = indicesCount;
						charProperty.IndicesCount = additionalIndicesCount;
						cacheCharPropertyArray.Add(charProperty);
					}

					verticesCount += additionalVerticesCount;
					indicesCount += additionalIndicesCount;
				}

				currentVisibleCharCount++;
			}
		}

		//collect rich text custom tag. custom tag use start/end mark, so put these code outside of visible-char-check.
		if (bRichText)
		{
			switch (richTextParseResult.CustomTagMode)
			{
			case LexUIRichTextParser::ECustomTagMode::Start:
			{
				FLexUIText_RichTextCustomTag customTag;
				customTag.TagName = richTextParseResult.CustomTag;
				customTag.CharIndexStart = currentVisibleCharCount - 1;//-1 as index
				customTag.CharIndexStart = FMath::Max(0, customTag.CharIndexStart);//incase first char is invisible char, that makes index == -1
				customTag.CharIndexEnd = -1;
				cacheRichTextCustomTagArray.Add(customTag);
			}
			break;
			case LexUIRichTextParser::ECustomTagMode::End:
			{
				int foundIndex = cacheRichTextCustomTagArray.IndexOfByPredicate([richTextParseResult](const FLexUIText_RichTextCustomTag& A) {
					return A.TagName == richTextParseResult.CustomTag;
					});
				if (foundIndex != -1)
				{
					cacheRichTextCustomTagArray[foundIndex].CharIndexEnd = currentVisibleCharCount - 1;//-1 as index
				}
			}
			break;
			}
		}

		currentLineOffset.X += charGeo.XAdvance + fontSpace.X;
		currentLineWidth += charGeo.XAdvance + fontSpace.X;

		//overflow
		{
			switch (overflowType)
			{
			case ELexUITextOverflowType::HorizontalOverflow:
			{
				//no need to do anything
			}
			break;
			case ELexUITextOverflowType::VerticalOverflow:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				int nextCharXAdv = GetCharGeoXAdv(TextProcessingArray[charIndex].Unicode, TextProcessingArray[charIndex + 1]
					, bRichText ? richTextPropertyArray[charIndex + 1] : richTextParseResult);

				if (charIndex + 2 < contentLength//check size
					&& FChar::IsPunct(TextProcessingArray[charIndex + 2].Unicode)//newline with punctuation
					&& charIndex + 2 != contentLength - 1//not last char
					)
				{
					nextCharXAdv += GetCharGeoXAdv(TextProcessingArray[charIndex+1].Unicode, TextProcessingArray[charIndex+2]
						, bRichText ? richTextPropertyArray[charIndex+2] : richTextParseResult);
					if (currentLineOffset.X + nextCharXAdv > width + UE_KINDA_SMALL_NUMBER)//if next char cannot fit this line, then add new line
					{
						auto nextChar = TextProcessingArray[charIndex + 1].Unicode;
						if (nextChar == '\r' || nextChar == '\n')
						{
							//next char is new line, no need to add new line
						}
						else
						{
							NewLine(caretCharIndex + 2, false, NewLineMode::Overflow, 0);
							continue;
						}
					}
				}
				else
				{
					if (currentLineOffset.X + nextCharXAdv > width + UE_KINDA_SMALL_NUMBER && wrappingPolicy == ETextWrappingPolicy::AllowPerCharacterWrapping)//if next char cannot fit this line, then add new line
					{
						auto nextChar = TextProcessingArray[charIndex + 1].Unicode;
						if (nextChar == '\r' || nextChar == '\n')
						{
							//next char is new line, no need to add new line
						}
						else
						{
							NewLine(caretCharIndex + 1, false, NewLineMode::Overflow, 0);
							continue;
						}
					}
				}
			}
			break;
			case ELexUITextOverflowType::Truncate:
			case ELexUITextOverflowType::Ellipsis:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				if (hasClampContent)continue;

				int nextCharXAdv = GetCharGeoXAdv(TextProcessingArray[charIndex].Unicode, TextProcessingArray[charIndex + 1]
					, bRichText ? richTextPropertyArray[charIndex + 1] : richTextParseResult);
				if (currentLineOffset.X + nextCharXAdv > width)//horizontal cannot fit next char
				{
					hasClampContent = true;
					outTruncated = true;
					currentLineWidth_ForClampContent = currentLineWidth;
					shouldSetParagraphHeightForClampContent = true;//paragraphHeight is set after NewLine, so we mark it and get clamp_ParagraphHeight later
					if (overflowType == ELexUITextOverflowType::Ellipsis)
					{
						//move back and replace chars by ...
						uint32 charCodeOfDots = 0x2026;//'…'
						auto charElementOfDots = FLexUIText_TextProcessingElement{charCodeOfDots, charIndex, 1, ELexUIText_CodeType::Text};
						auto charGeoOfDots = GetCharGeo(charCodeOfDots, charElementOfDots, fontSize, false);
						if (currentLineOffset.X < charGeoOfDots.XAdvance)//remove all if it can't fit the char-of-dots
						{
							originVertices.Reset();
							vertices.Reset();
							triangles.Reset();
						}
						else
						{
							auto lineOffsetPointToStripOff = currentLineOffset.X - charGeoOfDots.XAdvance - halfFontSpaceX;
							//remove char geometry on tail of data, if the char's vertex position greater than dots
							for (int charPropertyIndex = cacheCharPropertyArray.Num() - 1; charPropertyIndex >= 0; charPropertyIndex--)
							{
								bool bShouldStripOffThisChar = false;
								auto& charProperty = cacheCharPropertyArray[charPropertyIndex];
								for (int i = 0; i < charProperty.VertCount; i++)
								{
									auto vertIndex = charProperty.StartVertIndex + i;
									auto& vert = originVertices[vertIndex];
									if (vert.Position.Y > lineOffsetPointToStripOff)
									{
										bShouldStripOffThisChar = true;
										break;
									}
								}
								if (bShouldStripOffThisChar)
								{
									for (int i = 0; i < charProperty.VertCount; i++)
									{
										originVertices.Pop();
										vertices.Pop();
									}
									for (int i = 0; i < charProperty.IndicesCount; i++)
									{
										triangles.Pop();
									}
								}
								else
								{
									break;
								}
							}

							currentLineOffset.X = lineOffsetPointToStripOff;
							//push dots geometry to tail
							int additionalVerticesCount, additionalIndicesCount;
							font->PushCharData(
							charCodeOfDots, currentLineOffset, fontSpace, charGeoOfDots,
							richTextParseResult,
							originVertices.Num(), triangles.Num(),
							additionalVerticesCount, additionalIndicesCount,
							originVertices, vertices, triangles
							);
						}
					}
				}
			}
			break;
			}
		}
	}

	//additional data
	{
		//normal & tangent
		if (renderCanvas->GetActualRequireNormalAndTangent())
		{
			for (int i = 0; i < originVertices.Num(); i++)
			{
				originVertices[i].Normal = FVector3f(-1, 0, 0);
				originVertices[i].Tangent = FVector3f(0, 1, 0);
			}
		}
	}

	//verify custom tag
	if (bRichText)
	{
		for (int i = 0; i < cacheRichTextCustomTagArray.Num(); i++)
		{
			auto& item = cacheRichTextCustomTagArray[i];
			if (item.CharIndexEnd == -1)
			{
				item.CharIndexEnd = currentVisibleCharCount - 1;//-1 as index
			}
		}
	}

	//last line
	NewLine(bRichText ? Content.Len() : contentLength, true, NewLineMode::Overflow, 0); 
	//remove last line's space Y
	paragraphHeight -= fontSpace.Y;
	paragraphHeight_ForClampContent -= fontSpace.Y;
	auto paragraphHeightWithClamp = hasClampContent ? paragraphHeight_ForClampContent : paragraphHeight;

	textPreferredSize.X = FMath::Max(currentPreferredWidth, maxPreferredWidth);
	textPreferredSize.Y = paragraphHeight;

	float pivotOffsetX = width * (0.5f - pivot.X);
	float pivotOffsetY = height * (0.5f - pivot.Y);
	float xOffset = pivotOffsetX;
	switch (paragraphHAlign)
	{
	case ELexUITextParagraphHorizontalAlign::Left:
		xOffset += -width * 0.5f;
		break;
	case ELexUITextParagraphHorizontalAlign::Center:

		break;
	case ELexUITextParagraphHorizontalAlign::Right:
		xOffset += width * 0.5f;
		break;
	}
	float yOffset = pivotOffsetY - firstLineHeight * 0.5f;
	switch (paragraphVAlign)
	{
	case ELexUITextParagraphVerticalAlign::Top:
		yOffset += height * 0.5f;
		break;
	case ELexUITextParagraphVerticalAlign::Middle:
		yOffset += paragraphHeightWithClamp * 0.5f;
		break;
	case ELexUITextParagraphVerticalAlign::Bottom:
		yOffset += paragraphHeightWithClamp - height * 0.5f;
		break;
	}
	//caret property
	for (auto& linePropertyItem : cacheLinePropertyArray)
	{
		for (auto& charItem : linePropertyItem.CaretPropertyList)
		{
			charItem.CaretPosition.X += xOffset;
			charItem.CaretPosition.Y += yOffset;
		}
	}
	//image
	if (bRichText)
	{
		for (auto& imageItem : cacheRichTextImageTagArray)
		{
			imageItem.Position.X += xOffset;
			imageItem.Position.Y += yOffset;
		}
	}
	//emoji
	{
		for (auto& emojiItem : cacheEmojiArray)
		{
			emojiItem.Position.X += xOffset;
			emojiItem.Position.Y += yOffset;
		}
	}

	FLexUIGeometry::OffsetVertices(originVertices, originVertices.Num(), xOffset, yOffset);

	//snap pixel
	if (pixelPerfect)
	{
		AdjustPixelPerfectPos_For_UIText(originVertices, cacheCharPropertyArray, renderCanvas, LexText);
	}
}

#pragma endregion

void FLexUIGeometry::OffsetVertices(TArray<FLexUIOriginVertexData>& vertices, int count, float offsetX, float offsetY)
{
	for (int i = 0; i < count; i++)
	{
		auto& vertex = vertices[i].Position;
		vertex.Y += offsetX;
		vertex.Z += offsetY;
	}
}
void FLexUIGeometry::UpdateUIColor(FLexUIGeometry* uiGeo, FColor color)
{
	auto& vertices = uiGeo->Vertices;
	for (int i = 0; i < vertices.Num(); i++)
	{
		vertices[i].Color = color;
	}
}

void FLexUIGeometry::CalculatePivotOffset(
	float width, float height, FVector2f pivot
	, float& pivotOffsetX, float& pivotOffsetY
)
{
	pivotOffsetX = width * (0.5f - pivot.X);//width * 0.5f *(1 - pivot.X * 2)
	pivotOffsetY = height * (0.5f - pivot.Y);//height * 0.5f *(1 - pivot.Y * 2)
}

void FLexUIGeometry::CalculateOffsetAndSize(
	float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo
	, float& pivotOffsetX, float& pivotOffsetY, float& halfWidth, float& halfHeight
)
{
	CalculatePivotOffset(width, height, pivot, pivotOffsetX, pivotOffsetY);

	if (spriteInfo.HasPadding())
	{
		float widthScale = width / spriteInfo.GetSourceWidth();
		float heightScale = height / spriteInfo.GetSourceHeight();
		float geoWidth = spriteInfo.Width * widthScale;
		float geoHeight = spriteInfo.Height * heightScale;
		pivotOffsetX += (-width + geoWidth) * 0.5f + spriteInfo.Padding.Left * widthScale;
		pivotOffsetY += (-height + geoHeight) * 0.5f + spriteInfo.Padding.Bottom * heightScale;
		halfWidth = geoWidth * 0.5f;
		halfHeight = geoHeight * 0.5f;
	}
	else
	{
		halfWidth = width * 0.5f;
		halfHeight = height * 0.5f;
	}
}


void FLexUIGeometry::TransformVertices(ULexCanvas* canvas, ULexVisual* item, FLexUIGeometry* uiGeo)
{
	auto& vertices = uiGeo->Vertices;
	auto& originVertices = uiGeo->OriginVertices;
	auto vertexCount = vertices.Num();
	auto originVertexCount = originVertices.Num();
	if (originVertexCount > vertexCount)
	{
		originVertices.RemoveAt(vertexCount, originVertexCount - vertexCount);
	}
	else if (originVertexCount < vertexCount)
	{
		originVertices.AddDefaulted(vertexCount - originVertexCount);
	}

	auto inverseCanvasTf = canvas->GetWidget()->GetWorldTransform().Inverse();
	const auto& itemTf = item->GetWidget()->GetWorldTransform();
	FTransform itemToCanvasTf;
	FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);
	uiGeo->TransformRelativeToCanvas = itemToCanvasTf;
	auto itemToCanvasTf2D = ULexCanvas::ConvertTo2DTransform(itemToCanvasTf);
	FVector2D itemMin, itemMax;
	ULexCanvas::CalculateVisual2DBounds(item, itemToCanvasTf2D, itemMin, itemMax);
	uiGeo->BoundsMin2DInCanvasSpace = itemMin;
	uiGeo->BoundsMax2DInCanvasSpace = itemMax;

	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = FVector3f(itemToCanvasTf.TransformPosition(FVector(originVertices[i].Position)));
	}

	if (canvas->GetActualRequireNormalAndTangent())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentZ = itemToCanvasTf.TransformVector(FVector(originVertices[i].Normal));
			vertices[i].TangentZ.Vector.W = -127;

			vertices[i].TangentX = itemToCanvasTf.TransformVector(FVector(originVertices[i].Tangent));
		}
	}
}


