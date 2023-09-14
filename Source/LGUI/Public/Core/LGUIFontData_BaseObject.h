// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/UIGeometry.h"
#include "Core/RichTextParser.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUIFontData_BaseObject.generated.h"


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
	FLGUICharData_HighPrecision(const FLGUICharData& charData)
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

	FVector2f GetUV0()const
	{
		return FVector2f(uv0X, uv0Y);
	}
	FVector2f GetUV3()const
	{
		return FVector2f(uv3X, uv3Y);
	}
	FVector2f GetUV2()const
	{
		return FVector2f(uv0X, uv3Y);
	}
	FVector2f GetUV1()const
	{
		return FVector2f(uv3X, uv0Y);
	}
};

class UTexture2D;
class UUIText;

/**
 * base font class, UIText can use a implemented asset object to render text
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUIFontData_BaseObject : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitFont()PURE_VIRTUAL(ULGUISpriteData_BaseObject::InitFont, );

	virtual UMaterialInterface* GetFontMaterial(ELGUICanvasClipType clipType)PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetFontMaterial, return nullptr;);
	virtual UTexture2D* GetFontTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetFontTexture, return nullptr;);
	virtual FLGUICharData_HighPrecision GetCharData(const TCHAR& charCode, const float& charSize) PURE_VIRTUAL(ULGUIFontData_BaseObject::GetCharData, return FLGUICharData_HighPrecision(););
	virtual bool HasKerning() { return false; }
	virtual float GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize) { return 0; }
	virtual float GetLineHeight(const float& fontSize) { return fontSize; }
	virtual float GetVerticalOffset(const float& fontSize) { return 0; }
	virtual float GetFontSizeLimit() { return MAX_FLT; }
	virtual uint8 GetRequireAdditionalShaderChannels() { return 0; }
	virtual bool GetShouldAffectByPixelPerfect() { return true; }
	virtual bool GetNeedObjectScale() { return false; }

	/** this is called every time before create a string of char geometry */
	virtual void PrepareForPushCharData(UUIText* InText) {};
	/** create char geometry and push to vertices & triangleIndices array */
	virtual void PushCharData(
		TCHAR charCode, const FVector2f& lineOffset, const FVector2f& fontSpace, const FLGUICharData_HighPrecision& charData,
		const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIMeshIndexBufferType>& triangleIndices
	) {};

	virtual void AddUIText(UUIText* InText) {}
	virtual void RemoveUIText(UUIText* InText) {}

	static ULGUIFontData_BaseObject* GetDefaultFont();
};
