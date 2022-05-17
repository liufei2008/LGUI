// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "Engine.h"
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

/** Font asset for UIText to render. Import font asset generated from "Bitmap Font Generator". */
UCLASS(BlueprintType)
class LGUISDFFONT_API ULGUISDFFontData : public ULGUIFreeTypeRenderFontData
{
	GENERATED_BODY()
public:
	ULGUISDFFontData();
private:
	/** Use these material to render UIText, include clip material. */
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font")
		UMaterialInterface* DefaultMaterials[(int)ELGUICanvasClipType::COUNT];
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "2", UIMax = "100"))
		int FontSize = 32;
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "2", UIMax = "30"))
		int SDFRadius = 6;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI SDF Font", Transient)
		int LineHeight = -1;
	/** -1 means not set yet. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI SDF Font", Transient)
		int VerticalOffset = -1;

public:
	//Begin ULGUIFontDataBaseObject interface
	virtual UMaterialInterface* GetFontMaterial(ELGUICanvasClipType clipType)override { return DefaultMaterials[(int)clipType]; }
	virtual void PushCharData(
		TCHAR charCode, const FVector2D& lineOffset, const FVector2D& fontSpace, const FLGUICharData_HighPrecision& charData,
		const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(UUIText* InText)override;
	virtual uint8 GetRequireAdditionalShaderChannels()override;
	virtual float GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize);
	virtual float GetLineHeight(const float& fontSize);
	virtual float GetVerticalOffset(const float& fontSize);
	virtual bool GetCanAdjustPixelPerfect() { return false; }
	//End ULGUIFontDataBaseObject interface
protected:
	float boldSize; float italicSlop; float oneDivideFontSize;
	TMap<TCHAR, FLGUICharData> charDataMap;
	TMap<FLGUISDFFontKerningPair, int16> KerningPairsMap;
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)override;

	virtual bool GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)override;
	virtual void AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData)override;
	virtual void ScaleDownUVofCachedChars()override;
	virtual bool RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult)override;
	virtual void ClearCharDataCache()override;
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);
#endif
};
