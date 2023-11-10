// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerEnterExitInterface.h"
#include "Event/Interface/LGUIPointerDownUpInterface.h"
#include "Event/Interface/LGUIPointerClickInterface.h"
#include "Event/Interface/LGUIPointerDragInterface.h"
#include "Event/Interface/LGUIPointerDragDropInterface.h"
#include "Event/Interface/LGUIPointerScrollInterface.h"
#include "Event/Interface/LGUIPointerSelectDeselectInterface.h"

#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "LGUIDelegateHandleWrapper.h"
#include "Event/LGUIEventSystem.h"
#include "Components/ActorComponent.h"
#include "UIEventTriggerComponent.generated.h"

//a helper component for quick register and setup LGUIPointerEvent
UCLASS(HideCategories = (Collision, LOD, Physics, Cooking, Rendering, Activation, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEventTriggerComponent : public UActorComponent
	, public ILGUIPointerEnterExitInterface
	, public ILGUIPointerDownUpInterface
	, public ILGUIPointerClickInterface
	, public ILGUIPointerDragInterface
	, public ILGUIPointerDragDropInterface
	, public ILGUIPointerScrollInterface
	, public ILGUIPointerSelectDeselectInterface
{
	GENERATED_BODY()
protected:
	//inherited events of this component can bubble up?
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		bool AllowEventBubbleUp = false;
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerEnter = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerExit = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerDown = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerUp = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerClick = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerBeginDrag = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerDrag = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerEndDrag = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerDragDrop = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerScroll = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerSelect = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		FLGUIEventDelegate OnPointerDeselect = FLGUIEventDelegate(ELGUIEventDelegateParameterType::PointerEvent);

	FLGUIMulticastPointerEventDelegate OnPointerEnterCPP;
	FLGUIMulticastPointerEventDelegate OnPointerExitCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDownCPP;
	FLGUIMulticastPointerEventDelegate OnPointerUpCPP;
	FLGUIMulticastPointerEventDelegate OnPointerClickCPP;
	FLGUIMulticastPointerEventDelegate OnPointerBeginDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerEndDragCPP;
	FLGUIMulticastPointerEventDelegate OnPointerDragDropCPP;
	FLGUIMulticastPointerEventDelegate OnPointerScrollCPP;
	FLGUIMulticastBaseEventDelegate OnPointerSelectCPP;
	FLGUIMulticastBaseEventDelegate OnPointerDeselectCPP;
public:
	FDelegateHandle RegisterOnPointerEnter(const FLGUIPointerEventDelegate& InDelegate); 
	FDelegateHandle RegisterOnPointerExit(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerDown(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerUp(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerClick(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerBeginDrag(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerDrag(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerEndDrag(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerDragDrop(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerScroll(const FLGUIPointerEventDelegate& InDelegate);
	FDelegateHandle RegisterOnPointerSelect(const FLGUIBaseEventDelegate& InFunction);
	FDelegateHandle RegisterOnPointerDeselect(const FLGUIBaseEventDelegate& InFunction);

	FDelegateHandle RegisterOnPointerEnter(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerExit(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerDown(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerUp(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerClick(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerBeginDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerEndDrag(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerDragDrop(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerScroll(const TFunction<void(ULGUIPointerEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerSelect(const TFunction<void(ULGUIBaseEventData*)>& InFunction);
	FDelegateHandle RegisterOnPointerDeselect(const TFunction<void(ULGUIBaseEventData*)>& InFunction);

	void UnregisterOnPointerEnter(const FDelegateHandle& InHandle); 
	void UnregisterOnPointerExit(const FDelegateHandle& InHandle);
	void UnregisterOnPointerDown(const FDelegateHandle& InHandle);
	void UnregisterOnPointerUp(const FDelegateHandle& InHandle);
	void UnregisterOnPointerClick(const FDelegateHandle& InHandle);
	void UnregisterOnPointerBeginDrag(const FDelegateHandle& InHandle);
	void UnregisterOnPointerDrag(const FDelegateHandle& InHandle);
	void UnregisterOnPointerEndDrag(const FDelegateHandle& InHandle);
	void UnregisterOnPointerDragDrop(const FDelegateHandle& InHandle);
	void UnregisterOnPointerScroll(const FDelegateHandle& InHandle);
	void UnregisterOnPointerSelect(const FDelegateHandle& InHandle);
	void UnregisterOnPointerDeselect(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerEnter(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerExit(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerDown(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerUp(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerClick(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerBeginDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerEndDrag(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerDragDrop(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerScroll(const FLGUIPointerEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerSelect(const FLGUIBaseEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		FLGUIDelegateHandleWrapper RegisterOnPointerDeselect(const FLGUIBaseEventDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerEnter(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerExit(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerDown(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerUp(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerClick(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerBeginDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerEndDrag(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerDragDrop(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerScroll(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerSelect(const FLGUIDelegateHandleWrapper& InDelegateHandle);
	UFUNCTION(BlueprintCallable, Category = "UIEventTrigger")
		void UnregisterOnPointerDeselect(const FLGUIDelegateHandleWrapper& InDelegateHandle);

	virtual bool OnPointerEnter_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerExit_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDown_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerUp_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerDragDrop_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerScroll_Implementation(ULGUIPointerEventData* eventData)override;
	virtual bool OnPointerSelect_Implementation(ULGUIBaseEventData* eventData)override;
	virtual bool OnPointerDeselect_Implementation(ULGUIBaseEventData* eventData)override;
};
