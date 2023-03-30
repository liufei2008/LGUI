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
UCLASS(NotBlueprintable, NotBlueprintType, Experimental)
class LGUI_API ULGUIStaticSpriteAtlasData :public UObject
{
	GENERATED_BODY()
private:
	friend class FLGUIStaticSpriteAtlasDataCustomization;
	/** whether or not use srgb for generate atlas texture */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		bool atlasTextureUseSRGB = true;
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		TEnumAsByte<TextureFilter> atlasTextureFilter = TextureFilter::TF_Trilinear;
#if WITH_EDITORONLY_DATA
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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<ULGUISpriteData*> spriteArray;
#if WITH_EDITORONLY_DATA
	TArray<ULGUISpriteData*> prevSpriteArray;
	/** collection of all UISprite whitch use this atlas to render */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI", AdvancedDisplay)
		TArray<TWeakObjectPtr<UUISpriteBase>> renderSpriteArray;
#endif
	/**
	 * Store mip data, so we can recreate atlas texture with this data.
	 * @todo: Actually I want to save this only in cook time (reduce editor asset size), but I can't get texutre's pixel data in cook time.
	 */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		TArray<uint8> textureMipData;
	UPROPERTY()
		uint32 textureSize;
#if WITH_EDITOR
public:
	virtual void PreEditChange(FProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	void AddSpriteData(ULGUISpriteData* InSpriteData);
	void RemoveSpriteData(ULGUISpriteData* InSpriteData);
	void AddRenderSprite(UUISpriteBase* InSprite);
	void RemoveRenderSprite(UUISpriteBase* InSprite);
	/** Check sprite and render sprite, remove not valid. */
	void CheckSprite();
	bool PackAtlas();
	void MarkNotInitialized();

	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void WillNeverCacheCookedPlatformDataAgain()override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
private:
	bool PackAtlasTest(uint32 size, TArray<rbp::Rect>& result);
#endif
private:
	bool bIsInitialized = false;
public:
	bool InitCheck();
	UTexture2D* GetAtlasTexture();
};