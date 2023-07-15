// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/UIGeometry.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "Core/RichTextParser.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_DISABLE_OPTIMIZATION
#endif

FORCEINLINE float RoundToFloat(float value)
{
	return FMath::FloorToFloat(value + 0.5f);
}

DECLARE_CYCLE_STAT(TEXT("UIGeometry TransformPixelPerfectVertices"), STAT_TransformPixelPerfectVertices, STATGROUP_LGUI);

void UIGeometry::AdjustPixelPerfectPos(TArray<FLGUIOriginVertexData>& originVertices, int startIndex, int count, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	if (!ULGUICanvas::Is2DUITransform(componentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	for (int i = startIndex; i < count; i++)
	{
		auto item = originVertices[i].Position;

		auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(FVector(item));
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
void AdjustPixelPerfectPos_For_UIRectFillRadial360(TArray<FLGUIOriginVertexData>& originVertices, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	if (!ULGUICanvas::Is2DUITransform(componentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float inv_RootCanvasScale = 1.0f / rootCanvasScale;

	static TArray<int> vertArray = { 0, 2, 6, 8 };
	for (int i = 0; i < vertArray.Num(); i++)
	{
		int vertIndex = vertArray[i];
		auto originPos = originVertices[vertIndex].Position;

		auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(FVector(originPos));
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
void AdjustPixelPerfectPos_For_UIText(TArray<FLGUIOriginVertexData>& originVertices, const TArray<FUITextCharProperty>& cacheCharPropertyArray, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformPixelPerfectVertices);
	if (cacheCharPropertyArray.Num() <= 0)return;

	auto canvasUIItem = renderCanvas->GetRootCanvas()->GetUIItem();
	FTransform componentToCanvasTransform;
	componentToCanvasTransform = uiComp->GetComponentTransform() * canvasUIItem->GetComponentTransform().Inverse();
	if (!ULGUICanvas::Is2DUITransform(componentToCanvasTransform))return;//only 2d UI can do pixel perfect
	FTransform canvasToComponentTransform = componentToCanvasTransform.Inverse();

	auto halfCanvasWidth = canvasUIItem->GetWidth() * 0.5f;
	auto halfCanvasHeight = canvasUIItem->GetHeight() * 0.5f;
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
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

			auto canvasSpaceLocation = componentToCanvasTransform.TransformPosition(FVector(originPos));
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
void UIGeometry::UpdateUIRectSimpleVertex(UIGeometry* uiGeo,
	const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	LGUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	LGUIGeometrySetArrayNum(vertices, 4);
	LGUIGeometrySetArrayNum(originVertices, 4);
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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for(int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
void UIGeometry::UpdateUIProceduralRectSimpleVertex(UIGeometry* uiGeo,
	bool bEnableBody,
	bool bOuterShadow, const FVector2f& outerShadowOffset, const float& outerShadowSize, const float& outerShadowBlur, bool bSoftEdge,
	const float& width, const float& height, const FVector2f& pivot, 
	const FLGUISpriteInfo& uniformSpriteInfo, const FLGUISpriteInfo& spriteInfo,
	ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	//@todo: use a 9-sliced mesh for OuterShadow to reduce overdraw
	auto& triangles = uiGeo->triangles;
	LGUIGeometrySetArrayNum(triangles, bOuterShadow ? 12 : 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;

		if (bOuterShadow)
		{
			triangles[6] = 4;
			triangles[7] = 7;
			triangles[8] = 6;
			triangles[9] = 4;
			triangles[10] = 5;
			triangles[11] = 7;
		}
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	LGUIGeometrySetArrayNum(vertices, bOuterShadow ? 8 : 4);
	LGUIGeometrySetArrayNum(originVertices, bOuterShadow ? 8 : 4);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			//offset and size
			float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
			CalculateOffsetAndSize(width, height, pivot, uniformSpriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
			//positions
			float minX = -halfW + pivotOffsetX;
			float minY = -halfH + pivotOffsetY;
			float maxX = halfW + pivotOffsetX;
			float maxY = halfH + pivotOffsetY;
			if (bOuterShadow)
			{
				if (bSoftEdge)//offset 1 pixel to make edge smooth
				{
					originVertices[4].Position = FVector3f(0, minX - 1, minY - 1);
					originVertices[5].Position = FVector3f(0, maxX + 1, minY - 1);
					originVertices[6].Position = FVector3f(0, minX - 1, maxY + 1);
					originVertices[7].Position = FVector3f(0, maxX + 1, maxY + 1);
				}
				else
				{
					originVertices[4].Position = FVector3f(0, minX, minY);
					originVertices[5].Position = FVector3f(0, maxX, minY);
					originVertices[6].Position = FVector3f(0, minX, maxY);
					originVertices[7].Position = FVector3f(0, maxX, maxY);
				}
				if (!bEnableBody)//if disable body, then hide vertices
				{
					originVertices[4].Position = FVector3f::ZeroVector;
					originVertices[5].Position = FVector3f::ZeroVector;
					originVertices[6].Position = FVector3f::ZeroVector;
					originVertices[7].Position = FVector3f::ZeroVector;
				}

				minX += outerShadowOffset.X;
				minY += outerShadowOffset.Y;
				maxX += outerShadowOffset.X;
				maxY += outerShadowOffset.Y;
				float additionalShadowSize = outerShadowSize + outerShadowBlur * 0.5f;
				minX -= additionalShadowSize;
				maxX += additionalShadowSize;
				minY -= additionalShadowSize;
				maxY += additionalShadowSize;
				originVertices[0].Position = FVector3f(0, minX, minY);
				originVertices[1].Position = FVector3f(0, maxX, minY);
				originVertices[2].Position = FVector3f(0, minX, maxY);
				originVertices[3].Position = FVector3f(0, maxX, maxY);
			}
			else
			{
				if (bSoftEdge)//offset 1 pixel to make edge smooth
				{
					minX -= 1;
					maxX += 1;
					minY -= 1;
					maxY += 1;
				}
				originVertices[0].Position = FVector3f(0, minX, minY);
				originVertices[1].Position = FVector3f(0, maxX, minY);
				originVertices[2].Position = FVector3f(0, minX, maxY);
				originVertices[3].Position = FVector3f(0, maxX, maxY);

				if (!bEnableBody)//if disable body, then hide vertices
				{
					originVertices[0].Position = FVector3f::ZeroVector;
					originVertices[1].Position = FVector3f::ZeroVector;
					originVertices[2].Position = FVector3f::ZeroVector;
					originVertices[3].Position = FVector3f::ZeroVector;
				}
			}
			//snap pixel
			if (pixelPerfect)
			{
				int startIndex = 0;
				if (bOuterShadow)
					startIndex = 4;
				AdjustPixelPerfectPos(originVertices, startIndex, startIndex + 4, renderCanvas, uiComp);
			}
		}

		if (InVertexUVChanged)
		{			
			int vertStartIndex = 0;
			if (bOuterShadow)
			{
				auto UV0 = uniformSpriteInfo.GetUV0();
				auto UV3 = uniformSpriteInfo.GetUV3();
				vertices[0].TextureCoordinate[0] = FVector2f(UV0.X, UV0.Y);
				vertices[1].TextureCoordinate[0] = FVector2f(UV3.X, UV0.Y);
				vertices[2].TextureCoordinate[0] = FVector2f(UV0.X, UV3.Y);
				vertices[3].TextureCoordinate[0] = FVector2f(UV3.X, UV3.Y);

				float additionalShadowSize = outerShadowSize + outerShadowBlur * 0.5f;
				float additionalUVWidth = additionalShadowSize / width;
				float additionalUVHeight = additionalShadowSize / height;
				UV0.X -= additionalUVWidth;
				UV3.X += additionalUVWidth;
				UV0.Y += additionalUVHeight;
				UV3.Y -= additionalUVHeight;
				vertices[0].TextureCoordinate[3] = FVector2f(UV0.X, UV0.Y);
				vertices[1].TextureCoordinate[3] = FVector2f(UV3.X, UV0.Y);
				vertices[2].TextureCoordinate[3] = FVector2f(UV0.X, UV3.Y);
				vertices[3].TextureCoordinate[3] = FVector2f(UV3.X, UV3.Y);

				vertStartIndex = 4;
			}
			
			if (bSoftEdge)//since vertex is offset 1 pixel, we need to offset uv to make one pixel back
			{
				auto UV0 = uniformSpriteInfo.GetUV0();
				auto UV3 = uniformSpriteInfo.GetUV3();
				float onePixelUVWidth = 1.0f / width;
				float onePixelUVHeight = 1.0f / height;
				UV0.X -= onePixelUVWidth;
				UV3.X += onePixelUVWidth;
				UV0.Y += onePixelUVHeight;
				UV3.Y -= onePixelUVHeight;
				vertices[vertStartIndex].TextureCoordinate[0] = FVector2f(UV0.X, UV0.Y);
				vertices[vertStartIndex + 1].TextureCoordinate[0] = FVector2f(UV3.X, UV0.Y);
				vertices[vertStartIndex + 2].TextureCoordinate[0] = FVector2f(UV0.X, UV3.Y);
				vertices[vertStartIndex + 3].TextureCoordinate[0] = FVector2f(UV3.X, UV3.Y);
			}
			else
			{
				vertices[vertStartIndex].TextureCoordinate[0] = uniformSpriteInfo.GetUV0();
				vertices[vertStartIndex + 1].TextureCoordinate[0] = uniformSpriteInfo.GetUV1();
				vertices[vertStartIndex + 2].TextureCoordinate[0] = uniformSpriteInfo.GetUV2();
				vertices[vertStartIndex + 3].TextureCoordinate[0] = uniformSpriteInfo.GetUV3();
			}
			//uv3 store the info for sampling texture and sprite
			vertices[vertStartIndex].TextureCoordinate[3] = spriteInfo.GetUV0();
			vertices[vertStartIndex + 1].TextureCoordinate[3] = spriteInfo.GetUV1();
			vertices[vertStartIndex + 2].TextureCoordinate[3] = spriteInfo.GetUV2();
			vertices[vertStartIndex + 3].TextureCoordinate[3] = spriteInfo.GetUV3();
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}
	}
}
#pragma endregion
#pragma region UISprite_UITexture_Border
void UIGeometry::UpdateUIRectBorderVertex(UIGeometry* uiGeo, bool fillCenter,
	const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	int triangleCount = 54;
	if (fillCenter)
	{
		triangleCount = 54;
	}
	else
	{
		triangleCount = 48;
	}
	LGUIGeometrySetArrayNum(triangles, triangleCount);
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

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 16;
	LGUIGeometrySetArrayNum(vertices, verticesCount);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
			vertices[0].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, spriteInfo.uv0Y);
			vertices[1].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, spriteInfo.uv0Y);
			vertices[2].TextureCoordinate[0] = FVector2f(spriteInfo.buv3X, spriteInfo.uv0Y);
			vertices[3].TextureCoordinate[0] = FVector2f(spriteInfo.uv3X, spriteInfo.uv0Y);

			vertices[4].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, spriteInfo.buv0Y);
			vertices[5].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, spriteInfo.buv0Y);
			vertices[6].TextureCoordinate[0] = FVector2f(spriteInfo.buv3X, spriteInfo.buv0Y);
			vertices[7].TextureCoordinate[0] = FVector2f(spriteInfo.uv3X, spriteInfo.buv0Y);

			vertices[8].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, spriteInfo.buv3Y);
			vertices[9].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, spriteInfo.buv3Y);
			vertices[10].TextureCoordinate[0] = FVector2f(spriteInfo.buv3X, spriteInfo.buv3Y);
			vertices[11].TextureCoordinate[0] = FVector2f(spriteInfo.uv3X, spriteInfo.buv3Y);

			vertices[12].TextureCoordinate[0] = FVector2f(spriteInfo.uv0X, spriteInfo.uv3Y);
			vertices[13].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, spriteInfo.uv3Y);
			vertices[14].TextureCoordinate[0] = FVector2f(spriteInfo.buv3X, spriteInfo.uv3Y);
			vertices[15].TextureCoordinate[0] = FVector2f(spriteInfo.uv3X, spriteInfo.uv3Y);
		}

		if (InVertexColorChanged)
		{
			UpdateUIColor(uiGeo, color);
		}

		//additional data
		{
			//normal & tangent
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				float widthReciprocal = 1.0f / spriteInfo.width;
				float heightReciprocal = 1.0f / spriteInfo.height;
				float buv0X = spriteInfo.borderLeft * widthReciprocal;
				float buv3X = 1.0f - spriteInfo.borderRight * widthReciprocal;
				float buv0Y = 1.0f - spriteInfo.borderBottom * heightReciprocal;
				float buv3Y = spriteInfo.borderTop * heightReciprocal;

				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(buv0X, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(buv3X, 1);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 1);

				vertices[4].TextureCoordinate[1] = FVector2f(0, buv0Y);
				vertices[5].TextureCoordinate[1] = FVector2f(buv0X, buv0Y);
				vertices[6].TextureCoordinate[1] = FVector2f(buv3X, buv0Y);
				vertices[7].TextureCoordinate[1] = FVector2f(1, buv0Y);

				vertices[8].TextureCoordinate[1] = FVector2f(0, buv3Y);
				vertices[9].TextureCoordinate[1] = FVector2f(buv0X, buv3Y);
				vertices[10].TextureCoordinate[1] = FVector2f(buv3X, buv3Y);
				vertices[11].TextureCoordinate[1] = FVector2f(1, buv3Y);

				vertices[12].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[13].TextureCoordinate[1] = FVector2f(buv0X, 0);
				vertices[14].TextureCoordinate[1] = FVector2f(buv3X, 0);
				vertices[15].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
#pragma endregion

#pragma region UISprite_Tiled
void UIGeometry::UpdateUIRectTiledVertex(UIGeometry* uiGeo,
	const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const float& width, const float& height, const FVector2f& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	int rectangleCount = widthRectCount * heightRectCount;
	auto& triangles = uiGeo->triangles;
	auto triangleCount = 6 * rectangleCount;
	LGUIGeometrySetArrayNum(triangles, triangleCount);
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
	
	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 4 * rectangleCount;
	LGUIGeometrySetArrayNum(vertices, verticesCount);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
				float realHeight = heightRectIndex == heightRectCount ? heightRemainedRectSize : spriteInfo.height;
				for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
				{
					float realWidth = widthRectIndex == widthRectCount ? (widthRemainedRectSize) : spriteInfo.width;
					originVertices[vertIndex++].Position = FVector3f(0, x, y);
					originVertices[vertIndex++].Position = FVector3f(0, x + realWidth, y);
					originVertices[vertIndex++].Position = FVector3f(0, x, y + realHeight);
					originVertices[vertIndex++].Position = FVector3f(0, x + realWidth, y + realHeight);

					x += spriteInfo.width;
				}
				x = startX;
				y += spriteInfo.height;
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
			float remainedUV3X = spriteInfo.buv0X + (spriteInfo.buv3X - spriteInfo.buv0X) * widthRemainedRectSize / spriteInfo.width;
			float remainedUV3Y = spriteInfo.buv0Y + (spriteInfo.buv3Y - spriteInfo.buv0Y) * heightRemainedRectSize / spriteInfo.height;
			for (int heightRectIndex = 1; heightRectIndex <= heightRectCount; heightRectIndex++)
			{
				float realUV3Y = heightRectIndex == heightRectCount ? remainedUV3Y : spriteInfo.buv3Y;
				for (int widthRectIndex = 1; widthRectIndex <= widthRectCount; widthRectIndex++)
				{
					float realUV3X = widthRectIndex == widthRectCount ? remainedUV3X : spriteInfo.buv3X;
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, spriteInfo.buv0Y);
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(realUV3X, spriteInfo.buv0Y);
					vertices[vertIndex++].TextureCoordinate[0] = FVector2f(spriteInfo.buv0X, realUV3Y);
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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				for (int i = 0; i < verticesCount; i += 4)
				{
					vertices[i].TextureCoordinate[1] = FVector2f(0, 0);
					vertices[i + 1].TextureCoordinate[1] = FVector2f(1, 0);
					vertices[i + 2].TextureCoordinate[1] = FVector2f(0, 1);
					vertices[i + 3].TextureCoordinate[1] = FVector2f(1, 1);
				}
			}
		}
	}
}
#pragma endregion

#pragma region UISprite_Fill_Horizontal_Vertial
void UIGeometry::UpdateUIRectFillHorizontalVerticalVertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
	, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	auto triangleCount = 6;
	LGUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 4;
	LGUIGeometrySetArrayNum(vertices, 4);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
			float uvMinX = spriteInfo.uv0X;
			float uvMinY = spriteInfo.uv0Y;
			float uvMaxX = spriteInfo.uv3X;
			float uvMaxY = spriteInfo.uv3Y;

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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial90
void UIGeometry::UpdateUIRectFillRadial90Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
	, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	auto triangleCount = 6;
	LGUIGeometrySetArrayNum(triangles, 6);
	if (InTriangleChanged)
	{
		triangles[0] = 0;
		triangles[1] = 3;
		triangles[2] = 2;
		triangles[3] = 0;
		triangles[4] = 1;
		triangles[5] = 3;
	}

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 4;
	LGUIGeometrySetArrayNum(vertices, 4);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
		float uvMinX = spriteInfo.uv0X;
		float uvMinY = spriteInfo.uv0Y;
		float uvMaxX = spriteInfo.uv3X;
		float uvMaxY = spriteInfo.uv3Y;

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
		case UISpriteFillOriginType_Radial90::BottomLeft:
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
		case UISpriteFillOriginType_Radial90::TopLeft:
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
		case UISpriteFillOriginType_Radial90::TopRight:
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
		case UISpriteFillOriginType_Radial90::BottomRight:
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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial180
void UIGeometry::UpdateUIRectFillRadial180Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
	, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	auto triangleCount = 9;
	LGUIGeometrySetArrayNum(triangles, 9);
	if (InTriangleChanged)
	{
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

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 5;
	LGUIGeometrySetArrayNum(vertices, 5);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
		float uvMinX = spriteInfo.uv0X;
		float uvMinY = spriteInfo.uv0Y;
		float uvMaxX = spriteInfo.uv3X;
		float uvMaxY = spriteInfo.uv3Y;

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
		case UISpriteFillOriginType_Radial180::Bottom:
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
		case UISpriteFillOriginType_Radial180::Left:
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
		case UISpriteFillOriginType_Radial180::Top:
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
		case UISpriteFillOriginType_Radial180::Right:
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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 0);
				vertices[4].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
#pragma endregion
#pragma region UISprite_Fill_Radial360
void UIGeometry::UpdateUIRectFillRadial360Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
	, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
	, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
	bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
)
{
	auto& triangles = uiGeo->triangles;
	auto triangleCount = 24;
	LGUIGeometrySetArrayNum(triangles, 24);
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

	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
	auto verticesCount = 10;
	LGUIGeometrySetArrayNum(vertices, verticesCount);
	LGUIGeometrySetArrayNum(originVertices, verticesCount);
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
		float uvMinX = spriteInfo.uv0X;
		float uvMinY = spriteInfo.uv0Y;
		float uvMaxX = spriteInfo.uv3X;
		float uvMaxY = spriteInfo.uv3Y;
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
		case UISpriteFillOriginType_Radial360::Bottom:
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
		case UISpriteFillOriginType_Radial360::Right:
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
		case UISpriteFillOriginType_Radial360::Top:
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
		case UISpriteFillOriginType_Radial360::Left:
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
			if (renderCanvas->GetRequireNormal() || renderCanvas->GetRequireTangent())
			{
				for (int i = 0; i < originVertices.Num(); i++)
				{
					originVertices[i].Normal = FVector3f(-1, 0, 0);
					originVertices[i].Tangent = FVector3f(0, 1, 0);
				}
			}
			//uv1
			if (renderCanvas->GetRequireUV1())
			{
				vertices[0].TextureCoordinate[1] = FVector2f(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2f(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2f(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2f(1, 0);
			}
		}
	}
}
#pragma endregion

#pragma region UIText
#include "Core/ActorComponent/UIText.h"
void UIGeometry_AlignUITextLineVertex(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart
	, TArray<FLGUIOriginVertexData>& vertices, FUITextLineProperty& lineProperty
)
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
		auto& vertex = vertices[i].Position;
		vertex.Y += xOffset;
	}

	auto& charList = lineProperty.caretPropertyList;
	for (auto& item : charList)
	{
		item.caretPosition.X += xOffset;
	}
}
void UIGeometry_AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineHeight, float fontSize, int lineUIGeoVertStart
	, TArray<FLGUIOriginVertexData>& vertices
	, int lineImageStartIndex, TArray<FUIText_RichTextImageTag>& imageArray
)
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
}
#include "Core/LGUIRichTextCustomStyleData.h"
void UIGeometry::UpdateUIText(const FString& text, int32 visibleCharCount, float width, float height, const FVector2f& pivot
	, const FColor& color, uint8 canvasGroupAlpha, const FVector2f& fontSpace, UIGeometry* uiGeo, float fontSize
	, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
	, float maxHorizontalWidth, bool kerning
	, UITextFontStyle fontStyle, FVector2f& textRealSize
	, ULGUICanvas* renderCanvas, UUIText* uiComp
	, TArray<FUITextLineProperty>& cacheLinePropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
	, TArray<FUIText_RichTextImageTag>& cacheRichTextImageTagArray
	, ULGUIFontData_BaseObject* font, bool richText, int32 richTextFilterFlags)
{
	FString content = text;

	float maxFontSize = font->GetFontSizeLimit();
	fontSize = FMath::Clamp(fontSize, 0.0f, maxFontSize);
	bool pixelPerfect = uiComp->GetShouldAffectByPixelPerfect() && renderCanvas->GetActualPixelPerfect();
	float rootCanvasScale = renderCanvas->GetRootCanvas()->GetCanvasScale();
	float dynamicPixelsPerUnit = renderCanvas->GetActualDynamicPixelsPerUnit() * rootCanvasScale;
	float oneDivideRootCanvasScale = 1.0f / rootCanvasScale;
	float oneDivideDynamicPixelsPerUnit = 1.0f / dynamicPixelsPerUnit;
	bool shouldScaleFontSizeWithRootCanvas = false;
	if (renderCanvas->GetRootCanvas()->IsRenderToWorldSpace())
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

	font->PrepareForPushCharData(uiComp);
	bool useKerning = kerning && font->HasKerning();

	//rich text
	using namespace LGUIRichTextParser;
	static RichTextParser richTextParser;
	RichTextParseResult richTextParseResult;
	if (richText)
	{
		richTextParser.Clear();
		bool bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
		bool italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
		richTextParser.Prepare(fontSize, color, canvasGroupAlpha, bold, italic, richTextFilterFlags, richTextParseResult);
	}
	else
	{
		richTextParseResult.color = color;
		richTextParseResult.bold = fontStyle == UITextFontStyle::Bold || fontStyle == UITextFontStyle::BoldAndItalic;
		richTextParseResult.italic = fontStyle == UITextFontStyle::Italic || fontStyle == UITextFontStyle::BoldAndItalic;
		richTextParseResult.size = fontSize;
	}

	float verticalOffset = font->GetVerticalOffset(fontSize);//some font may not render at vertical center, use this to mofidy it. 0.25 * size is tested value for most fonts

	cacheLinePropertyArray.Reset();
	cacheCharPropertyArray.Reset();
	cacheRichTextCustomTagArray.Reset();
	cacheRichTextImageTagArray.Reset();
	int contentLength = content.Len();
	FVector2f currentLineOffset(0, 0);
	float originLineHeight = font->GetLineHeight(fontSize);
	float currentLineWidth = 0, currentLineHeight = originLineHeight, paragraphHeight = 0;//single line width, height, all line height
	float firstLineHeight = currentLineHeight;//first line height
	float maxLineWidth = 0;//if have multiple line
	int lineUIGeoVertStart = 0;//vertex index in originVertices of current line
	int currentVisibleCharCount = 0;//visible char count, skip invisible char(\r,\n,\t)
	int imageStartIndexInCurrentLine = 0;//
	FUITextLineProperty lineProperty;
	FVector2f caretPosition(0, 0);
	float halfFontSpaceX = fontSpace.X * 0.5f;
	int linesCount = 0;//how many lines, default is 1

	int verticesCount = 0;
	auto& originVertices = uiGeo->originVertices;
	auto& vertices = uiGeo->vertices;
	int indicesCount = 0;
	auto& triangles = uiGeo->triangles;
	 
	enum class NewLineMode
	{
		None,//not new line
		LineBreak,//this new line come from line break
		Space,//this new line come from space char
		Overflow,//this new line come from overflow
	};
	NewLineMode newLineMode = NewLineMode::None;

	auto NewLine = [&](int32 charIndex, bool withCaret)
	{
		//add end caret position
		currentLineWidth -= fontSpace.X;//last char of a line don't need space
		maxLineWidth = FMath::Max(maxLineWidth, currentLineWidth);

		FUITextCaretProperty caretProperty;
		caretProperty.caretPosition = caretPosition;
		caretProperty.charIndex = withCaret ? charIndex : -1;
		lineProperty.caretPropertyList.Add(caretProperty);
		if (richText)
		{
			UIGeometry_AlignUITextLineVertexForRichText(paragraphHAlign, currentLineWidth, currentLineHeight, fontSize, lineUIGeoVertStart, originVertices, imageStartIndexInCurrentLine, cacheRichTextImageTagArray);
			imageStartIndexInCurrentLine = cacheRichTextImageTagArray.Num();
		}
		else
		{
			UIGeometry_AlignUITextLineVertex(paragraphHAlign, currentLineWidth, lineUIGeoVertStart, originVertices, lineProperty);
		}
		cacheLinePropertyArray.Add(lineProperty);
		lineProperty = FUITextLineProperty();
		lineUIGeoVertStart = verticesCount;

		currentLineWidth = 0;
		currentLineOffset.X = 0;
		currentLineOffset.Y -= (richText ? currentLineHeight : originLineHeight) + fontSpace.Y;
		paragraphHeight += (richText ? currentLineHeight : originLineHeight) + fontSpace.Y;
		linesCount++;

		//set caret position for empty newline
		caretPosition.X = currentLineOffset.X - halfFontSpaceX;
		caretPosition.Y = currentLineOffset.Y;
		//store first line height for paragraph align
		if (linesCount == 1)
		{
			firstLineHeight = richText ? currentLineHeight : originLineHeight;
		}
		//set line height to origin
		currentLineHeight = originLineHeight;
	};

	auto IsRichTextImageSpace = [&](TCHAR charCode, const RichTextParseResult& richTextResult)
	{
		if (charCode == ' ')
		{
			if (richText && !richTextResult.imageTag.IsNone())
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
	auto IsSpace = [&](TCHAR charCode, const RichTextParseResult& richTextResult)
	{
		if (charCode == ' ')
		{
			if (richText && !richTextResult.imageTag.IsNone())
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
	auto GetCharGeo = [&](TCHAR prevCharCode, TCHAR charCode, float inFontSize)
	{
		auto charData = font->GetCharData(charCode, inFontSize);
		float calculatedCharFixedOffset = richText ? font->GetVerticalOffset(inFontSize) : verticalOffset;

		auto overrideCharData = charData;
		if (shouldScaleFontSizeWithRootCanvas)
		{
			if (pixelPerfect)
			{
				inFontSize = inFontSize * rootCanvasScale;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charCode, richTextParseResult))
				{
					overrideCharData.width = overrideCharData.height = overrideCharData.xadvance = inFontSize;//image use font size as width & height & xadvance
				}
				else
				{
					overrideCharData = font->GetCharData(charCode, inFontSize);

					overrideCharData.width = overrideCharData.width * oneDivideRootCanvasScale;
					overrideCharData.height = overrideCharData.height * oneDivideRootCanvasScale;
					overrideCharData.xadvance = overrideCharData.xadvance * oneDivideRootCanvasScale;
				}
				overrideCharData.xoffset = overrideCharData.xoffset * oneDivideRootCanvasScale;
				overrideCharData.yoffset = overrideCharData.yoffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
			}
			else if (dynamicPixelsPerUnit != 1.0f)
			{
				inFontSize = inFontSize * dynamicPixelsPerUnit;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charCode, richTextParseResult))
				{
					overrideCharData.width = overrideCharData.height = overrideCharData.xadvance = inFontSize;//image use font size as width & height & xadvance
				}
				else
				{
					overrideCharData = font->GetCharData(charCode, inFontSize);

					overrideCharData.width = overrideCharData.width * oneDivideDynamicPixelsPerUnit;
					overrideCharData.height = overrideCharData.height * oneDivideDynamicPixelsPerUnit;
					overrideCharData.xadvance = overrideCharData.xadvance * oneDivideDynamicPixelsPerUnit;
				}
				overrideCharData.xoffset = overrideCharData.xoffset * oneDivideDynamicPixelsPerUnit;
				overrideCharData.yoffset = overrideCharData.yoffset * oneDivideDynamicPixelsPerUnit + calculatedCharFixedOffset;
			}
			else
			{
				inFontSize = inFontSize * rootCanvasScale;
				inFontSize = FMath::Clamp(inFontSize, 0.0f, maxFontSize);
				if (IsRichTextImageSpace(charCode, richTextParseResult))
				{
					overrideCharData.width = overrideCharData.height = overrideCharData.xadvance = inFontSize;//image use font size as width & height & xadvance
				}
				else
				{
					overrideCharData = font->GetCharData(charCode, inFontSize);

					overrideCharData.width = overrideCharData.width * oneDivideRootCanvasScale;
					overrideCharData.height = overrideCharData.height * oneDivideRootCanvasScale;
					overrideCharData.xadvance = overrideCharData.xadvance * oneDivideRootCanvasScale;
				}
				overrideCharData.xoffset = overrideCharData.xoffset * oneDivideRootCanvasScale;
				overrideCharData.yoffset = overrideCharData.yoffset * oneDivideRootCanvasScale + calculatedCharFixedOffset;
			}
		}
		else
		{
			if (IsRichTextImageSpace(charCode, richTextParseResult))
			{
				overrideCharData.width = overrideCharData.height = overrideCharData.xadvance = inFontSize;//image use font size as width & height & xadvance
			}
			overrideCharData.yoffset += calculatedCharFixedOffset;
		}
		if (useKerning && prevCharCode != charCode)
		{
			auto kerning = font->GetKerning(prevCharCode, charCode, inFontSize);
			overrideCharData.xadvance += kerning;
			overrideCharData.xoffset += kerning;
		}

		return overrideCharData;
	};
	auto GetCharGeoXAdv = [&](TCHAR prevCharCode, TCHAR charCode, const RichTextParseResult& richTextResult)
	{
		if (IsRichTextImageSpace(charCode, richTextResult))
		{
			return richTextResult.size;//image use font size as width & height & xadvance
		}
		else
		{
			auto overrideFontSize = richText ? richTextResult.size : fontSize;
			auto charData = font->GetCharData(charCode, overrideFontSize);
			if (useKerning && prevCharCode != charCode)
			{
				auto kerning = font->GetKerning(prevCharCode, charCode, overrideFontSize);
				return charData.xadvance + kerning;
			}
			else
			{
				return charData.xadvance;
			}
		}
	};

	//pre parse rich text
	static TArray<RichTextParseResult> richTextPropertyArray;
	richTextPropertyArray.Reset();
	if (richText)
	{
		FString richTextContent;
		richTextContent.Reserve(content.Len());
		auto richTextCustomStyleData = uiComp->GetRichTextCustomStyleData();
		bool useCustomStyle = IsValid(richTextCustomStyleData);
		for (int charIndex = 0; charIndex < contentLength; charIndex++)
		{
			auto charCode = content[charIndex];
			richTextParseResult.customTag = NAME_None;
			richTextParseResult.customTagMode = CustomTagMode::None;
			richTextParseResult.charIndex = charIndex;
			richTextParser.ClearImageTag();
			while (richTextParser.Parse(content, contentLength, charIndex, richTextParseResult))
			{
				if (!richTextParseResult.imageTag.IsNone())//get image, append a blank placeholder
				{
					richTextContent.AppendChar(' ');
					richTextPropertyArray.Add(richTextParseResult);
					richTextParseResult.imageTag = NAME_None;//clear it
					richTextParser.ClearImageTag();
				}
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
			richTextParseResult.charIndex = charIndex;
			//convert custom tag to style
			if (useCustomStyle)
			{
				if (auto customStyleItemDataPtr = richTextCustomStyleData->GetDataMap().Find(richTextParseResult.customTag))
				{
					customStyleItemDataPtr->ApplyToRichTextParseResult(richTextParseResult);
				}
			}
			richTextPropertyArray.Add(richTextParseResult);
		}
		//replace text content with parsed rich text content
		content = richTextContent;
		contentLength = richTextContent.Len();
	}

	bool hasClampContent = false;
	int clamp_RestVerticesCount = 0;
	float clamp_CurrentLineWidth = 0;
	float clamp_ParagraphHeight = 0;
	TCHAR prevCharCode = '\0';//prev char code (not space or tab)
	for (int charIndex = 0; charIndex < contentLength; charIndex++)
	{
		auto charCode = content[charIndex];
		auto caretCharIndex = charIndex;
		if (richText)
		{
			richTextParseResult = richTextPropertyArray[charIndex];
			caretCharIndex = richTextParseResult.charIndex;
		}

		if (charCode == '\n' || charCode == '\r')//10 -- \n, 13 -- \r
		{
			NewLine(richText ? richTextParseResult.charIndex : charIndex, true);
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
			if (IsSpace(charCode, richTextParseResult))
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
		
		auto charGeo = GetCharGeo(charIndex == 0 ? charCode : prevCharCode, charCode, richText ? richTextParseResult.size : fontSize);
		//caret property
		caretPosition.X = currentLineOffset.X - halfFontSpaceX;
		caretPosition.Y = currentLineOffset.Y;
		FUITextCaretProperty caretProperty;
		caretProperty.caretPosition = caretPosition;
		caretProperty.charIndex = caretCharIndex;
		lineProperty.caretPropertyList.Add(caretProperty);

		caretPosition.X += fontSpace.X + charGeo.xadvance;//for line's last char's caret position

		if (IsSpace(charCode, richTextParseResult))//char is space
		{
			if (overflowType == UITextOverflowType::VerticalOverflow//char is space and UIText can have multi line, then we need to calculate if the following words can fit the rest space, if not means new line
				|| overflowType == UITextOverflowType::HorizontalAndVerticalOverflow
				)
			{
				auto prevCharCodeOfForwardChar = prevCharCode;
				float spaceNeeded = GetCharGeoXAdv(prevCharCodeOfForwardChar, charCode, richTextParseResult);
				prevCharCodeOfForwardChar = charCode;
				spaceNeeded += fontSpace.X;
				for (int forwardCharIndex = charIndex + 1, forwardVisibleCharIndex = currentVisibleCharCount; forwardCharIndex < contentLength && forwardVisibleCharIndex < visibleCharCount; forwardCharIndex++)
				{
					auto charCodeOfForwardChar = content[forwardCharIndex];
					if (IsSpace(charCodeOfForwardChar, richTextParseResult))//space
					{
						break;
					}
					if (charCodeOfForwardChar == '\n' || charCodeOfForwardChar == '\r' || charCodeOfForwardChar == '\t')//\n\r\t
					{
						break;
					}
					spaceNeeded += GetCharGeoXAdv(prevCharCodeOfForwardChar, charCodeOfForwardChar, richTextParseResult);
					spaceNeeded += fontSpace.X;
					forwardVisibleCharIndex++;
					prevCharCodeOfForwardChar = charCodeOfForwardChar;
				}
				float maxWidthToCompare = overflowType == UITextOverflowType::HorizontalAndVerticalOverflow ? maxHorizontalWidth : width;
				if (currentLineOffset.X + spaceNeeded > maxWidthToCompare)
				{
					NewLine(caretCharIndex, false);
					newLineMode = NewLineMode::Space;
					continue;
				}
			}
		}

		prevCharCode = charCode;
		//char geometry
		if (IsRichTextImageSpace(charCode, richTextParseResult))
		{
			FUIText_RichTextImageTag imageTagData;
			imageTagData.TagName = richTextParseResult.imageTag;
			imageTagData.Position = FVector2D(currentLineOffset.X + charGeo.xadvance * 0.5f, currentLineOffset.Y);
			imageTagData.Size = charGeo.xadvance;
			imageTagData.TintColor = richTextParseResult.hasColor ? richTextParseResult.color : FColor::White;
			cacheRichTextImageTagArray.Add(imageTagData);
			currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.size);
		}
		else
		{
			if (charCode != ' ' && charCode != '\t')//skip invisible char
			{
				if (richText)
				{
					currentLineHeight = FMath::Max(currentLineHeight, richTextParseResult.size);
				}

				int additionalVerticesCount, additionalIndicesCount;
				font->PushCharData(
					charCode, currentLineOffset, fontSpace, charGeo,
					richTextParseResult,
					verticesCount, indicesCount,
					additionalVerticesCount, additionalIndicesCount,
					originVertices, vertices, triangles
				);

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

				currentVisibleCharCount++;
			}
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
				customTag.CharIndexStart = currentVisibleCharCount - 1;//-1 as index
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
					cacheRichTextCustomTagArray[foundIndex].CharIndexEnd = currentVisibleCharCount - 1;//-1 as index
				}
			}
			break;
			}
		}

		currentLineOffset.X += charGeo.xadvance + fontSpace.X;
		currentLineWidth += charGeo.xadvance + fontSpace.X;

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
				int nextCharXAdv = GetCharGeoXAdv(content[charIndex], content[charIndex + 1], richText ? richTextPropertyArray[charIndex + 1] : richTextParseResult);
				if (currentLineOffset.X + nextCharXAdv > width)//if next char cannot fit this line, then add new line
				{
					auto nextChar = content[charIndex + 1];
					if (nextChar == '\r' || nextChar == '\n')
					{
						//next char is new line, no need to add new line
					}
					else
					{
						NewLine(caretCharIndex + 1, false);
						newLineMode = NewLineMode::Overflow;
						continue;
					}
				}
			}
			break;
			case UITextOverflowType::HorizontalAndVerticalOverflow:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				int nextCharXAdv = GetCharGeoXAdv(content[charIndex], content[charIndex + 1], richText ? richTextPropertyArray[charIndex + 1] : richTextParseResult);
				if (currentLineOffset.X + nextCharXAdv > maxHorizontalWidth)//if next char cannot fit max line, then add new line
				{
					auto nextChar = content[charIndex + 1];
					if (nextChar == '\r' || nextChar == '\n')
					{
						//next char is new line, no need to add new line
					}
					else
					{
						NewLine(caretCharIndex + 1, false);
						newLineMode = NewLineMode::Overflow;
						continue;
					}
				}
			}
			break;
			case UITextOverflowType::ClampContent:
			{
				if (charIndex + 1 == contentLength)continue;//last char
				if (hasClampContent)continue;

				int nextCharXAdv = GetCharGeoXAdv(content[charIndex], content[charIndex + 1], richText ? richTextPropertyArray[charIndex + 1] : richTextParseResult);
				if (currentLineOffset.X + nextCharXAdv > width)//horizontal cannot fit next char
				{
					hasClampContent = true;
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
				item.CharIndexEnd = currentVisibleCharCount - 1;//-1 as index
			}
		}
	}

	//clamp content
	if (hasClampContent)
	{
		//set rest char's position to invisible
		int allVerticesCount = originVertices.Num();
		for (int vertIndex = clamp_RestVerticesCount; vertIndex < allVerticesCount; vertIndex++)
		{
			originVertices[vertIndex].Position = FVector3f::ZeroVector;
		}

		currentLineWidth = clamp_CurrentLineWidth;
		paragraphHeight = clamp_ParagraphHeight;
	}

	//last line
	NewLine(richText ? text.Len() : contentLength, true); 
	//remove last line's space Y
	paragraphHeight -= fontSpace.Y;

	textRealSize.X = maxLineWidth;
	textRealSize.Y = paragraphHeight;

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
	for (auto& linePropertyItem : cacheLinePropertyArray)
	{
		for (auto& charItem : linePropertyItem.caretPropertyList)
		{
			charItem.caretPosition.X += xOffset;
			charItem.caretPosition.Y += yOffset;
		}
	}
	//image
	if (richText)
	{
		for (auto& imageItem : cacheRichTextImageTagArray)
		{
			imageItem.Position.X += xOffset;
			imageItem.Position.Y += yOffset;
		}
	}

	UIGeometry::OffsetVertices(originVertices, verticesCount, xOffset, yOffset);

	//snap pixel
	if (pixelPerfect)
	{
		AdjustPixelPerfectPos_For_UIText(originVertices, cacheCharPropertyArray, renderCanvas, uiComp);
	}
}

