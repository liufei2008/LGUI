// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UIBackgroundBlur.generated.h"

//UI element that can add blur effect on background renderred image, just like UMG's BackgroundBlur.
//Known issue: in UE 4.24 packaged game, the blurred image is slightly lagged behide current screen image, still working on it.
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent), Experimental)
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
#define MAX_BlurStrength 100.0f
#define INV_MAX_BlurStrength 0.01f
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float blurStrength = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool applyAlphaToBlur = true;
	//No need to change this because default value can give you good result
	UPROPERTY(EditAnywhere, Category = "LGUI") 
		int maxDownSampleLevel = 6;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBlurStrength() { return blurStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetMaxDownSampleLevel() { return maxDownSampleLevel; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBlurStrength(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaxDownSampleLevel(int newValue);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual bool NeedTextureToCreateGeometry()override { return false; }

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UPROPERTY(Transient) UTextureRenderTarget2D* blurEffectRenderTarget1 = nullptr;
	UPROPERTY(Transient) UTextureRenderTarget2D* blurEffectRenderTarget2 = nullptr;
	float inv_SampleLevelInterval = 1.0f;
	FVector2D inv_TextureSize;
	TArray<FLGUIPostProcessVertex> copyRegionVertexArray;
	FCriticalSection mutex;
	FORCEINLINE float GetBlurStrengthInternal();
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView);
	virtual void OnRenderPostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef ScreenImage, TShaderMap<FGlobalShaderType>* GlobalShaderMap, const FMatrix& ViewProjectionMatrix, const TFunction<void()>& DrawPrimitive)override;
};
