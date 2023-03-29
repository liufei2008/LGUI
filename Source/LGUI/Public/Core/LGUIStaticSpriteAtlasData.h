// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUIStaticSpriteAtlasData.generated.h"

class ULGUISpriteData;
class UUISpriteBase;

//Static packing sprite into atlas
UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULGUIStaticSpriteAtlasData :public UObject
{
	GENERATED_BODY()
private:
	friend class FLGUIStaticSpriteAtlasDataCustomization;
	static uint32 TextureNameSufix;
#if WITH_EDITORONLY_DATA
	/** whether or not use srgb for generate atlas texture */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		bool atlasTextureUseSRGB = true;
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		TEnumAsByte<TextureFilter> atlasTextureFilter = TextureFilter::TF_Trilinear;
	/** space between two sprites when package into atlas */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		int32 spaceBetweenSprites = 2;
	/** Repeat edge pixel fill spaced between other sprites in atlas texture */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		int32 edgePixelPadding = 2;
	/** If the result atlas texture's size is larger than this, then packing operation will abort. */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		uint32 maxAtlasTextureSize = 4096;
#endif

	/** Generated atlas texture. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
		UTexture2D* atlasTexture = nullptr;
	/** Collected sprite array to pack. */
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		TArray<ULGUISpriteData*> spriteArray;
#if WITH_EDITORONLY_DATA
	/** collection of all UISprite whitch use this atlas to render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", AdvancedDisplay)
		TArray<TWeakObjectPtr<UUISpriteBase>> renderSpriteArray;
#endif
	/** Store mip data, so we can recreate atlas texture use this data. */
	UPROPERTY()
		TArray<uint8> textureMipData;
	UPROPERTY()
		uint32 textureSize;
#if WITH_EDITOR
public:
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	void AddSpriteData(ULGUISpriteData* InSpriteData);
	void RemoveSpriteData(ULGUISpriteData* InSpriteData);
	void AddRenderSprite(UUISpriteBase* InSprite);
	void RemoveRenderSprite(UUISpriteBase* InSprite);
	void ClearRenderSprite();
	bool PackAtlas();
	bool InitCheck();
	void MarkNotInitialized();
private:
	bool PackAtlasTest(uint32 size, TArray<rbp::Rect>& result);
#endif
private:
	bool bIsInitialized = false;
public:
	UTexture2D* GetAtlasTexture();
	DECLARE_EVENT(ULGUIStaticSpriteAtlasData, FLGUIAtlasMapChangeEvent);
	FLGUIAtlasMapChangeEvent OnAtlasMapChanged;
};