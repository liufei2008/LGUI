// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Core/LGUISpriteData.h"
#include "Core/LGUIFontData.h"
#include "LGUI.h"

class LGUI_API UIGeometry
{
private:	
	TArray<FVector> transformedVertices;//vertex position transformed in Canvas space
	TArray<FVector> transformedNormals;//vertex normal transformed in Canvas space
	TArray<FVector> transformedTangents;//vertex tangent transformed in Canvas space
public:
	//these two parameters below for store vertex and triangle count of origin data. after GeometryModifier if add new vertex or triangles, we can find origin vertex and triangle by using these two parameters
	int32 originVerticesCount = 0;//origin vertices count
	int32 originTriangleCount = 0;//origin triangle indices count

	TArray<FVector> vertices;//vertex position
	TArray<uint16> triangles;//triangle indices
	TArray<FVector2D> uvs;//vertex texcoordinate 0
	TArray<FVector2D> uvs1;//vertex texcoordinate 1
	TArray<FVector2D> uvs2;//vertex texcoordinate 1
	TArray<FVector2D> uvs3;//vertex texcoordinate 1
	TArray<FVector> normals;//vertex normal
	TArray<FVector> tangents;//vertex tangent
	TArray<FColor> colors;//vertex color
	UTexture* texture = nullptr;
	UMaterialInterface* material = nullptr;
	int depth;//depth of this UIRenderable
	int drawcallIndex = -1;//index of drawcall(which collect this geometry for render) in drawcall list, -1 means not add to drawcall yet
	bool isFontTexture = false;//the texture of this geometry is font texture or not

	const TArray<FVector>& GetTransformedVertices() { return transformedVertices; }
	const TArray<FVector>& GetTransformedNormals() { return transformedNormals; }
	const TArray<FVector>& GetTransformedTangents() { return transformedTangents; }

	void Clear()
	{
		vertices.Empty();
		triangles.Empty();
		uvs.Empty();
		uvs1.Empty();
		uvs2.Empty();
		uvs3.Empty();
		normals.Empty();
		tangents.Empty();
		colors.Empty();
		transformedVertices.Empty();
		transformedNormals.Empty();
		transformedTangents.Empty();
	}

	bool CheckDataValid()
	{
		if (vertices.Num() != uvs.Num())
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexUVCount:%d!"), vertices.Num(), uvs.Num());
			return false;
		}
		if (vertices.Num() != colors.Num())
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexColorCount:%d!"), vertices.Num(), colors.Num());
			return false;
		}
		if (vertices.Num() != uvs1.Num() && uvs1.Num() != 0)
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexUV1Count:%d!"), vertices.Num(), uvs1.Num());
			return false;
		}
		if (vertices.Num() != uvs2.Num() && uvs2.Num() != 0)
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexUV2Count:%d!"), vertices.Num(), uvs2.Num());
			return false;
		}
		if (vertices.Num() != uvs3.Num() && uvs3.Num() != 0)
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexUV3Count:%d!"), vertices.Num(), uvs3.Num());
			return false;
		}
		if (vertices.Num() != normals.Num() && normals.Num() != 0)
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexNormalCount:%d!"), vertices.Num(), normals.Num());
			return false;
		}
		if (vertices.Num() != tangents.Num() && tangents.Num() != 0)
		{
			UE_LOG(LGUI, Error, TEXT("[UIGeometry::CheckDataValid]VertexPosition:%d not equal VertexTangentCount:%d!"), vertices.Num(), tangents.Num());
			return false;
		}
		return true;
	}

#pragma region UISprite_UITexture_Simple
public:
	static void FromUIRectSimple(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUIRectSimpleUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo = nullptr);
	static void UpdateUIRectSimpleVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot);
#pragma endregion
#pragma region UISprite_UITexture_Border
public:
	static void FromUIRectBorder(float& width, float& height, const FVector2D& pivot, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool fillCenter, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUIRectBorderUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo = nullptr);
	static void UpdateUIRectBorderVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, const FLGUISpriteInfo* spriteInfo);
#pragma endregion
#pragma region UISprite_Tiled
public:
	static void FromUIRectTiled(const float& width, const float& height, const FVector2D& pivot, const FColor& color, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUIRectTiledUV(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize);
	static void UpdateUIRectTiledVertex(TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, const float& width, const float& height, const FVector2D& pivot, const int& widthRectCount, const int& heightRectCount, const float& widthRemainedRectSize, const float& heightRemainedRectSize);
#pragma endregion

#pragma region UIText
public:
	static void FromUIText(FString& content, int32 visibleCharCount, float& width, float& height, const FVector2D& pivot, FColor color, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, int fontSize, uint8 paragraphHAlign, uint8 paragraphVAlign, uint8 overflowType, bool adjustWidth, bool adjustHeight, uint8 fontStyle, FVector2D& textRealSize, float dynamicPixelsPerUnit, TArray<struct FUITextLineProperty>& cacheTextPropertyList, const TArray<struct FUITextCharGeometry>& cacheTextGeometryList, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUITextVertexOrUV(FString& content, float& width, float& height, const FVector2D& pivot, FVector2D fontSpace, TSharedPtr<UIGeometry> uiGeo, int fontSize, uint8 paragraphHAlign, uint8 paragraphVAlign, uint8 overflowType, bool adjustWidth, bool adjustHeight, uint8 fontStyle, FVector2D& textRealSize, float dynamicPixelsPerUnit, bool updateVertex, bool updateUV, TArray<struct FUITextLineProperty>& cacheTextPropertyList, const TArray<struct FUITextCharGeometry>& cacheTextGeometryList);
private:
	static void UpdateUITextLineVertex(int pivotHAlign, float sentenceWidth, TArray<FVector*>& sentenceUIGeoVertexList, struct FUITextLineProperty& sentenceProperty);
#pragma endregion

#pragma region UISector
public:
	static void FromUISector(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUISectorUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, const FLGUISpriteInfo* spriteInfo);
	static void UpdateUISectorVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment);
#pragma endregion

#pragma region UIRing
public:
	static void FromUIRing(float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, uint8 uvType, FColor color, float ringWidth, TSharedPtr<UIGeometry> uiGeo, const FLGUISpriteInfo* spriteInfo, bool requireNormal, bool requireTangent, bool requireUV1);
	static void UpdateUIRingUV(TSharedPtr<UIGeometry> uiGeo, uint8 uvType, float startAngle, float endAngle, uint8 segment, float ringWidth, float& width, float& height, const FLGUISpriteInfo* spriteInfo);
	static void UpdateUIRingVertex(TSharedPtr<UIGeometry> uiGeo, float& width, float& height, const FVector2D& pivot, float startAngle, float endAngle, uint8 segment, float ringWidth);
#pragma endregion

public:
	static void UpdateUIColor(TSharedPtr<UIGeometry> uiGeo, FColor color);
	static void TransformVertices(class ULGUICanvas* canvas, class UUIRenderable* item, TSharedPtr<UIGeometry> uiGeo, bool requireNormal, bool requireTangent);
	static void CheckAndApplyAdditionalChannel(TSharedPtr<UIGeometry> uiGeo);
	static void CalculatePivotOffset(const float& width, const float& height, const FVector2D& pivot, float& pivotOffsetX, float& pivotOffsetY, float& halfW, float& halfH);
private:
	static void OffsetVertices(TArray<FVector>& vertices, float offsetX, float offsetY);
	static void OffsetVertices(TArray<FVector*>& vertices, float offsetX, float offsetY);
};
