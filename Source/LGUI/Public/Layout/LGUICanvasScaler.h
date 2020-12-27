// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "LGUICanvasScaler.generated.h"

UENUM(BlueprintType)
enum class LGUIScaleMode:uint8
{
	//1 unit is 1 pixel render in screen
	ConstantPixelSize,
	//scale UI with reference resolution and screen resolution
	ScaleWithScreenSize,
};
UENUM(BlueprintType)
enum class LGUIScreenMatchMode :uint8
{
	//user "MatchFromWidthToHeight" and "ReferenceResolution" properties to control size and scale UI
	MatchWidthOrHeight,
	//if viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then expand size and scale UI
	Expand,
	//if viewport's aspect ratio not match "ReferenceResolution"'s aspect ratio, then shrink size and scale UI
	Shrink,
};
//put this on a actor with LGUICanvas component. use this to scale UI element. one hierarchy should only have one UICanvasScalar.
//tweak parameters to make your UI adapt to different screen resolution.
//for 4.26: ScreenSpaceUI's root UIItem will be set to world origin, otherwise rect clip will not render correctly. still working on it.
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API ULGUICanvasScaler :public UActorComponent
{
	GENERATED_BODY()

public:
	ULGUICanvasScaler();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

#if WITH_EDITOR
	FDelegateHandle EditorTickDelegateHandle;
	FDelegateHandle EditorViewportIndexAndKeyChangeDelegateHandle;
	FDelegateHandle LGUIPreview_ViewportIndexChangeDelegateHandle;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void OnRegister();
	void OnUnregister();
	void DrawVirtualCamera();
	void OnEditorTick(float DeltaTime);
	void OnEditorViewportIndexAndKeyChange();
	void OnPreviewSetting_EditorPreviewViewportIndexChange();
#endif
	void OnViewportParameterChanged();
	void CheckAndApplyViewportParameter();
	void OnViewportResized(FViewport*, uint32);
	FDelegateHandle _ViewportResizeDelegateHandle;

	friend class FUICanvasScalerCustomization;

	//Virtual Camera Projection Type
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay, meta = (DisplayName = "Projection Type"))
		TEnumAsByte<ECameraProjectionMode::Type> ProjectionType = ECameraProjectionMode::Perspective;
	//Virtual Camera field of view (in degrees). */
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay, meta = (DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float FOVAngle = 90;
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		float NearClipPlane = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI", AdvancedDisplay)
		float FarClipPlane = 10000;
	
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIScaleMode UIScaleMode = LGUIScaleMode::ConstantPixelSize;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FVector2D ReferenceResolution = FVector2D(1280, 720);
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "1.0", DisplayName = "Match"))
		float MatchFromWidthToHeight = 0;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIScreenMatchMode ScreenMatchMode = LGUIScreenMatchMode::MatchWidthOrHeight;

	bool CheckCanvas();
	UPROPERTY(Transient) class ULGUICanvas* Canvas = nullptr;
	void SetCanvasProperties();

	FIntPoint ViewportSize = FIntPoint(2, 2);//current viewport size
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
		LGUIScaleMode GetUIScaleMode() { return UIScaleMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D GetReferenceResolution() { return ReferenceResolution; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetMatchFromWidthToHeight() { return MatchFromWidthToHeight; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		LGUIScreenMatchMode GetScreenMatchMode() { return ScreenMatchMode; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIScaleMode(LGUIScaleMode value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetReferenceResolution(FVector2D value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetMatchFromWidthToHeight(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetScreenMatchMode(LGUIScreenMatchMode value);

	/**
	 * Convert position from viewport to LGUICanvas space.
	 * @param position The point's pixel position on viewport.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D ConvertPositionFromViewportToLGUICanvas(const FVector2D& position)const;
	/**
	 * Convert position from LGUICanvas space to viewport.
	 * @param position The point's position in LGUICanvas space.
	 */
	UFUNCTION(BlueprintCallable, Category = LGUI)
		FVector2D ConvertPositionFromLGUICanvasToViewport(const FVector2D& position)const;
};