// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "Camera/CameraTypes.h"
#include "LGUICanvasScaler.generated.h"

UENUM(BlueprintType, Category = LGUI)
enum class ELGUICanvasScaleMode:uint8
{
	/** 1 unit is 1 pixel render in screen*/
	ConstantPixelSize,
	/** scale UI with reference resolution and screen resolution*/
	ScaleWithScreenSize,
	/**
	 * Assign CustomScale parameter to use a custom class calculate resolution and scale.
	 */
	Custom,
};

#ifndef LGUIScaleMode
#define LGUIScaleMode UE_DEPRECATED_MACRO(5.0, "LGUIScaleMode has been renamed to ELGUICanvasScaleMode") ELGUICanvasScaleMode
#endif

UENUM(BlueprintType, Category = LGUI)
enum class ELGUICanvasScreenMatchMode :uint8
{
	/** Use "MatchFromWidthToHeight" and "ReferenceResolution" properties to control size and scale UI*/
	MatchWidthOrHeight,
	/** If viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then expand size and scale UI*/
	Expand,
	/** if viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then shrink size and scale UI*/
	Shrink,
};

#ifndef LGUIScreenMatchMode
#define LGUIScreenMatchMode UE_DEPRECATED_MACRO(5.0, "LGUIScreenMatchMode has been renamed to ELGUICanvasScreenMatchMode") ELGUICanvasScreenMatchMode
#endif

UCLASS(BlueprintType, Blueprintable, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULGUICanvasScalerCustomScale: public UObject
{
	GENERATED_BODY()
public:
	/** Initialize, called when LGUICanvasScaler Awake. */
	virtual void Init(class ULGUICanvasScaler* InCanvasScaler);
	/** Called when LGUICanvasScaler calculate viewport size and scale. */
	virtual void CalculateSizeAndScale(class ULGUICanvasScaler* InCanvasScaler, const FIntPoint& InViewportSize, FIntPoint& OutLGUICanvasSize, float& OutScale);
protected:
	/** Initialize, called when LGUICanvasScaler Awake. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CalculateSizeAndScale"), Category = "LGUI")
	void ReceiveInit(class ULGUICanvasScaler* InCanvasScaler);
	/** Called when LGUICanvasScaler calculate viewport size and scale. */
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "CalculateSizeAndScale"), Category = "LGUI")
	void ReceiveCalculateSizeAndScale(class ULGUICanvasScaler* InCanvasScaler, const FIntPoint& InViewportSize, FIntPoint& OutLGUICanvasSize, float& OutScale);
};

/**
 * Put this on a actor with LGUICanvas component. Use this to scale UI element to adapt different screen resolution.
 * One hierarchy should only have one UICanvasScalar.
 */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUICanvasScaler :public ULGUILifeCycleBehaviour
{
	GENERATED_BODY()

public:
	ULGUICanvasScaler();

protected:
	virtual void Awake()override;
	virtual void OnEnable() override;
	virtual void OnDisable()override;

	void OnRegister();
	void OnUnregister();
#if WITH_EDITOR
	FDelegateHandle EditorTickDelegateHandle;
	FDelegateHandle EditorViewportIndexAndKeyChangeDelegateHandle;
	FDelegateHandle LGUIPreview_ViewportIndexChangeDelegateHandle;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void DrawVirtualCamera();
	void DrawViewportArea();
	void OnEditorTick(float DeltaTime);
	void OnEditorViewportIndexAndKeyChange();
	void OnPreviewSetting_EditorPreviewViewportIndexChange();
#endif
	void OnViewportParameterChanged();
	void CheckAndApplyViewportParameter();
	void OnViewportResized(FViewport*, uint32);
	FDelegateHandle _ViewportResizeDelegateHandle;

	friend class FUICanvasScalerCustomization;

	/** Virtual Camera Projection Type.*/
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay, meta = (DisplayName = "Projection Type"))
		TEnumAsByte<ECameraProjectionMode::Type> ProjectionType = ECameraProjectionMode::Perspective;
	/** Virtual Camera field of view (in degrees). */
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay, meta = (DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float FOVAngle = 90;
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		float NearClipPlane = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		float FarClipPlane = 10000;
	
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUICanvasScaleMode UIScaleMode = ELGUICanvasScaleMode::ConstantPixelSize;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D ReferenceResolution = FVector2D(1280, 720);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Match"))
		float MatchFromWidthToHeight = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUICanvasScreenMatchMode ScreenMatchMode = ELGUICanvasScreenMatchMode::MatchWidthOrHeight;

	/**
	 * Use this to do custom scale. Only valid if UIScaleMode = Custom.
	 * Will fallback to "ConstantPixelSize" if not assign this value.
	 */
	UPROPERTY(EditAnywhere, Instanced, Category = LGUI)
		TObjectPtr<ULGUICanvasScalerCustomScale> CustomScale;

	bool CheckCanvas();
	UPROPERTY(Transient) TObjectPtr<class ULGUICanvas> Canvas = nullptr;
	void SetCanvasProperties();

	/** Current viewport size*/
	FIntPoint ViewportSize = FIntPoint(2, 2);
public:

	UFUNCTION(BlueprintCallable, Category = LGUI)
		TEnumAsByte<ECameraProjectionMode::Type> GetProjectionType()const { return ProjectionType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetFovAngle()const { return FOVAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetNearClipPlane()const { return NearClipPlane; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetFarClipPlane()const { return FarClipPlane; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFovAngle(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetNearClipPlane(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFarClipPlane(float value);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasScaleMode GetUIScaleMode() { return UIScaleMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetReferenceResolution() { return ReferenceResolution; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetMatchFromWidthToHeight() { return MatchFromWidthToHeight; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUICanvasScreenMatchMode GetScreenMatchMode() { return ScreenMatchMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ULGUICanvasScalerCustomScale* GetCustomScale()const { return CustomScale; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIScaleMode(ELGUICanvasScaleMode value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetReferenceResolution(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetMatchFromWidthToHeight(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetScreenMatchMode(ELGUICanvasScreenMatchMode value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetCustomScale(ULGUICanvasScalerCustomScale* value);

	/** By default, LGUICanvasScaler only update when needed(eg. viewport size change). Use this function to force update. */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void ForceUpdate();

	/**
	 * Convert position from viewport to LGUICanvas space.
	 * @param position The point's pixel position on viewport.
	 * @return Position in LGUICanvas space, left bottom is zero point.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D ConvertPositionFromViewportToLGUICanvas(const FVector2D& position)const;
	/**
	 * Convert position from LGUICanvas space to viewport.
	 * @param position The point's position in LGUICanvas space.
	 * @return Position in viewport, pixel unit, left top is zero point.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D ConvertPositionFromLGUICanvasToViewport(const FVector2D& position)const;
	/**
	 * NOTE!!! This is only for screen-space-UI, don't use this for convert world space position!!!
	 * Project 3D screen-space-UI element's position to 2D screen-space-UI.
	 * @param	Position3D	GetWorldLocation from the UI element (world location).
	 * @param	OutPosition2D	2D Position in screen-space, left bottom is zero point.
	 * @param	bool	convert may fail.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool Project3DToScreen(const FVector& Position3D, FVector2D& OutPosition2D)const;
};