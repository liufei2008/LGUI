// Copyright 2025-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Core/LexUISpriteInfo.h"
#include "LexUIImageBrush.generated.h"

UENUM(BlueprintType)
enum class ELexUIImageBrushDrawType:uint8
{
	None,
	/** Draw a 3x3 box, where the sides and the middle stretch based on the Margin */
	Box,
	/** Draw a 3x3 border where the sides tile and the middle is empty */
	Border,
	/** Draw an image; margin is ignored */
	Image,
	/** Draw a sliced image with its resizable sections tiled instead of stretched */
	//Tiled,
	/**
	 * A Filled Image will display a section of the image, with the rest of transparent.
	 * The FillAmount determines how much of the Image to show, and FillMethod controls the shape in which the Image will be cut.
	 */
	//Filled,
};


USTRUCT(BlueprintType)
struct LGUI_API FLexUIImageBrush
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush", meta = (AllowPrivateAccess = "true", DisplayThumbnail = "true", DisplayName = "Image"
		, AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface,/Script/LGUI.LexUISpriteData_BaseObject"))
	TObjectPtr<UObject> ResourceObject;
public:
	FLexUIImageBrush();
	static FName GetPropertyName_ResourceObject()
	{
		return GET_MEMBER_NAME_CHECKED(FLexUIImageBrush, ResourceObject);
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	FColor TintColor = FColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	ELexUIImageBrushDrawType DrawAs = ELexUIImageBrushDrawType::Box;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	FVector2f ImageSize = FVector2f(32, 32);
	/** Margin size from 0 to 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush", meta = (UVSpace="true"))
	FMargin Margin;
	/** UV region for an image, xy for min and zw for max. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	FVector4f UVRegion = FVector4f(0,0,1,1);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	float PixelsPerUnitMultiplier = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ImageBrush")
	ELexUISpriteFlipMode FlipMode = ELexUISpriteFlipMode::Nothing;
	
	UObject* GetResourceObject()const { return ResourceObject; }
	void SetResourceObject(UObject* Value) { ResourceObject = Value; }
};
