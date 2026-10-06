// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexLayout.h"
#include "LexLayoutSelfGrid.generated.h"

/**
 * Provide item properties for GridContainer,
 */
UCLASS( ClassGroup=(LGUI), DisplayName="LayoutSelf-Grid", Experimental)
class LGUI_API ULexLayoutSelfGrid : public ULexLayoutSelf
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, Category = "LexUI")
	int RowIndex = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	int RowCount = 1;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	int ColumnIndex = 0;
	UPROPERTY(EditAnywhere, Category = "LexUI")
	int ColumnCount = 1;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* Widget) const override;
	
	void SetSizeByLayoutContainer(FVector2f Value); 

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetRowIndex()const { return RowIndex; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetRowCount()const { return RowCount; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetColumnIndex()const { return ColumnIndex; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	int GetColumnCount()const { return ColumnCount; }

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetRowIndex(int Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetRowCount(int Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetColumnIndex(int Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
	void SetColumnCount(int Value);
};
