// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Components/LexText.h"
#include "Components/LexSprite.h"
#include "Core/LexUIMeshIndex.h"
#include "Core/LexUIMeshVertex.h"

struct FLexUISpriteInfo;
struct FLexUITextLineProperty;
class ULexUIFontData_BaseObject;
class ULexCanvas;
class ULexWidget;
class ULexVisual;

/** Origin position/ normal/ tangent stored in UI item's local space */
struct FLexUIOriginVertexData
{
public:
	FLexUIOriginVertexData()
	{
		Position = FVector3f::ZeroVector;
		Normal = FVector3f(-1, 0, 0);
		Tangent = FVector3f(0, 1, 0);
	}
	FLexUIOriginVertexData(FVector3f InPosition)
	{
		Position = InPosition;
		Normal = FVector3f(-1, 0, 0);
		Tangent = FVector3f(0, 1, 0);
	}
	FLexUIOriginVertexData(FVector3f InPosition, FVector3f InNormal, FVector3f InTangent)
	{
		Position = InPosition;
		Normal = InNormal;
		Tangent = InTangent;
	}
	FVector3f Position;
	FVector3f Normal;
	FVector3f Tangent;
};

class LGUI_API FLexUIGeometry
{
public:
	FLexUIGeometry() = default;

	// Explicit copy constructor
	FLexUIGeometry(const FLexUIGeometry& Other)
		: OriginVertices(Other.OriginVertices),
		  Vertices(Other.Vertices),
		  Triangles(Other.Triangles),
		  Texture(Other.Texture),
		  Material(Other.Material),
		  bIsFont(Other.bIsFont),
		  bSupportDrawcallBatching(Other.bSupportDrawcallBatching),
		  TransformRelativeToCanvas(Other.TransformRelativeToCanvas),
		  BoundsMin2DInCanvasSpace(Other.BoundsMin2DInCanvasSpace),
		  BoundsMax2DInCanvasSpace(Other.BoundsMax2DInCanvasSpace)
	{
		
	}

	// Explicit assignment operator
	FLexUIGeometry& operator=(const FLexUIGeometry& Other)
	{
		if (this != &Other)
		{
			OriginVertices = Other.OriginVertices;
			Vertices = Other.Vertices;
			Triangles = Other.Triangles;
			Texture = Other.Texture;
			Material = Other.Material;
			bIsFont = Other.bIsFont;
			bSupportDrawcallBatching = Other.bSupportDrawcallBatching;
			TransformRelativeToCanvas = Other.TransformRelativeToCanvas;
			BoundsMin2DInCanvasSpace = Other.BoundsMin2DInCanvasSpace;
			BoundsMax2DInCanvasSpace = Other.BoundsMax2DInCanvasSpace;
		}
		return *this;
	}

	//is calculating vertices?
	std::atomic<bool> bIsCalculating = false;
	//local space vertex position/ normal/ tangent
	TArray<FLexUIOriginVertexData> OriginVertices;
	//vertex buffer, position/normal/tangent is stored as transformed space(Canvas space), origin position/normal/tangent is stored in originVertices/originNormals/originTangents
	TArray<FLexUIMeshVertex> Vertices;
	//triangle indices
	TArray<FLexUIMeshIndex> Triangles;

	TWeakObjectPtr<UTexture> Texture = nullptr;
	TWeakObjectPtr<UMaterialInterface> Material = nullptr;
	bool bIsFont = false;
	bool bSupportDrawcallBatching = true;

	FTransform TransformRelativeToCanvas;
	FVector2D BoundsMin2DInCanvasSpace;
	FVector2D BoundsMax2DInCanvasSpace;

	void CopyDataForPrepare(const FLexUIGeometry& Other)
	{
		Vertices.SetNumUninitialized(Other.Vertices.Num());
		FMemory::Memcpy(Vertices.GetData(), Other.Vertices.GetData(), Other.Vertices.Num() * sizeof(FLexUIMeshVertex));
		Triangles.SetNumUninitialized(Other.Triangles.Num());
		FMemory::Memcpy(Triangles.GetData(), Other.Triangles.GetData(), Other.Triangles.Num() * sizeof(FLexUIMeshIndex));
		
		Texture = Other.Texture;
		Material = Other.Material;
		bIsFont = Other.bIsFont;
		bSupportDrawcallBatching = Other.bSupportDrawcallBatching;
		
		BoundsMin2DInCanvasSpace = Other.BoundsMin2DInCanvasSpace;
		BoundsMax2DInCanvasSpace = Other.BoundsMax2DInCanvasSpace;
	}

