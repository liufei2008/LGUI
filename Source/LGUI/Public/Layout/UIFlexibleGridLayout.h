// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithChildren.h"
#include "Layout/Margin.h"
#include "UIFlexibleGridLayout.generated.h"

UENUM(BlueprintType)
enum class EUIFlexibleGridLayoutCellSizeType:uint8
{
	Constant,
	Ratio,
};

USTRUCT(BlueprintType)
struct LGUI_API FUIFlexibleGridLayoutCellData
{
	GENERATED_BODY()
public:
	FUIFlexibleGridLayoutCellData() {}
	FUIFlexibleGridLayoutCellData(float InSize, EUIFlexibleGridLayoutCellSizeType InSizeType = EUIFlexibleGridLayoutCellSizeType::Ratio)
	{
		this->Size = InSize;
		this->SizeType = InSizeType;
	}

	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0"))
		float Size = 1.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		EUIFlexibleGridLayoutCellSizeType SizeType = EUIFlexibleGridLayoutCellSizeType::Ratio;

	bool operator == (const FUIFlexibleGridLayoutCellData& Other)const
	{
		return this->Size == Other.Size
			&& this->SizeType == Other.SizeType
			;
	}
};

class UUIFlexibleGridLayoutElement;

/**
 * Flexible & Responsive grid based layout.
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent))
class LGUI_API UUIFlexibleGridLayout : public UUILayoutWithChildren
{
	GENERATED_BODY()

public:
	UUIFlexibleGridLayout();

	virtual void OnRebuildLayout()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<FUIFlexibleGridLayoutCellData>& GetColumns()const { return Columns; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<FUIFlexibleGridLayoutCellData>& GetRows()const { return Rows; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetRowCount()const { return Rows.Num(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetColumnCount()const { return Columns.Num(); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FVector2D GetSpacing()const { return Spacing; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRows(const TArray<FUIFlexibleGridLayoutCellData>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColumns(const TArray<FUIFlexibleGridLayoutCellData>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(const FVector2D& value);

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void GetLayoutElement(AActor* InActor, UActorComponent*& OutLayoutElement, bool& OutIgnoreLayout)const override;


	friend class FUIFlexibleGridLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D Spacing;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FUIFlexibleGridLayoutCellData> Columns;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<FUIFlexibleGridLayoutCellData> Rows;
};
