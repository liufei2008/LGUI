// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UIItem.h"
#include "Components/ActorComponent.h"
#include "LGUILifeCycleBehaviour.generated.h"

class USceneComponent;

/**
 * Base class for LGUI's life cycle behviour related component.
 * I'm trying to make this ULGUILifeCycleBehaviour more like Unity's MonoBehaviour. You will see it contains function like Awake/Start/Update/OnDestroy/OnEnable/OnDisable.
 * So you should use Awake/Start instead of BeginPlay, Update instead of Tick, OnDestroy instead of EndPlay.
 * Awake execute order in prefab: higher in hierarchy will execute earlier, so scripts on root actor will execute the first.
 */
UCLASS(ClassGroup = (LGUI), Abstract, Blueprintable, HideCategories = (Activation), DisplayName = "LGUI LifeCycle Behaviour")
class LGUI_API ULGUILifeCycleBehaviour : public UActorComponent
{
	GENERATED_BODY()
public:
	ULGUILifeCycleBehaviour();
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

	virtual void OnRegister()override;
	virtual void OnUnregister()override;
private:
	UPROPERTY(EditAnywhere, Category = "LGUILifeCycleBehaviour")
		bool enable = true;
#if WITH_EDITORONLY_DATA
	/** This will allow Update function execute in edit mode. */
	UPROPERTY(EditAnywhere, Category = "LGUILifeCycleBehaviour")
		bool executeInEditMode = false;
	FDelegateHandle EditorTickDelegateHandle;
#endif
protected:
	//for UI object
	virtual void OnUIActiveInHierarchyStateChanged(bool InState);
	FDelegateHandle UIActiveInHierarchyStateChangedDelegateHandle;
	//for not UI object
	void OnComponentRenderStateDirty(UActorComponent& InComp);
	FDelegateHandle ComponentRenderStateDirtyDelegateHandle;
protected:
	uint8 bIsAwakeCalled : 1;
	uint8 bIsStartCalled : 1;
	uint8 bIsEnableCalled : 1;
	uint8 bCanExecuteUpdate : 1;
	uint8 bIsAddedToUpdate : 1;
	uint8 bPrevIsRootComponentVisible : 1;
	/** use this to tell if the class is compiled from blueprint, only blueprint can execute ReceiveXXX. */
	uint8 bCanExecuteBlueprintEvent : 1;
protected:
	friend class ULGUIManagerWorldSubsystem;
	UPROPERTY(Transient) mutable TWeakObjectPtr<USceneComponent> RootComp = nullptr;
	virtual bool IsAllowedToCallAwake()const;
	virtual bool IsAllowedToCallOnEnable()const;
	virtual void SetActiveStateForEnableAndDisable(bool activeOrInactive);
	/**
	 * This function is always called before any Start functions and also after a prefab is instantiated.
	 * This is a good replacement for BeginPlay in LGUI's Prefab workflow. Because Awake will execute after all prefab serialization and object reference is done.
	 * NOTE!!! If RootComponent is UIItem: if UIItem is not "ActiveInHierarchy" during start up, then Awake is not called until "ActiveInHierarchy" becomes true.
	 * Awake execute order in prefab: higher in hierarchy will execute earlier, so scripts on root actor will execute the first.
	 */
	virtual void Awake();
	/** Executed after Awake when GetIsActiveAndEnable is true, or when GetIsActiveAndEnable become true. */
	virtual void OnEnable();
	/** Start is called before the first frame update only if GetIsActiveAndEnable is true. */
	virtual void Start();
	/** Update is called once per frame if GetIsActiveAndEnable is true. */
	virtual void Update(float DeltaTime);
	/** Executed when GetIsActiveAndEnable become false. */
	virtual void OnDisable();
	/**
	 * Executed when this behaviour is destroyed.
	 * If Awake is not executed, then OnDestroy won't execute too.
	 */
	virtual void OnDestroy();

	virtual void Call_Awake();
	void Call_Start();
	void Call_OnEnable();
	void Call_OnDisable();

	/**
	 * This function is always called before any Start functions and also after a prefab is instantiated.
	 * This is a good replacement for BeginPlay in LGUI's Prefab workflow. Because Awake will execute after all prefab serialization and object reference is done.
	 * NOTE!!! If RootComponent is UIItem: if UIItem is inactive during start up, then Awake is not called until it is made active.
	 * Awake execute order in prefab: higher in hierarchy will execute earlier, so scripts on root actor will execute the first.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Awake"), Category = "LGUILifeCycleBehaviour")void ReceiveAwake();
	/** Executed after Awake when GetIsActiveAndEnable is true, or when GetIsActiveAndEnable become true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnEnable"), Category = "LGUILifeCycleBehaviour")void ReceiveOnEnable();
	/** Start is called before the first frame update only if GetIsActiveAndEnable is true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Start"), Category = "LGUILifeCycleBehaviour")void ReceiveStart();
	/** Update is called once per frame if GetIsActiveAndEnable is true. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Update"), Category = "LGUILifeCycleBehaviour")void ReceiveUpdate(float DeltaTime);
	/** Executed when GetIsActiveAndEnable become false. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDisable"), Category = "LGUILifeCycleBehaviour")void ReceiveOnDisable();
	/**
	 * Executed when this behaviour is destroyed.
	 * If Awake is not executed, then OnDestroy won't execute too.
	 */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnDestroy"), Category = "LGUILifeCycleBehaviour")void ReceiveOnDestroy();

public:
	/**
	 * Set if this component can execute "Update" event or not. "CanExecuteUpdate" is true by default.
	 * NOTE!!! This will not immediately affect "Update" event, "Update" event's state will only change after "Awake" "Start" "OnEnable" "OnDisable".
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		void SetCanExecuteUpdate(bool value);

	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		void SetEnable(bool value);
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		bool GetEnable() const { return enable; }
	/**
	 * return true when RootComponent is visible and this component is enable.
	 * NOTE!!! If RootComponent is UIItem: return true when UIItem is "ActiveInHierarchy" and this component is enable. "ActiveInHierarchy" is related to UIItem's IsUIActive.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		virtual bool GetIsActiveAndEnable() const;

	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		USceneComponent* GetRootSceneComponent() const;

	/** Same as DuplicateActor */
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour", meta = (DeterminesOutputType = "OriginObject"))
		AActor* InstantiateActor(AActor* OriginObject, USceneComponent* Parent);
	/** Same as LoadPrefab */
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		AActor* InstantiatePrefab(class ULGUIPrefab* OriginObject, USceneComponent* Parent);
	/** Same as LoadPrefabWithTransform */
	UFUNCTION(BlueprintCallable, Category = "LGUILifeCycleBehaviour")
		AActor* InstantiatePrefabWithTransform(class ULGUIPrefab* OriginObject, USceneComponent* Parent, FVector Location, FRotator Rotation, FVector Scale);
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};