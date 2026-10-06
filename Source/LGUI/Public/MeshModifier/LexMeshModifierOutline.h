// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierOutline.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="Outline", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierOutline : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierOutline();

protected:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FColor OutlineColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		FVector2f OutlineSize = FVector2f(1, 1);
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bMultiplySourceAlpha = true;
	/** Default is 4 direction. 8 direction will get nicer look. */
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (DisplayName = "Use 8 Direction"))
		bool bUse8Direction = false;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, uint8 InSourceAlpha);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FColor GetOutlineColor()const { return OutlineColor; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		FVector2f GetOutlineSize()const { return OutlineSize; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetUse8Direction()const { return bUse8Direction; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetOutlineColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetOutlineSize(FVector2f Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetUse8Direction(bool Value);
};
