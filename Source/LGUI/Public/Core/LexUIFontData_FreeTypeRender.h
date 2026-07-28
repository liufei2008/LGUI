// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Core/LexUIFontData_BaseObject.h"
#include "LexUISettings.h"
#include "LexUIFontData_FreeTypeRender.generated.h"

class ULexText;

#if WITH_FREETYPE
struct FT_GlyphSlotRec_;
struct FT_LibraryRec_;
struct FT_FaceRec_;
#endif

UENUM(BlueprintType)
enum class ELexUIDynamicFontDataType :uint8
{
	/** Use custom external font file */
	CustomFontFile,
	/**
	 * Use existing UnrealEngine's font.
	 * Note: if UnrealEngine's font use 'Lazy Load' loading policy, then LexUI will load target font file by itself.
	 */
	EngineFont,
};

#define ONE_DIVIDE_64 0.015625f //(1.0f / 64.0f)

/**
 * Font asset for UIText to render
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULexUIFontData_FreeTypeRender : public ULexUIFontData_BaseObject
{
	GENERATED_BODY()
protected:
	friend class FLexUIFontDataCustomization;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexUIDynamicFontDataType FontType = ELexUIDynamicFontDataType::CustomFontFile;
	/** Font file path, absolute path or relative to ProjectDir */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString FontFilePath;
	/** Font file use relative path(relative to ProjectDir) or absolute path. After build your game, remember to copy your font file to target path, unless "useExternalFileOrEmbedInToUAsset" is false */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bUseRelativeFilePath = true;
	/** When in build, use external file or embed into uasset. But in editor, will always load from fontFilePath. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bUseExternalFileOrEmbedInToUAsset = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TObjectPtr<class UFontFace> EngineFont;

	UPROPERTY(EditAnywhere, Category = "LGUI")
	bool bCultureFont = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="bCultureFont"))
	TMap<FString, TSoftObjectPtr<class UFontFace>> CultureFontMap;
	void UpdateFontOnCultureChanged();
	FDelegateHandle OnCultureChangedDelegateHandle;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		int FontFace = 0;
	/** Current using font face has kerning? */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Transient, AdvancedDisplay)
		bool bHasKerning = false;
	
	/**
	 * when packing char pixel into one single atlas texture, LexUI will use this size to create a blank Texture2DArray, then insert char pixel.
	*/
	UPROPERTY(EditAnywhere, Category = "LGUI")
	ELexUIAtlasTextureSizeType TextureSizeType = ELexUIAtlasTextureSizeType::SIZE_2048x2048;
	/**
	 * rect pack use small cells to pack glyphs, and move to next cell if current cell is full. smaller value get better performance, but leave more garbage area.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	ELexUIAtlasTextureSizeType RectPackCellSizeType = ELexUIAtlasTextureSizeType::SIZE_256x256;

	/** Texture of this font */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TObjectPtr<UTexture2DArray> Texture;
	/** IntermediateTexture for Updating Texture2DArray. */ 
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
	TObjectPtr<UTexture2D> IntermediateTexture;
	int32 CurrentTextureSlice = 0;

	/** if not find char in current font, LexUI will search the char in this font array until find it. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<TObjectPtr<ULexUIFontData_FreeTypeRender>> FallbackFontArray;

	virtual void FinishDestroy()override;

	/** when draw a rectangle, need to expend 1 pixel to avoid too sharp pixel at edge */
	virtual int32 Get_SPACE_NEED_EXPEND()const { return 1; };
	/** space between glyph in texture */
	virtual int32 Get_SPACE_BETWEEN_GLYPH()const { return 1; };
