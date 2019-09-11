// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#pragma warning(disable:4668)
#pragma warning(disable:4005)

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RHI.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "LGUIEditHelper.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "LGUIFontData.generated.h"

USTRUCT(BlueprintType)
struct FLGUIFontKeyData
{
	GENERATED_BODY()
public:
	FLGUIFontKeyData() {}
	FLGUIFontKeyData(const uint16& inCharIndex, const uint16& inCharSize, const bool& inBorder, const bool& inItalic)
	{
		this->charIndex = inCharIndex;
		this->charSize = inCharSize;
		this->border = inBorder;
		this->italic = inItalic;
	}
	uint16 charIndex;
	uint16 charSize;
	bool border;
	bool italic;
	bool operator==(const FLGUIFontKeyData& other)const
	{
		return this->charIndex == other.charIndex && this->charSize == other.charSize && this->border == other.border && this->italic == other.italic;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLGUIFontKeyData& other)
	{
		return HashCombine(other.italic, HashCombine(other.border, HashCombine(GetTypeHash(other.charIndex), GetTypeHash(other.charSize))));
	}
};

USTRUCT(BlueprintType)
struct FLGUICharData
{
	GENERATED_BODY()
		FLGUICharData(){}
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 width = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 height = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 xoffset = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 yoffset = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 xadvance;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float uv0X;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float uv0Y;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float uv3X;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float uv3Y;

	FVector2D GetUV0()
	{
		return FVector2D(uv0X, uv0Y);
	}
	FVector2D GetUV3()
	{
		return FVector2D(uv3X, uv3Y);
	}
	FVector2D GetUV2()
	{
		return FVector2D(uv0X, uv3Y);
	}
	FVector2D GetUV1()
	{
		return FVector2D(uv3X, uv0Y);
	}
};

class UUIText;
UCLASS(BlueprintType)
class LGUI_API ULGUIFontData : public UObject
{
	GENERATED_BODY()
public:
	ULGUIFontData();
	~ULGUIFontData();

	//Font file path, absolute path or relative to ProjectDir
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FString fontFilePath;
	//Font file use relative path(relative to ProjectDir) or absolute path. After build your game, remember to copy your font file to target path, unless "useExternalFileOrEmbedInToUAsset" is false
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useRelativeFilePath = true;
	//When in build, use external file or embed into uasset. But in editor, will always load from fontFilePath.
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useExternalFileOrEmbedInToUAsset = false;
	//Some font text may not renderred at vertical center, use this to offset
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float fixedVerticalOffset = 0.0f;
	//Texture of this font
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		UTexture2D* texture;

	void InitFreeType();
	void DeinitFreeType();
	virtual void FinishDestroy()override;

	const float MAX_UINT16_RECEPROCAL = 1.0f / 65536.0f;
	const int32 SPACE_NEED_EXPEND = 1;//when draw a rectangle, need to expend 1 pixel to avoid too sharp edge
	const int32 SPACE_NEED_EXPENDx2 = SPACE_NEED_EXPEND * 2;
	const int32 SPACE_BETWEEN_GLYPH = SPACE_NEED_EXPEND + 1;//space between char in texture
	const int32 SPACE_BETWEEN_GLYPHx2 = SPACE_BETWEEN_GLYPH * 2;

	FORCEINLINE void AddUIText(UUIText* InText);
	FORCEINLINE void RemoveUIText(UUIText* InText);
private:
	//Collection of UIText which use this font to render.
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		TArray<TWeakObjectPtr<UUIText>> renderTextArray;

	friend class FLGUIFontDataCustomization;
	UPROPERTY()
		TArray<uint8> fontBinaryArray;
	UPROPERTY(Transient)
		TArray<uint8> tempFontBinaryArray;//temp array for storing data, because freetype need to load font to memory and keep alive
	UPROPERTY(Transient)
		TMap<FLGUIFontKeyData, FLGUICharData> charDataMap;

	rbp::MaxRectsBinPack binPack;//for rect packing
	int32 textureSize;//current texture size
	float fullTextureSizeReciprocal;//1.0 / textureSize

	FLGUIFontKeyData cacheFontKey;
	FLGUICharData cacheCharData;

	FT_Library library = nullptr;
	FT_Face face = nullptr;
	bool alreadyInitialized = false;
	FLGUICharData* PushCharIntoFont(const uint16& charIndex, const uint16& charSize, const bool& bold, const bool& italic);
	FT_Matrix GetItalicMatrix();
	/*Insert rect into area, assign pixel if succeed
	 return: if can fit in rect area return true, else false
	*/
	bool PackRectAndInsertChar(const int32 InBoldOffset, const FT_Bitmap& InCharBitmap, const FT_GlyphSlot& InSlot);
	void UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData);
	void UpdateFontTextureRegion(UTexture2D* Texture, FUpdateTextureRegion2D* Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData);

	void CreateFontTexture(int oldTextureSize, int newTextureSize);
public:
	FLGUICharData* GetCharData(const uint16& charIndex, const uint16& charSize, const bool& bold, const bool& italic);
#if WITH_EDITOR
	void ReloadFont();
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	static ULGUIFontData* GetDefaultFont();
};
