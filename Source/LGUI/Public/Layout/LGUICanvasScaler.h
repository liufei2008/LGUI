// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIItem.h"
#include "LGUICanvasScaler.generated.h"

UENUM(BlueprintType)
enum class LGUIScaleMode:uint8
{
	ScaleWithScreenWidth,
	ScaleWithScreenHeight,
	ConstantPixelSize,
};
//put this on a actor with LGUICanvas component. use this to scale UI element. one hierarchy should only have one UICanvasScalar
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
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void OnRegister();
	void OnUnregister();
	void DrawVirtualCamera();
	void OnEditorTick(float DeltaTime);
#endif
	void OnViewportParameterChanged();
	void CheckAndApplyViewportParameter();

	friend class FUICanvasScalerCustomization;

	//Virtual Camera Projection Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (DisplayName = "Projection Type"))
		TEnumAsByte<ECameraProjectionMode::Type> ProjectionType;
	//Virtual Camera field of view (in degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float FOVAngle = 90;
	//The desired width (in world units) of the orthographic view (ignored in Perspective mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float OrthoWidth = 100;
	
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIScaleMode UIScaleMode = LGUIScaleMode::ScaleWithScreenHeight;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredHeight = 1080;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredWidth = 1920;

	bool CheckCanvas();
	UPROPERTY(Transient) class ULGUICanvas* Canvas = nullptr;

	FIntPoint PrevViewportSize = FIntPoint(0, 0);//prev frame viewport size
public:

	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredWidth() { return PreferredWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredHeight() { return PreferredHeight; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		TEnumAsByte<ECameraProjectionMode::Type> GetProjectionType()const { return ProjectionType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetFovAngle()const { return FOVAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetOrthoWidth()const { return OrthoWidth; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredWidth(float InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredHeight(float InValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFovAngle(float value);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOrthoWidth(float value);
};