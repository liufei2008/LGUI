// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerEnterExitInterface.h"
#include "Event/LGUIPointerDownUpInterface.h"
#include "Event/LGUIPointerClickInterface.h"
#include "Event/LGUIPointerDragInterface.h"
#include "Event/LGUIPointerDragEnterExitInterface.h"
#include "Event/LGUIPointerDragDropInterface.h"
#include "Event/LGUIPointerScrollInterface.h"
#include "Event/LGUIPointerSelectDeselectInterface.h"

#include "Event/LGUIDrawableEvent.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "Event/LGUIEventSystemActor.h"
#include "Components/ActorComponent.h"
#include "UIEventTriggerComponent.generated.h"

//a helper component for quick register and setup LGUIPointerEvent
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEventTriggerComponent : public UActorComponent
	, public ILGUIPointerEnterExitInterface
	, public ILGUIPointerDownUpInterface
	, public ILGUIPointerClickInterface
	, public ILGUIPointerDragInterface
	, public ILGUIPointerDragEnterExitInterface
	, public ILGUIPointerDragDropInterface
	, public ILGUIPointerScrollInterface
	, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()
	
public:	
	UUIEventTriggerComponent();
protected:
	virtual void BeginPlay() override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//inherited events of this component can bubble up?
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") bool AllowEventBubbleUp = false;
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerEnter = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerExit = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDown = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerUp = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerClick = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerBeginDrag = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDrag = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerEndDrag = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDragEnter = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDragExit = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDragDrop = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerScroll = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerSelect = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") FLGUIDrawableEvent OnPointerDeselect = FLGUIDrawableEvent(LGUIDrawableEventParameterType::PointerEvent);

	FLGUIMulticastPointerEventDelegate OnPointerEnterCPP;
	FLGUIMulticastPointerEventDelegate OnPointerExitCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDownCPP;
	FLGUIMulticastPointerEventDelegate OnPointerUpCPP;
	FLGUIMulticastPointerEventDelegate OnPointerClickCPP;
	FLGUIMulticastPointerEventDelegate OnPointerBeginDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerEndDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragEnterCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragExitCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragDropCPP;
	FLGUIMulticastPointerEventDelegate OnPointerScrollCPP;
	FLGUIMulticastPointerEventDelegate OnPointerSelectCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDeselectCPP;
public:
	void RegisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate); 
	void RegisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDragEnter(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDragExit(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerSelect(const FLGUIPointerEventDelegate& InDelegate);
	void RegisterOnPointerDeselect(const FLGUIPointerEventDelegate& InDelegate);

	void UnregisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate); 
	void UnregisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDragEnter(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDragExit(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerSelect(const FLGUIPointerEventDelegate& InDelegate);
	void UnregisterOnPointerDeselect(const FLGUIPointerEventDelegate& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerEnter(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerExit(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDown(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerUp(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerClick(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerBeginDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerEndDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDragEnter(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDragExit(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDragDrop(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerScroll(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerSelect(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")FLGUIDelegateHandleWrapper RegisterOnPointerDeselect(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerEnter(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerExit(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDown(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerUp(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerClick(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerBeginDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerEndDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDragEnter(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDragExit(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDragDrop(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerScroll(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerSelect(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")void UnregisterOnPointerDeselect(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	virtual bool OnPointerEnter_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerExit_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerClick_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragEnter_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragExit_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDragDrop_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerSelect_Implementation(const FLGUIPointerEventData& eventData)override;
	virtual bool OnPointerDeselect_Implementation(const FLGUIPointerEventData& eventData)override;
};
