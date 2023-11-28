// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUISpriteInfo.generated.h"


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

	/** left point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv0X = 0;
	/** bottom point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv0Y = 1;
	/** right point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv3X = 1;
	/** top point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float uv3Y = 0;

	/** border left point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv0X = 0;
	/** border bottom point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv0Y = 1;
	/** border right point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv3X = 1;
	/** border top point uv */
	UPROPERTY(VisibleAnywhere, Category = "LGUI", AdvancedDisplay)
		float buv3Y = 0;

public:
	FVector2D GetUV0()const { return FVector2D(uv0X, uv0Y); }
	FVector2D GetUV1()const { return FVector2D(uv3X, uv0Y); }
	FVector2D GetUV2()const { return FVector2D(uv0X, uv3Y); }
	FVector2D GetUV3()const { return FVector2D(uv3X, uv3Y); }

	auto GetUVCenter()const { return FVector2D((uv3X - uv0X) * 0.5f + uv0X, (uv0Y - uv3Y) * 0.5f + uv3Y); }

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
