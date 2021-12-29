// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
#include "Layout/Margin.h"
#include "UIHorizontalLayout.generated.h"

/**
 * Layout child elements side by side horizontally
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIHorizontalLayout : public UUILayoutWithAnimation
{
	GENERATED_BODY()
public:
	virtual void OnRebuildLayout()override;
	
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetSpacing()const { return Spacing; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELGUILayoutAlignmentType GetAlign()const { return Align; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildrenWidth()const { return ExpendChildrenWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildrenHeight()const { return ExpendChildrenHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetWidthFitToChildren()const { return WidthFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetActuralRange()const { return ActuralRange; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPadding(FMargin value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpacing(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlign(ELGUILayoutAlignmentType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpendChildrenWidth(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetExpendChildrenHeight(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetWidthFitToChildren(bool value);

	virtual bool CanControlChildAnchor_Implementation()const override;
	virtual bool CanControlChildHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlChildWidth_Implementation()const override;
	virtual bool CanControlChildHeight_Implementation()const override;
	virtual bool CanControlChildAnchorLeft_Implementation()const override;
	virtual bool CanControlChildAnchorRight_Implementation()const override;
	virtual bool CanControlChildAnchorBottom_Implementation()const override;
	virtual bool CanControlChildAnchorTop_Implementation()const override;

	virtual bool CanControlSelfAnchor_Implementation()const override;
	virtual bool CanControlSelfHorizontalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfVerticalAnchoredPosition_Implementation()const override;
	virtual bool CanControlSelfWidth_Implementation()const override;
	virtual bool CanControlSelfHeight_Implementation()const override;
	virtual bool CanControlSelfAnchorLeft_Implementation()const override;
	virtual bool CanControlSelfAnchorRight_Implementation()const override;
	virtual bool CanControlSelfAnchorBottom_Implementation()const override;
	virtual bool CanControlSelfAnchorTop_Implementation()const override;

protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;

	friend class FUIHorizontalLayoutCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FMargin Padding;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float Spacing;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUILayoutAlignmentType Align = ELGUILayoutAlignmentType::UpperLeft;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpendChildrenWidth = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool ExpendChildrenHeight = false;
	/** this object's width set to children range */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool WidthFitToChildren = false;

	/** actural children fill range */
	float ActuralRange;

	TArray<float> childrenWidthList;
};
