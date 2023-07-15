// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
#include "Layout/Margin.h"
#include "UIHorizontalLayout.generated.h"

/**
 * Layout child elements side by side horizontally
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIHorizontalLayout : public UUILayoutWithAnimation
{
	GENERATED_BODY()
public:
	virtual void OnRebuildLayout()override;
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetSpacing()const { return Spacing; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELGUILayoutAlignmentType GetAlign()const { return Align; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpandChildrenWidth()const { return ExpandChildrenWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpandChildrenHeight()const { return ExpandChildrenHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren()const { return WidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHeightFitToChildren()const { return HeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetHeightFitToChildrenFromMinToMax()const { return HeightFitToChildrenFromMinToMax; }
	/** return children's fill range in horizontal */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetActuralRange()const { return ActuralRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlign(ELGUILayoutAlignmentType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpandChildrenWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpandChildrenHeight(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildrenFromMinToMax(float value);

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;

	friend class FUIHorizontalLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float Spacing;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUILayoutAlignmentType Align = ELGUILayoutAlignmentType::UpperLeft;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpandChildrenWidth = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpandChildrenHeight = false;
	/** this object's width set to all children's range */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool WidthFitToChildren = false;
	/** this object's height set to children height */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HeightFitToChildren = false;
	/** lerp from min child's height to max child's height */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
		float HeightFitToChildrenFromMinToMax = 1.0f;
	/** Whether children widths are scaled by their horizontal scale. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseChildrenScaleOnWidth = false;

	/** actural children's fill range in horizontal */
	float ActuralRange;

	TArray<float> childrenWidthList;
};
