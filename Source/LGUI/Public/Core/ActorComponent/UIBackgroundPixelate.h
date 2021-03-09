// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIPostProcess.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "UIBackgroundPixelate.generated.h"

/** 
 * UI element that can make the background look pixelated
 * Use it on ScreenSpaceUI.
 * May have issue when MSAA is on.
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
	/** Pixelate effect strength. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (ClampMin = "0.0", ClampMax = 100.0f))
		float pixelateStrength = 10.0f;
	/** Will alpha affect pixelate strength? If true, then 0 alpha means 0 pixelate strength, and 1 alpha means full pixelate strength. */
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
	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	TArray<FLGUIPostProcessVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	void UpdateRegionVertex();
	FCriticalSection mutex;
	FORCEINLINE float GetStrengthInternal();
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView);
	virtual void OnRenderPostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef ScreenImage, TShaderMap<FGlobalShaderType>* GlobalShaderMap, const FMatrix& ViewProjectionMatrix)override;
};