	/** 
	 * Clear vertices and triangle indices data and keep memory, so when the data array do SetNumUninitialized (or similar function, which just change num but not memory), the origin data is still there.
	 * e.g. The following lines use InTriangleChanged to tell if we need to set actual data in triangles, after SetNumUninitialized, the old triangles value is good to use.
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
		Vertices.Reset();
		Triangles.Reset();
		OriginVertices.Reset();
	}

	/**
	 * Unlike default TArray's SetNum, this function only Construct new item when get new memory.
	 * SetNum will Construct item from Num to NewNum, include old existing memory (memory between Num and Max), which is not what I want.
	 * What I want is, use default value only on new memory, so new item will not contain NaN value.
	 */
	template<class T>
	static void LexUIGeometrySetArrayNum(TArray<T>& InArray, int32 NewNum, bool bAllowShrinking = true)
	{
		auto PrevMax = InArray.Max();
		if (NewNum > InArray.Max())
		{
			InArray.AddUninitialized(InArray.Max() - InArray.Num());//Set Num to Max and can keep existing memory.
			InArray.SetNumZeroed(NewNum, bAllowShrinking ? EAllowShrinking::Yes : EAllowShrinking::No);//New memory will be Zeroed.
		}
		else
		{
			InArray.SetNumUninitialized(NewNum, bAllowShrinking ? EAllowShrinking::Yes : EAllowShrinking::No);
		}
		//SetNum could change array max, so mem-zero the additional memory
		if (InArray.Max() > PrevMax)
		{
			FMemory::Memzero((uint8*)InArray.GetData() + PrevMax * sizeof(T), (InArray.Max() - PrevMax) * sizeof(T));
		}
	}

#pragma region UISprite_UITexture_Simple
public:
	static void UpdateUIRectSimpleVertex(FLexUIGeometry* uiGeo, 
		float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
	static void UpdateRectBlockVertex(FLexUIGeometry* uiGeo,
		bool bEnableOuterShadow, FVector2f outerShadowOffset, float outerShadowSize, float outerShadowBlur, bool bSoftEdge,
		float width, float height, FVector2f pivot, 
		const FLexUISpriteInfo& uniformSpriteInfo, const FLexUISpriteInfo& spriteInfo,
		ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void UpdateUIRectBorderVertex(FLexUIGeometry* uiGeo, bool fillCenter,
		float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		float pixelsPerUnitMultiplier,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void UpdateUIRectTiledVertex(FLexUIGeometry* uiGeo, 
		const FLexUISpriteInfo& spriteInfo, ULexCanvas* renderCanvas, ULexVisual* uiComp, float width, float height, FVector2f pivot, const int& widthRectCount, const int& heightRectCount, float widthRemainedRectSize, float heightRemainedRectSize, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Horizontal_Vertical
public:
	static void UpdateUIRectFillHorizontalVerticalVertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
		, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
		, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial90
public:
	static void UpdateUIRectFillRadial90Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
		, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial90 originType
		, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial180
public:
	static void UpdateUIRectFillRadial180Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
		, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial180 originType
		, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UISprite_Fill_Radial360
public:
	static void UpdateUIRectFillRadial360Vertex(FLexUIGeometry* uiGeo, float width, float height, FVector2f pivot
		, const FLexUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, ELexUISpriteFillOriginType_Radial360 originType
		, ULexCanvas* renderCanvas, ULexVisual* uiComp, FColor color,
		bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged
	);
#pragma endregion
#pragma region UIText
public:
	static void UpdateUIText(const FString& Content
		, TArray<FLexUIText_TextProcessingElement>& TextProcessingArray
		, float width, float height, FVector2f pivot
		, FColor color, uint8 RenderOpacityForRichText, FVector2f fontSpace, FLexUIGeometry* uiGeo, float fontSize
		, ELexUITextParagraphHorizontalAlign paragraphHAlign, ELexUITextParagraphVerticalAlign paragraphVAlign, ELexUITextOverflowType overflowType
		, ETextWrappingPolicy wrappingPolicy, bool bUseKerning
		, ELexUITextFontStyle fontStyle, FVector2f& textPreferredSize, bool& outTruncated
		, ULexCanvas* renderCanvas, class ULexText* lexText
		, TArray<FLexUITextLineProperty>& cacheLinePropertyArray, TArray<FLexUITextCharProperty>& cacheCharPropertyArray, TArray<FLexUIText_RichTextCustomTag>& cacheRichTextCustomTagArray
		, TArray<FLexUIText_RichTextImageTag>& cacheRichTextImageTagArray
		, TArray<FLexUIText_Emoji>& cacheEmojiArray
		, ULexUIFontData_BaseObject* font, bool bRichText, int32 richTextFilterFlags);
#pragma endregion

public:
	static void UpdateUIColor(FLexUIGeometry* uiGeo, FColor color);
	static void TransformVertices(class ULexCanvas* canvas, class ULexVisual* item, FLexUIGeometry* uiGeo);
	static void CalculatePivotOffset(
		float width, float height, FVector2f pivot
		, float& pivotOffsetX, float& pivotOffsetY
	);
	static void CalculateOffsetAndSize(
		float width, float height, FVector2f pivot, const FLexUISpriteInfo& spriteInfo
		, float& pivotOffsetX, float& pivotOffsetY, float& halfWidth, float& halfHeight
	);
	static void AdjustPixelPerfectPos(
		TArray<FLexUIOriginVertexData>& originVertices, int startIndex, int count
		, ULexCanvas* RenderCanvas, ULexVisual* Visual
	);
private:
	static void OffsetVertices(TArray<FLexUIOriginVertexData>& vertices, int count, float offsetX, float offsetY);
};
