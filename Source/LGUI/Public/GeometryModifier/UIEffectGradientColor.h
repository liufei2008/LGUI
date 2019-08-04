﻿// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectGradientColor.generated.h"


UENUM(BlueprintType)
enum class EUIEffectGradientColorDirection :uint8
{
	BottomToTop,
	TopToBottom,
	LeftToRight,
	RightToLeft,
	FourCornor,
};
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectGradientColor : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectGradientColor();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIEffectGradientColorDirection directionType = EUIEffectGradientColorDirection::BottomToTop;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool multiplySourceAlpha = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor color1 = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor color2 = FColor::White;

	//only use for FourCornor
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor color3 = FColor::Black;
	//only use for FourCornor
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor color4 = FColor::White;
public:
	virtual void ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)override;
};
