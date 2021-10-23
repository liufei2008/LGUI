// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Core/UIWidget.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "UIItem.generated.h"

class ULGUICanvas;

UENUM(BlueprintType, Category = LGUI)
enum class UIItemType :uint8
{
	None,
	UIItem,
	UIBatchGeometryRenderable,
};

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

#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
	/** USceneComponent Interface. Only needed for show rect range in editor */
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	/** update UI immediately in edit mode */
	virtual void EditorForceUpdateImmediately();
#endif
	virtual void OnAttachmentChanged() override;
	
#pragma region LGUIBehaviour
private:
	TInlineComponentArray<class ULGUIBehaviour*> LGUIBehaviourArray;
	void CallUIComponentsActiveInHierarchyStateChanged();
	void CallUIComponentsChildActiveInHierarchyStateChanged(UUIItem* child, bool activeOrInactive);
	void CallUIComponentsDimensionsChanged(bool positionChanged, bool sizeChanged);
	void CallUIComponentsChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged);
	void CallUIComponentsAttachmentChanged();
	void CallUIComponentsChildAttachmentChanged(UUIItem* child, bool attachOrDettach);
	void CallUIComponentsInteractionStateChanged();
	void CallUIComponentsChildHierarchyIndexChanged(UUIItem* child);
protected://these funcions are same as UIBehaviour's, for easier use
	/** Called when RootUIComp IsActiveInHierarchy state is changed */
	virtual void OnUIActiveInHierachy(bool activeOrInactive) { }
	/** 
	 * Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged) { }
	/**
	 * Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	 * @param positionChanged	relative position
	 */
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged) { }
	/** Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed */
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive) { }
	/** Called when RootUIComp attach to a new parent */
	virtual void OnUIAttachmentChanged() { }
	/** Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp  */
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) { }
	/** Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not) */
	virtual void OnUIInteractionStateChanged(bool interactableOrNot) { }
	/** Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children */
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child) { }
public:
	void AddLGUIBehaviourComponent(class ULGUIBehaviour* InComp) { LGUIBehaviourArray.AddUnique(InComp); }
	void RemoveLGUIBehaviourComponent(class ULGUIBehaviour* InComp) { LGUIBehaviourArray.RemoveSingleSwap(InComp); }
#pragma endregion LGUIBehaviour
protected:
	virtual bool MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit /* = NULL */, EMoveComponentFlags MoveFlags /* = MOVECOMP_NoFlags */, ETeleportType Teleport /* = ETeleportType::None */)override;
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None)override;
	virtual void OnChildAttached(USceneComponent* ChildComponent)override;
	virtual void OnChildDetached(USceneComponent* ChildComponent)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
private:
	void CalculateAnchorFromTransform();
	void ApplyAnchorOffsetX(float newOffset);
	void ApplyAnchorOffsetY(float newOffset);
	void ApplyHorizontalStretch(FVector2D newStretch);
	void ApplyVerticalStretch(FVector2D newStretch);
	void CalculateTransformFromAnchor();
	FORCEINLINE void CalculateHorizontalStretchFromAnchorAndSize();
	FORCEINLINE void CalculateVerticalStretchFromAnchorAndSize();
	/** @return		true if size changed, else false */
	FORCEINLINE bool CalculateHorizontalAnchorAndSizeFromStretch();
	/** @return		true if size changed, else false */
	FORCEINLINE bool CalculateVerticalAnchorAndSizeFromStretch();
public:
	/** update layout */
	virtual void UpdateLayout(bool& parentLayoutChanged, bool shouldUpdateLayout);
	virtual void UpdateGeometry();
protected:
	/** UIItem's hierarchy changed */
	virtual void UIHierarchyChanged();
	/** update render geometry */
	/** called when attach to a new RenderCanvas. */
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas);

	/** Called when a new LGUICanvas is registerred on self/parent's actor */
	void RegisterRenderCanvas();
	/** Called when LGUICanvas is unregisterred on self/parent's actor */
	void UnregisterRenderCanvas();

