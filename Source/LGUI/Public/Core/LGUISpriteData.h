// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteData_BaseObject.h"
#include "LGUISpriteData.generated.h"


class ULGUISpriteData;
class UUISpriteBase;
struct FLGUIDynamicSpriteAtlasData;
class ULGUIStaticSpriteAtlasData;

#define WARNING_ATLAS_SIZE 4096

/**
 * A sprite-data type that can do automatic packing
 */
UCLASS(BlueprintType)
class LGUI_API ULGUISpriteData :public ULGUISpriteData_BaseObject
{
	GENERATED_BODY()
private:
	friend class FLGUISpriteDataCustomization;
	friend class ULGUISpriteDataFactory;
	friend struct FLGUIDynamicSpriteAtlasData;
	friend class ULGUIStaticSpriteAtlasData;
	/**
	 * Texture of this sprite. Sprite is acturally renderred from atlas texture, so spriteTexture is not needed if atlasdata is packed; But! since atlas texture is packed at runtime, we must include spriteTexture inside final package.
	 * Donot modify spriteTexture's setting unless you know what you doing
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		UTexture2D* spriteTexture;
	/** Information needed for render this sprite */
	UPROPERTY(EditAnywhere, Category = LGUI)
		FLGUISpriteInfo spriteInfo;

	/**
	 * Use a StaticSpriteAtlasData to pack multiple sprites into single atlas texture. The packing process is in editor and cook time, no performance impack at runtime.
	 * Support mipmaps.
	 */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking")
		ULGUIStaticSpriteAtlasData* packingAtlas = nullptr;
	/**
	 * Sprites that have same packingTag will be packed into same atlas at runtime. If packingTag is None, then the UISprite which render this LGUISpriteData will be treated as a UITexture.
	 * Not support mipmaps.
	 * Only valid if PackingAtals is empty.
	 */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking")
		FName packingTag = TEXT("Main");
	/** Repeat edge pixel fill spaced between other sprites in atlas texture */
	UPROPERTY(EditAnywhere, Category = "AtlasPacking")
		bool useEdgePixelPadding = true;
private:
	bool isInitialized = false;
	UPROPERTY(Transient)UTexture2D * atlasTexture = nullptr;
	bool PackageSprite();
	bool InsertTexture(FLGUIDynamicSpriteAtlasData* InAtlasData);
	void CheckSpriteTexture();
	void CopySpriteTextureToAtlas(rbp::Rect InPackedRect, int32 InAtlasTexturePadding);
public:
	bool GetUseEdgePixelPadding()const { return useEdgePixelPadding; }
	ULGUIStaticSpriteAtlasData* GetPackingAtlas()const { return packingAtlas; }
	void ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv);
	//Begin ULGUISpriteData_BaseObject interface
	virtual UTexture2D * GetAtlasTexture()override;
	virtual const FLGUISpriteInfo& GetSpriteInfo()override;
	virtual bool IsIndividual()const override;
	virtual void AddUISprite(TScriptInterface<class IUISpriteRenderableInterface> InUISprite)override;
	virtual void RemoveUISprite(TScriptInterface<class IUISpriteRenderableInterface> InUISprite)override;
	virtual bool ReadPixel(const FVector2D& InUV, FColor& OutPixel)const override;
	virtual bool SupportReadPixel()const override;
	//End ULGUISpriteData_BaseObject interface

	/** initialize sprite data */
	void InitSpriteData();
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool HavePackingTag()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI") const FName& GetPackingTag()const;

	/**
	 * Create a LGUIspriteData with provided parameter. This can use at runtime
	 * @param Outer						Outer of the result LGUISpriteData
	 * @param inSpriteTexture			Use this texture to create
	 * @param inHorizontalBorder		Horizontal border value, x for left, y for right, will be convert to uint16</param>
	 * @param inVerticalBorder			Vertical border value, x for top, y for bottom, will be convert to uint16</param>
	 * @param inPackingTag				see "packingTag" property
	 * @return							Created LGUISpriteData, nullptr if something wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (WorldContext = "WorldContextObject"))
		static ULGUISpriteData* CreateLGUISpriteData(UObject* Outer, UTexture2D* inSpriteTexture, FVector2D inHorizontalBorder = FVector2D::ZeroVector, FVector2D inVerticalBorder = FVector2D::ZeroVector, FName inPackingTag = TEXT("Main"));

	/**
	 * If texture is changed, use this to reload texture.
	 * Not support packingAtlas (static packing).
	 */
	void ReloadTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture2D* GetSpriteTexture()const { return spriteTexture; }
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	static void MarkAllSpritesNeedToReinitialize();
#endif
	static void CheckAndApplySpriteTextureSetting(UTexture2D* InSpriteTexture);
	static ULGUISpriteData* GetDefaultWhiteSolid();
};