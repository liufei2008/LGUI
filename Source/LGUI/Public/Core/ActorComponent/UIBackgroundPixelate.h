﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UIBackgroundPixelate.generated.h"

/** 
 * UI element that can make the background look pixelated
 */
UCLASS(ClassGroup = (LGUI), NotBlueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIBackgroundPixelate : public UUIPostProcess
{
	GENERATED_BODY()

public:	
	UUIBackgroundPixelate(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
#define MAX_PixelateStrength 100.0f
#define INV_MAX_PixelateStrength 0.01f
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float pixelateStrength = 10.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool applyAlphaToStrength = true;
public:
	UFUNCTION(BlueprintCallable, Category="LGUI")
		float GetPixelateStrength() const { return pixelateStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		bool GetApplyAlphaToStrength()const { return applyAlphaToStrength; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetPixelateStrength(float newValue);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetApplyAlphaToStrength(bool newValue);
protected:
	virtual void OnBeforeCreateOrUpdateGeometry()override {}
	virtual bool NeedTextureToCreateGeometry()override { return false; }

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UPROPERTY(Transient) UTextureRenderTarget2D* helperRenderTarget = nullptr;
	TArray<FLGUIPostProcessVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	void UpdateRegionVertex();
	FCriticalSection mutex;
	FORCEINLINE float GetStrengthInternal();
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView);
	virtual void OnRenderPostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef ScreenImage, FGlobalShaderMap* GlobalShaderMap, const FMatrix& ViewProjectionMatrix, const TFunction<void()>& DrawPrimitive)override;
};
