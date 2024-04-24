// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UIPanelLayoutBase.h"
#include "Components/SlateWrapperTypes.h"
#include "UIPanelLayout_VerticalBox.generated.h"

/**
 * Layout child elements side by side horizontally
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIPanelLayout_VerticalBox : public UUIPanelLayoutWithOverrideOrder
{
	GENERATED_BODY()
protected:
	/** this object's width set to all children's range */
	UPROPERTY(EditAnywhere, Category = "Panel Layout")
		bool bWidthFitToChildren = false;
	/** lerp from min child's width to max child's width */
	UPROPERTY(EditAnywhere, Category = "Panel Layout", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bWidthFitToChildren"))
		float WidthFitToChildrenFromMinToMax = 1.0f;
	/** this object's height set to children height */
	UPROPERTY(EditAnywhere, Category = "Panel Layout")
		bool bHeightFitToChildren = false;
public:
	virtual void OnRebuildLayout()override;

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;

	virtual UClass* GetPanelLayoutSlotClass()const override;
#if WITH_EDITOR
	virtual FText GetCategoryDisplayName()const override;
	virtual bool CanMoveChildToCell(UUIItem* InChild, EMoveChildDirectionType InDirection)const override;
	virtual void MoveChildToCell(UUIItem* InChild, EMoveChildDirectionType InDirection)override;
#endif
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		bool GetWidthFitToChildren()const { return bWidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		bool GetHeightFitToChildren()const { return bHeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		float GetWidthFitToChildrenFromMinToMax()const { return WidthFitToChildrenFromMinToMax; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetWidthFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetHeightFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetWidthFitToChildrenFromMinToMax(float Value);
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API UUIPanelLayout_VerticalBox_Slot : public UUIPanelLayoutSlotWithOverrideOrder
{
	GENERATED_BODY()
protected:
	friend class FUIPanelLayoutVerticalBoxSlotCustomization;
	/** The padding area between the slot and the content it contains. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		FMargin Padding;
	/** How much space this slot should occupy in the direction of the panel. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		FSlateChildSize SizeRule;
	/** The alignment of the object horizontally. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;
	/** The alignment of the object vertically. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment;
public:
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		const FMargin& GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		const FSlateChildSize& GetSizeRule()const { return SizeRule; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EHorizontalAlignment GetHorizontalAlignment()const { return HorizontalAlignment; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EVerticalAlignment GetVerticalAlignment()const { return VerticalAlignment; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetPadding(const FMargin& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetSizeRule(const FSlateChildSize& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetHorizontalAlignment(const EHorizontalAlignment& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetVerticalAlignment(const EVerticalAlignment& Value);
};
