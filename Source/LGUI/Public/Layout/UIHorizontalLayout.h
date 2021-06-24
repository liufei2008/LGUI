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
#if WITH_EDITOR
	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildAnchorOffsetX()override;
	virtual bool CanControlChildAnchorOffsetY()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfAnchorOffsetX()override;
	virtual bool CanControlSelfAnchorOffsetY()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
#endif
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
