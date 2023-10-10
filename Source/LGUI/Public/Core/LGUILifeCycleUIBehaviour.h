// Copyright 2019-Present LexLiu. All Rights Reserved.

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

	virtual void OnRegister()override;
	virtual void OnUnregister()override;
public:

	UE_DEPRECATED(4.24, "GetRootComponent not valid anymore, use GetRootUIComponent instead")
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour", meta = (DeprecatedFunction, DeprecationMessage = "GetRootComponent not valid anymore, use GetRootUIComponent instead"))
		UUIItem* GetRootComponent() const { return GetRootUIComponent(); }
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour", meta = (DisplayName="Get Root UI Component"))
		UUIItem* GetRootUIComponent() const;
private:
	enum class ECallbackFunctionType :int32
	{
		Call_OnUIDimensionsChanged,
		Call_OnUIChildDimensionsChanged,
		Call_OnUIChildAcitveInHierarchy,
		Call_OnUIAttachmentChanged,
		Call_OnUIChildAttachmentChanged,
		Call_OnUIInteractionStateChanged,
		Call_OnUIChildHierarchyIndexChanged,
		COUNT,
	};
	/** Some UI callback functions want execute before Awake, but most behaviours should executed inside or after Awake. So use this array to cache these callbacks and execute when Awake called. */
	TArray<TFunction<void()>> CallbacksBeforeAwake;
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
	virtual void OnUIActiveInHierarchyStateChanged(bool InState)override final { OnUIActiveInHierachy(InState); }

	virtual void Call_Awake()override;
	
	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	virtual void OnUIActiveInHierachy(bool activeOrInactive);
	/** 
	 * Called when RootUIComp->AnchorData is changed or scale is changed. 
	 */
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	/**
	 * Called when RootUIComp's attachchildren->AnchorData is changed or scale is changed.
	 */
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
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

	void Call_OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	void Call_OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	void Call_OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive);
	void Call_OnUIAttachmentChanged();
	void Call_OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach);
	void Call_OnUIInteractionStateChanged(bool interactableOrNot);
	void Call_OnUIChildHierarchyIndexChanged(UUIItem* child);


	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIActiveInHierarchy"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIActiveInHierarchy(bool activeOrInactive);
	/** 
	 * Called when RootUIComp->AnchorData is changed  or scale is changed.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIDimensionsChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	/** 
	 * Called when RootUIComp's attachchildren->AnchorData is changed or scale is changed.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildDimensionsChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAcitveInHierarchy"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive);
	/** Called when RootUIComp attach to a new parent */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIAttachmentChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIAttachmentChanged();
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAttachmentChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach);
	/** Called when RootUIComp's interaction state changed(when UICanvasGroup component allow interaction or not) */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIInteractionStateChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIInteractionStateChanged(bool interactableOrNot);
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildHierarchyIndexChanged"), Category = "LGUILifeCycleBehaviour") void ReceiveOnUIChildHierarchyIndexChanged(UUIItem* child);
};