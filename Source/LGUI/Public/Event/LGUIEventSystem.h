// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "LGUIDelegateDeclaration.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIEventSystem.generated.h"

class ULGUIPointerEventData;
class UUISelectableComponent;
class ULGUIBaseInputModule;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FLGUIHitDynamicDelegate, bool, isHit, const FHitResult&, hitResult, USceneComponent*, hitComponent);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIPointerEventDynamicDelegate, ULGUIPointerEventData*, pointerEventData);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIBaseEventDynamicDelegate, ULGUIBaseEventData*, eventData);


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

	bool isNavigationActive = false;
	void BeginNavigation();

	FORCEINLINE void ProcessInputEvent();
public:
	UE_DEPRECATED(4.23, "Use LGUI_StandaloneInputModule's InputTrigger instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (AdvancedDisplay = "inMouseButtonType", DeprecatedFunction, DeprecationMessage="Use LGUI_StandaloneInputModule's InputTrigger instead."))
		void InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType = EMouseButtonType::Left) {};
	UE_DEPRECATED(4.23, "Use LGUI_StandaloneInputModule's InputScroll instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "Use LGUI_StandaloneInputModule's InputScroll instead."))
		void InputScroll(const float& inAxisValue) {};

	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationLeft();
	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationRight();
	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationUp();
	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationDown();
	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationNext();
	/** Call InputNavigationBegin to activate navigation before this function. Only the component which inherit UISelectable can be navigate to */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationPrev();
	/** Activate navigation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationBegin();
	/** Cancel navigation */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationEnd();
	/** is navigation mode acivated */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsNavigationActive()const { return isNavigationActive; }

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
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectComponentWithDefault(USceneComponent* InSelectComp);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetCurrentSelectedComponent() { return selectedComponent; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUIBaseInputModule* GetCurrentInputModule();
	/** event data for navigation */
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		ULGUIBaseEventData* defaultEventData;
protected:
	/** call back for hit event */
	FLGUIMulticastHitDelegate hitEvent;
	/** call back for all event */
	FLGUIMulticastBaseEventDelegate globalListenerPointerEvent;
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
protected:
	UPROPERTY(Transient)USceneComponent* selectedComponent = nullptr;
	/** current navigation selectable component */
	UPROPERTY(Transient)UUISelectableComponent* navigationSelectableComponent = nullptr;
public:
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

	void LogEventData(ULGUIBaseEventData* eventData);


	void BubbleOnPointerEnter(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerExit(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDown(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerUp(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerClick(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerBeginDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerEndDrag(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerScroll(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDragEnter(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDragExit(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerDragDrop(AActor* actor, ULGUIPointerEventData* eventData);
	void BubbleOnPointerSelect(AActor* actor, ULGUIBaseEventData* eventData);
	void BubbleOnPointerDeselect(AActor* actor, ULGUIBaseEventData* eventData);
};

/*
 * This is a preset actor that contans a LGUIEventSystem component
 */
UCLASS()
class LGUI_API ALGUIEventSystemActor : public AActor
{
	GENERATED_BODY()

public:
	ALGUIEventSystemActor();
protected:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class ULGUIEventSystem* EventSystem;
};
