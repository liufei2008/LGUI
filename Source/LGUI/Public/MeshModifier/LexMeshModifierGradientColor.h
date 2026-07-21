// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexMeshModifierBase.h"
#include "LexMeshModifierGradientColor.generated.h"


UENUM(BlueprintType, Category = LGUI)
enum class ELexMeshModifierGradientColorDirection :uint8
{
	BottomToTop,
	TopToBottom,
	LeftToRight,
	RightToLeft,
	FourCorner,
};
UCLASS(ClassGroup = (LGUI), Blueprintable, DisplayName="GradientColor", meta = (BlueprintSpawnableComponent))
class LGUI_API ULexMeshModifierGradientColor : public ULexMeshModifierBase
{
	GENERATED_BODY()

public:	
	ULexMeshModifierGradientColor();

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELexMeshModifierGradientColorDirection DirectionType = ELexMeshModifierGradientColorDirection::BottomToTop;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bMultiplySourceAlpha = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor Color1 = FColor::Black;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor Color2 = FColor::White;

	//only use for FourCorner
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="DirectionType==ELexMeshModifierGradientColorDirection::FourCorner"))
		FColor Color3 = FColor::Black;
	//only use for FourCorner
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="DirectionType==ELexMeshModifierGradientColorDirection::FourCorner"))
		FColor Color4 = FColor::White;
	FORCEINLINE void ApplyColorAndAlpha(FColor& InOutColor, FColor InTintColor);
public:
	virtual void ModifyUIGeometry(FLexUIGeometry& InGeometry
		, bool InTriangleChanged, bool InUVChanged, bool InColorChanged, bool InVertexPositionChanged
	)override;
	virtual void ModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor)override
	{
		OutTriangleIndices = false;
		OutVertexPosition = false;
		OutUV = false;
		OutColor = true;
	};

	UFUNCTION(BlueprintCallable, Category = "LGUI")
	ELexMeshModifierGradientColorDirection GetDirectionType()const{return DirectionType;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	bool GetMultiplySourceAlpha()const{return bMultiplySourceAlpha;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FColor GetColor1()const{return Color1;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FColor GetColor2()const{return Color2;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FColor GetColor3()const{return Color3;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	FColor GetColor4()const{return Color4;}
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetDirectionType(ELexMeshModifierGradientColorDirection Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetMultiplySourceAlpha(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetColor1(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetColor2(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetColor3(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetColor4(FColor Value);
};