#pragma endregion

void UIGeometry::OffsetVertices(TArray<FLGUIOriginVertexData>& vertices, int count, float offsetX, float offsetY)
{
	for (int i = 0; i < count; i++)
	{
		auto& vertex = vertices[i].Position;
		vertex.Y += offsetX;
		vertex.Z += offsetY;
	}
}
void UIGeometry::UpdateUIColor(UIGeometry* uiGeo, const FColor& color)
{
	auto& vertices = uiGeo->vertices;
	for (int i = 0; i < vertices.Num(); i++)
	{
		vertices[i].Color = color;
	}
}

void UIGeometry::CalculatePivotOffset(
	const float& width, const float& height, const FVector2f& pivot
	, float& pivotOffsetX, float& pivotOffsetY
)
{
	pivotOffsetX = width * (0.5f - pivot.X);//width * 0.5f *(1 - pivot.X * 2)
	pivotOffsetY = height * (0.5f - pivot.Y);//height * 0.5f *(1 - pivot.Y * 2)
}

void UIGeometry::CalculateOffsetAndSize(
	const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo
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
void UIGeometry::TransformVertices(ULGUICanvas* canvas, UUIBaseRenderable* item, UIGeometry* uiGeo)
{
	SCOPE_CYCLE_COUNTER(STAT_TransformVertices);

	auto& vertices = uiGeo->vertices;
	auto& originVertices = uiGeo->originVertices;
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
	
	FLGUICacheTransformContainer tempTf;
	canvas->GetCacheUIItemToCanvasTransform(item, tempTf);
	auto itemToCanvasTf = tempTf.Transform;


	for (int i = 0; i < vertexCount; i++)
	{
		vertices[i].Position = FVector3f(itemToCanvasTf.TransformPosition(FVector(originVertices[i].Position)));
	}

	if (canvas->GetRequireNormal())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentZ = itemToCanvasTf.TransformVector(FVector(originVertices[i].Normal));
			vertices[i].TangentZ.Vector.W = -127;
		}
	}
	if (canvas->GetRequireTangent())
	{
		for (int i = 0; i < vertexCount; i++)
		{
			vertices[i].TangentX = itemToCanvasTf.TransformVector(FVector(originVertices[i].Tangent));
		}
	}
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
UE_ENABLE_OPTIMIZATION
#endif
