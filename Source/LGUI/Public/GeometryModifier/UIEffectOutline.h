// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
		FVector2D outlineSize = FVector2D(1, 1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool multiplySourceAlpha = true;
	/** Default is 4 direction. 8 direction will get nicer look. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Use 8 Direction"))
		bool use8Direction = false;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, uint8 InSourceAlpha);
public:
	virtual void ModifyUIGeometry(UIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetOutlineColor()const { return outlineColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetOutlineSize()const { return outlineSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUse8Direction()const { return use8Direction; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineColor(FColor newColor);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineSize(FVector2D newSize);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUse8Direction(bool newValue);
};
