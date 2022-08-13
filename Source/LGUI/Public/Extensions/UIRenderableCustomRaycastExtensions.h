// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "UIRenderableCustomRaycastExtensions.generated.h"

#if 0
/**
 * Raycast hit test in circle area.
 */
UCLASS(ClassGroup = (LGUI), BlueprintType)
class LGUI_API UUIRenderableCustomRaycast_Circle : public UUIRenderableCustomRaycast
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		float RadiusRange = 1.0f;
public:
	virtual bool Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)override;
};
#endif

/**
 * Raycast hit test in visible pixel.
 * Only support UI element type which can read pixel value from texture:
	 *			1. UITexture that use a Texture2D can work perfectly.
	 *			2. UISprite which use dynamic atlas packing can not work.
	 *			3. UIText which use dynamic font can not work.
 */
UCLASS(ClassGroup = (LGUI), BlueprintType)
class LGUI_API UUIRenderableCustomRaycast_VisiblePixel : public UUIRenderableCustomRaycast
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		float VisibilityThreshold = 0.1f;
	/** Use one of pixel's rgba channel, 0123 as rgba, default is 3 means alpha channel */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 PixelChannel = 3;
public:
	virtual bool Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetVisibilityThreshold()const { return VisibilityThreshold; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		uint8 GetPixelChannel()const { return PixelChannel; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVisibilityThreshold(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPixelChannel(uint8 value);
};
