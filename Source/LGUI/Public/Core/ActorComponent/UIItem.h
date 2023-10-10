// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Core/UIAnchorData.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "LTweener.h"
#include "UIItem.generated.h"

class ULGUICanvas;
class UUICanvasGroup;
enum class ELGUIRenderMode : uint8;

DECLARE_MULTICAST_DELEGATE_OneParam(FUIItemActiveInHierarchyStateChangedMulticastDelegate, bool);
DECLARE_DELEGATE_OneParam(FUIItemActiveInHierarchyStateChangedDelegate, bool);

/**
 * Base class for almost all UI related things.
 */
UCLASS(HideCategories = ( LOD, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIItem : public USceneComponent
{
	GENERATED_BODY()

public:	
	UUIItem(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostLoad()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditUndo()override;
	//virtual void PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation)override;
	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent)override;
	/** USceneComponent Interface. Only needed for show rect range in editor */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	/** update UI immediately in edit mode */
	virtual void EditorForceUpdate();//@todo: remove this
#endif
#if WITH_EDITORONLY_DATA
	static TSet<FName> PersistentOverridePropertyNameSet;
#endif
	static const FName GetAnchorDataPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIItem, AnchorData);
	}
	static const FName GetHierarchyIndexPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex);
	}
	static const FName GetTraceChannelPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIItem, traceChannel);
	}
	
#pragma region LGUILifeCycleUIBehaviour
private:
	TInlineComponentArray<class ULGUILifeCycleUIBehaviour*> LGUILifeCycleUIBehaviourArray;
	void CallUILifeCycleBehavioursActiveInHierarchyStateChanged();
	void CallUILifeCycleBehavioursChildActiveInHierarchyStateChanged(UUIItem* child, bool activeOrInactive);
	void CallUILifeCycleBehavioursDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	void CallUILifeCycleBehavioursChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged);
	void CallUILifeCycleBehavioursAttachmentChanged();
	void CallUILifeCycleBehavioursChildAttachmentChanged(UUIItem* child, bool attachOrDettach);
	void CallUILifeCycleBehavioursInteractionStateChanged();
	void CallUILifeCycleBehavioursChildHierarchyIndexChanged(UUIItem* child);
protected://these funcions are same as UIBehaviour's, for easier use
	/** Called when this IsActiveInHierarchy state is changed */
	virtual void OnUIActiveInHierachy(bool activeOrInactive) { }
	/** 
	 * Called when this->AnchorData is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged) { }
	/**
	 * Called when this's attachchildren->AnchorData is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged) { }
	/** Called when this's attachchildren IsActiveInHierarchy state is changed */
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive) { }
	/** Called when this attach to a new parent */
	virtual void OnUIAttachmentChanged() { }
	/** Called when this's attachchildren is attached to this or detached from this  */
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) { }
	/** Called when this's interaction state changed(when UICanvasGroup component allow interaction or not) */
	virtual void OnUIInteractionStateChanged(bool interactableOrNot) { }
	/** Called when this's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child) { }
public:
	void AddLGUILifeCycleUIBehaviourComponent(class ULGUILifeCycleUIBehaviour* InComp) { LGUILifeCycleUIBehaviourArray.AddUnique(InComp); }
	void RemoveLGUILifeCycleUIBehaviourComponent(class ULGUILifeCycleUIBehaviour* InComp) { LGUILifeCycleUIBehaviourArray.RemoveSingleSwap(InComp); }
#pragma endregion LGUILifeCycleUIBehaviour
protected:
	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit /* = NULL */, EMoveComponentFlags MoveFlags /* = MOVECOMP_NoFlags */, ETeleportType Teleport /* = ETeleportType::None */)override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None)override;
	virtual void OnChildAttached(USceneComponent* ChildComponent)override;
	virtual void OnChildDetached(USceneComponent* ChildComponent)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy)override;
	void OnUIDetachedFromParent();
	void OnUIAttachedToParent();
public:
	void CalculateAnchorFromTransform();
	void CalculateTransformFromAnchor();
	void CalculateTransformFromAnchor(bool& OutHorizontalPositionChanged, bool& OutVerticalPositionChanged);
public:

	FDelegateHandle RegisterUIHierarchyChanged(const FSimpleDelegate& InCallback);
	void UnregisterUIHierarchyChanged(const FDelegateHandle& InHandle);
protected:
	/** UIItem's hierarchy changed */
	void UIHierarchyChanged(ULGUICanvas* ParentRenderCanvas, UUICanvasGroup* ParentCanvasGroup, UUIItem* ParentRoot);
	FSimpleMulticastDelegate UIHierarchyChangedDelegate;
	/** called when RenderCanvas changed. */
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas);
	void SetRenderCanvas(ULGUICanvas* InNewCanvas);
