// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "LGUIDelegateDeclaration.h"
#include "Raycaster/LGUIBaseRaycaster.h"
#include "LGUIDelegateHandleWrapper.h"
#include "LGUIEventSystem.generated.h"

struct FLGUIPointerEventData;
class UUISelectableComponent;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FLGUIHitDynamicDelegate, bool, isHit, const FHitResult&, hitResult, USceneComponent*, hitComponent);
DECLARE_DYNAMIC_DELEGATE_OneParam(FLGUIPointerEventDynamicDelegate, const FLGUIPointerEventData&, pointerEvent);

/*
 This is a preset actor that contans a LGUIEventSystem component
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

//InputTrigger and InputScroll need mannually setup
//about the event bubble: if all interface of target component and actor return true, then event will bubble up. if no interface found on target, then event will bubble up
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API ULGUIEventSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	ULGUIEventSystem();

	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DisplayName = "Get LGUI Event System Instance"))
		static ULGUIEventSystem* GetLGUIEventSystemInstance();
protected:
	static ULGUIEventSystem* Instance;//a level should only have one LGUIEventSystemActpr
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = LGUI)
		bool outputLog = false;
#endif
	UPROPERTY(VisibleAnywhere, Category = LGUI)
		bool bRayEventEnable = true;
	bool nowIsTriggerPressed = false;
	EMouseButtonType mouseButtonType = EMouseButtonType::Left;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)
		FLGUIPointerEventData eventData;

	bool isNavigationActive = false;
	void BeginNavigation();
public:
	//TriggerInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (AdvancedDisplay = "inMouseButtonType"))
		void InputTrigger(bool inTriggerPress, EMouseButtonType inMouseButtonType = EMouseButtonType::Left);
	//ScrollInput, need mannually setup
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputScroll(const float& inAxisValue);

	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationLeft();
	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationRight();
	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationUp();
	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationDown();
	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationNext();
	//Call InputNavigationBegin to activate navigation before this function. Only component which inherit UISelectable can be navigate to
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationPrev();
	//Activate navigation
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationBegin();
	//Cancel navigation
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void InputNavigationEnd();
	//is navigation mode acivated
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsNavigationActive()const { return isNavigationActive; }

	//clear event. eg when mouse is hovering a UI and highlight, and then event is disabled, we can use this to clear the hover event
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ClearEvent();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetCurrentHitComponent();
	//SetRaycast enable or disable
	//@param	clearEvent		call ClearEvent after disable Raycast
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRaycastEnable(bool enable, bool clearEvent = false);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetSelectComponent(USceneComponent* InSelectComp);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		USceneComponent* GetCurrentSelectedComponent() { return selectedComponent; }
protected:
	//call back for hit event
	FLGUIMulticastHitDelegate hitEvent;
	//call back for all event
	FLGUIMulticastPointerEventDelegate globalListenerPointerEvent;
public:
	void RegisterHitEvent(const FLGUIHitDelegate& InEvent);
	void UnregisterHitEvent(const FLGUIHitDelegate& InEvent);
	void RegisterGlobalListener(const FLGUIPointerEventDelegate& InEvent);
	void UnregisterGlobalListener(const FLGUIPointerEventDelegate& InEvent);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		FLGUIDelegateHandleWrapper RegisterHitEvent(const FLGUIHitDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void UnregisterHitEvent(const FLGUIDelegateHandleWrapper& InHandle);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FLGUIDelegateHandleWrapper RegisterGlobalListener(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void UnregisterGlobalListener(const FLGUIDelegateHandleWrapper& InHandle);
protected:
	bool prevIsTriggerPressed = false;
	EMouseButtonType prevPressTriggerType = EMouseButtonType::Left;
	UPROPERTY(Transient)USceneComponent* enterComponent = nullptr;
	UPROPERTY(Transient)USceneComponent* selectedComponent = nullptr;
	UPROPERTY(Transient)USceneComponent* dragEnterComponent = nullptr;
	bool enterComponentEventFireOnAllOrOnlyTarget = false;
	bool dragComponentEventFireOnAllOrOnlyTarget = false;
	bool selectedComponentEventFireOnAllOrOnlyTarget = false;
	bool dragEnterComponentEventFireOnAllOrOnlyTarget = false;

	bool pressHitSomething = false;//is hit something when trigger press?
	bool startToDrag = false;//is start to drag?

	FVector prevHitPoint = FVector(0, 0, 0);//prev frame hit point

	UPROPERTY(Transient)UUISelectableComponent* selectableComponent = nullptr;//current navigation selectable component

	struct FHitResultContainerStruct
	{
		FHitResult hitResult;
		bool eventFireOnAll = false;

		FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

		ULGUIBaseRaycaster* raycaster = nullptr;
	};
	TArray<FHitResultContainerStruct> multiHitResult;//temp array for hit result

	bool isUpFiredAtCurrentFrame = false;//is PointerUp event in current frame is called?
	bool isExitFiredAtCurrentFrame = false;//is PointerExit event in current frame is called?
	bool isEndDragFiredAtCurrentFrame = false;//is EndDrag event in current frame is called?
	bool isDragExitFiredAtCurrentFrame = false;//is EndDrag event in current frame is called?
protected:
	bool LineTraceAndEvent();
	bool LineTrace(FHitResultContainerStruct& hitResult);

	void CallOnPointerEnter(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerExit(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerDown(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerUp(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerClick(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerBeginDrag(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerDrag(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerEndDrag(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);

	void CallOnPointerScroll(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);

	void CallOnPointerDragEnter(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerDragExit(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerDragDrop(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);

	void CallOnPointerSelect(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);
	void CallOnPointerDeselect(USceneComponent* component, FLGUIPointerEventData& eventData, bool eventFireOnAll);

	void LogEventData(FLGUIPointerEventData& eventData, bool eventFireOnAll);


	void BubbleOnPointerEnter(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerExit(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDown(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerUp(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerClick(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerBeginDrag(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDrag(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerEndDrag(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerScroll(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDragEnter(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDragExit(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDragDrop(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerSelect(AActor* actor, FLGUIPointerEventData& eventData);
	void BubbleOnPointerDeselect(AActor* actor, FLGUIPointerEventData& eventData);
};

#define CALL_LGUIINTERFACE(component, inEventData, eventFireOnAll, interface, function)\
{\
	inEventData.eventType = EPointerEventType::##function##;\
	inEventData.hitComponent = component;\
	LogEventData(inEventData, eventFireOnAll);\
	bool eventAllowBubble = true;\
	if (eventFireOnAll)\
	{\
		auto ownerActor = component->GetOwner();\
		if (ownerActor->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
		{\
			if (ILGUIPointer##interface##Interface::Execute_OnPointer##function##(ownerActor, inEventData) == false)\
			{\
				eventAllowBubble = false;\
			}\
		}\
		auto components = ownerActor->GetComponents();\
		for (auto item : components)\
		{\
			if (item->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
			{\
				if (ILGUIPointer##interface##Interface::Execute_OnPointer##function##(item, inEventData) == false)\
				{\
					eventAllowBubble = false;\
				}\
			}\
		}\
	}\
	else\
	{\
		if (component->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
		{\
			if (ILGUIPointer##interface##Interface::Execute_OnPointer##function##(component, inEventData) == false)\
			{\
				eventAllowBubble = false;\
			}\
		}\
	}\
	if (eventAllowBubble)\
	{\
		if (auto parentActor = component->GetOwner()->GetAttachParentActor())\
		{\
			BubbleOnPointer##function##(parentActor, inEventData);\
		}\
	}\
	if (globalListenerPointerEvent.IsBound())\
	{\
		globalListenerPointerEvent.Broadcast(inEventData);\
	}\
}

#define BUBBLE_LGUIINTERFACE(actor, inEventData, interface, function)\
{\
	bool eventAllowBubble = true;\
	if (actor->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
	{\
		if (ILGUIPointer##interface##Interface::Execute_OnPointer##function##(actor, inEventData) == false)\
		{\
			eventAllowBubble = false;\
		}\
	}\
	auto components = actor->GetComponents();\
	for (auto item : components)\
	{\
		if (item->GetClass()->ImplementsInterface(ULGUIPointer##interface##Interface::StaticClass()))\
		{\
			if (ILGUIPointer##interface##Interface::Execute_OnPointer##function##(item, inEventData) == false)\
			{\
				eventAllowBubble = false;\
			}\
		}\
	}\
	if (eventAllowBubble)\
	{\
		if (auto parentActor = actor->GetAttachParentActor())\
		{\
			BubbleOnPointer##function##(parentActor, inEventData);\
		}\
	}\
}