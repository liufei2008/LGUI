// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/LGUIFreeTypeRenderFontData.h"
#include "LGUIFontData.generated.h"

struct FLGUIFontKeyData
{
public:
	FLGUIFontKeyData() {}
	FLGUIFontKeyData(const TCHAR& inCharCode, const uint16& inCharSize)
	{
		this->charCode = inCharCode;
		this->charSize = inCharSize;
	}
	TCHAR charCode = 0;
	uint16 charSize = 0;
	bool operator==(const FLGUIFontKeyData& other)const
	{
		return this->charCode == other.charCode && this->charSize == other.charSize;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLGUIFontKeyData& other)
	{
		return HashCombine(GetTypeHash(other.charCode), GetTypeHash(other.charSize));
	}
};

/**
 * Font asset for UIText to render
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIFontData : public ULGUIFreeTypeRenderFontData
{
	GENERATED_BODY()
protected:
	/** angle of italic style in degree */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float italicAngle = 15.0f;
	/** bold size radio for bold style, large number create more bold effect */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float boldRatio = 0.015f;
public:
	//Begin ULGUIFreeTypeRenderFontData interface
	virtual void PushCharData(
		TCHAR charCode, const FVector2f& lineOffset, const FVector2f& fontSpace, const FLGUICharData_HighPrecision& charData,
		const LGUIRichTextParser::RichTextParseResult& richTextProperty,
		int verticesStartIndex, int indicesStartIndex,
		int& outAdditionalVerticesCount, int& outAdditionalIndicesCount,
		TArray<FLGUIOriginVertexData>& originVertices, TArray<FLGUIMeshVertex>& vertices, TArray<FLGUIMeshIndexBufferType>& triangleIndices
	)override;
	virtual void PrepareForPushCharData(UUIText* InText)override;
	//End ULGUIFreeTypeRenderFontData interface
protected:
	float boldSize; float italicSlop;
	TMap<FLGUIFontKeyData, FLGUICharData> charDataMap;
	virtual UTexture2D* CreateFontTexture(int InTextureSize)override;
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize)override;

	virtual bool GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult)override;
	virtual void AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData)override;
	virtual void ScaleDownUVofCachedChars()override;
	virtual bool RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult)override;
	virtual void ClearCharDataCache()override;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
