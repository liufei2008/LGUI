// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "Layout/Margin.h"
#include "UIGridLayout.generated.h"

//UENUM()
//enum class EUIGridLayoutStartCorner :uint8
//{
//	UpperLeft,
//	UpperRight,
//	LowerLeft,
//	LowerRight,
//};

UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIGridLayout : public UUILayoutBase
{
	GENERATED_BODY()

public:
	virtual void OnRebuildLayout()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding() { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetSpacing() { return Spacing; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHorizontalOrVertical() { return HorizontalOrVertical; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetDependOnSizeOrCount() { return DependOnSizeOrCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildSize() { return ExpendChildSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetCellSize() { return CellSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetLineCount() { return LineCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren() { return WidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHeightFitToChildren() { return HeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetActuralRange() { return ActuralRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHorizontalOrVertical(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetDependOnSizeOrCount(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpendChildSize(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCellSize(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetLineCount(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildren(bool value);
	
	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
protected:

	friend class FUIGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D Spacing;
	//Which direction to tile children, true for horizontal, false for vertical
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HorizontalOrVertical = true;
	//Arrange by size(CellSize) or by count(LineCount), true for size, false for count
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool DependOnSizeOrCount = true;
	//If depend on count, then expend cell size to fill. 
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpendChildSize = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D CellSize = FVector2D(100, 100);
	UPROPERTY(EditAnywhere, Category = "LGUI")
		uint32 LineCount = 5;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool WidthFitToChildren = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HeightFitToChildren = false;

	//actural children fill range
	FVector2D ActuralRange;
};
