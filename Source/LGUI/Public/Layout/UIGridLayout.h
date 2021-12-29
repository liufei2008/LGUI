// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
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

/**
 * Layout child elements in grid
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIGridLayout : public UUILayoutWithAnimation
{
	GENERATED_BODY()

public:
	virtual void OnRebuildLayout()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetSpacing()const { return Spacing; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELGUILayoutAlignmentType GetAlign()const { return Align; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetLastLineCanAlign()const { return LastLineCanAlign; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHorizontalOrVertical()const { return HorizontalOrVertical; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetDependOnSizeOrCount()const { return DependOnSizeOrCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildSize()const { return ExpendChildSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetCellSize()const { return CellSize; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetMaxItemCountInOneLine()const { return LineCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren()const { return WidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHeightFitToChildren()const { return HeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetActuralRange()const { return ActuralRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlign(ELGUILayoutAlignmentType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetLastLineCanAlign(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHorizontalOrVertical(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetDependOnSizeOrCount(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpendChildSize(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCellSize(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaxItemCountInOneLine(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildren(bool value);

	virtual bool CanControlChildAnchor_Implementation()const override;
	virtual bool CanControlChildHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildWidth_Implementation()const override;
	virtual bool CanControlChildHeight_Implementation()const override;
	virtual bool CanControlChildAnchorLeft_Implementation()const override;
	virtual bool CanControlChildAnchorRight_Implementation()const override;
	virtual bool CanControlChildAnchorBottom_Implementation()const override;
	virtual bool CanControlChildAnchorTop_Implementation()const override;

	virtual bool CanControlSelfAnchor_Implementation()const override;
	virtual bool CanControlSelfHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfWidth_Implementation()const override;
	virtual bool CanControlSelfHeight_Implementation()const override;
	virtual bool CanControlSelfAnchorLeft_Implementation()const override;
	virtual bool CanControlSelfAnchorRight_Implementation()const override;
	virtual bool CanControlSelfAnchorBottom_Implementation()const override;
	virtual bool CanControlSelfAnchorTop_Implementation()const override;
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;

	friend class FUIGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D Spacing;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUILayoutAlignmentType Align = ELGUILayoutAlignmentType::UpperLeft;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool LastLineCanAlign = true;
	/** Which direction to tile children, true for horizontal, false for vertical. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HorizontalOrVertical = true;
	/** Arrange by size(CellSize) or by count(MaxItemCountInOneLine), true for size, false for count. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool DependOnSizeOrCount = true;
	/** 
	 * If depend on count:
	 *	If Horizontal then expend cell height to fill.
	 *	If Vertical then expend cell width to fill.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpendChildSize = false;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition="DependOnSizeOrCount==true || ExpendChildSize==false"))
		FVector2D CellSize = FVector2D(100, 100);
	/** Max item count in one line. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (DisplayName = "MaxItemCountInOneLine", ClampMin="1"))
		uint32 LineCount = 5;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool WidthFitToChildren = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HeightFitToChildren = false;

	//actural children fill range
	FVector2D ActuralRange;
};
