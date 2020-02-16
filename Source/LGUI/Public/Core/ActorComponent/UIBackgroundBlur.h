// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "UIBackgroundBlur.generated.h"

//UI element that can add blur effect on background renderred image, just like UMG's BackgroundBlur
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIBackgroundBlur : public UUIPostProcess
{
	GENERATED_BODY()

public:	
	UUIBackgroundBlur(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = "100.0"))
		float blurStrength = 0.0f;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBlurStrength() { return blurStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBlurStrength(float newValue);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual bool NeedTextureToCreateGeometry()override { return false; }

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UPROPERTY(Transient) UTextureRenderTarget2D* BlurEffectRenderTarget = nullptr;
	bool bNeedToResize = false;
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView);
	virtual void OnRenderPostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef ScreenImage, TShaderMap<FGlobalShaderType>* GlobalShaderMap, const FMatrix& ViewProjectionMatrix, FGraphicsPipelineStateInitializer& GraphicsPSOInit, const TFunction<void()>& DrawPrimitive)override;
};
