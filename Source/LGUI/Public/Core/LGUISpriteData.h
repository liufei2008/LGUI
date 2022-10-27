// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteData_BaseObject.h"
#include "LGUISpriteData.generated.h"


class ULGUISpriteData;
class UUISpriteBase;
struct FLGUIAtlasData;

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
	friend struct FLGUIAtlasData;
	/**
	 * Texture of this sprite. Sprite is acturally renderred from atlas texture, so spriteTexture is not needed if atlasdata is packed; But! since atlas texture is packed at runtime, we must include spriteTexture inside final package.
	 * Donot modify spriteTexture's setting unless you know what you doing
	 */
	UPROPERTY(EditAnywhere, Category = LGUI)
		UTexture2D* spriteTexture;
	/** Information needed for render this sprite */
	UPROPERTY(EditAnywhere, Category = LGUI)
		FLGUISpriteInfo spriteInfo;
	/** Repeat edge pixel fill spaced between other sprites in atlas texture */
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool useEdgePixelPadding = true;
	/** Sprites that have same packingTag will be packed into same atlas. If packingTag is None, then the UISprite which render this LGUISpriteData will be treated as a UITexture */
	UPROPERTY(EditAnywhere, Category = LGUI)
		FName packingTag = TEXT("Main");
private:
	bool isInitialized = false;
	UPROPERTY(Transient)UTexture2D * atlasTexture = nullptr;
	bool PackageSprite();
	bool InsertTexture(FLGUIAtlasData* InAtlasData);
	void CheckSpriteTexture();
	void CopySpriteTextureToAtlas(rbp::Rect InPackedRect, int32 InAtlasTexturePadding);
	void ApplySpriteInfoAfterStaticPack(const rbp::Rect& InPackedRect, float InAtlasTextureSizeInv, UTexture2D* InAtlasTexture);
public:
	//Begin ULGUISpriteData_BaseObject interface
	virtual UTexture2D * GetAtlasTexture()override;
	virtual const FLGUISpriteInfo& GetSpriteInfo()override;
	virtual bool IsIndividual()const override;
	virtual void AddUISprite(UUISpriteBase* InUISprite)override;
	virtual void RemoveUISprite(UUISpriteBase* InUISprite)override;
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

	/** if texture is changed, use this to reload texture */
	void ReloadTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture2D* GetSpriteTexture()const { return spriteTexture; }
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
	static void MarkAllSpritesNeedToReinitialize();
#endif
	static void CheckAndApplySpriteTextureSetting(UTexture2D* InSpriteTexture);
	static ULGUISpriteData* GetDefaultWhiteSolid();
};