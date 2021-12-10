// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "LGUILifeCycleUIBehaviour.generated.h"

class UUIItem;
/**
 * Base class for LGUI's life cycle behviour related component, which is special made for UI, contains some easy-to-use event/functions.
 * This type of component should be attached to an actor which have UIItem as RootComponent, so the UI's callback will execute (OnUIXXX).
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable, HideCategories=(Activation), DisplayName = "LGUI LifeCycle UI Behaviour")
class LGUI_API ULGUILifeCycleUIBehaviour : public ULGUILifeCycleBehaviour
{
	GENERATED_BODY()	
public:	
	ULGUILifeCycleUIBehaviour();
protected:
	friend class UUIItem;
	virtual void BeginPlay() override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;

protected:
	virtual bool IsAllowedToCallOnEnable()const override;
	virtual bool IsAllowedToCallAwake()const override;
public:
	/**
	 * return true when this behaviour is active in hierarchy and enable. "active in hierarchy" is related to UIItem's IsUIActive.
	 */
	virtual bool GetIsActiveAndEnable() const override;

	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		UUIItem* GetRootUIComponent() const;
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class UUIItem;
	/** This owner's RootComponent cast to UIItem */
	UPROPERTY(Transient) mutable TWeakObjectPtr<UUIItem> RootUIComp = nullptr;
	/** Check and get RootUIItem */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool CheckRootUIComponent() const;
	TArray<TFunction<void()>> CallbacksBeforeAwake;

	virtual void Awake()override;
	
	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	virtual void OnUIActiveInHierachy(bool activeOrInactive);
	/** 
	 * Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged);
	/**
	 * Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed.
	 * @param positionChanged	relative position
	 */
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged);
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive);
	/** Called when RootUIComp attach to a new parent */
	virtual void OnUIAttachmentChanged();
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach);
	/** Called when RootUIComp's interaction state changed(when UICanvasGroup component allow interaction or not) */
	virtual void OnUIInteractionStateChanged(bool interactableOrNot);
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child);

	void Call_OnUIDimensionsChanged(bool positionChanged, bool sizeChanged);
	void Call_OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged);
	void Call_OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive);
	void Call_OnUIAttachmentChanged();
	void Call_OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach);
	void Call_OnUIInteractionStateChanged(bool interactableOrNot);
	void Call_OnUIChildHierarchyIndexChanged(UUIItem* child);


	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIActiveInHierarchy"), Category = "LGUILifeCycleBehaviour") void OnUIActiveInHierarchyBP(bool activeOrInactive);
	/** 
	 * Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed
	 * @param positionChanged	relative position
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIDimensionsChanged"), Category = "LGUILifeCycleBehaviour") void OnUIDimensionsChangedBP(bool positionChanged, bool sizeChanged);
	/** 
	 * Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed.
	 * @param positionChanged	relative position
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildDimensionsChanged"), Category = "LGUILifeCycleBehaviour") void OnUIChildDimensionsChangedBP(UUIItem* child, bool positionChanged, bool sizeChanged);
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAcitveInHierarchy"), Category = "LGUILifeCycleBehaviour") void OnUIChildAcitveInHierarchyBP(UUIItem* child, bool ativeOrInactive);
	/** Called when RootUIComp attach to a new parent */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIAttachmentChanged"), Category = "LGUILifeCycleBehaviour") void OnUIAttachmentChangedBP();
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAttachmentChanged"), Category = "LGUILifeCycleBehaviour") void OnUIChildAttachmentChangedBP(UUIItem* child, bool attachOrDetach);
	/** Called when RootUIComp's interaction state changed(when UICanvasGroup component allow interaction or not) */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIInteractionStateChanged"), Category = "LGUILifeCycleBehaviour") void OnUIInteractionStateChangedBP(bool interactableOrNot);
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildHierarchyIndexChanged"), Category = "LGUILifeCycleBehaviour") void OnUIChildHierarchyIndexChangedBP(UUIItem* child);
};