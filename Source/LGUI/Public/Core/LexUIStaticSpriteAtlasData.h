// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexUISettings.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LexUIStaticSpriteAtlasData.generated.h"

class ULexUISpriteData;
class ULexSpriteBase;
class ILexUISpriteRenderInterface;

//Static packing Sprite into atlas
UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULexUIStaticSpriteAtlasData :public UObject
{
	GENERATED_BODY()
private:
	friend class FLexUIStaticSpriteAtlasDataCustomization;
	/** weather or not use srgb for generated atlas texture */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		bool AtlasTextureUseSRGB = true;
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		TEnumAsByte<TextureFilter> AtlasTextureFilter = TextureFilter::TF_Trilinear;
#if WITH_EDITORONLY_DATA
	/** space between two sprites when package into atlas */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		int32 SpaceBetweenSprites = 2;
	/** Repeat edge pixel fill spaced between other sprites in atlas texture */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
		int32 EdgePixelPadding = 2;
	/** If packing atlas texture's size is larger than this, then packing operation will abort. */
	UPROPERTY(EditAnywhere, Category = "Atlas-Setting")
	ELexUIAtlasTextureSizeType MaxTextureSize = ELexUIAtlasTextureSizeType::SIZE_2048x2048;
#endif

	/** Generated atlas texture. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI")
		TArray<TObjectPtr<UTexture2D>> AtlasTextureArray;
#if WITH_EDITORONLY_DATA
	/** Collected Sprite array to pack. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TArray<TObjectPtr<ULexUISpriteData>> SpriteDataArray;
	UPROPERTY(Transient)
	TArray<TObjectPtr<ULexUISpriteData>> PrevSpriteDataArray;
	/** collection of all objects that use this atlas to render. Object must implement IUISpriteRenderableInterface. */
	UPROPERTY(VisibleAnywhere, Transient, Category = "LexUI", AdvancedDisplay)
		TArray<TWeakObjectPtr<UObject>> RenderSpriteArray;
	/**
	 * Store texture mip data, so we can recreate atlas texture with this data.
	 */
	UPROPERTY(Transient)
	TArray<uint8> TexturePixelData;
	UPROPERTY()
	FString TexturePixelDataMD5;
	UPROPERTY()
	bool bIsAtlasPackDirty = true;
#endif
	UPROPERTY()
	TArray<int32> TextureSizeArray;
	UPROPERTY()
	TArray<uint8> TexturePixelDataForBuild;
#if WITH_EDITOR
	FString GetCacheDataPath(const FString& InFileName)const;
	/** Check Sprite and render Sprite, remove not valid. */
	void CheckSprite();
	bool PackAtlas();
public:
	virtual void PostInitProperties() override;
	virtual void PreEditChange(FProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	bool ContainsSpriteData(ULexUISpriteData* InSpriteData)const;
	void AddSpriteData(ULexUISpriteData* InSpriteData);
	void RemoveSpriteData(ULexUISpriteData* InSpriteData);
	void AddRenderSprite(TScriptInterface<ILexUISpriteRenderInterface> InSprite);
	void RemoveRenderSprite(TScriptInterface<ILexUISpriteRenderInterface> InSprite);
	
	void MarkNotInitialized();
	void MarkAtlasPackDirty();
	/** Return true if some spriteData is invalid */
	bool CheckInvalidSpriteData()const;
	void CleanupInvalidSpriteData();

	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
	virtual void WillNeverCacheCookedPlatformDataAgain()override;
	virtual void ClearCachedCookedPlatformData(const ITargetPlatform* TargetPlatform)override;
private:
	bool TryPackAtlas(ULexUISpriteData* Sprite, rbp::MaxRectsBinPack& RectBinPack, TArray<rbp::Rect>& PackedRects, TArray<ULexUISpriteData*>& PackedSprites);
	bool bWarningIsAlreadyAppearedAtCurrentPackingSession = false;
	bool bIsYesToAll = false;
	bool bIsNoToAll = false;
	bool bIsAddedToDelayedCall = false;
#endif
private:
	bool bIsInitialized = false;
	virtual void BeginDestroy()override;
	bool InitCheck();
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UTexture2D* GetAtlasTexture(int32 Index);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool ReadPixel(int InTextureIndex, const FVector2D& InUV, FColor& OutPixel);
};