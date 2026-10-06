// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierShadow.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="Shadow", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierShadow : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierShadow();

protected:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FColor ShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bMultiplySourceColor = true;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bMultiplySourceAlpha = true;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FVector3f ShadowOffset = FVector3f(0, 1, -1);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	FColor GetShadowColor()const { return ShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	bool GetMultiplySourceColor()const { return bMultiplySourceColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	bool GetMultiplySourceAlpha()const { return bMultiplySourceAlpha; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	FVector3f GetShadowOffset()const { return ShadowOffset; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetShadowColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetMultiplySourceColor(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetMultiplySourceAlpha(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetShadowOffset(FVector3f Value);
};