public:
	/** Called by LGUICanvas, when a new LGUICanvas is registerred on self actor */
	void RegisterRenderCanvas(ULGUICanvas* InRenderCanvas);
	/** Called by LGUICanvas, when LGUICanvas is unregisterred on self actor */
	void UnregisterRenderCanvas();
protected:
	void RenewCanvasGroupRecursive(UUICanvasGroup* InParentCanvasGroup);
	void RenewRenderCanvasRecursive(ULGUICanvas* InParentRenderCanvas);

protected:
	/** AnchorData contains rect transform and color */
	UPROPERTY(EditAnywhere, Category = "LGUI-AnchorData")
		FUIAnchorData AnchorData;
	/** parent in hierarchy */
	UPROPERTY(Transient) mutable TWeakObjectPtr<UUIItem> ParentUIItem = nullptr;
	/** root in hierarchy */
	mutable TWeakObjectPtr<UUIItem> RootUIItem = nullptr;//don't mark this Transactional, because undo or redo will call register/unregister, which will trigger check RootUIItem
	/** UI children array, sorted by hierarchy index */
	UPROPERTY(Transient) mutable TArray<TObjectPtr<UUIItem>> UIChildren;
	/** check valid, incase unnormally deleting actor, like undo */
	void EnsureUIChildrenValid();
	void EnsureUIChildrenSorted()const;
