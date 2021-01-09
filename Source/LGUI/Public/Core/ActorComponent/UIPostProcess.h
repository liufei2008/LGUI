// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIRenderable.h"
#include "UIPostProcess.generated.h"

/** 
 * UI element that can add post processing effect
 * Only valid on ScreenSpaceUI
 */
UCLASS(Abstract, NotBlueprintable, Experimental)
class LGUI_API UUIPostProcess : public UUIRenderable
{
	GENERATED_BODY()

public:	
	UUIPostProcess(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	/** game thread function. setup anything you need on game thread before rendering, like create RenderTargetTexture */
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView) {};
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenImage				the full screen render image
	 * @param	ViewProjectionMatrix	for vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 * @param	DrawPrimitive			this is a function to draw the geometry that you defined in OnCreateGeometry/OnUpdateGeometry
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList, 
		FTexture2DRHIRef ScreenImage, 
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		const TFunction<void()>& DrawPrimitive
	) {};
};
