// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "UIFlexibleGridLayoutElement.generated.h"

/**
 * Set layout element property for UIFlexibleGridLayout
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent))
class LGUI_API UUIFlexibleGridLayoutElement : public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetRowIndex()const { return RowIndex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetRowCount()const { return RowCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetColumnIndex()const { return ColumnIndex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetColumnCount()const { return ColumnCount; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetIgnoreLayout()const { return bIgnoreLayout; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRowIndex(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRowCount(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColumnIndex(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColumnCount(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetIgnoreLayout(bool value);
protected:

	//friend class FUIFlexibleGridLayoutElementCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int RowIndex = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int RowCount = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int ColumnIndex = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int ColumnCount = 1;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bIgnoreLayout;

	UPROPERTY(Transient)
		class UUIFlexibleGridLayout* ParentLayout = nullptr;

	bool CheckParentLayout();
};
