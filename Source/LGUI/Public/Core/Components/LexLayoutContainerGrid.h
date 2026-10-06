// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexLayout.h"
#include "Layout/Margin.h"
#include "LexLayoutContainerGrid.generated.h"

UENUM(BlueprintType)
enum class ELexLayoutGridSizeType:uint8
{
	/**
	 * Auto will use max preferred-size of cell content
	 */
	//Auto,
	
	/**
	 * Fixed pixel value
	 */
	Fixed,
	/**
	 * 
	 */
	Ratio,
};

USTRUCT(BlueprintType)
struct LGUI_API FLexLayoutGridSize
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "LexUI")
	ELexLayoutGridSizeType Type = ELexLayoutGridSizeType::Ratio;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (UIMin = "0.0"))
	float FixedValue = 100.0f;
	UPROPERTY(EditAnywhere, Category = "LexUI", meta = (UIMin = "0.0"))
	float RatioValue = 1.0f;

	FLexLayoutGridSize() {}
	FLexLayoutGridSize(ELexLayoutGridSizeType InType, float InValue)
	{
		this->Type = InType;
		switch (InType)
		{
			case ELexLayoutGridSizeType::Fixed:
			this->FixedValue = InValue;
			break;
			case ELexLayoutGridSizeType::Ratio:
			this->RatioValue = InValue;
			break;
		}
	}
	bool operator == (const FLexLayoutGridSize& Other)const
	{
		return this->FixedValue == Other.FixedValue
			&& this->RatioValue == Other.RatioValue
			&& this->Type == Other.Type
			;
	}
};

class ULexLayoutSelfGrid;

/**
 * Flexible & Responsive grid based layout.
 */
UCLASS( ClassGroup=(LGUI), DisplayName="LayoutContainer-Grid", Experimental)
class LGUI_API ULexLayoutContainerGrid : public ULexLayoutContainer
{
	GENERATED_BODY()

public:
	ULexLayoutContainerGrid();

private:
	friend class FUIFlexibleGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	FVector2D Spacing;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	TArray<FLexLayoutGridSize> Columns;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	TArray<FLexLayoutGridSize> Rows;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* TargetWidget)const override;
	virtual void CalculateLayout()override;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	FMargin GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	const TArray<FLexLayoutGridSize>& GetColumns()const { return Columns; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	const TArray<FLexLayoutGridSize>& GetRows()const { return Rows; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetRowCount()const { return Rows.Num(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetColumnCount()const { return Columns.Num(); }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	FVector2D GetSpacing()const { return Spacing; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetPadding(FMargin Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetRows(const TArray<FLexLayoutGridSize>& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetColumns(const TArray<FLexLayoutGridSize>& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetSpacing(const FVector2D& Value);
};
