// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "LGUIBehaviour.generated.h"

class UUIItem;
/**
 * Base class for ui behviour related component, contains some easy-to-use event/functions.
 * This type of component should be attached to an actor which have UIItem as RootComponent, so the UI's callback will execute (OnUIXXX).
 * I'm trying to make this ULGUIBehaviour more like Unity's MonoBehaviour. You will see it contains function like Awake/Start/Update/OnDestroy/OnEnable/OnDisable.
 * So you should use Awake/Start instead of BeginPlay, Update instead of Tick, OnDestroy instead of EndPlay.
 * When a LGUIBehaviour is created, if GetIsActiveAndEnable=true (enabled and activeInHierarchy), then these event will execute with the order:
 *		Awake-->OnEnable-->Start. In the same frame, all OnEnable will execute after all Awake, all Start will execute after all OnEnable, all Update will execute after all Start.
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable, HideCategories=(Activation))
class LGUI_API ULGUIBehaviour : public UActorComponent
{
	GENERATED_BODY()	
public:	
	ULGUIBehaviour();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy)override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	UPROPERTY(EditAnywhere, Category = "LGUIBehaviour")
		bool enable = true;
private:
	bool isOnDisableCalled = false;
protected:
	friend class ALGUIManagerActor;

	/** Use this for initialization. All Awake is execute before all Start in same frame */
	virtual void Awake();
	/** Executed when GetIsActiveAndEnable state change to true. */
	virtual void OnEnable();
	/** Use this for initialization. All Awake is execute before all Start in same frame */
	virtual void Start();
	/** Update is called once per frame. */
	virtual void Update(float DeltaTime);
	/** Executed when GetIsActiveAndEnable state change to false. */
	virtual void OnDisable();
	/** Executed when this behaviour is destroyed. */
	virtual void OnDestroy();

	/** Use this for initialization. All Awake is execute before all Start in same frame */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Awake"), Category = "LGUIBehaviour")void AwakeBP();
	/** Executed when GetIsActiveAndEnable state change to true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEnable"), Category = "LGUIBehaviour")void OnEnableBP();
	/** Use this for initialization. All Awake is execute before all Start in same frame */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Start"), Category = "LGUIBehaviour")void StartBP();
	/** Update is called once per frame. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update"), Category = "LGUIBehaviour")void UpdateBP(float DeltaTime);
	/** Executed when GetIsActiveAndEnable state change to false. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDisable"), Category = "LGUIBehaviour")void OnDisableBP();
	/** Executed when this behaviour is destroyed. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDestroy"), Category = "LGUIBehaviour")void OnDestroyBP();

	/**
	 * Return true when this behaviour is active in hierarchy and enable.
	 * "active in hierarchy" is related to UIItem's IsUIActive 
	 */
	FORCEINLINE bool GetIsActiveAndEnable();
public:
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		void SetEnable(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		bool GetEnable() const { return enable; }
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		UUIItem* GetRootComponent() const;
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		USceneComponent* GetRootSceneComponent() const;
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour", meta = (DeterminesOutputType = "OriginObject"))
		AActor* InstantiateActor(AActor* OriginObject, USceneComponent* Parent);
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		AActor* InstantiatePrefab(class ULGUIPrefab* OriginObject, USceneComponent* Parent);
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		AActor* InstantiatePrefabWithTransform(class ULGUIPrefab* OriginObject, USceneComponent* Parent, FVector Location, FRotator Rotation, FVector Scale);
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	friend class UUIItem;
	/** This owner's RootComponent cast to UIItem */
	UPROPERTY(Transient) mutable UUIItem* RootUIComp = nullptr;
	/** Check and get RootUIItem */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	bool CheckRootUIComponent() const;
	
	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	virtual void OnUIActiveInHierachy(bool activeOrInactive) { OnUIActiveInHierarchyBP(activeOrInactive); }
	/** 
	 * Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged) { OnUIDimensionsChangedBP(positionChanged, sizeChanged); }
	/**
	 * Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed.
	 * @param positionChanged	relative position
	 */
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged) { OnUIChildDimensionsChangedBP(child, positionChanged, sizeChanged); }
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive) { OnUIChildAcitveInHierarchyBP(child, ativeOrInactive); }
	/** Called when RootUIComp attach to a new parent */
	virtual void OnUIAttachmentChanged() { OnUIAttachmentChangedBP(); }
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) { OnUIChildAttachmentChangedBP(child, attachOrDetach); }
	/** Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not) */
	virtual void OnUIInteractionStateChanged(bool interactableOrNot) { OnUIInteractionStateChangedBP(interactableOrNot); }
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child) { OnUIChildHierarchyIndexChangedBP(child); }


	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIActiveInHierarchy"), Category = "LGUIBehaviour") void OnUIActiveInHierarchyBP(bool activeOrInactive);
	/** 
	 * Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed
	 * @param positionChanged	relative position
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIDimensionsChanged"), Category = "LGUIBehaviour") void OnUIDimensionsChangedBP(bool positionChanged, bool sizeChanged);
	/** 
	 * Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed.
	 * @param positionChanged	relative position
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildDimensionsChanged"), Category = "LGUIBehaviour") void OnUIChildDimensionsChangedBP(UUIItem* child, bool positionChanged, bool sizeChanged);
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAcitveInHierarchy"), Category = "LGUIBehaviour") void OnUIChildAcitveInHierarchyBP(UUIItem* child, bool ativeOrInactive);
	/** Called when RootUIComp attach to a new parent */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIAttachmentChanged"), Category = "LGUIBehaviour") void OnUIAttachmentChangedBP();
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildAttachmentChanged"), Category = "LGUIBehaviour") void OnUIChildAttachmentChangedBP(UUIItem* child, bool attachOrDetach);
	/** Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not) */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIInteractionStateChanged"), Category = "LGUIBehaviour") void OnUIInteractionStateChangedBP(bool interactableOrNot);
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnUIChildHierarchyIndexChanged"), Category = "LGUIBehaviour") void OnUIChildHierarchyIndexChangedBP(UUIItem* child);
};