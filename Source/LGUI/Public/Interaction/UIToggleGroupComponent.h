// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/LGUILifeCycleUIBehaviour.h"
#include "LGUIDelegateHandleWrapper.h"
#include "Event/LGUIEventDelegate.h"
#include "UIToggleGroupComponent.generated.h"

class UUIToggleComponent;
DECLARE_DELEGATE_OneParam(FLGUIToggleGroupDelegate, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIToggleGroupMulticastDelegate, int32);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIToggleGroupDynamicDelegate, int32, Index);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleGroupComponent : public ULGUILifeCycleUIBehaviour
{
	GENERATED_BODY()
public:
	UUIToggleGroupComponent();
protected:
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI-ToggleGroup", AdvancedDisplay) TWeakObjectPtr<UUIToggleComponent> LastSelect = nullptr;
	UPROPERTY(Transient, VisibleAnywhere, Category = "LGUI-ToggleGroup", AdvancedDisplay) TArray<TWeakObjectPtr<UUIToggleComponent>> ToggleCollection;
	bool bNeedToSortToggleCollection = false;
	void SortToggleCollection();
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		bool bAllowNoneSelected = true;
	FLGUIToggleGroupMulticastDelegate OnToggleCPP;
	/* Called when selection change of this toggle group. Parameter is selected toggle item's actor, or null if none selected. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		FLGUIEventDelegate OnToggle;
public:
	void AddToggleComponent(UUIToggleComponent* InComp);
	void RemoveToggleComponent(UUIToggleComponent* InComp);

	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetSelection(UUIToggleComponent* Target);
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void ClearSelection();
	/** Return current selected toggle item. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		UUIToggleComponent* GetSelectedItem()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		bool GetAllowNoneSelected()const { return bAllowNoneSelected; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void SetAllowNoneSelected(bool InBool) { bAllowNoneSelected = InBool; }
	/** return toggle's index in this group. return -1 if not belong to this group. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		int32 GetToggleIndex(const UUIToggleComponent* InComp)const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		UUIToggleComponent* GetToggleByIndex(int32 InIndex)const;

	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group.
	 * Parameter of the event is selected toggle component's index in group, or -1 if none selected.
	 */
	FDelegateHandle RegisterToggleEvent(const FLGUIToggleGroupDelegate& InDelegate);
	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group.
	 * Parameter of the event is selected toggle component's index in group, or -1 if none selected.
	 */
	FDelegateHandle RegisterToggleEvent(const TFunction<void(int32)>& InFunction);
	/**
	 * Unregister toggle change event.
	 */
	void UnregisterToggleEvent(const FDelegateHandle& InHandle);

	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group.
	 * Parameter of the event is selected toggle component's index in group, or -1 if none selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		FLGUIDelegateHandleWrapper RegisterToggleEvent(const FLGUIToggleGroupDynamicDelegate& InDelegate);
	/**
	 * Unregister toggle change event.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
