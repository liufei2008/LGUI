// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUIBehaviour.h"
#include "UILayoutElement.generated.h"

UENUM()
enum class ELayoutElementType :uint8
{
	//AutoSize, after ConstantSize and RatioSize are allocated, left size is for AutoSize. euqal to without UILayoutElement
	AutoSize,
	//ignore parent Layout
	IgnoreLayout,
	//ConstantSize
	ConstantSize,
	//RatioSize, set size by ratio. total size is 1.0
	RatioSize,
};
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUILayoutElement : public ULGUIBehaviour
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELayoutElementType GetLayoutType() { return LayoutElementType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIgnoreLayout() { return LayoutElementType == ELayoutElementType::IgnoreLayout; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLayoutType(ELayoutElementType InType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetConstantSize() { return ConstantSize; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetRatioSize() { return RatioSize; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetConstantSize(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRatioSize(float value);
private:
	friend class FUILayoutElementCustomization;
	UPROPERTY(Transient) class UUILayoutBase* ParentLayout = nullptr;
	bool CheckParentLayout();

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELayoutElementType LayoutElementType = ELayoutElementType::AutoSize;
	//ConstantSize
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float ConstantSize = 100;
	//total size is 1.0, set size by ratio
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float RatioSize = 0.5f;
};
