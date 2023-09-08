﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "LGUIDelegateDeclaration.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIEventSystem.generated.h"

class ULGUIPointerEventData;
class ULGUIBaseInputModule;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FLGUIHitDynamicDelegate, bool, isHit, const FHitResult&, hitResult, USceneComponent*, hitComponent);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIPointerEventDynamicDelegate, ULGUIPointerEventData*, pointerEventData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIBaseEventDynamicDelegate, ULGUIBaseEventData*, eventData);

DECLARE_MULTICAST_DELEGATE_TwoParams(FLGUIPointerInputChange_MulticastDelegate, int, ELGUIPointerInputType);
DECLARE_DELEGATE_TwoParams(FLGUIPointerInputChange_Delegate, int, ELGUIPointerInputType);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FLGUIPointerInputChange_DynamicDelegate, int, pointerID, ELGUIPointerInputType, type);

/**
 * This is the place for manage LGUI's input/raycast/event.
 * InputTrigger and InputScroll need mannually setup in InputModule.
 * About event bubble: if all interface of target component and actor return true, then event will bubble up. if no interface found on target, then event will bubble up
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUIEventSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGUIEventSystem();

	UFUNCTION(BlueprintPure, Category = LGUI, meta = (WorldContext = "WorldContextObject", DisplayName = "Get LGUI Event System Instance"))
		static ULGUIEventSystem* GetLGUIEventSystemInstance(UObject* WorldContextObject);
protected:
	/** a world should only have one LGUIEventSystem */
	static TMap<UWorld*, ULGUIEventSystem*> WorldToInstanceMap;
	bool existInInstanceMap = false;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy()override;

protected:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool outputLog = false;
#endif
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		bool bRayEventEnable = true;

	void ProcessInputEvent();
public:
	/** clear event. eg when mouse is hovering a UI and highlight, and then event is disabled, we can use this to clear the hover event */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ClearEvent();
	/** 
	 * SetRaycast enable or disable
	 * @param	clearEvent		call ClearEvent after disable Raycast
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRaycastEnable(bool enable, bool clearEvent = false);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectComponent(USceneComponent* InSelectComp, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);
	static void SetSelectComponent(ULGUIEventSystem* InEventSystem, USceneComponent* InSelectComp, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectComponentWithDefault(USceneComponent* InSelectComp);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetCurrentSelectedComponent(int InPointerID)const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIBaseInputModule* GetCurrentInputModule();

	UPROPERTY(VisibleAnywhere, Category = LGUI)
		mutable TMap<int, TObjectPtr<ULGUIPointerEventData>> pointerEventDataMap;
	/**
	 * Get PointerEventData by given pointerID.
	 * @param	pointerID	0 for mouse input, touch-id for touch input, or other customized value
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIPointerEventData* GetPointerEventData(int pointerID = 0, bool createIfNotExist = false)const;
	/**
	 * Remove a PointerEventData. If you ensure that you will not use it anymore, then you can remove it.
	 * @param	pointerID	0 for mouse input, touch-id for touch input, or other customized value
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void RemovePointerEventData(int pointerID);
protected:
	/** called for pointer hit anything */
	FLGUIMulticastHitDelegate hitEvent;
	/** called for all pointer event */
	FLGUIMulticastBaseEventDelegate globalListenerPointerEvent;
	/** called when any pointerEventData's input type is changed */
	FLGUIPointerInputChange_MulticastDelegate inputChangeDelegate;
