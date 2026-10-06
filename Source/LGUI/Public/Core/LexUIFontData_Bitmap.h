// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/LexUIFontData_FreeTypeRender.h"
#include "LexUIFontData_Bitmap.generated.h"

struct FLexUIBitmapCharKey
{
public:
	FLexUIBitmapCharKey() {}
	FLexUIBitmapCharKey(uint32 InCharCode, uint16 InCharSize, bool InIsBold)
	{
		this->CharCode = InCharCode;
		this->CharSize = InCharSize;
		this->bBold = InIsBold;
	}
	uint32 CharCode = 0;
	uint16 CharSize = 0;
	bool bBold = false;
	bool operator==(const FLexUIBitmapCharKey& other)const
	{
		return this->CharCode == other.CharCode && this->CharSize == other.CharSize && this->bBold == other.bBold;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLexUIBitmapCharKey& other)
	{
		return HashCombine(GetTypeHash(other.CharCode), GetTypeHash(other.CharSize), GetTypeHash(other.bBold));
	}
};

/**
 * Bitmap font asset for render text.
 * NOTE!!! This type is not maintained anymore, new features will not implement, use DistanceField font instead.
 */
UCLASS(BlueprintType)
class LGUI_API ULexUIFontData_Bitmap : public ULexUIFontData_FreeTypeRender
{
	GENERATED_BODY()
protected:
	/** angle of italic style in degree */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float ItalicAngle = 15.0f;
	/** bold size radio for bold style, large number create more bold effect */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float BoldRatio = 0.06f;
public:
	//Begin ULexUIFontData_FreeTypeRender interface
	virtual void PushCharData(
		uint32 charCode, FVector2f lineOffset, FVector2f fontSpace, const FLexUICharData& charData,
		const LexUIRichTextParser::FRichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLexUIOriginVertexData>& originVertices, TArray<FLexUIMeshVertex>& vertices, TArray<FLexUIMeshIndex>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(ULexText* InText)override;
	//End ULexUIFontData_FreeTypeRender interface
protected:
	float BoldSize; float ItalicSlop;
	TMap<FLexUIBitmapCharKey, FLexUICharData> CharDataMap;
	virtual UTexture2DArray* CreateFontTexture(int InTextureSize, int InSliceCount)override;
	virtual UTexture2D* CreateIntermediateTexture(int InTextureSize) override;
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)override;

	virtual bool GetCharDataFromCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& OutResult)override;
	virtual void AddCharDataToCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& CharData)override;
	virtual bool RenderGlyph(uint32 CharCode, float CharSize, bool IsBold, FGlyphBitmap& OutResult)override;
	virtual void ClearCharDataCache()override;

	virtual bool GetSupportDynamicPixelsPerUnit()override { return true; }
	virtual ELexUIFontTextureMark GetFontTextureMark() override{ return ELexUIFontTextureMark::Bitmap; }
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
