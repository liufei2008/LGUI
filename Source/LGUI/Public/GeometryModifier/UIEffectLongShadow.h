// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectLongShadow.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectLongShadow : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectLongShadow();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor shadowColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector shadowSize = FVector(0, 1, -1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 shadowSegment = 5;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool useGradientColor = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor gradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool multiplySourceAlpha = true;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha);
public:
	virtual void ModifyUIGeometry(UIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetShadowColor()const { return shadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector GetShadowSize()const { return shadowSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		uint8 GetShadowSegments()const { return shadowSegment; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseGradientColor()const { return useGradientColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetGradientColor()const { return gradientColor; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowColor(FColor newColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowSize(FVector newSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowSegment(uint8 newSegment);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseGradientColor(bool newBool);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientColor(FColor newColor);
};
