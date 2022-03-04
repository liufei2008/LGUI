// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Core/LGUISettings.h"
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

	FVector2f GetUV0()
	{
		return FVector2f(uv0X, uv0Y);
	}
	FVector2f GetUV3()
	{
		return FVector2f(uv3X, uv3Y);
	}
	FVector2f GetUV2()
	{
		return FVector2f(uv0X, uv3Y);
	}
	FVector2f GetUV1()
	{
		return FVector2f(uv3X, uv0Y);
	}
};

class UTexture2D;
class UUIText;

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
	virtual float GetFixedVerticalOffset() { return 0.0f; }
	virtual void InitFont() {};

	virtual void AddUIText(UUIText* InText) {}
	virtual void RemoveUIText(UUIText* InText) {}
};
