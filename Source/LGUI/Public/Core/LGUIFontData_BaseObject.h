// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/UIGeometry.h"
#include "Core/RichTextParser.h"
#include "LGUIFontData_BaseObject.generated.h"


struct FLGUIFontKeyData
{
public:
	FLGUIFontKeyData() {}
	FLGUIFontKeyData(const TCHAR& inCharIndex, const uint16& inCharSize)
	{
		this->charIndex = inCharIndex;
		this->charSize = inCharSize;
	}
	TCHAR charIndex = 0;
	uint16 charSize = 0;
	bool operator==(const FLGUIFontKeyData& other)const
	{
		return this->charIndex == other.charIndex && this->charSize == other.charSize;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLGUIFontKeyData& other)
	{
		return HashCombine(GetTypeHash(other.charIndex), GetTypeHash(other.charSize));
	}
};

USTRUCT(BlueprintType)
struct FLGUICharData
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	uint16 width = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	uint16 height = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	int16 xoffset = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	int16 yoffset = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	int16 xadvance = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	float uv0X = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	float uv0Y = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	float uv3X = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	float uv3Y = 0;
};
struct FLGUICharData_HighPrecision
{
	FLGUICharData_HighPrecision() {}
	FLGUICharData_HighPrecision(FLGUICharData charData)
	{
		width = charData.width;
		height = charData.height;
		xoffset = charData.xoffset;
		yoffset = charData.yoffset;
		xadvance = charData.xadvance;
		uv0X = charData.uv0X;
		uv0Y = charData.uv0Y;
		uv3X = charData.uv3X;
		uv3Y = charData.uv3Y;
	}
	float width = 0;
	float height = 0;
	float xoffset = 0;
	float yoffset = 0;
	float xadvance = 0;
	float uv0X = 0;
	float uv0Y = 0;
	float uv3X = 0;
	float uv3Y = 0;

	FVector2D GetUV0()
	{
		return FVector2D(uv0X, uv0Y);
	}
	FVector2D GetUV3()
	{
		return FVector2D(uv3X, uv3Y);
	}
	FVector2D GetUV2()
	{
		return FVector2D(uv0X, uv3Y);
	}
	FVector2D GetUV1()
	{
		return FVector2D(uv3X, uv0Y);
	}
};

class UTexture2D;
class UUIText;

struct FUITextCharGeometry
{
	float geoWidth = 0;
	float geoHeight = 0;
	float xadvance = 0;
	float xoffset = 0;
	float yoffset = 0;

	FVector2D uv0 = FVector2D(0, 0);
	FVector2D uv1 = FVector2D(0, 0);
	FVector2D uv2 = FVector2D(0, 0);
	FVector2D uv3 = FVector2D(0, 0);
};

/**
 * font asset for UIText to render
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUIFontData_BaseObject : public UObject
{
	GENERATED_BODY()
public:
	virtual UTexture2D* GetFontTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetFontTexture, return nullptr;);
	virtual FLGUICharData_HighPrecision GetCharData(const TCHAR& charIndex, const uint16& charSize) PURE_VIRTUAL(ULGUIFontData_BaseObject::GetCharData, return FLGUICharData_HighPrecision(););
	virtual float GetBoldRatio() { return 0.015f; }
	virtual float GetItalicAngle() { return 15.0f; }
	virtual bool HasKerning() { return false; }
	virtual int16 GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const uint16& charSize) { return 0; }
	virtual uint16 GetLineHeight(const uint16& fontSize) { return 0; }
	virtual float GetVerticalOffset(const uint16& fontSize) { return 0; }
	virtual void InitFont() {};
	/** create char geometry and push to vertices & triangleIndices array */
	virtual void PushCharData(
		TCHAR charCode, const FVector2D& lineOffset, const FVector2D& fontSpace, const FUITextCharGeometry& charGeo,
		bool bold, float boldSize, bool italic, float italicSlop, const FColor& color,
		int verticesStartIndex, int indicesStartIndex, 
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
	) {};
	/** for rich text */
	virtual void PushCharData(
		TCHAR charCode, const FVector2D& lineOffset, const FVector2D& fontSpace, const FUITextCharGeometry& charGeo,
		float boldSize, float italicSlop, const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
	) {};

	virtual void AddUIText(UUIText* InText) {}
	virtual void RemoveUIText(UUIText* InText) {}
};
