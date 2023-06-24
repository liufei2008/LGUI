// Copyright 2019-present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUIFreeTypeRenderFontData.h"
#include "LGUISDFFontData.generated.h"

struct FLGUISDFFontKerningPair
{
public:
	FLGUISDFFontKerningPair() {}
	FLGUISDFFontKerningPair(const TCHAR& InLeftCharCode, const TCHAR& InRightCharCode)
	{
		this->LeftCharCode = InLeftCharCode;
		this->RightCharCode = InRightCharCode;
	}
	TCHAR LeftCharCode = 0;
	TCHAR RightCharCode = 0;
	bool operator==(const FLGUISDFFontKerningPair& other)const
	{
		return this->LeftCharCode == other.LeftCharCode && this->RightCharCode == other.RightCharCode;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLGUISDFFontKerningPair& other)
	{
		return HashCombine(GetTypeHash(other.LeftCharCode), GetTypeHash(other.RightCharCode));
	}
};

/** SDF(Signed Distance Field) Font asset for UIText to render smooth scaled sdf font. */
UCLASS(BlueprintType, meta = (DisplayName = "LGUI SDF Font Data"))
class LGUI_API ULGUISDFFontData : public ULGUIFreeTypeRenderFontData
{
	GENERATED_BODY()
public:
	ULGUISDFFontData();
private:
	/** Use these material to render SDF font for UIText, include clip material. */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font")
		TObjectPtr<UMaterialInterface> SDFDefaultMaterials[(int)ELGUICanvasClipType::Custom];
	/** Font size when render glyph. */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "16", UIMax = "100"))
		int FontSize = 32;
	/** The radius of the distance field in pixels */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "2", UIMax = "30"))
		int SDFRadius = 6;
	/** Angle of italic style in degree */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font")
		float ItalicAngle = 15.0f;
	/**
	 * bold size radio for bold style, large number create more bold effect.
	 * this parameter is related with SDFRadius & FontSize, smaller SDFRadius & FontSize will need larger BoldRatio to render.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font")
		float BoldRatio = 0.15f;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI SDF Font", Transient)
		int LineHeight = -1;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI SDF Font", Transient)
		int VerticalOffset = -1;

public:
	//Begin ULGUIFontDataBaseObject interface
	virtual UMaterialInterface* GetFontMaterial(ELGUICanvasClipType clipType)override;
	virtual void PushCharData(
		TCHAR charCode, const FVector2f& lineOffset, const FVector2f& fontSpace, const FLGUICharData_HighPrecision& charData,
		const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(UUIText* InText)override;
	virtual uint8 GetRequireAdditionalShaderChannels()override;
	virtual float GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize) override;
	virtual float GetLineHeight(const float& fontSize) override;
	virtual float GetVerticalOffset(const float& fontSize) override;
	virtual bool GetShouldAffectByPixelPerfect() override{ return false; }
	//End ULGUIFontDataBaseObject interface
protected:
	float italicSlop; float oneDivideFontSize;
	TMap<TCHAR, FLGUICharData> charDataMap;
	TMap<FLGUISDFFontKerningPair, int16> KerningPairsMap;
	virtual UTexture2D* CreateFontTexture(int InTextureSize)override;
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)override;

	virtual bool GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)override;
	virtual void AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData)override;
	virtual void ScaleDownUVofCachedChars()override;
	virtual bool RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult)override;
	virtual void ClearCharDataCache()override;

	//SDF font already have space between glyphs
	virtual int32 Get_SPACE_NEED_EXPEND()const override { return 0; };
	virtual int32 Get_SPACE_BETWEEN_GLYPH()const override { return 0; };
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
	virtual void PostInitProperties()override;

	void CheckMaterials();
};
