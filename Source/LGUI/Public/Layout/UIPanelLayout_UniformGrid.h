// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UIPanelLayoutBase.h"
#include "Components/SlateWrapperTypes.h"
#include "UIPanelLayout_UniformGrid.generated.h"

/**
 * Layout child elements side by side horizontally
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIPanelLayout_UniformGrid : public UUIPanelLayoutBase
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
	/** lerp from min child's height to max child's height */
	UPROPERTY(EditAnywhere, Category = "Panel Layout", meta = (ClampMin = "0.0", ClampMax = "1.0", EditCondition = "bHeightFitToChildren"))
		float HeightFitToChildrenFromMinToMax = 1.0f;
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
		float GetHeightFitToChildrenFromMinToMax()const { return HeightFitToChildrenFromMinToMax; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetWidthFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetHeightFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetWidthFitToChildrenFromMinToMax(float Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetHeightFitToChildrenFromMinToMax(float Value);
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API UUIPanelLayout_UniformGrid_Slot : public UUIPanelLayoutSlotBase
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	void PostEditUndo()override;
#endif
	friend class FUIPanelLayoutUniformGridSlotCustomization;
	/** The padding area between the slot and the content it contains. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		FMargin Padding;
	/** The alignment of the object horizontally. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment;
	/** The alignment of the object vertically. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment;
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot", meta = (UIMin = "0"))
		int Column = 0;
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot", meta = (UIMin = "0"))
		int Row = 0;
public:
	void VerifyColumnAndRow(UUIPanelLayout_UniformGrid* Layout);

	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		const FMargin& GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EHorizontalAlignment GetHorizontalAlignment()const { return HorizontalAlignment; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EVerticalAlignment GetVerticalAlignment()const { return VerticalAlignment; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetColumn()const { return Column; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetRow()const { return Row; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetPadding(const FMargin& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetHorizontalAlignment(EHorizontalAlignment Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetVerticalAlignment(EVerticalAlignment Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetColumn(int Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetRow(int Value);
};
