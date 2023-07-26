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
	UE_DEPRECATED(5.0, "Use GetExpandChildWidthArea instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetExpandChildWidthArea instead."))
		bool GetExpendChildrenWidth()const { return GetExpandChildWidthArea(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpandChildWidthArea()const { return ExpandChildWidthArea; }
	UE_DEPRECATED(5.0, "Use GetExpandChildHeightArea instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use GetExpandChildHeightArea instead."))
		bool GetExpendChildrenHeight()const { return GetExpandChildHeightArea(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpandChildHeightArea()const { return ExpandChildHeightArea; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren()const { return WidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHeightFitToChildren()const { return HeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetHeightFitToChildrenFromMinToMax()const { return HeightFitToChildrenFromMinToMax; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlChildWidth()const { return ControlChildWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetControlChildHeight()const { return ControlChildHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetUseChildScaleOnWidth()const { return UseChildScaleOnWidth; }
	/** return children's fill range in horizontal */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetActuralRange()const { return ActuralRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlign(ELGUILayoutAlignmentType value);
	UE_DEPRECATED(5.0, "Use SetExpandChildWidthArea instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetExpandChildWidthArea instead."))
		void SetExpendChildrenWidth(bool value) { SetExpandChildWidthArea(value); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpandChildWidthArea(bool value);
	UE_DEPRECATED(5.0, "Use SetExpandChildHeightArea instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetExpandChildHeightArea instead."))
		void SetExpendChildrenHeight(bool value) { SetExpandChildHeightArea(value); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpandChildHeightArea(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildren(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildrenFromMinToMax(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUseChildScaleOnWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlChildWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetControlChildHeight(bool value);

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
	/** Control child width to fit it's area */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlChildWidth = true;
	/** Control child height to fit it's area */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ControlChildHeight = true;
	/** Expand width area for child */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpandChildWidthArea = false;
	/** Expand height area for child */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpandChildHeightArea = false;
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
		bool UseChildScaleOnWidth = false;

	/** actural children's fill range in horizontal */
	float ActuralRange;

	struct FChildSize
	{
		float AreaWidth;
		float Width;
	};
	TArray<FChildSize> childrenWidthList;
};
