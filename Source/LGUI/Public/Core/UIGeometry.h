// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UISprite.h"
#include "Core/LGUIIndexBuffer.h"
#include "Core/LGUIMeshVertex.h"

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
		Position = FVector::ZeroVector;
		Normal = FVector(-1, 0, 0);
		Tangent = FVector(0, 1, 0);
	}
	FLGUIOriginVertexData(FVector InPosition)
	{
		Position = InPosition;
		Normal = FVector(-1, 0, 0);
		Tangent = FVector(0, 1, 0);
	}
	FLGUIOriginVertexData(FVector InPosition, FVector InNormal, FVector InTangent)
	{
		Position = InPosition;
		Normal = InNormal;
		Tangent = InTangent;
	}
	FVector Position;
	FVector Normal;
	FVector Tangent;
};

class LGUI_API UIGeometry
{
public:
	//local space vertex position/ normal/ tangent
	TArray<FLGUIOriginVertexData> originVertices;
	//vertex buffer, position/normal/tangent is stored as transformed space(Canvas space), origin position/normal/tangent is stored in originVertices/originNormals/originTangents
	TArray<FLGUIMeshVertex> vertices;
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
		FMemory::Memcpy(Target->vertices.GetData(), vertices.GetData(), vertices.Num() * sizeof(FLGUIMeshVertex));

		bool triangleCountChanged = false;
		if (triangles.Num() != Target->triangles.Num())
		{
			Target->triangles.SetNumUninitialized(triangles.Num());
			triangleCountChanged = true;
		}
		FMemory::Memcpy(Target->triangles.GetData(), triangles.GetData(), triangles.Num() * sizeof(FLGUIIndexType));

		return verticesCountChanged || triangleCountChanged;
	}

	/**
	 * Unlike default TArray's SetNum, this LGUIGeometrySetArrayNum only Construct new item when get new memory.
	 * SetNum will Construct item from Num to NewNum, include old existing memory (memory between Num and Max), which is not what I want.
	 * What I want is, use default value only on new memory, so new item will not contains NaN value.
	 */
	template<class T>
	static void LGUIGeometrySetArrayNum(TArray<T>& InArray, int32 NewNum, bool bAllowShrinking = true)
	{
		auto PrevMax = InArray.Max();
		if (NewNum > InArray.Max())
		{
			InArray.AddUninitialized(InArray.Max() - InArray.Num());//Set Num to Max and can keep existing memory.
			InArray.SetNumZeroed(NewNum, bAllowShrinking);//New memory will be Zeroed.
		}
		else
		{
			InArray.SetNumUninitialized(NewNum, bAllowShrinking);
		}
		//SetNum could change array max, so memzero the additional memory
		if (InArray.Max() > PrevMax)
		{
			FMemory::Memzero((uint8*)InArray.GetData() + PrevMax * sizeof(T), (InArray.Max() - PrevMax) * sizeof(T));
		}
	}

#pragma region UISprite_UITexture_Simple
public:
	static void UpdateUIRectSimpleVertex(UIGeometry* uiGeo, 
		const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
	static void UpdateUIProceduralRectSimpleVertex(UIGeometry* uiGeo,
		bool bEnableBody,
		bool bOuterShadow, const FVector2f& outerShadowOffset, const float& outerShadowSize, const float& outerShadowBlur, bool bSoftEdge,
		const float& width, const float& height, const FVector2f& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void UpdateUIRectBorderVertex(UIGeometry* uiGeo, bool fillCenter,
		const float& width, const float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void UpdateUIRectTiledVertex(UIGeometry* uiGeo, 
		const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Horizontal_Vertical
public:
	static void UpdateUIRectFillHorizontalVerticalVertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
		, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial90
public:
	static void UpdateUIRectFillRadial90Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
		, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial180
public:
	static void UpdateUIRectFillRadial180Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
		, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial360
public:
	static void UpdateUIRectFillRadial360Vertex(UIGeometry* uiGeo, const float& width, const float& height, const FVector2D& pivot
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
		, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp, const FColor& color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UIText
public:
	static void UpdateUIText(const FString& text, int32 visibleCharCount, float width, float height, const FVector2D& pivot
		, const FColor& color, uint8 canvasGroupAlpha, const FVector2D& fontSpace, UIGeometry* uiGeo, float fontSize
		, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
		, float maxHorizontalWidth, bool kerning
		, UITextFontStyle fontStyle, FVector2D& textRealSize
		, ULGUICanvas* renderCanvas, class UUIText* uiComp
		, TArray<FUITextLineProperty>& cacheLinePropertyArray, TArray<FUITextCharProperty>& cacheCharPropertyArray, TArray<FUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
		, TArray<FUIText_RichTextImageTag>& cacheRichTextImageTagArray
		, ULGUIFontData_BaseObject* font, bool richText, int32 richTextFilterFlags);
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
	static void AdjustPixelPerfectPos(
		TArray<FLGUIOriginVertexData>& originVertices, int startIndex, int count
		, ULGUICanvas* renderCanvas, UUIBaseRenderable* uiComp
	);
private:
	static void OffsetVertices(TArray<FLGUIOriginVertexData>& vertices, int count, float offsetX, float offsetY);
};
