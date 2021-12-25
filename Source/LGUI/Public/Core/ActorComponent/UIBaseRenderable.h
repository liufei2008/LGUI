// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "UIBaseRenderable.generated.h"

class UIGeometry;
class UMaterialInterface;
class ULGUICanvas;
class UUIDrawcall;

UENUM(BlueprintType, Category = LGUI)
enum class EUIRenderableType :uint8
{
	None,
	UIBatchGeometryRenderable,
	UIPostProcessRenderable,
	UIDirectMeshRenderable,
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
	virtual void OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport = ETeleportType::None)override;
	virtual void OnRegister()override;
	virtual void OnUnregister()override;
	EUIRenderableType uiRenderableType = EUIRenderableType::None;

	/**
	 * Render color of UI element.
	 * Color may be override by UISelectable(UIButton, UIToggle, UISlider ...), if UISelectable's transition set to "Color Tint".
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor Color = FColor::White;
	/** Only valid if RaycastTarget is true. true - linetrace hit real mesh triangles, false - linetrace hit AnchorData rectangle */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
		bool bRaycastComplex = false;

	bool LineTraceUIGeometry(TSharedPtr<UIGeometry> InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End);
public:

	virtual void ApplyUIActiveState() override;
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;

	/** get UI renderable type */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		EUIRenderableType GetUIRenderableType()const { return uiRenderableType; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetColor() const { return Color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAlpha() const { return ((float)Color.A) / 255; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetRaycastComplex() { return bRaycastComplex; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetColor(FColor value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAlpha(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetRaycastComplex(bool value) { bRaycastComplex = value; }

	uint8 GetFinalAlpha()const;
	float GetFinalAlpha01()const;
	/** get final color, calculate alpha */
	FColor GetFinalColor()const;
	static float Color255To1_Table[256];

	TSharedPtr<UUIDrawcall> drawcall = nullptr;//drawcall that response for this UI. @todo: use TWeakPtr

	void MarkColorDirty();
	virtual void MarkAllDirtyRecursive()override;
	virtual void SetOnLayoutChange(bool InTransformChange, bool InPivotChange, bool InSizeChange, bool InDiscardCache)override;
	/** Called by LGUICanvas when begin to collect geometry for render */
	virtual void UpdateGeometry() {};

	/** return bounds min max point in self local space, for LGUICanvas to tell if geometry overlap with each other. */
	virtual void GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const;

	virtual void OnCanvasGroupAlphaChange()override;
protected:
	uint8 bColorChanged : 1;
	uint8 bTransformChanged : 1;
};
