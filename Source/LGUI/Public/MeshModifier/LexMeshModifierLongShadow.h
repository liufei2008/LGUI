// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierLongShadow.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="LongShadow", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierLongShadow : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierLongShadow();

protected:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FColor ShadowColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FVector3f ShadowSize = FVector3f(0, 1, -1);
	UPROPERTY(EditAnywhere, Category = "LexUI")
		uint8 ShadowSegment = 5;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bUseGradientColor = true;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FColor GradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bMultiplySourceAlpha = true;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FColor GetShadowColor()const { return ShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FVector3f GetShadowSize()const { return ShadowSize; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		uint8 GetShadowSegments()const { return ShadowSegment; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetUseGradientColor()const { return bUseGradientColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FColor GetGradientColor()const { return GradientColor; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetShadowColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetShadowSize(FVector3f Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetShadowSegment(uint8 Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetUseGradientColor(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetGradientColor(FColor Value);
};
