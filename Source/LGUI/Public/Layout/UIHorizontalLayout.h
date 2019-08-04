// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutBase.h"
#include "Layout/Margin.h"
#include "UIHorizontalLayout.generated.h"

UCLASS( ClassGroup=(LGUI), meta=(BlueprintSpawnableComponent) )
class LGUI_API UUIHorizontalLayout : public UUILayoutBase
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUIHorizontalLayout();
protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
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
		bool GetWidthFitToChildren() { return WidthFitToChildren; }
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
		void SetWidthFitToChildren(bool value);

	virtual bool CanControlChildAnchor()override;
	virtual bool CanControlChildWidth()override;
	virtual bool CanControlChildHeight()override;
	virtual bool CanControlSelfHorizontalAnchor()override;
	virtual bool CanControlSelfVerticalAnchor()override;
	virtual bool CanControlSelfWidth()override;
	virtual bool CanControlSelfHeight()override;
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
	//this object's width set to children range
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool WidthFitToChildren = false;

	//actural children fill range
	float ActuralRange;

	TArray<float> childrenWidthList;
};
