// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexWidgetSubObjectBehaviour.h"
#include "LTweener.h"
#include "Utils/LexUIUtils.h"
#include "LexVisual.generated.h"

struct FLexUIHitResult;
class FLexUIGeometry;
class UMaterialInterface;
class ULexCanvas;
class ULexVisualCustomRaycast;
class ULexVisual;

/**
 * This component is only used when LexVisual's RaycastType = Custom
 */
UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexVisualCustomRaycast :public UObject
{
	GENERATED_BODY()

protected:
	/**
	 * Called by LexVisual when do raycast hit test.
	 * @param	InVisual			The LexVisual object which call this Raycast function
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	OutHitPoint				Hit point position in this UI's local space
	 * @param	OutHitNormal			Hit point normal in this UI's local space
	 * @return	true if hit this UI object
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "LGUI", meta = (DisplayName = "Raycast"))
		bool ReceiveRaycast(const ULexVisual* InVisual, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)const;
	/**
	 * Get pixel value at hit point.
	 * Only support UI element type which can read pixel value from texture:
	 *		1. UISprite which use LGUIStaticSpriteAtlasData can work perfectly.
	 *		2. UITexture can work with some specific setting.
	 *		3. UIText which use dynamic font can not work. (Currently all LGUI's built-in font is dynamic)
	 * Will fallback to Geometry if ui element not support this raycast type.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		static bool GetRaycastPixelFromUIBatchMeshVisual(const class ULexVisualBatchMesh* InVisual, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector2D& OutUV, FColor& OutPixel, FVector& OutHitPoint, FVector& OutHitNormal);
public:
	/**
	 * Called by LexVisual when do raycast hit test.
	 * @param	InVisual			The LexVisual object which call this Raycast function
	 * @param	InLocalSpaceRayStart	Ray start point in this UI's local space
	 * @param	InLocalSpaceRayEnd		Ray end point in this UI's local space
	 * @param	OutHitPoint				Hit point position in this UI's local space
	 * @param	OutHitNormal			Hit point normal in this UI's local space
	 * @return	true if hit this UI object
	 */
	virtual bool Raycast(const ULexVisual* InVisual, const FVector& InLocalSpaceRayStart, const FVector& InLocalSpaceRayEnd, FVector& OutHitPoint, FVector& OutHitNormal)const;
};

UENUM(BlueprintType, Category = LGUI)
enum class ELexVisualType :uint8
{
	None,
	BatchMesh,
	PostProcess,
	DirectMesh,
};

// Defines how mouse or ray hit on LexVisual
UENUM(BlueprintType, Category = LGUI)
enum class ELexVisualRaycastType :uint8
{
	/** Hit on rect range */
	Rect = 0,
	/** Hit on actual triangle mesh */
	Mesh = 1,
	/**
	 * Hit on main texture's pixel, if the pixel is not transparent then hit test success.
	 * Only support UI element type which can read pixel value from texture:
	 *		1. UISprite which use LexUIStaticSpriteAtlasData can work perfectly.
	 *		2. UITexture can work with some specific setting.
	 *		3. UIText which use dynamic font can NOT work. (Currently all LGUI's built-in font is dynamic)
	 * Will fallback to Mesh if ui element not support this raycast type.
	 */
	VisiblePixel = 3,
	/** Use a user defined ULexVisualCustomRaycast to process the raycast hit. */
	Custom = 2,
};

/** Base class of UI element that can be rendered by LexCanvas */
UCLASS(Blueprintable, BlueprintType, Abstract, DefaultToInstanced)
class LGUI_API ULexVisual : public ULexWidgetSubObjectBehaviour
{
	GENERATED_BODY()

public:	
	ULexVisual(const FObjectInitializer& ObjectInitializer);
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
protected:
	friend class FLexVisualCustomization;
	friend class ULexWidget;
	virtual void PostReinitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	ELexVisualType VisualType = ELexVisualType::None;

	/**
	 * Render color of UI element.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI", Getter, Setter, BlueprintReadWrite)
	FColor Color = FColor::White;
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
	bool bRaycastTarget = true;
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast", meta=(EditCondition=bRaycastTarget))
	ELexVisualRaycastType RaycastType = ELexVisualRaycastType::Rect;
	/** Custom raycast object to handle raycast behaviour when LexUI do raycast hit test. Only valid if RaycastType is Custom. */
	UPROPERTY(EditAnywhere, Instanced, Category = "LGUI-Raycast")
	TObjectPtr<ULexVisualCustomRaycast> CustomRaycastObject;
	/** Pixel's alpha value threshold, if hit a pixel which alpha value is less than this value, then hit test return false. */
	UPROPERTY(EditAnywhere, Category = "LGUI-Raycast")
	float VisiblePixelThreshold = 0.1f;

