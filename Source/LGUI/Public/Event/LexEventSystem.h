// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "LexDelegateDeclaration.h"
#include "Core/LexUIBehaviour.h"
#include "Core/Components/LexWidget.h"
#include "LexEventSystem.generated.h"

class ULexPointerEventData;
class ULexBaseInputModule;

DECLARE_MULTICAST_DELEGATE_TwoParams(FLexUIPointerInputTypeChangedDelegate, int, ELexUIPointerInputType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLexUIPointerInputChangedDynamicDelegate, int, PointID, ELexUIPointerInputType, InputType);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FLexUIRaycastHitDelegate, bool, const FLexUIHitResult&, ULexWidget*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLexUIRaycastHitDynamicDelegate, bool, IsHit, const FLexUIHitResult&, HitResult, ULexWidget*, HitObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLexUIBaseEventDataDynamicDelegate, ULexBaseEventData*, Data);

/**
 * This is the place for manage LexUI's input/raycast/event.
 * InputTrigger and InputScroll need manually setup in InputModule.
 * About event bubble: if all interface of target component return true, then event will bubble up. if no interface found on target, then event will bubble up
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent), HideCategories = (Sockets, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile, Navigation))
class LGUI_API ULexEventSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	ULexEventSystem();

	UFUNCTION(BlueprintPure, Category = LGUI, meta = (WorldContext = "WorldContextObject", DisplayName = "Get Lex Event System Instance"))
		static ULexEventSystem* GetLexEventSystemInstance(UObject* WorldContextObject, int UserIndex);
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginDestroy()override;

protected:

	UPROPERTY(EditAnywhere, Category = LGUI, Getter)
	int UserIndex = 0;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool bOutputLog = false;
#endif
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		bool bRayEventEnable = true;

	void ProcessInputEvent();

	UPROPERTY(VisibleAnywhere, Category = LGUI, AdvancedDisplay)
	TWeakObjectPtr<ULexBaseInputModule> CurrentInputModule = nullptr;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
	int GetUserIndex()const{return UserIndex;}

	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULexBaseInputModule* GetCurrentInputModule()const{return CurrentInputModule.Get();}
	void SetInputModule(ULexBaseInputModule* InputModule);
	void ClearInputModule();
	
	/** clear event. eg when mouse is hovering a UI and highlight, and then event is disabled, we can use this to clear the hover event */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ClearEvent();
	/** 
	 * SetRaycast enable or disable
	 * @param	bClearEvent		call ClearEvent after disable Raycast
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRaycastEnable(bool bEnable, bool bClearEvent = false);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectWidget(ULexWidget* InSelectWidget, ULexBaseEventData* EventData);
	static void SetSelectWidget(ULexEventSystem* InEventSystem, ULexWidget* InSelectWidget, ULexBaseEventData* EventData);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectComponentWithDefault(ULexWidget* InSelectWidget);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULexWidget* GetCurrentSelectedComponent(int InPointerID)const;
	
	/**
	 * Get PointerEventData by given pointerID.
	 * @param	PointerID	0 for mouse input, touch-id for touch input, or other customized value
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULexPointerEventData* GetPointerEventData(int PointerID = 0, bool bCreateIfNotExist = false)const;
	/**
	 * Remove a PointerEventData. If you ensure that you will not use it anymore, then you can remove it.
	 * @param	PointerID	0 for mouse input, touch-id for touch input, or other customized value
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void RemovePointerEventData(int PointerID);
protected:
	UPROPERTY(VisibleAnywhere, Category = LGUI)
	mutable TMap<int, TObjectPtr<ULexPointerEventData>> PointerEventDataMap;
	
	/** called for pointer hit anything */
	FLexUIRaycastHitDelegate RaycastHitEvent;
	UPROPERTY(BlueprintAssignable, Category = LGUI, DisplayName="RaycastHitEvent")
	FLexUIRaycastHitDynamicDelegate RaycastHitEventBP;
	
	/** called for all pointer && navigation event */
	FLexUIMulticastDelegateBaseEventData InputEvent;
	UPROPERTY(BlueprintAssignable, Category = LGUI, DisplayName="InputEvent")
	FLexUIBaseEventDataDynamicDelegate InputEventBP;
	
	/** called when any pointerEventData's input type is changed */
	FLexUIPointerInputTypeChangedDelegate PointerInputTypedChangedEvent;
