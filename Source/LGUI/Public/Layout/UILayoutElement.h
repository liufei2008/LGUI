// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "UILayoutElement.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELayoutElementType :uint8
{
	/* AutoSize, after ConstantSize and RatioSize are allocated, left size is for AutoSize. euqal to without UILayoutElement. */
	AutoSize,
	/* Ignore parent Layout. */
	IgnoreLayout,
	/* Constant size. */
	ConstantSize,
	/* RatioSize, set size by ratio. total size is 1.0. */
	RatioSize,
};
/**
 * Attach to layout's child, make is specific or ignore layout
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUILayoutElement : public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELayoutElementType GetLayoutType()const { return LayoutElementType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIgnoreLayout()const { return LayoutElementType == ELayoutElementType::IgnoreLayout; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetConstantSize()const { return ConstantSize; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetRatioSize()const { return RatioSize; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLayoutType(ELayoutElementType InType);
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
