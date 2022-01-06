// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Core/LGUISettings.h"
#include "Core/LGUIFontData_BaseObject.h"
#include "LGUIFontData.generated.h"


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
	 * Use existing UMG font.
	 * Note: if UMG font use 'Lazy Load' loading policy, then LGUI will load target font file by itself.
	 */
	UnrealFont,
};

/**
 * Font asset for UIText to render
 */
UCLASS(BlueprintType)
class LGUI_API ULGUIFontData : public ULGUIFontData_BaseObject
{
	GENERATED_BODY()
private:
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
	//UPROPERTY(EditAnywhere, Category = "LGUI")
	//	TScriptInterface<class UFontFaceInterface> test;
#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		TArray<FString> subFaces;
	TArray<FString> CacheSubFaces(FT_LibraryRec_* InFTLibrary, const TArray<uint8>& InMemory);
#endif

	/** Some font text may not renderred at vertical center, use this to offset */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float fixedVerticalOffset = 0.0f;
	/** angle of italic style in degree */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float italicAngle = 15.0f;
	/** bold size radio for bold style, large number create more bold effect */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float boldRatio = 0.015f;
	
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
		ELGUIAtlasTextureSizeType initialSize = ELGUIAtlasTextureSizeType::SIZE_256x256;

	void InitFreeType();
	void DeinitFreeType();
	virtual void FinishDestroy()override;

	/** when draw a rectangle, need to expend 1 pixel to avoid too sharp edge */
	const int32 SPACE_NEED_EXPEND = 1;
	const int32 SPACE_NEED_EXPENDx2 = SPACE_NEED_EXPEND * 2;
	/** space between char in texture */
	const int32 SPACE_BETWEEN_GLYPH = SPACE_NEED_EXPEND + 1;
	const int32 SPACE_BETWEEN_GLYPHx2 = SPACE_BETWEEN_GLYPH * 2;

public:
	//Begin ULGUIFontData_BaseObject interface
	virtual UTexture2D* GetFontTexture()override;
	virtual FLGUICharData_HighPrecision GetCharData(const TCHAR& charIndex, const uint16& charSize)override;
	virtual float GetBoldRatio() override{ return boldRatio; }
	virtual float GetItalicAngle()override { return italicAngle; }
	virtual float GetFixedVerticalOffset()override { return fixedVerticalOffset - 0.25f; }//-0.25 is a common number for most fonts
	virtual void InitFont()override;
	virtual void AddUIText(UUIText* InText)override;
	virtual void RemoveUIText(UUIText* InText)override;
	//End ULGUIFontData_BaseObject interface
private:
	/** Collection of UIText which use this font to render. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIText>> renderTextArray;

	friend class FLGUIFontDataCustomization;
	UPROPERTY()
		TArray<uint8> fontBinaryArray;
	/** temp array for storing data, because freetype need to load font to memory and keep alive */
	UPROPERTY(Transient)
		TArray<uint8> tempFontBinaryArray;
	TMap<FLGUIFontKeyData, FLGUICharData> charDataMap;
	struct FLGUIAtlasData* packingAtlasData = nullptr;
	FDelegateHandle packingAtlasTextureExpandDelegateHandle;

	/** for rect packing */
	rbp::MaxRectsBinPack binPack;
	TArray<rbp::Rect> freeRects;
	/** current texture size */
	int32 textureSize;
	/** 1.0 / textureSize */
	float fullTextureSizeReciprocal;

	FLGUIFontKeyData cacheFontKey;
	FLGUICharData cacheCharData;

	FT_LibraryRec_* library = nullptr;
	FT_FaceRec_* face = nullptr;
	bool alreadyInitialized = false;
	bool usePackingTag = false;
	FLGUICharData* PushCharIntoFont(const TCHAR& charIndex, const uint16& charSize);
	/**
	 * Insert rect into area, assign pixel if succeed
	 * return: if can fit in rect area return true, else false
	 */
	bool PackRectAndInsertChar(FT_GlyphSlotRec_* InSlot, rbp::MaxRectsBinPack& InOutBinpack, UTexture2D* InTexture);
	void UpdateFontTextureRegion(UTexture2D* Texture, FUpdateTextureRegion2D* Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData);

	void CreateFontTexture(int oldTextureSize, int newTextureSize);
	void ApplyPackingAtlasTextureExpand(UTexture2D* newTexture, int newTextureSize);
public:
#if WITH_EDITOR
	void ReloadFont();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	static ULGUIFontData* GetDefaultFont();
};