	virtual bool LineTraceUIRect(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const;
	virtual bool LineTraceUIGeometry(FLexUIGeometry* InGeo, FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const;
	virtual bool LineTraceUICustom(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const;
	
	void UpdateGeometryWidgetPropertyData(TArray<struct FLexUIMeshVertex>& InVertices, int InValidNumVertices, int InDataStartPosition);
public:
	static const FName GetPropertyName_Color()
	{
		return GET_MEMBER_NAME_CHECKED(ULexVisual, Color);
	}
	
	/** get visual type */
	UFUNCTION(BlueprintCallable, Category = LGUI)
	ELexVisualType GetVisualType()const { return VisualType; }

	UFUNCTION()
	FColor GetColor() const { return Color; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	float GetAlpha() const { return FLexUIUtils::ByteToFloat01(Color.A); }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	bool GetRaycastTarget()const{return bRaycastTarget;}
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	ELexVisualRaycastType GetRaycastType()const { return RaycastType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		ULexVisualCustomRaycast* GetCustomRaycastObject()const { return CustomRaycastObject; }

	UFUNCTION()
	void SetColor(FColor Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetAlpha(float Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetRaycastTarget(bool Value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetRaycastType(ELexVisualRaycastType Value) { RaycastType = Value; }
	/** Set custom raycast object to handle raycast behaviour, only valid if RaycastType is Custom */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	void SetCustomRaycastObject(ULexVisualCustomRaycast* Value);

	uint8 GetFinalAlpha()const;
	/** get final alpha, calculated with inherited RenderOpacity */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetFinalAlpha01()const;
	/** get final color, calculated with inherited RenderOpacity */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		FColor GetFinalColor()const;

	UFUNCTION(BlueprintCallable, Category = "LexUI")
	virtual bool LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const;

	int GetClipDataStartPosition()const;
	UTexture* GetClipDataTexture()const;

	virtual void OnPixelSnappingChanged(){};
	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange){};
	virtual void OnTransformChanged(bool InPositionChanged, bool InScaleChanged);
	virtual void OnRenderCanvasChanged(ULexCanvas* InOldCanvas, ULexCanvas* InNewCanvas);
	
	void MarkColorDirty();
	void CheckClipDataStartPosition();
	virtual void MarkAllDirty();
	
	/** Called by LexCanvas when begin to collect geometry for render */
	virtual void UpdateGeometry() {};
	/** Called by LexCanvas after create MaterialInstanceDynamic for this object or it's draw-call */
	virtual void OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) {};

	/** will this UI element affected by canvas's pixel perfect property? */
	virtual bool GetShouldAffectByPixelSnapping()const { return true; };
	/** return bounds min max point in self local space, for LexCanvas to tell if geometry overlap with each other. */
	virtual void GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const;
	/** editor only, return 3d bounds in self local space */
	virtual void GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const;
	
	/**
	 * The preferred width this layout element should be allocated if there is sufficient space.
	 * Can be -1 to ignore it.
	 */
	virtual float GetPreferredWidth()const{return -1;}
	/**
	 * The preferred height this layout element should be allocated if there is sufficient space.
	 * Can be -1 to ignore it.
	 */
	virtual float GetPreferredHeight()const{return -1;}

	static int WidgetPropertyDataLength;

	void SetWidgetPropertyDataStartPosition(int InPosition);
	bool IsRegisteredToCanvas()const{return WidgetPropertyDataStartPosition != INDEX_NONE;}
	int GetWidgetPropertyDataStartPosition()const{return WidgetPropertyDataStartPosition;}
protected:
	uint8 bColorChanged : 1;
	uint8 bTransformChanged : 1;
	uint8 bClipDataPositionChanged : 1;
	uint8 bWidgetPropertyDataStartPositionChanged : 1;
	uint8 bWidgetPropertyDataFontMarkDirty : 1;
	int ClipDataStartPosition = 0;
	int WidgetPropertyDataStartPosition = INDEX_NONE;

	void FillWidgetPropertyDataForMaterial(bool bNeedSize, bool bNeedCenterPosition)const;
	void FillWidgetPropertyDataForMaterial_ClipDataCoordinate(class ULexUIDataAsTexture* DataAsTexture)const;
	// Fill initial mark data, only do this when first create widget property data or when render canvas changed
	void FillWidgetPropertyDataForMaterial_InitialMark(class ULexUIDataAsTexture* DataAsTexture, uint8 FontMark)const;
public:
#pragma region TweenAnimation
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* ColorTo(FColor endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* ColorFrom(FColor startValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "delay,ease"), Category = "LTweenLGUI")
		ULTweener* AlphaFrom(float startValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase ease = ELTweenEase::OutCubic);
#pragma endregion
};
