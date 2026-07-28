// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LexUIFontData_FreeTypeRender.h"
#include "LexUIFontData_DistanceField.generated.h"

struct FLexUIDistanceFieldCharKey
{
public:
	FLexUIDistanceFieldCharKey() {}
	FLexUIDistanceFieldCharKey(uint32 InCharCode, bool InBold)
	{
		this->CharCode = InCharCode;
		this->bBold = InBold;
	}
	uint32 CharCode = 0;
	bool bBold = false;
	bool operator==(const FLexUIDistanceFieldCharKey& other)const
	{
		return this->CharCode == other.CharCode && this->bBold == other.bBold;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLexUIDistanceFieldCharKey& other)
	{
		return HashCombine(GetTypeHash(other.CharCode), GetTypeHash(other.bBold));
	}
};
struct FLexUIDistanceFieldFontKerningPair
{
public:
	FLexUIDistanceFieldFontKerningPair() {}
	FLexUIDistanceFieldFontKerningPair(const uint32& InLeftCharCode, const uint32& InRightCharCode)
	{
		this->LeftCharCode = InLeftCharCode;
		this->RightCharCode = InRightCharCode;
	}
	uint32 LeftCharCode = 0;
	uint32 RightCharCode = 0;
	bool operator==(const FLexUIDistanceFieldFontKerningPair& other)const
	{
		return this->LeftCharCode == other.LeftCharCode && this->RightCharCode == other.RightCharCode;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLexUIDistanceFieldFontKerningPair& other)
	{
		return HashCombine(GetTypeHash(other.LeftCharCode), GetTypeHash(other.RightCharCode));
	}
};

/** SDF(Signed Distance Field) Font asset for render smooth scaled sdf text. */
UCLASS(BlueprintType)
class LGUI_API ULexUIFontData_DistanceField : public ULexUIFontData_FreeTypeRender
{
	GENERATED_BODY()
public:
	ULexUIFontData_DistanceField();
private:
	/** Font size when render glyph. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "16", UIMax = "100"))
		int SampleFontSize = 64;
	/** The radius of the distance field in pixels. Normally just use 1/4 of FontSize */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "1"))
		int SDFRadius = 16;
	/** Angle of italic style in degree */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float ItalicAngle = 15.0f;
	/**
	 * bold size radio for bold style, large number create more bold effect.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		float BoldRatio = 0.06f;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", Transient)
		int LineHeight = -1;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", Transient)
		int VerticalOffset = -1;
	UPROPERTY(EditAnywhere, Transient, Category = "LGUI", Transient)
	float AdditionalVerticalOffset = 0.0f;

public:
	//Begin ULexUIFontData_BaseObject interface
	virtual UMaterialInterface* GetFontMaterial()override;
	virtual void PushCharData(
		uint32 charCode, FVector2f lineOffset, FVector2f fontSpace, const FLexUICharData& charData,
		const LexUIRichTextParser::FRichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLexUIOriginVertexData>& originVertices, TArray<FLexUIMeshVertex>& vertices, TArray<FLexUIMeshIndex>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(ULexText* InText)override;
	virtual bool GetRequireNormalAndTangent()override;
	virtual float GetKerning(uint32 leftCharIndex, uint32 rightCharIndex, float charSize) override;
	virtual float GetLineHeight(float fontSize) override;
	virtual float GetVerticalOffset(float fontSize) override;
	virtual bool GetShouldAffectByPixelPerfect() override{ return false; }
	virtual bool GetNeedObjectScale() override{ return true; }//sdf font need scale value in material
	virtual ELexUIFontTextureMark GetFontTextureMark() override{ return ELexUIFontTextureMark::DistanceField; }
	virtual float GetBoldRatio() override{ return BoldRatio; }
	//End ULexUIFontData_BaseObject interface
	float GetSampleFontSize()const{return SampleFontSize;}
protected:
	float ItalicSlop = 0.0f; float OneDivideFontSize = 1.0f; float ExpandMeshSize = 0; float ObjectScale = 0;
	TMap<FLexUIDistanceFieldCharKey, FLexUICharData> CharDataMap;
	TMap<FLexUIDistanceFieldFontKerningPair, int16> KerningPairsMap;
	virtual UTexture2DArray* CreateFontTexture(int InTextureSize, int InSliceCount)override;
	virtual UTexture2D* CreateIntermediateTexture(int InTextureSize) override;
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)override;

	virtual bool GetCharDataFromCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& OutResult)override;
	virtual void AddCharDataToCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& CharData)override;
	virtual bool RenderGlyph(uint32 CharCode, float CharSize, bool IsBold, FGlyphBitmap& OutResult)override;
	virtual void ClearCharDataCache()override;

	//SDF font already have space between glyphs
	virtual int32 Get_SPACE_NEED_EXPEND()const override { return 0; };
	virtual int32 Get_SPACE_BETWEEN_GLYPH()const override { return 0; };
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
	virtual void PostInitProperties()override;
};
