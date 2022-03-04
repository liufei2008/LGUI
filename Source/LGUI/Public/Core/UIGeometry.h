﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UISprite.h"
#include "Extensions/UIPolygon.h"
#include "Core/LGUIIndexBuffer.h"

struct FLGUISpriteInfo;
struct FUITextLineProperty;
class ULGUIFontData_BaseObject;
class ULGUICanvas;
class UUIItem;
class UUIBaseRenderable;

/** Origin position/ normal/ tangent stored in UI item's local space */
struct FLGUIOriginVertexData
{
public:
	FLGUIOriginVertexData()
	{
		Position = FVector3f::ZeroVector;
		Normal = FVector3f(-1, 0, 0);
		Tangent = FVector3f(0, 1, 0);
	}
	FLGUIOriginVertexData(FVector3f InPosition)
	{
		Position = InPosition;
		Normal = FVector3f(-1, 0, 0);
		Tangent = FVector3f(0, 1, 0);
	}
	FLGUIOriginVertexData(FVector3f InPosition, FVector3f InNormal, FVector3f InTangent)
	{
		Position = InPosition;
		Normal = InNormal;
		Tangent = InTangent;
	}
	FVector3f Position;
	FVector3f Normal;
	FVector3f Tangent;
};

class LGUI_API UIGeometry
{
public:
	//local space vertex position/ normal/ tangent
	TArray<FLGUIOriginVertexData> originVertices;
	//vertex buffer, position/normal/tangent is stored as transformed space(Canvas space), origin position/normal/tangent is stored in originVertices/originNormals/originTangents
	TArray<FDynamicMeshVertex> vertices;
	//triangle indices
	TArray<FLGUIIndexType> triangles;

	TWeakObjectPtr<UTexture> texture = nullptr;
	TWeakObjectPtr<UMaterialInterface> material = nullptr;

	/** 
	 * Clear vertices and triangle indices data and keep memory, so when the data array do SetNumUninitialized (or similar function, which just change num by not memory), the origin data is still there.
	 * eg. The following lines use InTriangleChanged to tell if we need to set actual data in triangles, after SetNumUninitialized, the old triangles value is good to use.
	 *		
			auto& triangles = uiGeo->triangles;
			triangles.SetNumUninitialized(6);
			if (InTriangleChanged)
			{
				triangles[0] = 0;
				triangles[1] = 3;
				triangles[2] = 2;
				triangles[3] = 0;
				triangles[4] = 1;
				triangles[5] = 3;
			}
	 */
	void Clear()
	{
		vertices.Reset();
		triangles.Reset();
		originVertices.Reset();
	}
	/**
	 * Fill this data to other.
	 * @return true if any data size changed, false otherwise
	 */
	bool CopyTo(UIGeometry* Target)
	{
		bool verticesCountChanged = false;
		if (vertices.Num() != Target->vertices.Num())
		{
			Target->vertices.SetNumUninitialized(vertices.Num());
			Target->originVertices.SetNumUninitialized(vertices.Num());
			verticesCountChanged = true;
		}
		FMemory::Memcpy(Target->originVertices.GetData(), originVertices.GetData(), originVertices.Num() * sizeof(FLGUIOriginVertexData));
		FMemory::Memcpy(Target->vertices.GetData(), vertices.GetData(), vertices.Num() * sizeof(FDynamicMeshVertex));

		bool triangleCountChanged = false;
		if (triangles.Num() != Target->triangles.Num())
		{
			Target->triangles.SetNumUninitialized(triangles.Num());
			triangleCountChanged = true;
		}
		FMemory::Memcpy(Target->triangles.GetData(), triangles.GetData(), triangles.Num() * sizeof(FLGUIIndexType));

		return verticesCountChanged || triangleCountChanged;
	}

#pragma region UISprite_UITexture_Simple
public:
	static void UpdateUIRectSimpleVertex(UIGeometry* uiGeo, 
		const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void UpdateUIRectBorderVertex(UIGeometry* uiGeo, bool fillCenter,
		const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void UpdateUIRectTiledVertex(UIGeometry* uiGeo, 
		const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const float& width, const float& height, const FVector2f& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Horizontal_Vertical
public:
	static void UpdateUIRectFillHorizontalVerticalVertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial90
public:
	static void UpdateUIRectFillRadial90Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial180
public:
	static void UpdateUIRectFillRadial180Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial360
public:
	static void UpdateUIRectFillRadial360Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2f& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UIText
public:
	static void UpdateUIText(const FString& text, int32 visibleCharCount, float& width, float& height, const FVector2f& pivot
		, const FColor& color, const FVector2f& fontSpace, UIGeometry* uiGeo, const float& fontSize
		, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
		, bool adjustWidth, bool adjustHeight
		, UITextFontStyle fontStyle, FVector2f& textRealSize
		, ULGUICanvas* renderCanvas, UUIItem* uiComp
		, TArray<FUITextLineProperty>& cacheTextPropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
		, ULGUIFontData_BaseObject* font, bool richText);
private:
	static void AlignUITextLineVertex(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart, TArray<FLGUIOriginVertexData>& vertices, FUITextLineProperty& sentenceProperty);
	static void AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineHeight, float fontSize, int lineUIGeoVertStart, TArray<FLGUIOriginVertexData>& vertices);
#pragma endregion

public:
	static void UpdateUIColor(UIGeometry* uiGeo, const FColor& color);
	static void TransformVertices(class ULGUICanvas* canvas, class UUIBaseRenderable* item, UIGeometry* uiGeo);
	static void CalculatePivotOffset(
		const float& width, const float& height, const FVector2f& pivot
		, float& pivotOffsetX, float& pivotOffsetY
	);
	static void CalculateOffsetAndSize(
		const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo
		, float& pivotOffsetX, float& pivotOffsetY, float& halfWidth, float& halfHeight
	);
private:
	static void OffsetVertices(TArray<FLGUIOriginVertexData>& vertices, int count, float offsetX, float offsetY);
};