public:
	void RegisterHitEvent(const FLGUIHitDelegate& InEvent);
	void UnregisterHitEvent(const FLGUIHitDelegate& InEvent);
	/**
	 * Register a global event listener, that listener will called when any LGUIEventSystem's event is executed.
	 * @param InEvent Callback delegate, you can cast LGUIBaseEventData to LGUIPointerEventData if you need.
	 */
	void RegisterGlobalListener(const FLGUIBaseEventDelegate& InEvent);
	void UnregisterGlobalListener(const FLGUIBaseEventDelegate& InEvent);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		FLGUIDelegateHandleWrapper RegisterHitEvent(const FLGUIHitDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void UnregisterHitEvent(const FLGUIDelegateHandleWrapper& InHandle);
	/**
	 * Register a global event listener, that listener will called when any LGUIEventSystem's event is executed.
	 * @param InDelegate Callback delegate, you can cast LGUIBaseEventData to LGUIPointerEventData if you need.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FLGUIDelegateHandleWrapper RegisterGlobalListener(const FLGUIBaseEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void UnregisterGlobalListener(const FLGUIDelegateHandleWrapper& InHandle);
	void RaiseHitEvent(bool hitOrNot, const FHitResult& hitResult, USceneComponent* hitComponent);

	UE_DEPRECATED(5.0, "Use IsPointerOverUIByPointerID instead.")
		bool IsPointerOverUI(int pointerID = 0) { return IsPointerOverUIByPointerID(pointerID); }
	/**
	 * Tell if the pointer hovering on any UI object.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsPointerOverUIByPointerID(int pointerID = 0);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHighlightedComponentForNavigation(USceneComponent* InComp, int InPointerID);	
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetHighlightedComponentForNavigation(int InPointerID)const;

	UE_DEPRECATED(5.0, "Use SetPointerInputTypeByPointerID instead.")
		bool SetPointerInputType(int InPointerID, ELGUIPointerInputType InInputType) { return SetPointerInputTypeByPointerID(InPointerID, InInputType); }
	/**
	 * Set input type of the pointer, can be pointer or navigation.
	 * @return true- input type changed, false- otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool SetPointerInputTypeByPointerID(int InPointerID, ELGUIPointerInputType InInputType);
	/**
	 * Set input type of the pointer, can be pointer or navigation..
	 * @return true- input type changed, false- otherwise.
	 */
	bool SetPointerInputType(ULGUIPointerEventData* InPointerEventData, ELGUIPointerInputType InInputType);
	/**
	 * Set the pointer's inputType as navigation.
	 * @param InPointerID target pointer's ID.
	 * @param InDefaultHighlightedComponent default highlighted component for navigation input.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ActivateNavigationInput(int InPointerID, USceneComponent* InDefaultHighlightedComponent = nullptr);

	void RegisterInputChangeEvent(const FLGUIPointerInputChange_Delegate& pointerInputChange);
	void UnregisterInputChangeEvent(const FDelegateHandle& delegateHandle);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FLGUIDelegateHandleWrapper RegisterInputChangeEvent(const FLGUIPointerInputChange_DynamicDelegate& pointerInputChange);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void UnregisterInputChangeEvent(FLGUIDelegateHandleWrapper delegateHandle);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ELGUIPointerInputType defaultInputType = ELGUIPointerInputType::Pointer;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		ELGUIEventFireType eventFireTypeForNavigation = ELGUIEventFireType::TargetActorAndAllItsComponents;
	/**
	 * If keep pressing the navigate button for a while, then will trigger the process of continuous navigation.
	 * This is the interval trigger time of continuous navigation
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		float navigateInputInterval = 0.2f;
	/**
	 * If keep pressing the navigate button for a while, then will trigger the process of continuous navigation.
	 * This is the time to trigger the process.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = LGUI)
		float navigateInputIntervalForFirstTime = 0.5f;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerEnter(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = false);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerExit(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = false);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDown(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerUp(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerClick(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerBeginDrag(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDrag(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerEndDrag(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerScroll(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDragDrop(USceneComponent* TargetRootComponent, ULGUIPointerEventData* PointerEventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerSelect(USceneComponent* TargetRootComponent, ULGUIBaseEventData* EventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = false);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDeselect(USceneComponent* TargetRootComponent, ULGUIBaseEventData* EventData, ELGUIEventFireType EventFireType, bool AllowEventBubbleUp = false);

	void CallOnPointerEnter(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerExit(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDown(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerUp(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerClick(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerBeginDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerEndDrag(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerScroll(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDragDrop(USceneComponent* component, ULGUIPointerEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerSelect(USceneComponent* component, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);
	void CallOnPointerDeselect(USceneComponent* component, ULGUIBaseEventData* eventData, ELGUIEventFireType eventFireType);

	static void BubbleOnPointerEnter(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerExit(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerDown(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerUp(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerClick(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerBeginDrag(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerDrag(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerEndDrag(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerScroll(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerDragDrop(AActor* actor, ULGUIPointerEventData* eventData);
	static void BubbleOnPointerSelect(AActor* actor, ULGUIBaseEventData* eventData);
	static void BubbleOnPointerDeselect(AActor* actor, ULGUIBaseEventData* eventData);

	void LogEventData(ULGUIBaseEventData* eventData);
};

/*
 * This is a preset actor that contans a LGUIEventSystem component
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API ALGUIEventSystemActor : public AActor
{
	GENERATED_BODY()

public:
	ALGUIEventSystemActor();
protected:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class ULGUIEventSystem> EventSystem;
};
