// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIWidget.generated.h"


UENUM(BlueprintType)
enum class UIAnchorHorizontalAlign :uint8
{
	/** Usually dont use this "None" mode */
	None	UMETA(DisplayName = "None"),
	Left	UMETA(DisplayName = "Left"),
	Center	UMETA(DisplayName = "Center"),
	Right	UMETA(DisplayName = "Right"),
	Stretch UMETA(DisplayName = "Stretch"),
};
UENUM(BlueprintType)
enum class UIAnchorVerticalAlign :uint8
{
	/** Usually dont use this "None" mode */
	None	UMETA(DisplayName = "None"),
	Top		UMETA(DisplayName = "Top"),
	Middle	UMETA(DisplayName = "Middle"),
	Bottom	UMETA(DisplayName = "Bottom"),
	Stretch UMETA(DisplayName = "Stretch"),
};

/** UI's base data, rect transform, color */
USTRUCT(BlueprintType)
struct LGUI_API FUIWidget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		int32 depth = 0;
	/** sprite may override by UISelectable(UIButton, UIToggle, UISlider ...) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FColor color = FColor::White;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D pivot = FVector2D(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		UIAnchorHorizontalAlign anchorHAlign = UIAnchorHorizontalAlign::Center;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		UIAnchorVerticalAlign anchorVAlign = UIAnchorVerticalAlign::Middle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float anchorOffsetX = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float anchorOffsetY = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float width = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float height = 100;
	/** distance from self bottom to parent left */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float stretchLeft = 0;
	/** distance from self right to parent right */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float stretchRight = 0;
	/** distance from self top to parent top */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float stretchTop = 0;
	/** distance from self bottom to parent bottom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float stretchBottom = 0;
};
