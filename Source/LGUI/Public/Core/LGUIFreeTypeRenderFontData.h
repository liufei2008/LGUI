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
#if WITH_FREETYPE
struct FT_GlyphSlotRec_;
struct FT_LibraryRec_;
struct FT_FaceRec_;
#endif

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
UCLASS(Abstract, BlueprintType)
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
	
	/**
	 * Packing tag of this font. If packingTag is not none, then LGUI will search UISprite's atlas packingTag, and pack font texture into sprite atlas's texture.
	 * This can be very useful to reduce drawcall.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FName packingTag;
	/**
	 * when packing char pixel into one single atlas texture, we will use this size to create a blank texture, then insert char pixel. if texture is full(cannot insert anymore), a new larger texture will be created.
	 * if initialSize is too small, some lag or freeze may happen when creating new texture.
	 * if initialSize is too big, it is not much efficient to sample very big texture on GPU.
	*/
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIAtlasTextureSizeType initialSize = ELGUIAtlasTextureSizeType::SIZE_1024x1024;
	/**
	 * rect pack use small cells to pack glyph in, and move to next cell if current cell is full. smaller value get better performance, but leave more garbage area.
	 * this value defines the cell size. must not larger then InitialSize and only allow pow of 2.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int32 rectPackCellSize = 256;

	/** Texture of this font */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		UTexture2D* texture;

	/** if not find char in current font, LGUI will search the char in this font array until find it. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<ULGUIFreeTypeRenderFontData*> fallbackFontArray;

	virtual void FinishDestroy()override;

	/** when draw a rectangle, need to expend 1 pixel to avoid too sharp pixel at edge */
	virtual int32 Get_SPACE_NEED_EXPEND()const { return 1; };
	/** space between glyph in texture */
	virtual int32 Get_SPACE_BETWEEN_GLYPH()const { return 1; };
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
	/** save data when useExternalFileOrEmbedInToUAsset=false */
	UPROPERTY()
		TArray<uint8> fontBinaryArray;
	/** temp array for storing font binary data, because freetype need to load font from it so we need keep it alive */
	TArray<uint8> tempFontBinaryArray;
	struct FLGUIAtlasData* packingAtlasData = nullptr;
	FDelegateHandle packingAtlasTextureExpandDelegateHandle;

	/** for rect packing */
	rbp::MaxRectsBinPack binPack;
	TArray<rbp::Rect> freeRects;
	/** current texture size */
	int32 textureSize;
	/** 1.0 / textureSize */
	float oneDivideTextureSize;

#if WITH_FREETYPE
	FT_LibraryRec_* library = nullptr;
	FT_FaceRec_* face = nullptr;
	void InitFreeType();
	void DeinitFreeType();
	FT_GlyphSlotRec_* RenderGlyphOnFreeType(const TCHAR& charCode, const float& charSize);

#if WITH_EDITOR
	TArray<FString> CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory);
#endif
#endif
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", AdvancedDisplay)
		TArray<FString> subFaces;
#endif
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
	void RenewFontTexture(int oldTextureSize, int newTextureSize);

	virtual UTexture2D* CreateFontTexture(int InTextureSize)PURE_VIRTUAL(ULGUIFreeTypeRenderFontData::CreateFontTexture, return nullptr;);
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
