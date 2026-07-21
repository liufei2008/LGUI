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
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor OutlineColor = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2f OutlineSize = FVector2f(1, 1);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bMultiplySourceAlpha = true;
	/** Default is 4 direction. 8 direction will get nicer look. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "Use 8 Direction"))
		bool bUse8Direction = false;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, uint8 InSourceAlpha);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetOutlineColor()const { return OutlineColor; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2f GetOutlineSize()const { return OutlineSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUse8Direction()const { return bUse8Direction; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetOutlineSize(FVector2f Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUse8Direction(bool Value);
};
