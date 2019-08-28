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
enum class ELGUIRenderMode :uint8
{
	//render in screen space. 
	//Note that in this render mode, custom material is not supported
	ScreenSpaceOverlay,
	//render in world space, so post process effect will affect ui
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
	class UTextureRenderTarget2D* GetPreviewRenderTarget();
	void DrawVirtualCamera();
#endif
	void RuntimeCheckAndApplyValue();

	friend class FUIRootCustomization;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		ELGUIRenderMode RenderMode = ELGUIRenderMode::WorldSpace;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		LGUIScaleMode UIScaleMode = LGUIScaleMode::ScaleWithScreenHeight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (DisplayName = "Projection Type"))
		TEnumAsByte<ECameraProjectionMode::Type> ProjectionType;
	//Camera field of view (in degrees). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI", meta = (DisplayName = "Field of View", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0"))
		float FOVAngle = 90;
	/** The desired width (in world units) of the orthographic view (ignored in Perspective mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LGUI")
		float OrthoWidth = 100;
	//This can avoid half-pixel render
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool SnapPixel = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredHeight = 1080;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float PreferredWidth = 1920;
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
	void OnViewportParameterChanged();

	TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> ViewExtension;
	virtual void BeginDestroy();

	FIntPoint PrevViewportSize = FIntPoint(0,0);//prev frame viewport size
	float DistanceToCamera = 30.0f;
	int DrawcallSubmittedPanelCount = 0;//how many panels have submit drawcall
	bool NeedToRebuildAllDrawcall = true;

	bool CheckUIPanel();
	UPROPERTY(Transient) UUIPanel* RootUIPanel = nullptr;

	UPROPERTY(Transient)TArray<UUIPanel*> PanelsBelongToThisUIRoot;
	void ApplyToPanelDefaultMaterials();
	void BuildProjectionMatrix(FIntPoint ViewportSize, ECameraProjectionMode::Type ProjectionType, float FOV, float InOrthoWidth, FMatrix& ProjectionMatrix);
public:
	FORCEINLINE UUIPanel* GetRootUIPanel() { return RootUIPanel; }

	void AddPanel(UUIPanel* InPanel);
	void RemovePanel(UUIPanel* InPanel);
	FORCEINLINE TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> GetViewExtension();

	UFUNCTION(BlueprintCallable, Category = LGUI)
		ELGUIRenderMode GetRenderMode() { return RenderMode; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredWidth() { return PreferredWidth; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetPreferredHeight() { return PreferredHeight; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		TEnumAsByte<ECameraProjectionMode::Type> GetProjectionType() { return ProjectionType; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetFieldOfView() { return FOVAngle; }
	UFUNCTION(BlueprintCallable, Category = LGUI)
		float GetOrthoWith() { return OrthoWidth; }

	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredWidth(float InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPreferredHeight(float InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetProjectionType(TEnumAsByte<ECameraProjectionMode::Type> InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetFieldOfView(float InValue);
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetOrthoWidth(float InValue);

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
		TArray<UUIPanel*>& GetPanelsBelongToThis() { return PanelsBelongToThisUIRoot; }

	FORCEINLINE bool ShouldSnapPixel() { return RenderMode == ELGUIRenderMode::ScreenSpaceOverlay && SnapPixel; }

	//called when a UIPanel attach to this UIPanel as a child
	UFUNCTION(BlueprintImplementableEvent, Category = LGUI)void OnUIPanelAttached(UUIPanel* InPanel);
	UFUNCTION(BlueprintImplementableEvent, Category = LGUI)void OnUIPanelDetached(UUIPanel* InPanel);

	//sort panel by depth, so after collect drawcall, UI will render with right order
	FORCEINLINE void SortUIPanelOnDepth();

	FORCEINLINE FMatrix GetViewProjectionMatrix();
	FORCEINLINE FMatrix GetProjectionMatrix();
	FORCEINLINE FVector GetViewLocation();
	FORCEINLINE FMatrix GetViewRotationMatrix();
	FORCEINLINE FRotator GetViewRotator();
	FORCEINLINE FIntPoint GetViewportSize();
};