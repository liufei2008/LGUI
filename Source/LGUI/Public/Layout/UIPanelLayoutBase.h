// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UILayoutWithChildren.h"
#include "UIPanelLayoutBase.generated.h"

class UUIPanelLayoutSlotBase;
class UUIPanelLayoutElement;

UCLASS(Abstract)
class LGUI_API UUIPanelLayoutBase : public UUILayoutWithChildren
{
	GENERATED_BODY()
public:
	UUIPanelLayoutBase();
protected:
	virtual void GetLayoutElement(UUIItem* InChild, UObject*& OutLayoutElement, bool& OutIgnoreLayout)const;
	virtual void RebuildChildrenList()const override;
	virtual void SortChildrenList()const override;
	virtual void Awake() override;

	virtual void OnUIChildAcitveInHierarchy(UUIItem* InChild, bool InUIActive)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* InChild, bool attachOrDetach)override;
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* InChild)override;

	void CleanMapChildToSlot();

	UPROPERTY(VisibleAnywhere, Instanced, Category = "Panel Layout", AdvancedDisplay)
		mutable TMap<TObjectPtr<UUIItem>, TObjectPtr<UUIPanelLayoutSlotBase>> MapChildToSlot;
public:
	TObjectPtr<UUIPanelLayoutSlotBase> GetChildSlot(UUIItem* InChild);
	virtual UClass* GetPanelLayoutSlotClass()const PURE_VIRTUAL(UUIPanelLayoutBase::GeneratePanelLayoutSlot, return nullptr;);
#if WITH_EDITOR
	/** Return category name for editor display */
	virtual FText GetCategoryDisplayName()const;
#endif
};

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced)
class LGUI_API UUIPanelLayoutSlotBase : public UObject
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
	void PostEditUndo()override;
#endif
	/** The desired with and height that this layout element trying to fit. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		FVector2D DesiredSize = FVector2D(100, 100);
	/**
	 * Use this value as layout order instead of HierarchyIndex. Lower value will go left-top position.
	 * If 0 then use default order.
	 */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		int OverrideLayoutOrder = 0;
	/** Ignore parent layout. */
	UPROPERTY(EditAnywhere, Category = "Panel Layout Slot")
		bool bIgnoreLayout = false;
public:
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		FVector2D GetDesiredSize()const { return DesiredSize; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		int GetOverrideLayoutOrder()const { return OverrideLayoutOrder; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		bool GetIgnoreLayout()const { return bIgnoreLayout; }
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetIgnoreLayout(bool Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetOverrideLayoutOrder(int Value);
	UFUNCTION(BlueprintCallable, Category = "Panel Layout Slot")
		void SetDesiredSize(const FVector2D& Value);
};
