// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UIBackgroundBlur.generated.h"

/** 
 * UI element that can add blur effect on background renderred image, just like UMG's BackgroundBlur.
 */
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
#define MAX_BlurStrength 100.0f
#define INV_MAX_BlurStrength 0.01f
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float blurStrength = 10.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool applyAlphaToBlur = true;
	/** No need to change this because default value can give you good result */
	UPROPERTY(EditAnywhere, Category = "LGUI") 
		int maxDownSampleLevel = 6;
	/** use strengthTexture's red channel to control blur strength, 0-no blur, 1-full blur */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTexture2D* strengthTexture;
	/** use maskTexture's red channel to mask out blur result */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UTexture2D* maskTexture;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetBlurStrength() const { return blurStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		int GetMaxDownSampleLevel() const { return maxDownSampleLevel; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetApplyAlphaToBlur()const { return applyAlphaToBlur; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture2D* GetStrengthTexture()const { return strengthTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UTexture2D* GetMaskTexture()const { return maskTexture; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetBlurStrength(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaxDownSampleLevel(int newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetApplyAlphaToBlur(bool newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStrengthTexture(UTexture2D* newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetMaskTexture(UTexture2D* newValue);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual bool NeedTextureToCreateGeometry()override { return false; }

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UPROPERTY(Transient) UTextureRenderTarget2D* blurEffectRenderTarget1 = nullptr;
	UPROPERTY(Transient) UTextureRenderTarget2D* blurEffectRenderTarget2 = nullptr;
	float inv_SampleLevelInterval = 1.0f;
	FVector2D inv_TextureSize;
	TArray<FLGUIPostProcessVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	void UpdateRegionVertex();
	FCriticalSection mutex;
	FORCEINLINE float GetBlurStrengthInternal();
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView);
	virtual void OnRenderPostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef ScreenImage, FGlobalShaderMap* GlobalShaderMap, const FMatrix& ViewProjectionMatrix, const TFunction<void()>& DrawPrimitive)override;
};
