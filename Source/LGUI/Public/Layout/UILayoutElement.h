// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "ILGUILayoutElementInterface.h"
#include "UILayoutElement.generated.h"


/**
 * Attach to layout's child, make is specific or ignore layout
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUILayoutElement : public ULGUILifeCycleUIBehaviour, public ILGUILayoutElementInterface
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//Begin LGUILayoutElement interface
	ELayoutElementType GetLayoutType_Implementation()const { return LayoutElementType; }
	bool GetIgnoreLayout_Implementation()const { return LayoutElementType == ELayoutElementType::IgnoreLayout; }
	float GetConstantSize_Implementation()const { return ConstantSize; }
	float GetRatioSize_Implementation()const { return RatioSize; }
	//End LGUILayoutElement interface

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
