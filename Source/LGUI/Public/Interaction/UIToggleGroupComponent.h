// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Event/LGUIDrawableEvent.h"
#include "UIToggleGroupComponent.generated.h"

class UUIToggleComponent;
DECLARE_DELEGATE_OneParam(FLGUIToggleGroupDelegate, UUIToggleComponent*);
DECLARE_MULTICAST_DELEGATE_OneParam(FLGUIToggleGroupMulticastDelegate, UUIToggleComponent*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIToggleGroupDynamicDelegate, UUIToggleComponent*, ToggleItem);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIToggleGroupComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(Transient) TWeakObjectPtr<UUIToggleComponent> LastSelect = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		bool bAllowNoneSelected = true;
	FLGUIToggleGroupMulticastDelegate OnToggleCPP;
	/* Called when selection change of this toggle group. Parameter is selected toggle item's actor, or null if none selected. */
	UPROPERTY(EditAnywhere, Category = "LGUI-ToggleGroup")
		FLGUIDrawableEvent OnToggle = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Actor);
public:
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

	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group. Parameter of the event is selected toggle component, or null if none selected.
	 */
	FDelegateHandle RegisterToggleEvent(const FLGUIToggleGroupDelegate& InDelegate);
	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group. Parameter of the event is selected toggle component, or null if none selected.
	 */
	FDelegateHandle RegisterToggleEvent(const TFunction<void(UUIToggleComponent*)>& InFunction);
	/**
	 * Unregister toggle change event.
	 */
	void UnregisterToggleEvent(const FDelegateHandle& InHandle);

	/**
	 * Register toggle change event.
	 * Event will be called when selection change of this toggle group. Parameter of the event is selected toggle component, or null if none selected.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		FLGUIDelegateHandleWrapper RegisterToggleEvent(const FLGUIToggleGroupDynamicDelegate& InDelegate);
	/**
	 * Unregister toggle change event.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ToggleGroup")
		void UnregisterToggleEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