protected:
	/** widget contains rect transform and color */
	UPROPERTY(EditAnywhere, Category = "LGUI-Widget")
		FUIWidget widget;
	/** parent's final alpha value, multiplyed by parent's parent's parent's... alpha value */
	float calculatedParentAlpha = 1.0f;
	/** parent in hierarchy */
	mutable TWeakObjectPtr<UUIItem> ParentUIItem = nullptr;
	/** root in hierarchy */
	mutable TWeakObjectPtr<UUIItem> RootUIItem = nullptr;
	/** UI children array, sorted by hierarchy index */
	UPROPERTY(Transient) TArray<UUIItem*> UIChildren;
	/** check valid, incase unnormally deleting actor, like undo */
	void CheckCacheUIChildren();
	void SortCacheUIChildren();
	/** alpha inherit from parent or not */
	UPROPERTY(EditAnywhere, Category = "LGUI-Widget")
		bool inheritAlpha = true;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		bool GetInheritAlpha()const { return inheritAlpha; }
#pragma region Widget
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		const FUIWidget& GetWidget()const { return widget; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetWidget(const FUIWidget& inWidget);
	/**
	 * Set UIItem depth.
	 * @param	propagateToChildren	if true, set this UIItem's depth and all UIItems that is attached to this UIItem, not just set absolute value, but keep child's relative depth to this one.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetDepth(int32 depth, bool propagateToChildren = false);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetColor(FColor color);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAlpha(float newAlpha);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetWidth(float newWidth);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetHeight(float newHeight);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetStretchLeft(float newLeft);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetStretchRight(float newRight);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetStretchTop(float newTop);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetStretchBottom(float newBottom);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetHorizontalStretch(FVector2D newStretch);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetVerticalStretch(FVector2D newStretch);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetPivot(FVector2D pivot);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorHAlign(UIAnchorHorizontalAlign align);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorVAlign(UIAnchorVerticalAlign align);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffsetX(float newOffset);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffsetY(float newOffset);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffset(FVector2D newOffset);
	void SetCalculatedParentAlpha(float alpha);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		int GetDepth() const { return widget.depth; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FColor GetColor() const { return widget.color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetAlpha() const { return ((float)widget.color.A) / 255; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetWidth() const { return widget.width; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetHeight() const { return widget.height; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FVector2D GetPivot() const { return widget.pivot; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		UIAnchorHorizontalAlign GetAnchorHAlign() const { return widget.anchorHAlign; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		UIAnchorVerticalAlign GetAnchorVAlign() const { return widget.anchorVAlign; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetAnchorOffsetX() const { return widget.anchorOffsetX; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetAnchorOffsetY() const { return widget.anchorOffsetY; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FVector2D GetAnchorOffset()const { return FVector2D(widget.anchorOffsetX, widget.anchorOffsetY); }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetStretchLeft() const { return widget.stretchLeft; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetStretchRight() const { return widget.stretchRight; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetStretchTop() const { return widget.stretchTop; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetStretchBottom() const { return widget.stretchBottom; }
#pragma endregion
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetCalculatedParentAlpha() const { return calculatedParentAlpha; }

	/** This can auto calculate dimensions */
	UE_DEPRECATED(4.23, "Use SetRelativeLocation instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRelativeLocation instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIRelativeLocation(FVector newLocation);
	/** This can auto calculate dimensions */
	UE_DEPRECATED(4.23, "Use SetRelativeLocationAndRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRelativeLocationAndRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIRelativeLocationAndRotation(const FVector& newLocation, const FRotator& newRotation);
	/** This can auto calculate dimensions */
	UE_DEPRECATED(4.23, "Use SetRelativeLocationAndRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRelativeLocationAndRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIRelativeLocationAndRotationQuat(const FVector& newLocation, const FQuat& newRotation);
	/** This is a simple one parameter version */
	UE_DEPRECATED(4.23, "Use SetRelativeRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRelativeRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIRelativeRotation(const FRotator& newRotation);
	/** This is a simple one parameter version */
	UE_DEPRECATED(4.23, "Use SetRelativeRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRelativeRotation instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIRelativeRotationQuat(const FQuat& newRotation);
	/** This can auto calculate dimensions */
	UE_DEPRECATED(4.23, "Use AttachToComponent instead. Because LGUI can automatically calculate anchor parameters after transform changed.")
	UFUNCTION(BlueprintCallable, Category = "LGUI", meta = (DeprecatedFunction, DeprecationMessage = "Use AttachToComponent instead. Because LGUI can automatically calculate anchor parameters after transform changed."))
		void SetUIParent(UUIItem* inParent, bool keepWorldTransform = false);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FVector2D GetLocalSpaceLeftBottomPoint()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FVector2D GetLocalSpaceRightTopPoint()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		FVector2D GetLocalSpaceCenter()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetLocalSpaceLeft()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetLocalSpaceRight()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetLocalSpaceBottom()const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		float GetLocalSpaceTop()const;

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIItem* GetParentAsUIItem()const;
	/** get UI children array, sorted by hierarchy index */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<UUIItem*>& GetAttachUIChildren()const { return UIChildren; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIItem* GetAttachUIChild(int index)const;
	/** Get root canvas of hierarchy */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULGUICanvas* GetRootCanvas()const;
	/** Get LGUICanvasScaler from root canvas, return null if not have one */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		class ULGUICanvasScaler* GetCanvasScaler()const;
#ifdef LGUI_DRAWCALLMODE_AUTO
	/** For autoManageDepth, return bounds min max point in self local space */
	virtual void GetLocalSpaceMinMaxPoint_ForAutoManageDepth(FVector2D& min, FVector2D& max)const;
#endif
	void MarkLayoutDirty(bool sizeChange);
	void MarkColorDirty();

	/** mark all dirty for UI element to update, include all children */
	virtual void MarkAllDirtyRecursive();
protected:
	virtual void MarkCanvasUpdate();
	virtual void WidthChanged();
	virtual void HeightChanged();	
	virtual void PivotChanged();
	virtual void DepthChanged();

#pragma region InteractionGroup
protected:
	//
	bool allUpParentGroupAllowInteraction = true;
	void SetChildInteractionGroupStateChangeRecursive(bool InParentInteractable);
public:
	void SetInteractionGroupStateChange(bool InInteractable, bool InIgnoreParentGroup);
	void SetInteractionGroupStateChange();
	bool IsGroupAllowInteraction()const;
#pragma endregion InteractionGroup
#pragma region UIActive
protected:
	/** all up parent IsUIActive is true, then this is true. if any up parent is false, then this is false */
	bool allUpParentUIActive = true;
	void SetChildUIActiveRecursive(bool InUpParentUIActive);
	/**
	 * Active ui is visible and interactable.
	 * If parent or parent's parent... IsUIActive is false, then this ui is not visible and not interactable.
	 */
	UPROPERTY(EditAnywhere, Category = LGUI, meta = (DisplayName = "Is UI Active"))
		bool bIsUIActive = true;
	/** apply IsUIActive state */
	virtual void ApplyUIActiveState();
	void OnChildActiveStateChanged(UUIItem* child);
public:
	/** Set this UI element's bIsUIActive */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		virtual void SetIsUIActive(bool active);
	UE_DEPRECATED(4.24, "Use SetIsUIActive instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "Use SetIsUIActive instead."))
		virtual void SetUIActive(bool active);
	/** is UI active itself, parent not count */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIsUIActiveSelf()const { return bIsUIActive; }
	UE_DEPRECATED(4.24, "Use GetIsUIActiveSelf instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "Use GetIsUIActiveSelf instead."))
		bool IsUIActiveSelf()const { return bIsUIActive; }
	/** is UI active hierarchy. if all up parent of this ui item is active then return this->IsUIActive. if any up parent ui item is not active then return false */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetIsUIActiveInHierarchy()const { return bIsUIActive && allUpParentUIActive; };
	UE_DEPRECATED(4.24, "Use GetIsUIActiveInHierarchy instead.")
	UFUNCTION(BlueprintCallable, Category = LGUI, meta = (DeprecatedFunction, DeprecationMessage = "Use GetIsUIActiveInHierarchy instead."))
		bool IsUIActiveInHierarchy()const { return bIsUIActive && allUpParentUIActive; };
#pragma endregion UIActive

#pragma region HierarchyIndex
protected:
	/** hierarchy index */
	UPROPERTY(EditAnywhere, Category = LGUI)
		int32 hierarchyIndex = INDEX_NONE;
	int32 flattenHierarchyIndex = 0;
	void OnChildHierarchyIndexChanged(UUIItem* child);
private:
	//@todo: optimize this calculation
	void RecalculateFlattenHierarchyIndex();
	void CalculateFlattenHierarchyIndex_Recursive(int& parentFlattenHierarchyIndex);
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetHierarchyIndex() const { return hierarchyIndex; }
	/** Get flatten hierarchy index, calculate from the first top most UIItem. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetFlattenHierarchyIndex()const { return flattenHierarchyIndex; }
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
		FString GetDisplayName()const { return displayName; }
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
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
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

	/** get UI element type */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UIItemType GetUIItemType()const { return itemType; }
	bool IsCanvasUIItem() { return bIsCanvasUIItem; }
protected:
	friend class FUIItemCustomization;
	friend class ULGUICanvas;
	UIItemType itemType = UIItemType::None;
	/** LGUICanvas which render this UI element */
	mutable TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
	/** is this UIItem's actor have LGUICanvas component */
	mutable uint16 bIsCanvasUIItem:1;
	uint16 bCanSetAnchorFromTransform : 1;

	uint16 bColorChanged:1;//vertex color chnaged
	uint16 bLayoutChanged:1;//layout changed
	uint16 bSizeChanged : 1;//rect size changed
	uint16 bShouldUpdateLayout : 1;//if any child layout changed
	/** update prev frame's data */
	virtual void UpdateBasePrevData();

	/** use these bool value and change origin bool value to false, so after UpdateLayout/Geometry if origin bool value changed to true again we call tell LGUICanvas to update again  */
	uint16 cacheForThisUpdate_ColorChanged:1, cacheForThisUpdate_LayoutChanged:1, cacheForThisUpdate_SizeChanged:1, cacheForThisUpdate_ShouldUpdateLayout:1;
	virtual void UpdateCachedData();
	virtual void UpdateCachedDataBeforeGeometry();

	/** find LGUICanvas which render this UI element */
	bool CheckRenderCanvas()const;
	/** find root UIItem of hierarchy */
	void CheckRootUIItem();

	/** mark any child's layout change, only for RootUIItem */
	void MarkUpdateLayout();

	void UpdateChildLayoutRecursive(UUIItem* target, bool parentLayoutChanged);
public:
	/** Called from LGUIManagerActor */
	void UpdateRootUIItemLayout();
public:
	uint8 GetFinalAlpha()const;
	float GetFinalAlpha01()const;
	/** get final color, calculate alpha */
	FColor GetFinalColor()const;
	static float Color255To1_Table[256];

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)class UUIItemEditorHelperComp* HelperComp = nullptr;
#endif

#if WITH_EDITORONLY_DATA
private:
	/** prev frame anchor horizontal alignment */
	UIAnchorHorizontalAlign prevAnchorHAlign;
	/** prev frame anchor vertical alignemnt */
	UIAnchorVerticalAlign prevAnchorVAlign;
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
#if WITH_EDITORONLY_DATA
	//just need these view releated parameters from FSceneView(GetViewRelevance(const FSceneView* View)), but don't know how to get them at anywhere else, so just put them here.
	static FIntRect viewRect;
	static FViewMatrices viewMatrices;
#endif
	UPROPERTY(Transient)UUIItem* Parent = nullptr;
	virtual UBodySetup* GetBodySetup()override;
	UPROPERTY(Transient, DuplicateTransient)
		class UBodySetup* BodySetup;
	void UpdateBodySetup();
};