public:
	const TMap<int, TObjectPtr<ULexPointerEventData>>& GetPointerEventDataMap()const{return PointerEventDataMap;}
	
	FLexUIRaycastHitDelegate& GetRaycastHitEvent(){return RaycastHitEvent;}
	FLexUIMulticastDelegateBaseEventData& GetInputEvent(){return InputEvent;}
	FLexUIPointerInputTypeChangedDelegate& GetInputChangedEvent(){return PointerInputTypedChangedEvent;}
	
	void RaiseHitEvent(bool bHitOrNot, const FLexUIHitResult& HitResult, ULexWidget* HitComponent);
	
	/**
	 * Tell if the pointer hovering on any UI object.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsPointerOverUIByPointerID(int PointerID = 0);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHighlightedComponentForNavigation(ULexWidget* InComp, int InPointerID);	
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULexWidget* GetHighlightedComponentForNavigation(int InPointerID)const;
	
	/**
	 * Set input type of the pointer, can be pointer or navigation.
	 * @return true- input type changed, false- otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool SetPointerInputTypeByPointerID(int InPointerID, ELexUIPointerInputType InInputType);
	/**
	 * Set input type of the pointer, can be pointer or navigation..
	 * @return true- input type changed, false- otherwise.
	 */
	bool SetPointerInputType(ULexPointerEventData* InPointerEventData, ELexUIPointerInputType InInputType);
	/**
	 * Set the pointer's inputType as navigation.
	 * @param InPointerID target pointer's ID.
	 * @param InDefaultHighlightedComponent default highlighted component for navigation input.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ActivateNavigationInput(int InPointerID, ULexWidget* InDefaultHighlightedComponent = nullptr);
private:
	UPROPERTY(EditAnywhere, Getter, Setter, Category = LGUI)
	ELexUIPointerInputType DefaultInputType = ELexUIPointerInputType::Pointer;
	/**
	 * If keep pressing the navigate button for a while, then will trigger the process of continuous navigation.
	 * This is the time to trigger the process.
	 */
	UPROPERTY(EditAnywhere, Getter, Setter, Category = LGUI)
	float NavigateInputIntervalForFirstTime = 0.5f;
	/**
	 * If keep pressing the navigate button for a while, then will trigger the process of continuous navigation.
	 * This is the interval trigger time of continuous navigation
	 */
	UPROPERTY(EditAnywhere, Getter, Setter, Category = LGUI)
	float NavigateInputInterval = 0.2f;
	
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ELexUIPointerInputType GetDefaultInputType()const{return DefaultInputType;}
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetNavigateInputIntervalForFirstTime()const{return NavigateInputIntervalForFirstTime;}
	UFUNCTION(BlueprintCallable, Category = LGUI)
	float GetNavigateInputInterval()const{return NavigateInputInterval;}

	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetDefaultInputType(ELexUIPointerInputType Value){ DefaultInputType = Value;}
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetNavigateInputIntervalForFirstTime(float Value){ NavigateInputIntervalForFirstTime = Value;}
	UFUNCTION(BlueprintCallable, Category = LGUI)
	void SetNavigateInputInterval(float Value){ NavigateInputInterval = Value;}
public:
	template<class UEventData, class UInterfaceFunction>
	static void ExecuteLexUIInterface(ULexWidget* Widget,
		UEventData* EventData,
		UClass* InterfaceClass, UInterfaceFunction InterfaceFunction,
		bool AllowEventBubbleUp)
	{
		bool TempAllowEventBubbleUp = AllowEventBubbleUp;
		for (auto& Comp : Widget->GetAllComponents())
		{
			if (Comp->GetClass()->ImplementsInterface(InterfaceClass))
			{
				if (InterfaceFunction(Comp, EventData) == false)
				{
					TempAllowEventBubbleUp = false;
				}
			}
		}
		if (TempAllowEventBubbleUp)
		{
			if (auto ParentWidget = Widget->GetParent())
			{
				ExecuteLexUIInterface(ParentWidget,
					EventData,
					InterfaceClass, InterfaceFunction, true);
			}
		}
	}
	template<class UEventData, class UInterfaceFunction, class UBubbleUpFunction>
	static void BubbleLexUIInterface(ULexWidget* Widget,
		UEventData* EventData, UClass* InterfaceClass,
		UInterfaceFunction InterfaceFunction, UBubbleUpFunction BubbleUpFunction)
	{
		bool TempAllowEventBubbleUp = true;
		for (auto& Comp : Widget->GetAllComponents())
		{
			if (Comp->GetClass()->ImplementsInterface(InterfaceClass))
			{
				if (InterfaceFunction(Comp, EventData) == false)
				{
					TempAllowEventBubbleUp = false;
				}
			}
		}
		if (TempAllowEventBubbleUp)
		{
			if (auto ParentActor = Widget->GetParent())
			{
				BubbleUpFunction(ParentActor, EventData);
			}
		}
	}
	static ULexWidget* GetEventHandler(ULexWidget* Widget, UClass* InterfaceClass);
	
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerEnter(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerExit(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDown(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerUp(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerClick(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerBeginDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerEndDrag(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerScroll(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnPointerDrop(ULexWidget* TargetWidget, ULexPointerEventData* PointerEventData, bool AllowEventBubbleUp = true);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnSelect(ULexWidget* TargetWidget, ULexBaseEventData* EventData, bool AllowEventBubbleUp = false);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		static void ExecuteEvent_OnDeselect(ULexWidget* TargetWidget, ULexBaseEventData* EventData, bool AllowEventBubbleUp = false);

	void CallOnPointerEnter(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerExit(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerDown(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerUp(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerClick(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerBeginDrag(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerDrag(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerEndDrag(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerScroll(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnPointerDrop(ULexWidget* RootComponent, ULexPointerEventData* EventData);
	void CallOnSelect(ULexWidget* RootComponent, ULexBaseEventData* EventData);
	void CallOnDeselect(ULexWidget* RootComponent, ULexBaseEventData* EventData);
	
	void LogEventData(ULexBaseEventData* EventData);
};

/*
 * This is a preset actor that contains a LexEventSystem component
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API ALexEventSystemActor : public AActor
{
	GENERATED_BODY()

public:
	ALexEventSystemActor();
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULexEventSystem> EventSystem;
};
