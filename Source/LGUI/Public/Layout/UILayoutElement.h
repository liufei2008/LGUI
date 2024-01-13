// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "ILGUILayoutElementInterface.h"
#include "UILayoutElement.generated.h"

UENUM(BlueprintType, Category=LGUI)
enum class EUILayoutElement_ConstantSizeType : uint8
{
	/** Use UI's size */
	UseUIItemSize,
	/** Use the ConstantSize parameter */
	UseCustomSize,
};
/**
 * Attach to layout's child, make it specific or ignore layout
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUILayoutElement : public ULGUILifeCycleUIBehaviour, public ILGUILayoutElementInterface
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;
	virtual void OnEnable()override;
	virtual void OnDisable()override;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	//Begin LGUILayoutElement interface
	virtual ELayoutElementType GetLayoutType_Implementation()const override;
	virtual float GetConstantSize_Implementation(ELayoutElementSizeType type)const override;
	virtual float GetRatioSize_Implementation(ELayoutElementSizeType type)const override;
	//End LGUILayoutElement interface

private:
	friend class FUILayoutElementCustomization;
	UPROPERTY(Transient) TObjectPtr<class UUILayoutWithChildren> ParentLayout = nullptr;
	bool CheckParentLayout();
	/** find new parent layout when attachement change */
	virtual void OnUIAttachmentChanged()override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELayoutElementType LayoutElementType = ELayoutElementType::AutoSize;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="LayoutElementType==ELayoutElementType::ConstantSize"))
		EUILayoutElement_ConstantSizeType ConstantSizeType = EUILayoutElement_ConstantSizeType::UseCustomSize;
	//ConstantSize
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(EditCondition="LayoutElementType==ELayoutElementType::ConstantSize&&ConstantSizeType==EUILayoutElement_ConstantSizeType::UseCustomSize"))
		float ConstantSize = 100;
	//total size is 1.0, set size by ratio
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float RatioSize = 0.5f;

public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		EUILayoutElement_ConstantSizeType GetConstantSizeType()const { return ConstantSizeType; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetLayoutType(ELayoutElementType InType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetConstantSize(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRatioSize(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetConstantSizeType(EUILayoutElement_ConstantSizeType value);
};
