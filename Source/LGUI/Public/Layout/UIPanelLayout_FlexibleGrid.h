// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UIPanelLayoutBase.h"
#include "Components/SlateWrapperTypes.h"
#include "UIPanelLayout_FlexibleGrid.generated.h"

UENUM(BlueprintType)
enum class EUIPanelLayout_FlexibleGridSizeRule :uint8
{
	Constant,
	Ratio,
};

USTRUCT(BlueprintType)
struct LGUI_API FUIPanelLayout_FlexibleGridSize
{
	GENERATED_BODY()
public:
	FUIPanelLayout_FlexibleGridSize() {}
	FUIPanelLayout_FlexibleGridSize(float InSize, EUIPanelLayout_FlexibleGridSizeRule InSizeType = EUIPanelLayout_FlexibleGridSizeRule::Ratio)
	{
		this->Value = InSize;
		this->SizeType = InSizeType;
	}

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0"))
		float Value = 1.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIPanelLayout_FlexibleGridSizeRule SizeType = EUIPanelLayout_FlexibleGridSizeRule::Ratio;

	bool operator == (const FUIPanelLayout_FlexibleGridSize& Other)const
	{
		return this->Value == Other.Value
			&& this->SizeType == Other.SizeType
			;
	}
};

/**
 * Layout child elements side by side horizontally
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIPanelLayout_FlexibleGrid : public UUIPanelLayoutBase
{
	GENERATED_BODY()
	UUIPanelLayout_FlexibleGrid();
protected:
	UPROPERTY(EditAnywhere, Category = "Panel Layout")
		TArray<FUIPanelLayout_FlexibleGridSize> Columns;
	UPROPERTY(EditAnywhere, Category = "Panel Layout")
		TArray<FUIPanelLayout_FlexibleGridSize> Rows;
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
		const TArray<FUIPanelLayout_FlexibleGridSize>& GetColumns()const { return Columns; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		const TArray<FUIPanelLayout_FlexibleGridSize>& GetRows()const { return Rows; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetColumns(const TArray<FUIPanelLayout_FlexibleGridSize>& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout")
		void SetRows(const TArray<FUIPanelLayout_FlexibleGridSize>& Value);
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
};

UCLASS(ClassGroup = LGUI, Blueprintable)
class LGUI_API UUIPanelLayout_FlexibleGrid_Slot : public UUIPanelLayoutSlotBase
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	void PostEditUndo()override;
#endif
	friend class FUIPanelLayoutFlexibleGridSlotCustomization;
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
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot", meta = (UIMin = "1"))
		int ColumnSpan = 1;
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot", meta = (UIMin = "0"))
		int Row = 0;
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot", meta = (UIMin = "1"))
		int RowSpan = 1;
public:
	void VerifyColumnAndRow(UUIPanelLayout_FlexibleGrid* Layout);

	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		const FMargin& GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EHorizontalAlignment GetHorizontalAlignment()const { return HorizontalAlignment; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		EVerticalAlignment GetVerticalAlignment()const { return VerticalAlignment; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetColumn()const { return Column; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetColumnSpan()const { return ColumnSpan; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetRow()const { return Row; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetRowSpan()const { return RowSpan; }

	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetPadding(const FMargin& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetHorizontalAlignment(const EHorizontalAlignment& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetVerticalAlignment(const EVerticalAlignment& Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetColumn(int Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetColumnSpan(int Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetRow(int Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetRowSpan(int Value);
};
