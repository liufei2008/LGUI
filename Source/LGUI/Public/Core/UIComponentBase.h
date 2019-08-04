// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "UIComponentBase.generated.h"

class UUIItem;
/*
Base class for ui actor component, contains some easy-to-use callback functions. 
This type of component should be only added to an actor which have UIItem as RootComponent
*/
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable)
class LGUI_API UUIComponentBase : public UActorComponent
{
	GENERATED_BODY()	
public:	
	UUIComponentBase();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class UUIItem;
	//This owner's RootComponent cast to UIItem
	UUIItem* RootUIComp;
	//Check and get RootUIItem
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool CheckRootUIComponent();
	//Called when RootUIComp IsActiveInHierarchy state is changed
	virtual void OnUIActiveInHierachy(bool activeOrInactive) { OnUIActiveInHierarchyBP(activeOrInactive); }
	//Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	//@param positionChanged	relative position
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged) { OnUIDimensionsChangedBP(positionChanged, sizeChanged); }
	//Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	//@param positionChanged	relative position
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged) { OnUIChildDimensionsChangedBP(child, positionChanged, sizeChanged); }
	//Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive) { OnUIChildAcitveInHierarchyBP(child, ativeOrInactive); }
	//Called when RootUIComp attach to a new parent
	virtual void OnUIAttachmentChanged() { OnUIAttachmentChangedBP(); }
	//Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp 
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) { OnUIChildAttachmentChangedBP(child, attachOrDetach); }
	//Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not)
	virtual void OnUIInteractionStateChanged(bool interactableOrNot) { OnUIInteractionStateChangedBP(interactableOrNot); }
	//Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child) { OnUIChildHierarchyIndexChangedBP(child); }


	//Called when RootUIComp IsActiveInHierarchy state is changed
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIActiveInHierarchy")) void OnUIActiveInHierarchyBP(bool activeOrInactive);
	//Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed
	//@param positionChanged	relative position
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIDimensionsChanged")) void OnUIDimensionsChangedBP(bool positionChanged, bool sizeChanged);
	//Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	//@param positionChanged	relative position
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildDimensionsChanged")) void OnUIChildDimensionsChangedBP(UUIItem* child, bool positionChanged, bool sizeChanged);
	//Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAcitveInHierarchy")) void OnUIChildAcitveInHierarchyBP(UUIItem* child, bool ativeOrInactive);
	//Called when RootUIComp attach to a new parent
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIAttachmentChanged")) void OnUIAttachmentChangedBP();
	//Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp 
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAttachmentChanged")) void OnUIChildAttachmentChangedBP(UUIItem* child, bool attachOrDetach);
	//Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not)
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIInteractionStateChanged")) void OnUIInteractionStateChangedBP(bool interactableOrNot);
	//Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildHierarchyIndexChanged")) void OnUIChildHierarchyIndexChangedBP(UUIItem* child);
};