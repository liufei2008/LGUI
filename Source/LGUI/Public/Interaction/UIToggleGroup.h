// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/LexUIBehaviour.h"
#include "Event/LexDelegateDeclaration.h"
#include "Event/LexUIEventDelegate.h"
#include "UIToggleGroup.generated.h"

class UUIToggle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIToggleGroupValueChangedEvent, int32, Index);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleGroup : public ULexUIBehaviour
{
	GENERATED_BODY()
public:
	UUIToggleGroup();
protected:
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI-ToggleGroup", AdvancedDisplay) TWeakObjectPtr<UUIToggle> LastSelect = nullptr;
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI-ToggleGroup", AdvancedDisplay) TArray<TWeakObjectPtr<UUIToggle>> ToggleCollection;
	bool bNeedToSortToggleCollection = false;
	void SortToggleCollection();
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		bool bAllowNoneSelected = true;
	
	FLexUIMulticastDelegateInt32 OnValueChangedCPP;
	/* Called when selection change of this toggle group. Parameter is selected toggle's index, or -1 if none selected. */
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Toggle")
	FUIToggleGroupValueChangedEvent OnValueChanged;
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup", DisplayName="OnValueChanged")
		FLexUIEventDelegate OnValueChangedED;
public:
	FLexUIMulticastDelegateInt32& GetOnValueChangedEvent(){return OnValueChangedCPP;}
	
	void AddToggleComponent(UUIToggle* InComp);
	void RemoveToggleComponent(UUIToggle* InComp);

	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetSelection(UUIToggle* Target);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void ClearSelection();
	/** Return current selected toggle item. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		UUIToggle* GetSelectedItem()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		bool GetAllowNoneSelected()const { return bAllowNoneSelected; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetAllowNoneSelected(bool InBool) { bAllowNoneSelected = InBool; }
	/** return toggle's index in this group. return -1 if not belong to this group. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		int32 GetToggleIndex(const UUIToggle* InComp)const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		UUIToggle* GetToggleByIndex(int32 InIndex)const;
};
