// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcessRenderable.h"
#include "LGUIDelegateHandleWrapper.h"
#include "UIFrameCapture.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FUIFrameCapture_OnFrameReady_DynamicDelegate, UTextureRenderTarget2D*, CapturedFrame);
DECLARE_DELEGATE_OneParam(FUIFrameCapture_OnFrameReady_Delegate, UTextureRenderTarget2D*);

/**
 * UI element that can capture screen as texture for further use.
 * Use it in ScreenSpace or WorldSpace-LGUIRenderer.
 * If android OpenGL ES3.1, need to enable "ProjectSettings/Platforms/Android/Build/Support Backbuffer Sampling on OpenGL".
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIFrameCapture : public UUIPostProcessRenderable
{
	GENERATED_BODY()

public:	
	UUIFrameCapture(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	/** Capture full screen or just this UI's rect area. */
	UPROPERTY(EditAnywhere, Category = "LGUI")
	bool bCaptureFullScreen = true;
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI")
	TObjectPtr<UTextureRenderTarget2D> CapturedFrame;

	DECLARE_MULTICAST_DELEGATE_OneParam(FUIFrameCapture_OnFrameReady_MulticastDelegate, UTextureRenderTarget2D*);
	FUIFrameCapture_OnFrameReady_MulticastDelegate OnFrameReady;
	bool bIsFrameReady = false;
	bool bPendingCapture = false;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
	UTextureRenderTarget2D* GetCapturedFrame()const { return CapturedFrame; }
	/**
	 * Do a one shot capture, register the delegate to get result.
	 * @param InDelegate Called when capture finish.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void DoCapture(const FUIFrameCapture_OnFrameReady_DynamicDelegate& InDelegate);
	void DoCapture(const FUIFrameCapture_OnFrameReady_Delegate& InDelegate);
	void DoCapture(const TFunction<void(UTextureRenderTarget2D*)>& InFunction);

	virtual TSharedPtr<FUIPostProcessRenderProxy> GetRenderProxy()override;
	virtual void MarkAllDirty()override;
protected:
	void MarkOneFrameCapture();
	virtual void SendRegionVertexDataToRenderProxy()override;
	void SendOthersDataToRenderProxy();
	void UpdateRenderTarget();
};
