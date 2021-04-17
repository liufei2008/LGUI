// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteData.generated.h"


/**
 * SpriteInfo contains information for render a sprite
 */
USTRUCT(BlueprintType)
struct LGUI_API FLGUISpriteInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint16 width = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI")
		uint16 height = 0;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 borderLeft = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 borderRight = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 borderTop = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint16 borderBottom = 0;

	float uv0X = 0;
	float uv0Y = 1;
	float uv3X = 1;
	float uv3Y = 0;

	float buv0X = 0;
	float buv0Y = 1;
	float buv3X = 1;
	float buv3Y = 0;

public:
	FVector2D GetUV0()const { return FVector2D(uv0X, uv0Y); }
	FVector2D GetUV1()const { return FVector2D(uv3X, uv0Y); }
	FVector2D GetUV2()const { return FVector2D(uv0X, uv3Y); }
	FVector2D GetUV3()const { return FVector2D(uv3X, uv3Y); }
	
	bool HasBorder()const;
	void ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal);
	void ApplyUV(int32 InX, int32 InY, int32 InWidth, int32 InHeight, float texFullWidthReciprocal, float texFullHeightReciprocal, const FVector4& uvRect);
	void ApplyBorderUV(float texFullWidthReciprocal, float texFullHeightReciprocal);
	void ScaleUV(float InMultiply)
	{
		uv0X *= InMultiply;
		uv0Y *= InMultiply;
		uv3X *= InMultiply;
		uv3Y *= InMultiply;

		buv0X *= InMultiply;
		buv3X *= InMultiply;
		buv0Y *= InMultiply;
		buv3Y *= InMultiply;
	}

	bool operator == (const FLGUISpriteInfo& Other)const
	{
		return width == Other.width 
			&& height == Other.height 
			&& borderLeft == Other.borderLeft
			&& borderRight == Other.borderRight 
			&& borderTop == Other.borderTop
			&& borderBottom == Other.borderBottom
			;
	}
	bool operator != (const FLGUISpriteInfo& Other)const
	{
		return width != Other.width
			|| height != Other.height
			|| borderLeft != Other.borderLeft
			|| borderRight != Other.borderRight
			|| borderTop != Other.borderTop
			|| borderBottom != Other.borderBottom
			;
	}
};

class ULGUISpriteData;
class UUISpriteBase;
struct FLGUIAtlasData;
class ULGUIAtlas;

#define WARNING_ATLAS_SIZE 4096

/**
 * A sprite contains a texture and a spritedata
 */
UCLASS(BlueprintType)
class LGUI_API ULGUISpriteData :public UObject
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
	/** initialize sprite data, only need to call once */
	UFUNCTION(BlueprintCallable, Category = "LGUI") void InitSpriteData();

	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture2D * InitAndGetAtlasTexture();
	UFUNCTION(BlueprintCallable, Category = "LGUI") const FLGUISpriteInfo& InitAndGetSpriteInfo();
	UFUNCTION(BlueprintCallable, Category = "LGUI") bool HavePackingTag()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI") const FName& GetPackingTag()const;

	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") UTexture2D * GetAtlasTexture()const;
	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") const FLGUISpriteInfo& GetSpriteInfo()const;
	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") void GetSpriteSize(int32& width, int32& height)const;
	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") void GetSpriteBorderSize(int32& borderLeft, int32& borderRight, int32& borderTop, int32& borderBottom)const;
	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") void GetSpriteUV(float& UV0X, float& UV0Y, float& UV3X, float& UV3Y)const;
	/** always remember to call InitSpriteData() before this function to initialize this SpriteData */
	UFUNCTION(BlueprintCallable, Category = "LGUI") void GetSpriteBorderUV(float& borderUV0X, float& borderUV0Y, float& borderUV3X, float& borderUV3Y)const;

	/**
	 * Create a LGUIspriteData with provided parameter. This can use at runtime
	 * @param inSpriteTexture			Use this texture to create
	 * @param inHorizontalBorder		Horizontal border value, x for left, y for right, will be convert to uint16</param>
	 * @param inVerticalBorder			Vertical border value, x for top, y for bottom, will be convert to uint16</param>
	 * @param inPackingTag				see "packingTag" property
	 * @return							Created LGUISpriteData, nullptr if something wrong.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (WorldContext = "WorldContextObject"))
		static ULGUISpriteData* CreateLGUISpriteData(UObject* WorldContextObject, UTexture2D* inSpriteTexture, FVector2D inHorizontalBorder = FVector2D::ZeroVector, FVector2D inVerticalBorder = FVector2D::ZeroVector, FName inPackingTag = TEXT("Main"));

	void AddUISprite(UUISpriteBase* InUISprite);
	void RemoveUISprite(UUISpriteBase* InUISprite);
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