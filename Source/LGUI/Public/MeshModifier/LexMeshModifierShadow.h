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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor ShadowColor = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bMultiplySourceAlpha = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector3f ShadowOffset = FVector3f(0, 1, -1);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetShadowColor()const { return ShadowColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector3f GetShadowOffset()const { return ShadowOffset; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetShadowOffset(FVector3f Value);
};
