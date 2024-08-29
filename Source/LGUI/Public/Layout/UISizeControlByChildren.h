// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithChildren.h"
#include "UISizeControlByChildren.generated.h"

/**
 * Use aspect ratio to control with and height.
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUISizeControlByChildren : public UUILayoutWithChildren
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren()const { return bWidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalWidth()const { return AdditionalWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetHeightFitToChildren()const { return bHeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAdditionalHeight()const { return AdditionalHeight; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalWidth(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetHeightFitToChildren(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAdditionalHeight(float Value);

	virtual bool GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const override;
protected:
	virtual void OnRebuildLayout()override;
	/** This UI's width fit to max size of child */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bWidthFitToChildren = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalWidth = 0;
	/** This UI's width fit to max size of child */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool bHeightFitToChildren = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float AdditionalHeight = 0;

	//these will not affect this Layout
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override {}
};
