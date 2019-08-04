// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/Actor/UIPanelActor.h"
#include "Core/ActorComponent/UIPanel.h"
#include "Core/ActorComponent/UIItem.h"
#include "UIRoot.generated.h"

UENUM(BlueprintType)
enum class LGUISnapMode :uint8
{
	SnapToViewTargetCamera,
	SnapToSceneCapture,
	//snap to nothing
	WorldSpace,
};
UENUM(BlueprintType)
enum class LGUIScaleMode:uint8
{
	ScaleWithScreenWidth,
	ScaleWithScreenHeight,
	ConstantPixelSize,
};
//put this on UIPanel actor. use this to scale and snap UI element. one hierarchy should only have one UIRoot
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent), Blueprintable)
class LGUI_API UUIRoot :public UActorComponent
{
	GENERATED_BODY()

public:
	UUIRoot();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;

#if WITH_EDITOR
	FDelegateHandle EditorTickDelegateHandle;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void OnEditorTick(float DeltaTime);
	void EditorApplyValue();
	void OnRegister();
	void OnUnregister();
#endif
	void RuntimeCheckAndApplyValue();

	friend class FUIRootCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUISnapMode UISnapMode = LGUISnapMode::WorldSpace;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIScaleMode UIScaleMode = LGUIScaleMode::ScaleWithScreenHeight;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ASceneCapture2D* SceneCapture;
	//Automatic change SceneCapture's RenderTarget size to viewport size
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool SnapRenderTargetToViewportSize = true;
	//This can avoid half-pixel render. Usually use for overlay UI.
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool SnapPixel = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredHeight = 1080;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredWidth = 1920;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float DistanceToCamera = 30.0f;

protected:
	//This will override all children panel's render material with OverrideMaterials when begin play
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseOverrideMaterials = false;
	//If material slot is empty, then that material will not be replaced
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UMaterialInterface* OverrideMaterials[3];
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UIOnlyOwnerSee = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UIOwnerNoSee = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool UseFirstPawnAsUIOwner;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		AActor* UIOwner;
private:
	void OnViewportParameterChanged(FIntPoint viewportSize, float fov, bool isOrthographic, float orthographicWidth);

	UPROPERTY(Transient) APlayerCameraManager* CameraManager = nullptr;
	FIntPoint PrevViewportSize = FIntPoint(0,0);//prev frame viewport size
	bool PrevIsOrthographic = false;
	float PrevOrthographicWidth = 100;
	float PrevFov = 45;
	bool NeedUpdate = false;

	bool CheckUIPanel();
	UPROPERTY(Transient) UUIPanel* RootUIPanel = nullptr;

	UPROPERTY(Transient)TArray<UUIPanel*> PanelsBelongToThisUIRoot;
	void ApplyToPanelDefaultMaterials();
public:
	FORCEINLINE UUIPanel* GetRootUIPanel() { return RootUIPanel; }

	void AddPanel(UUIPanel* InPanel);
	void RemovePanel(UUIPanel* InPanel);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		LGUISnapMode GetUISnapMode() { return UISnapMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		ASceneCapture2D* GetSceneCapture() { return SceneCapture; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredWidth() { return PreferredWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredHeight() { return PreferredHeight; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredWidth(float InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredHeight(float InValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIOnlyOwnerSee(bool InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIOwnerNoSee(bool InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUIOwner(AActor* InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetUseOverrideMaterials(bool InValue);

	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetUIOnlyOwnerSee() { return UIOnlyOwnerSee; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetUIOwnerNoSee() { return UIOwnerNoSee; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		AActor* GetUIOwner() { return UIOwner; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		bool GetUseOverrideMaterials() { return UseOverrideMaterials; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TArray<UUIPanel*> GetPanelsBelongToThis() { return PanelsBelongToThisUIRoot; }

	FORCEINLINE bool ShouldSnapPixel() { return UISnapMode != LGUISnapMode::WorldSpace && SnapPixel; }

	//called when a UIPanel attach to this UIPanel as a child
	UFUNCTION(BlueprintImplementableEvent, Category = LGUI)void OnUIPanelAttached(UUIPanel* InPanel);
	UFUNCTION(BlueprintImplementableEvent, Category = LGUI)void OnUIPanelDetached(UUIPanel* InPanel);

	//helper callback function for editor simulation
	UFUNCTION(BlueprintImplementableEvent, Category = LGUI)void EditorSwitchSimulation(bool InIsSimulation);
#if WITH_EDITOR
	FDelegateHandle EditorSwitchSimulationDelegateHandle;
#endif
};