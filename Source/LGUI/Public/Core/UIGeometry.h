// Copyright 2019-2022 LexLiu. All Rights Reserved.

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

class LGUI_API UIGeometry
{
public:
	//local space vertex position
	TArray<FVector> originPositions;
	//local space vertex normal
	TArray<FVector> originNormals;
	//local space vertex tangent
	TArray<FVector> originTangents;
	//vertex buffer, position/normal/tangent is stored as transformed space(Canvas space), origin position/normal/tangent is stored in originPositions/originNormals/originTangents
	TArray<FDynamicMeshVertex> vertices;
	//triangle indices
	TArray<FLGUIIndexType> triangles;

	TWeakObjectPtr<UTexture> texture = nullptr;
	TWeakObjectPtr<UMaterialInterface> material = nullptr;

	/** 
	 * Clear data and keep memory, so when the data array do SetNumUninitialized (or similar function, which just change num by not memory), the origin data is still there.
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
		originPositions.Reset();
		originNormals.Reset();
		originTangents.Reset();
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
			Target->originPositions.SetNumUninitialized(vertices.Num());
			Target->originNormals.SetNumUninitialized(originNormals.Num());
			Target->originTangents.SetNumUninitialized(originTangents.Num());
			verticesCountChanged = true;
		}
		FMemory::Memcpy(Target->originPositions.GetData(), originPositions.GetData(), originPositions.Num() * sizeof(FVector));
		FMemory::Memcpy(Target->originNormals.GetData(), originNormals.GetData(), originNormals.Num() * sizeof(FVector));
		FMemory::Memcpy(Target->originTangents.GetData(), originTangents.GetData(), originTangents.Num() * sizeof(FVector));
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
		const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void UpdateUIRectBorderVertex(UIGeometry* uiGeo, bool fillCenter,
		const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void UpdateUIRectTiledVertex(UIGeometry* uiGeo, 
		const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Horizontal_Vertical
public:
	static void UpdateUIRectFillHorizontalVerticalVertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial90
public:
	static void UpdateUIRectFillRadial90Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial180
public:
	static void UpdateUIRectFillRadial180Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial360
public:
	static void UpdateUIRectFillRadial360Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UIText
public:
	static void UpdateUIText(const FString& text, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
		, const FColor& color, const FVector2D& fontSpace, UIGeometry* uiGeo, const float& fontSize
		, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
		, bool adjustWidth, bool adjustHeight
		, UITextFontStyle fontStyle, FVector2D& textRealSize
		, ULGUICanvas* renderCanvas, UUIItem* uiComp
		, TArray<FUITextLineProperty>& cacheTextPropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
		, ULGUIFontData_BaseObject* font, bool richText);
private:
	static void AlignUITextLineVertex(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart, TArray<FVector>& vertices, FUITextLineProperty& sentenceProperty);
	static void AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineHeight, float fontSize, int lineUIGeoVertStart, TArray<FVector>& vertices);
#pragma endregion

public:
	static void UpdateUIColor(UIGeometry* uiGeo, const FColor& color);
	static void TransformVertices(class ULGUICanvas* canvas, class UUIBaseRenderable* item, UIGeometry* uiGeo);
	static void CalculatePivotOffset(
		const float& width, const float& height, const FVector2D& pivot
		, float& pivotOffsetX, float& pivotOffsetY
	);
	static void CalculateOffsetAndSize(
		const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo
		, float& pivotOffsetX, float& pivotOffsetY, float& halfWidth, float& halfHeight
	);
private:
	static void OffsetVertices(TArray<FVector>& vertices, int count, float offsetX, float offsetY);
};
