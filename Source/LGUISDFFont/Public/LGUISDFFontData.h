// Copyright 2021-present LexLiu. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "Core/LGUIFreeTypeRenderFontData.h"
#include "LGUISDFFontData.generated.h"

/** Font asset for UIText to render. Import font asset generated from "Bitmap Font Generator". */
UCLASS(BlueprintType)
class LGUISDFFONT_API ULGUISDFFontData : public ULGUIFreeTypeRenderFontData
{
	GENERATED_BODY()
public:
	ULGUISDFFontData();
private:

	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font")
		UMaterialInterface* DefaultMaterials[(int)ELGUICanvasClipType::COUNT];
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "2", UIMax = "120"))
		int FontSize = 32;
	UPROPERTY(EditAnywhere, Category = "LGUI SDF Font", meta = (UIMin = "0", UIMax = "30"))
		uint8 SDFRadius = 6;

public:
	//Begin ULGUIFontDataBaseObject interface
	virtual UMaterialInterface* GetFontMaterial(ELGUICanvasClipType clipType)override { return DefaultMaterials[(int)clipType]; }
	virtual void PushCharData(
		TCHAR charCode, const FVector2f& lineOffset, const FVector2f& fontSpace, const FLGUICharData_HighPrecision& charData,
		const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FDynamicMeshVertex>& vertices, TArray<FLGUIIndexType>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(UUIText* InText)override;
	virtual uint8 GetRequireAdditionalShaderChannels()override;
	//End ULGUIFontDataBaseObject interface
protected:
	float boldSize; float italicSlop; float oneDivideFontSize;
	TMap<TCHAR, FLGUICharData> charDataMap;
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
