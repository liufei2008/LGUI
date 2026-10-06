// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LexUISpriteData_BaseObject.h"
#include "LexUISpriteData.generated.h"


class ULexUISpriteData;
class ULexSpriteBase;

#define WARNING_ATLAS_SIZE 4096

UENUM(BlueprintType)
enum class ELexUISpritePackingType : uint8
{
	/** Static packing, Sprite will be packed in editor no matter if it used, not support runtime packing, support mipmap. */
	Static,
	/** Dynamic packing. Sprite will be packed when using, support runtime packing, not support mipmap. */
	Dynamic,
};

/**
 * A Sprite-data type that can do automatic packing
 */
UCLASS(BlueprintType)
class LGUI_API ULexUISpriteData :public ULexUISpriteData_BaseObject
{
	GENERATED_BODY()
	ULexUISpriteData();
private:
	friend class FLexUISpriteDataCustomization;
	friend class ULexUISpriteDataFactory;
	friend struct FLexUIDynamicSpriteAtlasData;
	friend class ULexUIStaticSpriteAtlasData;
	/**
	 * Texture of this Sprite. Sprite is actually rendered from atlas texture, so SpriteTexture is not needed if atlas-data is packed; But! since atlas texture is packed at runtime, we must include SpriteTexture inside final package.
	 * DoNot modify SpriteTexture's setting unless you know what you do
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		TObjectPtr<UTexture2D> SpriteTexture;
	/** Information needed for render this Sprite */
	UPROPERTY(EditAnywhere, Category = LGUI)
		FLexUISpriteInfo SpriteInfo;

	UPROPERTY(EditAnywhere, Category = "AtlasPacking")
	ELexUISpritePackingType PackingType = ELexUISpritePackingType::Dynamic;
	/**
	 * Use a StaticSpriteAtlasData to pack multiple sprites into single atlas texture. The packing process is in editor and cook time, no performance impact at runtime.
	 * Support mipmaps.
	 */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking", meta=(EditCondition="PackingType==ELexUISpritePackingType::Static"))
		TObjectPtr<ULexUIStaticSpriteAtlasData> PackingAtlas = nullptr;
	/** Texture index in atlas texture array */
	UPROPERTY(VisibleAnywhere, Category = "AtlasPacking", meta=(EditCondition="PackingType==ELexUISpritePackingType::Static"))
	int32 AtlasTextureIndex = 0;
	/**
	 * Sprites that have same PackingTag will be packed into same atlas at runtime. If PackingTag is None, then the LexSprite which render this LexUISpriteData will be treated as a LexTexture.
	 * Not support mipmaps.
	 */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking", meta=(EditCondition="PackingType==ELexUISpritePackingType::Dynamic"))
		FName PackingTag = TEXT("Main");
	
	/** Repeat edge pixel fill spaced between other sprites in atlas texture */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking")
		bool bUseEdgePixelPadding = true;
private:
	bool bIsInitialized = false;
	UPROPERTY(Transient)TObjectPtr<UTexture2D> AtlasTexture = nullptr;
	bool PackSprite();
	void CheckSpriteTexture();
public:
	bool GetUseEdgePixelPadding()const { return bUseEdgePixelPadding; }
	ULexUIStaticSpriteAtlasData* GetPackingAtlas()const { return PackingAtlas; }
	/**
	 * @return anything changed
	 */
	bool ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv);
	//Begin ULexUISpriteData_BaseObject interface
	virtual UTexture2D * GetAtlasTexture()override;
	virtual const FLexUISpriteInfo& GetSpriteInfo()override;
	virtual bool IsIndividual()const override;
	virtual void AddUISprite(TScriptInterface<class ILexUISpriteRenderInterface> InUISprite)override;
	virtual void RemoveUISprite(TScriptInterface<class ILexUISpriteRenderInterface> InUISprite)override;
	virtual bool ReadPixel(const FVector2D& InUV, FColor& OutPixel)const override;
	virtual bool SupportReadPixel()const override;
	//End ULexUISpriteData_BaseObject interface

	/** initialize Sprite data */
	void InitSpriteData();
	UFUNCTION(BlueprintCallable, Category = "LexUI") bool HavePackingTag()const;
	UFUNCTION(BlueprintCallable, Category = "LexUI") const FName& GetPackingTag()const;

	/**
	 * Create a LexUISpriteData with provided parameter. This can use at runtime
	 * @param Outer						Outer of the result LexUISpriteData
	 * @param InSpriteTexture			Use this texture to create
	 * @param InBorder					Border value
	 * @param InPackingTag				See "PackingTag" property
	 * @return							Created LexUISpriteData, nullptr if something wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		static ULexUISpriteData* CreateLexUISpriteData(UObject* Outer, UTexture2D* InSpriteTexture, FMargin InBorder, FName InPackingTag = TEXT("Main"));

	/**
	 * If texture is changed, use this to reload texture.
	 * Not support packingAtlas (static packing).
	 */
	void ReloadTexture();
	UFUNCTION(BlueprintCallable, Category = "LexUI") UTexture2D* GetSpriteTexture()const { return SpriteTexture; }
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void BeginCacheForCookedPlatformData(const ITargetPlatform* TargetPlatform) override;
	static void MarkAllSpritesNeedToReinitialize();
#endif
	static void CheckAndApplySpriteTextureSetting(UTexture2D* InSpriteTexture);
	static ULexUISpriteData* GetDefaultWhiteSolid();
	static ULexUISpriteData* GetDefaultFrameRect();
};