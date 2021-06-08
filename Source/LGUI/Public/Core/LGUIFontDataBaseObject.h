// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Core/LGUISettings.h"
#include "LGUIFontDataBaseObject.generated.h"


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

struct FLGUICharData
{
public:
	uint16 width = 0;
	uint16 height = 0;
	int16 xoffset = 0;
	int16 yoffset = 0;
	float xadvance = 0;
	float horizontalBearingY = 0;
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

/**
 * font asset for UIText to render
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIFontData_BaseObject : public UObject
{
	GENERATED_BODY()
public:
	virtual UTexture2D* GetFontTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetFontTexture, return nullptr;);
	virtual FLGUICharData* GetCharData(const TCHAR& charIndex, const uint16& charSize) PURE_VIRTUAL(ULGUIFontData_BaseObject::GetCharData, return nullptr;);
	virtual float GetBoldRatio() { return 0.015f; }
	virtual float GetItalicAngle() { return 15.0f; }
	virtual float GetFixedVerticalOffset() { return 0.0f; }
	virtual void InitFont() {};

	virtual void AddUIText(UUIText* InText) {}
	virtual void RemoveUIText(UUIText* InText) {}
};
