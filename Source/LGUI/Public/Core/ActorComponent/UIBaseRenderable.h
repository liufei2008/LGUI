// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIItem.h"
#include "LTweener.h"
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
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InUIRenderable			The UIBaseRenderable object which call this Raycast function
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	OutHitPoint				Hit point position in this UI's local space
	 * @param	OutHitNormal			Hit point normal in this UI's local space
	 * @return	true if hit this UI object
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "Raycast"))
		bool ReceiveRaycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal);
	/**
	 * Get pixel value at hit point.
	 * Only support UI element type which can read pixel value from texture:
	 *		1. UISprite which use LGUIStaticSpriteAtlasData can work perfectly.
	 *		2. UITexture can work with some specific setting.
	 *		3. UIText which use dynamic font can not work. (Currently all LGUI's built-in font is dynamic)
	 * Will fallback to Geometry if ui element not support this raycast type.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		static bool GetRaycastPixelFromUIBatchGeometryRenderable(class UUIBatchGeometryRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector2D& OutUV, FColor& OutPixel, FVector& OutHitPoint, FVector& OutHitNormal);
public:
	/**
	 * Called by UIBaseRenderable when do raycast hit test.
	 * @param	InUIRenderable			The UIBaseRenderable object which call this Raycast function
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	OutHitPoint				Hit point position in this UI's local space
	 * @param	OutHitNormal			Hit point normal in this UI's local space
	 * @return	true if hit this UI object
	 */
	virtual bool Raycast(UUIBaseRenderable* InUIRenderable, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal);
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
	Rect = 0,
	/** Hit on actual triangle geometry */
	Geometry = 1,
	/**
	 * Hit on main texture's pixel, if the pixel is not transparent then hit test success.
	 * Only support UI element type which can read pixel value from texture:
	 *		1. UISprite which use LGUIStaticSpriteAtlasData can work perfectly.
	 *		2. UITexture can work with some specific setting.
	 *		3. UIText which use dynamic font can not work. (Currently all LGUI's built-in font is dynamic)
	 * Will fallback to Geometry if ui element not support this raycast type.
	 */
	VisiblePixel = 3,
	/** Use a user defined UIRenderableCustomRaycast to process the raycast hit. */
	Custom = 2,
};

/** Base class of UI element that can be renderred by LGUICanvas */
UCLASS(Abstract, NotBlueprintable)
class LGUI_API UUIBaseRenderable : public UUIItem
{
	GENERATED_BODY()

public:	
	UUIBaseRenderable(const FObjectInitializer& ObjectInitializer);

protected:
	friend class FUIBaseRenderableCustomization;
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
	/** Custom raycast object to handle raycast behaviour when LGUI do raycast hit test. Only valid if RaycastTarget is true and RaycastType is Custom. */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI-Raycast", meta = (EditCondition = "bRaycastTarget==true&&RaycastType==EUIRenderableRaycastType::Custom"))
		UUIRenderableCustomRaycast* CustomRaycastObject;
	/** Pixel's alpha value threadhold, if hit a pixel which alpha value is less than this value, then hit test return false. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast", meta = (EditCondition = "bRaycastTarget==true&&RaycastType==EUIRenderableRaycastType::VisiblePixel"))
		float VisiblePixelThreadhold = 0.1f;

	bool LineTraceUIGeometry(UIGeometry* InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End);
	bool LineTraceUICustom(FHitResult& OutHit, const FVector& Start, const FVector& End);

    virtual void ApplyUIActiveState(bool InStateChange) override;
	virtual void OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)override;
public:
	static const FName GetColorPropertyName()
	{
		return GET_MEMBER_NAME_CHECKED(UUIBaseRenderable, Color);
	}
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
		UUIRenderableCustomRaycast* GetCustomRaycastObject()const { return CustomRaycastObject; }

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

	TSharedPtr<UUIDrawcall> drawcall = nullptr;//drawcall that response for this UI.

	void MarkColorDirty();
	virtual void MarkAllDirty()override;
	virtual void MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall = false) override;
	/** Called by LGUICanvas when begin to collect geometry for render */
	virtual void UpdateGeometry() {};
	/** Called by LGUICanvas when clip type changed */
	virtual void UpdateMaterialClipType() {};
	/** Called by LGUICanvas after create MaterialInstanceDynamic for this object or it's drawcall */
	virtual void OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) {};

	/** will this UI element affect by canvas's pixel perfect property? */
	virtual bool GetShouldAffectByPixelPerfect()const { return true; };
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
#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* ColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* ColorFrom(FColor startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaFrom(float startValue, float duration = 0.5f, float delay = 0.0f, LTweenEase ease = LTweenEase::OutCubic);
#pragma endregion
};