public:
	//Begin UObject
	virtual void PostLoad()override;
	virtual void BeginDestroy()override;
	//End UObject

	//Begin ULexUIFontData_BaseObject interface
	virtual void InitFont()override;
	virtual UMaterialInterface* GetFontMaterial()override { return nullptr; }
	virtual UTexture2DArray* GetFontTexture()override;
	virtual FLexUICharData GetCharData(uint32 CharCode, float CharSize, bool IsBold)override;
	virtual bool HasKerning()override { return bHasKerning; }
	virtual float GetKerning(uint32 LeftCharCode, uint32 RightCharCode, float CharSize)override;
	virtual float GetLineHeight(float FontSize)override;
	virtual float GetVerticalOffset(float FontSize)override;
	virtual float GetFontSizeLimit()override { return 200.0f; }//limit font size to 200. too large font size will result in extreme large texture

	virtual void AddUIText(ULexText* InText)override;
	virtual void RemoveUIText(ULexText* InText)override;
	//End ULexUIFontData_BaseObject interface

	void SetFontType(ELexUIDynamicFontDataType Value);
	void SetEngineFont(UFontFace* Value);
protected:
	/** Collection of UIText which use this font to render. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TArray<TWeakObjectPtr<ULexText>> RenderTextArray;

	friend class FLexUIFontData_FreeTypeRenderCustomization;
	/** save data when useExternalFileOrEmbedInToUAsset=false */
	UPROPERTY()
		TArray<uint8> FontBinaryArray;
	/** temp array for storing font binary data, because freetype need to load font from it so we need keep it alive */
	TArray<uint8> TempFontBinaryArray;
	FDelegateHandle PackingAtlasTextureExpandDelegateHandle;

	/** for rect packing */
	rbp::MaxRectsBinPack BinPack;
	TArray<rbp::Rect> FreeRectCells;
	/** 1.0 / textureSize */
	float OneDivideTextureSize;

#if WITH_FREETYPE
	FT_LibraryRec_* Library = nullptr;
	FT_FaceRec_* Face = nullptr;
	void InitFreeType();
	void DeinitFreeType();
	FT_GlyphSlotRec_* RenderGlyphOnFreeType(uint32 CharCode, float CharSize, float BoldSize);

#if WITH_EDITOR
	TArray<FString> CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory);
#endif
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", AdvancedDisplay)
		TArray<FString> SubFaces;
#endif
	bool bAlreadyInitialized = false;

	struct FGlyphBitmap
	{
		float width, height, hOffset, vOffset, hAdvance;
		/** memory will be passed to render thread and delete there too */
		TArray<unsigned char> buffer;
		/** single pixel data size in byte, eg: RGBA8-4 A8-1 */
		int pixelSize;
	};
	/**
	 * Insert rect into area, assign pixel if succeed
	 * return: if glyph can fit in rect area return true, else false
	 */
	bool PackRectAndInsertChar(const FGlyphBitmap& InGlyphBitmap, rbp::MaxRectsBinPack& InOutBinPack, UTexture2DArray* InTexture, FLexUICharData& OutResult);
	void UpdateFontTextureRegion(uint32 PosX, uint32 PosY, uint32 Slice, FUpdateTextureRegion2D Region, uint32 SrcPitch, uint32 SrcBpp, TArray<uint8> SrcData);
	void RenewFontTexture();

	virtual UTexture2DArray* CreateFontTexture(int InTextureSize, int InSliceCount)PURE_VIRTUAL(ULexUIFontData_FreeTypeRender::CreateFontTexture, return nullptr;);
	virtual UTexture2D* CreateIntermediateTexture(int InTextureSize)PURE_VIRTUAL(ULexUIFontData_FreeTypeRender::CreateIntermediateTexture, return nullptr;);
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize);

	virtual bool GetCharDataFromCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& OutResult) { return false; };
	virtual void AddCharDataToCache(uint32 CharCode, float CharSize, bool IsBold, FLexUICharData& CharData) {};
	virtual bool RenderGlyph(uint32 CharCode, float CharSize, bool IsBold, FGlyphBitmap& OutResult) { return false; };
	virtual void ClearCharDataCache() {};
public:
#if WITH_EDITOR
	void ReloadFont();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
#endif
};
