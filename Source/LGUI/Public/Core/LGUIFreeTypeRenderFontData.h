// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "LGUISettings.h"
#include "LGUIFreeTypeRenderFontData.generated.h"


class UTexture2D;
class UUIText;
struct FT_GlyphSlotRec_;
struct FT_LibraryRec_;
struct FT_FaceRec_;

UENUM(BlueprintType)
enum class ELGUIDynamicFontDataType :uint8
{
	/** Use custom external font file */
	CustomFontFile,
	/**
	 * Use existing UnrealEngine's font.
	 * Note: if UnrealEngine's font use 'Lazy Load' loading policy, then LGUI will load target font file by itself.
	 */
	UnrealFont,
};

UENUM(BlueprintType)
enum class ELGUIDynamicFontLineHeightType :uint8
{
	/** Get line height from font face data */
	FromFontFace,
	/** Use font size as line height */
	FontSizeAsLineHeight,
};

/**
 * Font asset for UIText to render
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIFreeTypeRenderFontData : public ULGUIFontData_BaseObject
{
	GENERATED_BODY()
protected:
	friend class FLGUIFontDataCustomization;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIDynamicFontDataType fontType = ELGUIDynamicFontDataType::CustomFontFile;
	/** Font file path, absolute path or relative to ProjectDir */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString fontFilePath;
	/** Font file use relative path(relative to ProjectDir) or absolute path. After build your game, remember to copy your font file to target path, unless "useExternalFileOrEmbedInToUAsset" is false */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useRelativeFilePath = true;
	/** When in build, use external file or embed into uasset. But in editor, will always load from fontFilePath. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useExternalFileOrEmbedInToUAsset = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		class UFontFace* unrealFont;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int fontFace = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIDynamicFontLineHeightType lineHeightType = ELGUIDynamicFontLineHeightType::FontSizeAsLineHeight;
	/** Current using font face has kerning? */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", Transient)
		bool hasKerning = false;
	//UPROPERTY(EditAnywhere, Category = "LGUI")
	//	TScriptInterface<class UFontFaceInterface> test;
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		TArray<FString> subFaces;
	TArray<FString> CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory);
#endif
	
	/**
	 * Packing tag of this font. If packingTag is not none, then LGUI will search UISprite's atlas packingTag, and pack font texture into sprite atlas's texture.
	 * This can be very useful to reduce drawcall.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName packingTag;
	/** Texture of this font */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		UTexture2D* texture;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIAtlasTextureSizeType initialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;

	void InitFreeType();
	void DeinitFreeType();
	FT_GlyphSlotRec_* RenderGlyphOnFreeType(const TCHAR& charCode, const float& charSize);
	virtual void FinishDestroy()override;

	/** when draw a rectangle, need to expend 1 pixel to avoid too sharp edge */
	const int32 SPACE_NEED_EXPEND = 1;
	const int32 SPACE_NEED_EXPENDx2 = SPACE_NEED_EXPEND * 2;
	/** space between char in texture */
	const int32 SPACE_BETWEEN_GLYPH = SPACE_NEED_EXPEND + 1;
	const int32 SPACE_BETWEEN_GLYPHx2 = SPACE_BETWEEN_GLYPH * 2;

public:
	//Begin ULGUIFontData_BaseObject interface
	virtual void InitFont()override;
	virtual UMaterialInterface* GetFontMaterial(ELGUICanvasClipType clipType)override { return nullptr; }
	virtual UTexture2D* GetFontTexture()override;
	virtual FLGUICharData_HighPrecision GetCharData(const TCHAR& charCode, const float& charSize)override;
	virtual bool HasKerning()override { return hasKerning; }
	virtual float GetKerning(const TCHAR& leftCharIndex, const TCHAR& rightCharIndex, const float& charSize)override;
	virtual float GetLineHeight(const float& fontSize)override;
	virtual float GetVerticalOffset(const float& fontSize)override;
	virtual float GetFontSizeLimit() { return 200.0f; }//limit font size to 200. too large font size will result in extream large texture

	virtual void AddUIText(UUIText* InText)override;
	virtual void RemoveUIText(UUIText* InText)override;
	//End ULGUIFontData_BaseObject interface
protected:
	/** Collection of UIText which use this font to render. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIText>> renderTextArray;

	friend class FLGUIFreeTypeRenderFontDataCustomization;
	UPROPERTY()
		TArray<uint8> fontBinaryArray;
	/** temp array for storing data, because freetype need to load font to memory and keep alive */
	UPROPERTY(Transient)
		TArray<uint8> tempFontBinaryArray;
	struct FLGUIAtlasData* packingAtlasData = nullptr;
	FDelegateHandle packingAtlasTextureExpandDelegateHandle;

	/** for rect packing */
	rbp::MaxRectsBinPack binPack;
	TArray<rbp::Rect> freeRects;
	/** current texture size */
	int32 textureSize;
	/** 1.0 / textureSize */
	float oneDiviceTextureSize;

	FT_LibraryRec_* library = nullptr;
	FT_FaceRec_* face = nullptr;
	bool alreadyInitialized = false;
	bool usePackingTag = false;

	struct FGlyphBitmap
	{
		int width, height, hOffset, vOffset, hAdvance;
		/** memory will passed to render thread and delete there too */
		unsigned char* buffer;
		/** single pixel data size in byte, eg RGBA8-4 A8-1 */
		int pixelSize;
	};
	/**
	 * Insert rect into area, assign pixel if succeed
	 * return: if can fit in rect area return true, else false
	 */
	bool PackRectAndInsertChar(const FGlyphBitmap& InGlyphBitmap, rbp::MaxRectsBinPack& InOutBinpack, UTexture2D* InTexture, FLGUICharData& OutResult);
	void UpdateFontTextureRegion(UTexture2D* Texture, FUpdateTextureRegion2D* Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData);
	void CreateFontTexture(int oldTextureSize, int newTextureSize);

	virtual UTexture2D* CreateTexture(int InTextureSize);
	virtual void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize);

	virtual bool GetCharDataFromCache(const TCHAR& charCode, const float& charSize, FLGUICharData_HighPrecision& OutResult) { return false; };
	virtual void AddCharDataToCache(const TCHAR& charCode, const float& charSize, const FLGUICharData& charData) {};
	virtual bool RenderGlyph(const TCHAR& charCode, const float& charSize, FGlyphBitmap& OutResult) { return false; };
	virtual void ScaleDownUVofCachedChars() {};
	virtual void ClearCharDataCache() {};
public:
#if WITH_EDITOR
	void ReloadFont();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
