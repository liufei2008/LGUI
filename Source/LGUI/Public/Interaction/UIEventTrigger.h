// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexPointerClickInterface.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Event/Interface/LexPointerDropInterface.h"
#include "Event/Interface/LexPointerScrollInterface.h"
#include "Event/Interface/LexSelectDeselectInterface.h"
#include "Event/LexUIEventDelegate.h"
#include "Event/LexDelegateDeclaration.h"
#include "Core/LexUIBehaviour.h"
#include "UIEventTrigger.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIEventTriggerPointerEvent, ULexPointerEventData*, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIEventTriggerBaseEvent, ULexBaseEventData*, Value);

//a helper component for quick register and setup LexPointerEvent
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEventTrigger : public ULexUIBehaviour
	, public ILexPointerEnterExitInterface
	, public ILexPointerDownUpInterface
	, public ILexPointerClickInterface
	, public ILexPointerDragInterface
	, public ILexPointerDropInterface
	, public ILexPointerScrollInterface
	, public ILexSelectDeselectInterface
{
	GENERATED_BODY()
protected:
	//inherited events of this component can bubble up?
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger") 
		bool AllowEventBubbleUp = false;
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerEnter") 
		FLexUIEventDelegate OnPointerEnterED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerExit") 
		FLexUIEventDelegate OnPointerExitED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerDown") 
		FLexUIEventDelegate OnPointerDownED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerUp") 
		FLexUIEventDelegate OnPointerUpED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerClick") 
		FLexUIEventDelegate OnPointerClickED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerBeginDrag") 
		FLexUIEventDelegate OnPointerBeginDragED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerDrag") 
		FLexUIEventDelegate OnPointerDragED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerEndDrag") 
		FLexUIEventDelegate OnPointerEndDragED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerDragDrop") 
		FLexUIEventDelegate OnPointerDragDropED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnPointerScroll") 
		FLexUIEventDelegate OnPointerScrollED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnSelect") 
		FLexUIEventDelegate OnSelectED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);
	UPROPERTY(EditAnywhere, Category = "UIEventTrigger", DisplayName="OnDeselect") 
		FLexUIEventDelegate OnDeselectED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::PointerEvent);

	FLexUIMulticastDelegatePointerEventData OnPointerEnterCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerExitCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerDownCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerUpCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerClickCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerBeginDragCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerDragCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerEndDragCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerDragDropCPP;
	FLexUIMulticastDelegatePointerEventData OnPointerScrollCPP;
	FLexUIMulticastDelegateBaseEventData OnSelectCPP;
	FLexUIMulticastDelegateBaseEventData OnDeselectCPP;

	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerEnter;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerExit;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerDown;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerUp;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerClick;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerBeginDrag;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerDrag;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerEndDrag;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerDragDrop;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerPointerEvent OnPointerScroll;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerBaseEvent OnSelect;
	UPROPERTY(BlueprintAssignable, Category = "UIEventTrigger")
	FUIEventTriggerBaseEvent OnDeselect;
public:
	FLexUIMulticastDelegatePointerEventData& GetOnPointerEnterEvent(){return OnPointerEnterCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerExitEvent(){return OnPointerExitCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerDownEvent(){return OnPointerDownCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerUpEvent(){return OnPointerUpCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerClickEvent(){return OnPointerClickCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerBeginDragEvent(){return OnPointerBeginDragCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerDragEvent(){return OnPointerDragCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerEndDragEvent(){return OnPointerEndDragCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerDragDropEvent(){return OnPointerDragDropCPP;}
	FLexUIMulticastDelegatePointerEventData& GetOnPointerScrollEvent(){return OnPointerScrollCPP;}
	FLexUIMulticastDelegateBaseEventData& GetOnSelectEvent(){return OnSelectCPP;}
	FLexUIMulticastDelegateBaseEventData& GetOnDeselectEvent(){return OnDeselectCPP;}
	
	virtual void OnPointerEnter_Implementation(ULexPointerEventData* EventData)override;
	virtual void OnPointerExit_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrop_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerScroll_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnSelect_Implementation(ULexBaseEventData* EventData)override;
	virtual bool OnDeselect_Implementation(ULexBaseEventData* EventData)override;
};
