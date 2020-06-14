// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/UISprite.h"

struct FLGUISpriteInfo;
struct FUITextLineProperty;
class ULGUIFontData;
class ULGUICanvas;
class UUIItem;

class LGUI_API UIGeometry
{
public:
	//these two parameters below for store vertex and triangle count of origin data. after GeometryModifier if add new vertex or triangles, we can find origin vertex and triangle by using these two parameters
	int32 originVerticesCount = 0;//origin vertices count
	int32 originTriangleCount = 0;//origin triangle indices count

	//local space vertex position
	TArray<FVector> originPositions;
	//local space vertex normal
	TArray<FVector> originNormals;
	//local space vertex tangent
	TArray<FVector> originTangents;
	//vertex buffer, position/normal/tangent is stored as transformed space(Canvas space), origin position/normal/tangent is stored in originPositions/originNormals/originTangents
	TArray<FDynamicMeshVertex> vertices;
	//triangle indices
	TArray<uint16> triangles;

	UTexture* texture = nullptr;
	UMaterialInterface* material = nullptr;
	int depth;//depth of this UIRenderable
	int drawcallIndex = -1;//index of drawcall(which collect this geometry for render) in drawcall list, -1 means not add to drawcall yet
	bool isFontTexture = false;//the texture of this geometry is font texture or not

	void Clear()
	{
		originVerticesCount = 0;
		originTriangleCount = 0;
		vertices.Reset();
		triangles.Reset();
		originPositions.Reset();
		originNormals.Reset();
		originTangents.Reset();
	}

#pragma region UISprite_UITexture_Simple
public:
	static void FromUIRectSimple(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectSimpleUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo);
	static void UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void FromUIRectBorder(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp
		, bool fillCenter);
	static void UpdateUIRectBorderUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo);
	static void UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void FromUIRectTiled(const float& width, const float& height, const FVector2D& pivot, const FColor& color, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectTiledUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize);
	static void UpdateUIRectTiledVertex(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, ULGUICanvas* renderCanvas, UUIItem* uiComp, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize);
#pragma endregion
#pragma region UISprite_Fill_Horizontal_Vertical
public:
	static void FromUIRectFillHorizontalVertical(float& width, float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, bool horizontalOrVertical
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectFillHorizontalVerticalVertex(float& width, float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount
		, bool horizontalOrVertical, bool updatePosition, bool updateUV
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion
#pragma region UISprite_Fill_Radial90
public:
	static void FromUIRectFillRadial90(float& width, float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectFillRadial90Vertex(float& width, float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial90 originType
		, bool updatePosition, bool updateUV
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion
#pragma region UISprite_Fill_Radial180
public:
	static void FromUIRectFillRadial180(float& width, float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectFillRadial180Vertex(float& width, float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial180 originType
		, bool updatePosition, bool updateUV
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion
#pragma region UISprite_Fill_Radial360
public:
	static void FromUIRectFillRadial360(float& width, float& height, const FVector2D& pivot, const FColor& color, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
	static void UpdateUIRectFillRadial360Vertex(float& width, float& height, const FVector2D& pivot, TSharedPtr<UIGeometry> uiGeo
		, const FLGUISpriteInfo& spriteInfo, bool flipDirection, float fillAmount, UISpriteFillOriginType_Radial360 originType
		, bool updatePosition, bool updateUV
		, ULGUICanvas* renderCanvas, UUIItem* uiComp);
#pragma endregion



#pragma region UIText
public:
	static void FromUIText(const FString& text, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
		, const FColor& color, const FVector2D& fontSpace, TSharedPtr<UIGeometry> uiGeo, const float& fontSize
		, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
		, bool adjustWidth, bool adjustHeight
		, UITextFontStyle fontStyle, FVector2D& textRealSize
		, ULGUICanvas* renderCanvas, UUIItem* uiComp
		, TArray<FUITextLineProperty>& cacheTextPropertyList, ULGUIFontData* font, bool richText);
	static void UpdateUIText(const FString& text, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot
		, const FColor& color, const FVector2D& fontSpace, TSharedPtr<UIGeometry> uiGeo, const float& fontSize
		, UITextParagraphHorizontalAlign paragraphHAlign, UITextParagraphVerticalAlign paragraphVAlign, UITextOverflowType overflowType
		, bool adjustWidth, bool adjustHeight
		, UITextFontStyle fontStyle, FVector2D& textRealSize
		, ULGUICanvas* renderCanvas, UUIItem* uiComp
		, TArray<FUITextLineProperty>& cacheTextPropertyList, ULGUIFontData* font, bool richText);
private:
	static void AlignUITextLineVertex(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, int lineUIGeoVertStart, TArray<FVector>& vertices, FUITextLineProperty& sentenceProperty);
	static void AlignUITextLineVertexForRichText(UITextParagraphHorizontalAlign pivotHAlign, float lineWidth, float lineHeight, float fontSize, int lineUIGeoVertStart, TArray<FVector>& vertices);
#pragma endregion

#pragma region UISector
public:
	static void FromUISector(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUISectorUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, const FLGUISpriteInfo& spriteInfo);
	static void UpdateUISectorVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment);
#pragma endregion

#pragma region UIRing
public:
	static void FromUIRing(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, float ringWidth, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo& spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUIRingUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, float ringWidth, float& width, float& height, const FLGUISpriteInfo& spriteInfo);
	static void UpdateUIRingVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, float ringWidth);
#pragma endregion

public:
	static void UpdateUIColor(TSharedPtr<UIGeometry> uiGeo, FColor color);
	static void TransformVertices(class ULGUICanvas* canvas, class UUIRenderable* item, TSharedPtr<UIGeometry> uiGeo);
	static void CalculatePivotOffset(const float& width, const float& height, const FVector2D& pivot, float& pivotOffsetX, float& pivotOffsetY, float& halfW, float& halfH);
private:
	static void OffsetVertices(TArray<FVector>& vertices, int count, float offsetX, float offsetY);
};
