// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIAnchorData.generated.h"

/** UI rect transform anchor data */
USTRUCT(BlueprintType)
struct LGUI_API FUIAnchorData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D Pivot = FVector2D(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D AnchorMin = FVector2D(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D AnchorMax = FVector2D(0.5f, 0.5f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D AnchoredPosition = FVector2D(0, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		FVector2D SizeDelta = FVector2D(100, 100);

	bool IsHorizontalStretched()const { return AnchorMin.X != AnchorMax.X; }
	bool IsVerticalStretched()const { return AnchorMin.Y != AnchorMax.Y; }
};