#pragma region AnchorData
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		const FUIAnchorData& GetAnchorData()const { return AnchorData; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetPivot() const { return AnchorData.Pivot; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetAnchorMin() const { return AnchorData.AnchorMin; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetAnchorMax() const { return AnchorData.AnchorMax; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetAnchoredPosition() const { return AnchorData.AnchoredPosition; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetSizeDelta() const { return AnchorData.SizeDelta; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetHorizontalAnchoredPosition() const { return AnchorData.AnchoredPosition.X; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetVerticalAnchoredPosition() const { return AnchorData.AnchoredPosition.Y; }

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetWidth() const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetHeight() const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetAnchorLeft()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetAnchorTop()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetAnchorRight()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetAnchorBottom()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorData(const FUIAnchorData& Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetPivot(FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorMin(FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorMax(FVector2D Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetHorizontalAndVerticalAnchorMinMax(FVector2D MinValue, FVector2D MaxValue, bool bKeepSize, bool bKeepRelativeLocation);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetHorizontalAnchorMinMax(FVector2D Value, bool bKeepSize = false, bool bKeepRelativeLocation = false);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetVerticalAnchorMinMax(FVector2D Value, bool bKeepSize = false, bool bKeepRelativeLocation = false);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchoredPosition(FVector2D Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetHorizontalAnchoredPosition(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetVerticalAnchoredPosition(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetSizeDelta(FVector2D Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetWidth(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetHeight(float Value);

	/** This function only valid if UIItem have parent */
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorLeft(float Value);
	/** This function only valid if UIItem have parent */
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorTop(float Value);
	/** This function only valid if UIItem have parent */
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorRight(float Value);
	/** This function only valid if UIItem have parent */
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		void SetAnchorBottom(float Value);

	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetLocalSpaceLeftBottomPoint()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetLocalSpaceRightTopPoint()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		FVector2D GetLocalSpaceCenter()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetLocalSpaceLeft()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetLocalSpaceRight()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetLocalSpaceBottom()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-AnchorData")
		float GetLocalSpaceTop()const;
#pragma endregion

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIItem* GetParentUIItem()const{ return ParentUIItem.Get(); }
	/** get UI children array, sorted by hierarchy index */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<UUIItem*>& GetAttachUIChildren()const { EnsureUIChildrenSorted(); return UIChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIItem* GetAttachUIChild(int index)const;
	/** Get root canvas of hierarchy */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULGUICanvas* GetRootCanvas()const;
	/** Get LGUICanvasScaler from root canvas, return null if not have one */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class ULGUICanvasScaler* GetCanvasScaler()const;

	/** mark all dirty for UI element to update, include all children */
	void MarkAllDirtyRecursive();
	virtual void MarkAllDirty();
	/** force refresh render canvas, remove from old and add to new */
	void ForceRefreshRenderCanvasRecursive();
#if WITH_EDITOR
	void ForceRefreshUIActiveStateRecursive();
#endif
	virtual void MarkRenderModeChangeRecursive(ULGUICanvas* Canvas, ELGUIRenderMode OldRenderMode, ELGUIRenderMode NewRenderMode);
private:
	void SetOnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange);
	void SetOnTransformChange(bool InPositionChanged, bool InScaleChanged);
protected:
	virtual void OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache = true);
public:
	virtual void MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall = false);
private:
	mutable float CacheWidth = 0, CacheHeight = 0, CacheAnchorLeft = 0, CacheAnchorRight = 0, CacheAnchorTop = 0, CacheAnchorBottom = 0;
	mutable uint8 bWidthCached : 1, bHeightCached : 1, bAnchorLeftCached : 1, bAnchorRightCached : 1, bAnchorTopCached : 1, bAnchorBottomCached : 1;
	mutable uint8 bNeedSortUIChildren : 1;
	FVector2f PrevScale2D = FVector2f::One();
#pragma region UICanvasGroup
protected:
	UPROPERTY(Transient) mutable TWeakObjectPtr<UUICanvasGroup> CanvasGroup;
	FDelegateHandle OnCanvasGroupAlphaChangeDelegateHandle;
	virtual void OnCanvasGroupAlphaChange() {};
	FDelegateHandle OnCanvasGroupInteractableStateChangeDelegateHandle;
	void OnCanvasGroupInteractableStateChange();
	bool bIsGroupAllowInteraction = true;
	void SetCanvasGroup(UUICanvasGroup* InNewCanvasGroup);
public:
	void RegisterCanvasGroup(UUICanvasGroup* InCanvasGroup);
	void UnregisterCanvasGroup();

	bool IsGroupAllowInteraction()const { return bIsGroupAllowInteraction; }
	/** return UICanvasGroup component that manager this UIItem. return null if there is no one. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUICanvasGroup* GetCanvasGroup()const { return CanvasGroup.Get(); }
#pragma endregion UICanvasGroup
#pragma region UIActive
public:
	void CheckUIActiveState();
protected:
	/** all up parent IsUIActive is true, then this is true. if any up parent is false, then this is false */
	bool bAllUpParentUIActive = true;
	void CheckChildrenUIActiveRecursive(bool InUpParentUIActive);
	/**
	 * Active ui is visible and interactable.
	 * If parent or parent's parent... IsUIActive is false, then this ui is not visible and not interactable.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (DisplayName = "Is UI Active"))
		bool bIsUIActive = true;
	/** apply IsUIActive state */
	virtual void ApplyUIActiveState(bool InStateChange);
	void OnChildActiveStateChanged(UUIItem* child);

	FUIItemActiveInHierarchyStateChangedMulticastDelegate UIActiveInHierarchyStateChangedDelegate;
public:
	FDelegateHandle RegisterUIActiveStateChanged(const FUIItemActiveInHierarchyStateChangedDelegate& InCallback);
	FDelegateHandle RegisterUIActiveStateChanged(const TFunction<void(bool)>& InCallback);
	void UnregisterUIActiveStateChanged(const FDelegateHandle& InHandle);

	/** Set this UI element's bIsUIActive */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		virtual void SetIsUIActive(bool active);
	/** is UI active itself, parent not count */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIsUIActiveSelf()const { return bIsUIActive; }
	/** is UI active hierarchy. if all up parent of this ui item is active then return this->IsUIActive. if any up parent ui item is not active then return false */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIsUIActiveInHierarchy()const { return bIsUIActive && bAllUpParentUIActive; };
#if WITH_EDITOR
	void SetIsTemporarilyHiddenInEditor_Recursive_By_IsUIActiveState();
#endif
#pragma endregion UIActive

#pragma region HierarchyIndex
protected:
	/** hierarchy index, hierarchy order, render order */
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 hierarchyIndex = INDEX_NONE;
	UPROPERTY(Transient, VisibleAnywhere, Category = LGUI, AdvancedDisplay)
	mutable int32 flattenHierarchyIndex = 0;
	void MarkFlattenHierarchyIndexDirty();
private:
	/** Only for RootUIItem */
	void RecalculateFlattenHierarchyIndex()const;
	void CalculateFlattenHierarchyIndex_Recursive(int& index)const;
	void ApplyHierarchyIndex();
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetHierarchyIndex() const { return hierarchyIndex; }
	/** Get flatten hierarchy index, calculate from the first top most UIItem. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetFlattenHierarchyIndex()const;
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHierarchyIndex(int32 InInt);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetAsFirstHierarchy();
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetAsLastHierarchy();
#pragma endregion HierarchyIndex

#pragma region Name
private:
	/** 
	 * This is useful when you need to find child UI element by name, use function "FindChildByDisplayName" or "FindChildArrayByDisplayName" to do it.
	 * Mostly the displayName is the same as Actor's ActorLabel. If you want to change it, just change the actor label( Actor's name in world outliner).
	 * If Actor's ActorLabel start with "//", then the "//" will be ignored.
	 * ActorLabel is only valid in editor, but this is also valid on runtime.
	 */
	UPROPERTY(VisibleAnywhere, Category = LGUI, AdvancedDisplay)
		FString displayName;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		const FString& GetDisplayName()const { return displayName; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetDisplayName(const FString& InName) { displayName = InName; }
	/** 
	 * Search in children and return the first UIItem that the displayName match input name.
	 * Support hierarchy nested search, eg: InName = "Content/ListItem/NameLabel".
	 * @param InName	The child's name that need to find, case sensitive
	 * @param IncludeChildren	Also search in children
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UUIItem* FindChildByDisplayName(const FString& InName, bool IncludeChildren = false)const;
	/**
	 * Like "FindChildByDisplayName", but return all children that match the case.
	 * @param InName	The child's name that need to find, case sensitive
	 * @param IncludeChildren	Also search in children
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TArray<UUIItem*> FindChildArrayByDisplayName(const FString& InName, bool IncludeChildren = false)const;
private:
	UUIItem* FindChildByDisplayNameWithChildren_Internal(const FString& InName)const;
	void FindChildArrayByDisplayNameWithChildren_Internal(const FString& InName, TArray<UUIItem*>& OutResultArray)const;
#pragma endregion Name

#pragma region Collider
protected:
	/**
	 * If this is a raycastTarget? Means LineTrace can hit this or not, for EventSystem interaction.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
		bool bRaycastTarget = false;
	/** traceChannel for line trace of EventSystem interaction */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast", meta=(EditCondition="bRaycastTarget"))
		TEnumAsByte<ETraceTypeQuery> traceChannel = TraceTypeQuery3;
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsRaycastTarget() const { return bRaycastTarget; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetRaycastTarget(bool NewBool);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetTraceChannel(TEnumAsByte<ETraceTypeQuery> InTraceChannel);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TEnumAsByte<ETraceTypeQuery> GetTraceChannel()const { return traceChannel; }
	virtual bool LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End);
#pragma endregion
	/** Get the canvas that render and update this UI element */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULGUICanvas* GetRenderCanvas() const;
	/** Is this UI element render to screen space overlay? */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool IsScreenSpaceOverlayUI()const;
	/** Is this UI element render to a RenderTarget? */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool IsRenderTargetUI()const;
	/** Is this UI element render in world space? */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool IsWorldSpaceUI()const;

	bool IsCanvasUIItem() { return bIsCanvasUIItem; }

	/** return root UIItem in hierarchy, could be null if not initialized yet. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIItem* GetRootUIItemInHierarchy()const { return RootUIItem.Get(); }
protected:
	friend class FUIItemCustomization;
	friend class ULGUICanvas;
	/** LGUICanvas which render this UI element */
	UPROPERTY(Transient) mutable TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
	/** is this UIItem's actor have LGUICanvas component */
	UPROPERTY(Transient) mutable uint8 bIsCanvasUIItem:1;
	uint8 bCanSetAnchorFromTransform : 1;

	/** Only for RootUIItem, if dirty then we need to recalculate it */
	mutable uint8 bFlattenHierarchyIndexDirty : 1;
#if WITH_EDITOR
	uint8 bUIActiveStateDirty : 1;
#endif

	/** find root UIItem of hierarchy */
	void CheckRootUIItem(UUIItem* RootUIItemInParent = nullptr);
	virtual bool LineTraceUIRect(FHitResult& OutHit, const FVector& Start, const FVector& End);
public:
#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* WidthTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* HeightTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* HorizontalAnchoredPositionTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* VerticalAnchoredPositionTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AnchoredPositionTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* PivotTo(FVector2D endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AnchorLeftTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AnchorRightTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AnchorTopTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AnchorBottomTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion
public:
#if WITH_EDITORONLY_DATA
	/** This is a helper component for calculate bounds, so we can double click to focus on this UIItem */
	UPROPERTY(Transient, NonTransactional)TObjectPtr<class UUIItemEditorHelperComp> HelperComp = nullptr;//@todo: better way to replace this?
#endif
};


//Editor only
//This component is only a helper component for UIItem! Don't use this!
//For UIItem's bounds, so we can double click a UIItem and focus on it.
UCLASS(HideCategories = (LOD, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile), NotBlueprintable, NotBlueprintType, Transient)
class LGUI_API UUIItemEditorHelperComp : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	UUIItemEditorHelperComp();
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
#if WITH_EDITOR
	virtual FPrimitiveSceneProxy* CreateSceneProxy()override;
#endif
	UPROPERTY(Transient)TObjectPtr<UUIItem> Parent = nullptr;
	virtual UBodySetup* GetBodySetup()override;
	UPROPERTY(Transient)
		TObjectPtr<class UBodySetup> BodySetup;
	void UpdateBodySetup();
};