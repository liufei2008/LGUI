// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometryModifierBase.h"
#include "UIEffectOutline.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEffectOutline : public UUIGeometryModifierBase
{
	GENERATED_BODY()

public:	
	UUIEffectOutline();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor outlineColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D outlineSize = FVector2D(1, -1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool multiplySourceAlpha = true;
public:
	virtual void ModifyUIGeometry(TSharedPtr<UIGeometry>& InGeometry, int32& InOutOriginVerticesCount, int32& InOutOriginTriangleIndicesCount, bool& OutTriangleChanged)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetOutlineColor()const { return outlineColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetOutlineSize()const { return outlineSize; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineColor(FColor newColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineSize(FVector2D newSize);
};
