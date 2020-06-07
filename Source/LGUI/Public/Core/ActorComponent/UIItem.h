// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "LGUI.h"
#include "Core/UIWidget.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "UIItem.generated.h"

class ULGUICanvas;

UENUM(BlueprintType)
enum class UIItemType :uint8
{
	None,
	UIItem,
	UIRenderable,
};

UCLASS(HideCategories = ( LOD, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIItem : public USceneComponent
{
	GENERATED_BODY()

public:	
	UUIItem(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITORONLY_DATA
	bool isPreEditChange = false;
	//prev frame location
	FVector prevRelativeLocation;
	//prev frame anchor horizontal alignment
	UIAnchorHorizontalAlign prevAnchorHAlign;
	//prev frame anchor vertical alignemnt
	UIAnchorVerticalAlign prevAnchorVAlign;
#endif
#if WITH_EDITOR
	virtual void PreEditChange(UProperty* PropertyAboutToChange)override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
	//USceneComponent Interface.Only needed for show rect range in editor
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//update UI immediately in edit mode
	virtual void EditorForceUpdateImmediately();
#endif
	virtual void OnAttachmentChanged() override;
	
#pragma region UIBaseComponent
private:
	TInlineComponentArray<class ULGUIBehaviour*> UIBaseComponentArray;
	void CallUIComponentsActiveInHierarchyStateChanged();
	void CallUIComponentsChildActiveInHierarchyStateChanged(UUIItem* child, bool activeOrInactive);
	void CallUIComponentsDimensionsChanged(bool positionChanged, bool sizeChanged);
	void CallUIComponentsChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged);
	void CallUIComponentsAttachmentChanged();
	void CallUIComponentsChildAttachmentChanged(UUIItem* child, bool attachOrDettach);
	void CallUIComponentsInteractionStateChanged();
	void CallUIComponentsChildHierarchyIndexChanged(UUIItem* child);
protected://these funcions are same as UIBaseComponent, for easier to use
	//Called when RootUIComp IsActiveInHierarchy state is changed
	virtual void OnUIActiveInHierachy(bool activeOrInactive) { }
	//Called when RootUIComp->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	//@param positionChanged	relative position
	virtual void OnUIDimensionsChanged(bool positionChanged, bool sizeChanged) { }
	//Called when RootUIComp's attachchildren->widget.width/height/anchorOffsetX/anchorOffsetY/stretchLeft/stretchRight/stretchBottom/stretchTop is changed. 
	//@param positionChanged	relative position
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged) { }
	//Called when RootUIComp's attachchildren IsActiveInHierarchy state is changed
	virtual void OnUIChildAcitveInHierarchy(UUIItem* child, bool ativeOrInactive) { }
	//Called when RootUIComp attach to a new parent
	virtual void OnUIAttachmentChanged() { }
	//Called when RootUIComp's attachchildren is attached to RootUIComp or detached from RootUIComp 
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach) { }
	//Called when RootUIComp's interaction state changed(when UIInteractionGroup component allow interaction or not)
	virtual void OnUIInteractionStateChanged(bool interactableOrNot) { }
	//Called when RootUIComp's attachchildren->SetHierarchyIndex() is called, usually used for layout to sort children
	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child) { }
public:
	void AddUIBaseComponent(class ULGUIBehaviour* InComp) { UIBaseComponentArray.Add(InComp); }
	void RemoveUIBaseComponent(class ULGUIBehaviour* InComp) { UIBaseComponentArray.Remove(InComp); }
#pragma endregion UIBaseComponent
protected:
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None)override;
	virtual void OnChildAttached(USceneComponent* ChildComponent)override;
	virtual void OnChildDetached(USceneComponent* ChildComponent)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
private:
	FORCEINLINE void CalculateHorizontalStretchFromAnchorAndSize();
	FORCEINLINE void CalculateVerticalStretchFromAnchorAndSize();
	//@param return		true if size changed, else false
	FORCEINLINE bool CalculateHorizontalAnchorAndSizeFromStretch();
	//@param return		true if size changed, else false
	FORCEINLINE bool CalculateVerticalAnchorAndSizeFromStretch();
#pragma region VertexPositionChangeCallback
private:
	FSimpleMulticastDelegate vertexPositionChangeCallback;
public:
	void RegisterVertexPositionChange(const FSimpleDelegate& InDelegate);
	void UnregisterVertexPositionChange(const FSimpleDelegate& InDelegate);
#pragma endregion VertexPositionChangeCallback
public:
	//update layout and geometry
	virtual void UpdateLayoutAndGeometry(bool& parentLayoutChanged, bool& parentTransformChanged);
	//UIItem's hierarchy changed
	virtual void UIHierarchyChanged();
	//@param return		true if size changed, else false
	bool CalculateLayoutRelatedParameters();
	//update render geometry
	virtual void UpdateGeometry(const bool& parentTransformChanged);

