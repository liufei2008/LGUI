// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIBaseRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;
class ULGUICanvas;
class UUIDrawcall;
class UUIRenderableCustomRaycast;
class UUIBaseRenderable;

/**
 * This component is only used when UIBaseRenderable's RaycastType = Custom
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API UUIRenderableCustomRaycast :public UObject
{
	GENERATED_BODY()

protected:
	/** Called by owner UIRenderable object when register, use it as initialize */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "Init"))
		void ReceiveInit(UUIBaseRenderable* InUIRenderable);
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	InHitPointOnPlane		Ray-Plane hit point in this UI's local space, relative to UI's pivot point
	 * @return	true if hit this UI object
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "Raycast"))
		bool ReceiveRaycast(const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, const FVector2D& InHitPointOnPlane);
public:
	/** Called by owner UIRenderable object when register, use it as initialize */
	virtual void Init(UUIBaseRenderable* InUIRenderable);
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	InHitPointOnPlane		Ray-Plane hit point in this UI's local space, relative to UI's pivot point
	 * @return	true if hit this UI object
	 */
	virtual bool Raycast(const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, const FVector2D& InHitPointOnPlane);
};

UENUM(BlueprintType, Category = LGUI)
enum class EUIRenderableType :uint8
{
	None,
	UIBatchGeometryRenderable,
	UIPostProcessRenderable,
	UIDirectMeshRenderable,
};

UENUM(BlueprintType, Category = LGUI)
enum class EUIRenderableRaycastType :uint8
{
	/** Hit on rect range */
	Rect,
	/** Hit on actual triangle geometry */
	Geometry,
	/** Use a user defined UIRenderableCustomRaycast to process the raycast hit. */
	Custom,
};

/** Base class of UI element that can be renderred by LGUICanvas */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIBaseRenderable : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIBaseRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	EUIRenderableType UIRenderableType = EUIRenderableType::None;

	/**
	 * Render color of UI element.
	 * Color may be override by UISelectable(UIButton, UIToggle, UISlider ...), if UISelectable's transition set to "Color Tint".
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor Color = FColor::White;
	/** Only valid if RaycastTarget is true. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast", meta = (EditCondition = "bRaycastTarget==true"))
		EUIRenderableRaycastType RaycastType = EUIRenderableRaycastType::Rect;
	/** Custom raycast object to handle raycast behaviour when LGUI do raycast hit test, Only valid if RaycastTarget is true and RaycastType is Custom. */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI-Raycast", meta = (EditCondition = "bRaycastTarget==true&&RaycastType==EUIRenderableRaycastType::Custom"))
		UUIRenderableCustomRaycast* CustomRaycastObject;

	bool LineTraceUIGeometry(UIGeometry* InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End);
	bool LineTraceUICustom(FHitResult& OutHit, const FVector& Start, const FVector& End);

    virtual void ApplyUIActiveState(bool InStateChange) override;
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
public:
	/** get UI renderable type */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		EUIRenderableType GetUIRenderableType()const { return UIRenderableType; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor() const { return Color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAlpha() const { return ((float)Color.A) / 255; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		EUIRenderableRaycastType GetRaycastType()const { return RaycastType; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlpha(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastType(EUIRenderableRaycastType Value) { RaycastType = Value; }
	/** Set custom raycast object to handle raycast behaviour, only valid if RaycastType is Custom */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetCustomRaycastObject(UUIRenderableCustomRaycast* Value);

	uint8 GetFinalAlpha()const;
	/** get final alpha, calculated with CanvasGroup's alpha */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFinalAlpha01()const;
	/** get final color, calculated with CanvasGroup's alpha */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetFinalColor()const;
	static float Color255To1_Table[256];

	TSharedPtr<UUIDrawcall> drawcall = nullptr;//drawcall that response for this UI.

	void MarkColorDirty();
	virtual void MarkAllDirty()override;
	virtual void MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall = false) override;
	/** Called by LGUICanvas when begin to collect geometry for render */
	virtual void UpdateGeometry() {};

	/** return bounds min max point in self local space, for LGUICanvas to tell if geometry overlap with each other. */
	virtual void GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const;
#if WITH_EDITOR
	/** editor only, return 3d bounds in self local space */
	virtual void GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const;
#endif

	virtual void OnCanvasGroupAlphaChange()override;
protected:
	uint8 bColorChanged : 1;
	uint8 bTransformChanged : 1;
public:
	/** Old data */
	UPROPERTY(VisibleAnywhere, Category = "LGUI-old")
		bool bRaycastComplex = false;
public:
	UE_DEPRECATED(4.24, "Use GetRaycastType instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI-old", meta = (DePrecatedFunction, DeprecationMessage = "Use GetRaycastType instead."))
		bool GetRaycastComplex() { return RaycastType == EUIRenderableRaycastType::Geometry; }
	UE_DEPRECATED(4.24, "Use SetRaycastType instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI-old", meta = (DePrecatedFunction, DeprecationMessage = "Use SetRaycastType instead."))
		void SetRaycastComplex(bool value) { RaycastType = bRaycastComplex ? EUIRenderableRaycastType::Geometry : EUIRenderableRaycastType::Rect; }
};
