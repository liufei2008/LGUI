// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithAnimation.h"
#include "Layout/Margin.h"
#include "UIVerticalLayout.generated.h"

/**
 * Layout child elements side by side vertically
 */
UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIVerticalLayout : public UUILayoutWithAnimation
{
	GENERATED_BODY()
public:
	virtual void OnRebuildLayout()override;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FMargin GetPadding() { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetSpacing() { return Spacing; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ELGUILayoutAlignmentType GetAlign() { return Align; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildrenWidth() { return ExpendChildrenWidth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetExpendChildrenHeight() { return ExpendChildrenHeight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetHeightFitToChildren() { return HeightFitToChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetActuralRange() { return ActuralRange; }

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
		void SetHeightFitToChildren(bool value);
#if WITH_EDITOR
	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
#endif
protected:
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;

	friend class FUIVerticalLayoutCustomization;
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
	/** this object's height set to children range */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool HeightFitToChildren = false;

	//actural children fill range
	float ActuralRange = 0;

	TArray<float> childrenHeightList;
};
