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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor ShadowColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector3f ShadowSize = FVector3f(0, 1, -1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint8 ShadowSegment = 5;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bUseGradientColor = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor GradientColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bMultiplySourceAlpha = true;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor, uint8 InOriginAlpha);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetShadowColor()const { return ShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector3f GetShadowSize()const { return ShadowSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		uint8 GetShadowSegments()const { return ShadowSegment; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseGradientColor()const { return bUseGradientColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetGradientColor()const { return GradientColor; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowSize(FVector3f Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowSegment(uint8 Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseGradientColor(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetGradientColor(FColor Value);
};
