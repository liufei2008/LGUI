// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/MaxRectsBinPack/MaxRectsBinPack.h"
#include "Engine/Texture2D.h"
#include "LGUISpriteData_BaseObject.generated.h"


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

	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 paddingLeft = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 paddingRight = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 paddingTop = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int16 paddingBottom = 0;

	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv0X = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv0Y = 1;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv3X = 1;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv3Y = 0;

	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv0X = 0;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv0Y = 1;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv3X = 1;
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv3Y = 0;

public:
	auto GetUV0()const { return FVector2f(uv0X, uv0Y); }
	auto GetUV1()const { return FVector2f(uv3X, uv0Y); }
	auto GetUV2()const { return FVector2f(uv0X, uv3Y); }
	auto GetUV3()const { return FVector2f(uv3X, uv3Y); }

	uint16 GetSourceWidth()const { return width + paddingLeft + paddingRight; }
	uint16 GetSourceHeight()const { return height + paddingTop + paddingBottom; }
	
	bool HasBorder()const;
	bool HasPadding()const;
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
			&& paddingLeft == Other.paddingLeft
			&& paddingRight == Other.paddingRight
			&& paddingTop == Other.paddingTop
			&& paddingBottom == Other.paddingBottom
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
			|| paddingLeft != Other.paddingLeft
			|| paddingRight != Other.paddingRight
			|| paddingTop != Other.paddingTop
			|| paddingBottom != Other.paddingBottom
			;
	}
};

class UUISpriteBase;

/**
 * Base class for sprite data.
 * A sprite is a small area renderred in a big atlas texture.
 */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUISpriteData_BaseObject :public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual UTexture2D* GetAtlasTexture()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetAtlasTexture, return nullptr;);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual const FLGUISpriteInfo& GetSpriteInfo()PURE_VIRTUAL(ULGUISpriteData_BaseObject::GetSpriteInfo, static FLGUISpriteInfo ForReturn; return ForReturn;);
	/** This sprite-data is a individal one? Means it will not pack into any atlas texture. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		virtual bool IsIndividual()const PURE_VIRTUAL(ULGUISpriteData_BaseObject::IsIndividual, return false;);

	virtual void AddUISprite(UUISpriteBase* InUISprite) {};
	virtual void RemoveUISprite(UUISpriteBase* InUISprite) {};

//#if WITH_EDITOR
//	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)override;
//#endif
};