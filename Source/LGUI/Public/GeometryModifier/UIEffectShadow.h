// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectShadow.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectShadow : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectShadow();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor shadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool multiplySourceAlpha = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D shadowOffset = FVector2D(1, -1);
public:
	virtual void ModifyUIGeometry(
		TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged,
		bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged
		)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetShadowColor()const { return shadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetShadowOffset()const { return shadowOffset; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowColor(FColor newColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowOffset(FVector2D newOffset);
};
