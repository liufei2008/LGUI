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

	virtual void OnRegister()override;
	virtual void OnUnregister()override;

	UPROPERTY(EditAnywhere, Category = "LGUIBehaviour")
		bool enable = true;
#if WITH_EDITORONLY_DATA
	/** This will allow Update function execute in edit mode. */
	UPROPERTY(EditAnywhere, Category = "LGUIBehaviour")
		bool executeInEditMode = false;
	FDelegateHandle EditorTickDelegateHandle;
#endif
private:
	uint8 isAwakeCalled : 1;
	uint8 isStartCalled : 1;
	uint8 isEnableCalled : 1;
	bool GetIsRootComponentActive()const;
protected:
	friend class ALGUIManagerActor;

	/**
	 * This function is always called before any Start functions and also after a prefab is instantiated.
	 * This is a good replacement for BeginPlay in LGUI's Prefab workflow. Because Awake will execute after all prefab serialization and object reference is done.
	 * If a UIItem is inactive during start up, then Awake is not called until it is made active.
	 */
	virtual void Awake();
	/** Executed after Awake when UI set to active and enable is true. */
	virtual void OnEnable();
	/** Start is called before the first frame update only if UI is active and enable is true. */
	virtual void Start();
	/** Update is called once per frame if if UI is active and enable is true. */
	virtual void Update(float DeltaTime);
	/** Executed when UI is active or enable become false. */
	virtual void OnDisable();
	/**
	 * Executed when this behaviour is destroyed.
	 * If Awake is not executed, then OnDestroy won't execute too.
	 */
	virtual void OnDestroy();

	/**
	 * This function is always called before any Start functions and also after a prefab is instantiated.
	 * This is a good replacement for BeginPlay in LGUI's Prefab workflow. Because Awake will execute after all prefab serialization and object reference is done.
	 * If a UIItem is inactive during start up, then Awake is not called until it is made active.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Awake"), Category = "LGUIBehaviour")void AwakeBP();
	/** Executed after Awake when UI set to active and enable is true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEnable"), Category = "LGUIBehaviour")void OnEnableBP();
	/** Start is called before the first frame update only if UI is active and enable is true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Start"), Category = "LGUIBehaviour")void StartBP();
	/** Update is called once per frame if if UI is active and enable is true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update"), Category = "LGUIBehaviour")void UpdateBP(float DeltaTime);
	/** Executed when GetIsActiveAndEnable state change to false. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDisable"), Category = "LGUIBehaviour")void OnDisableBP();
	/**
	 * Executed when this behaviour is destroyed.
	 * If Awake is not executed, then OnDestroy won't execute too.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDestroy"), Category = "LGUIBehaviour")void OnDestroyBP();

public:
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		void SetEnable(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		bool GetEnable() const { return enable; }
	/**
	 * @return true when this behaviour is active in hierarchy and enable. "active in hierarchy" is related to UIItem's IsUIActive
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUIBehaviour")
		bool GetIsActiveAndEnable() const;
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
	virtual void OnUIActiveInHierachy(bool activeOrInactive);
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