protected:
	//widget contains rect transform and color
	UPROPERTY(EditAnywhere, Category = "LGUI-Widget")
		FUIWidget widget;
	//parent's final alpha value, multiplyed by parent's parent's parent's... alpha value
	float calculatedParentAlpha = 1.0f;
	//parent in hierarchy
	UPROPERTY(Transient) mutable UUIItem* cacheParentUIItem = nullptr;
	//alpha inherit from parent or not
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
		void SetDepth(int32 depth);
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
		void SetAnchorHAlign(UIAnchorHorizontalAlign align, bool keepRelativeLocation = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorVAlign(UIAnchorVerticalAlign align, bool keepRelativeLocation = true);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffsetX(float newOffset);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffsetY(float newOffset);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetAnchorOffset(FVector2D newOffset);
	FORCEINLINE void SetCalculatedParentAlpha(float alpha);

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
	//This can auto calculate dimensions
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetUIRelativeLocation(FVector newLocation);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		void SetUIRelativeLocationAndRotation(const FVector& newLocation, const FQuat& newRotation);
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

	UFUNCTION(BlueprintCallable, Category = "LGUI-Widget")
		UUIItem* GetParentAsUIItem()const;

	void MarkVertexPositionDirty();
	void MarkColorDirty();

	//mark all dirty for UI element to update, include all children
	virtual void MarkAllDirtyRecursive();
protected:
	virtual void MarkCanvasUpdate();
	virtual void WidthChanged();
	virtual void HeightChanged();	


#pragma region InteractionGroup
protected:
	//
	bool allUpParentGroupAllowInteraction = true;
	void SetChildInteractionGroupStateChangeRecursive(bool InParentInteractable);
public:
	void SetInteractionGroupStateChange(bool InInteractable, bool InIgnoreParentGroup);
	void SetInteractionGroupStateChange();
	bool IsGroupAllowInteraction();
#pragma endregion InteractionGroup
#pragma region UIActive
protected:
	//all up parent IsUIActive is true, then this is true. if any up parent is false, then this is false
	bool allUpParentUIActive = true;
	void SetChildUIActiveRecursive(bool InUpParentUIActive);
	/*
	active ui is visible and can interact.
	if parent or parent's parent... IsUIActive is false, then this ui is not visible and not interactable
	*/
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay)
		bool bIsUIActive = true;
	//apply IsUIActive state
	virtual void ApplyUIActiveState();
	void OnChildActiveStateChanged(UUIItem* child);
public:
	//Set this UI element's bIsUIActive
	UFUNCTION(BlueprintCallable, Category = LGUI)
	virtual void SetUIActive(bool active);
	//is UI active itself, parent not count
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsUIActiveSelf()const { return bIsUIActive; }
	//is UI active hierarchy. if all up parent of this ui item is active then return this->IsUIActive. if any up parent ui item is not active then return false
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool IsUIActiveInHierarchy()const { return bIsUIActive && allUpParentUIActive; };
#pragma endregion UIActive
protected:
	//hierarchy index, for layout to sort order
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay)
		int32 hierarchyIndex;
	void OnChildHierarchyIndexChanged(UUIItem* child);
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		int32 GetHierarchyIndex() const { return hierarchyIndex; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetHierarchyIndex(int32 InInt);
#pragma region Collider
protected:
	//if this is a raycastTarget? means LineTrace can hit this or not. for EventSystem interaction
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay)
		bool bRaycastTarget = false;
	//traceChannel for line trace of EventSystem interaction
	UPROPERTY(EditAnywhere, Category = LGUI, AdvancedDisplay)
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

	ULGUICanvas* GetRenderCanvas() const { return RenderCanvas; }
	virtual bool IsScreenSpaceOverlayUI();

	//get UI element type
	UFUNCTION(BlueprintCallable, Category = LGUI)
		UIItemType GetUIItemType()const { return itemType; }
	bool IsCanvasUIItem() { return isCanvasUIItem; }
protected:
	friend class FUIItemCustomization;
	friend class ULGUICanvas;
	UIItemType itemType = UIItemType::None;
	//LGUICanvas which render this UI element
	UPROPERTY(Transient) ULGUICanvas* RenderCanvas = nullptr;
	//is this UIItem's actor have LGUICanvas component
	bool isCanvasUIItem = false;

	bool bDepthChanged = true;//depth changed
	bool bVertexPositionChanged = true;//vertex position changed
	bool bColorChanged = true;//vertex color chnaged
	bool bTransformChanged = true;//transform changed
	//update prev frame's data
	virtual void UpdateBasePrevData();

	//use these bool value and change origin bool value to false, so after UpdateLayout/Geometry if origin bool value changed to true again we call tell LGUICanvas to update again 
	bool cacheForThisUpdate_DepthChanged, cacheForThisUpdate_VertexPositionChanged, cacheForThisUpdate_ColorChanged, cacheForThisUpdate_TransformChanged;
	virtual void UpdateCachedData();
	virtual void UpdateCachedDataBeforeGeometry();

	//find LGUICanvas which render this UI element
	FORCEINLINE bool CheckRenderCanvas();

	FORCEINLINE bool IsFloatNotEqual(float a, float b);
public:
	uint8 GetFinalAlpha()const;
	float GetFinalAlpha01()const;
	//get final color, calculate alpha
	FColor GetFinalColor()const;
	static float Color255To1_Table[256];

#if WITH_EDITORONLY_DATA
	UPROPERTY(Transient)class UUIItemEditorHelperComp* HelperComp = nullptr;
#endif

#pragma region LGUIUpdateComponentToWorld
protected:
	void LGUIUpdateComponentToWorld();
	void LGUIPropagateTransformUpdate(bool inTransformChanged);
	void LGUIUpdateChildTransforms();

	static class UBoolProperty* bComponentToWorldUpdated_PropertyRef;
	static bool LGUIGetComponentToWorldUpdated(USceneComponent* Target);
	static void LGUISetComponentToWorldUpdated(USceneComponent* Target, bool value);
	static void LGUICheckComponentToWorldUpdatedProperty();
#pragma endregion
};


//Editor only
//This component is only a helper component for UIItem! Don't use this!
UCLASS(HideCategories = (LOD, Physics, Collision, Activation, Cooking, Rendering, Actor, Input, Lighting, Mobile), ClassGroup = (LGUI), NotBlueprintable, NotBlueprintType, Transient)
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