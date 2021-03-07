// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIBaseRenderable.h"
#include "UIPostProcess.generated.h"

/** 
 * UI element that can add post processing effect
 * Only valid on ScreenSpaceUI
 */
UCLASS(Abstract, NotBlueprintable, Experimental)
class LGUI_API UUIPostProcess : public UUIBaseRenderable
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
	virtual void ApplyUIActiveState() override;
	TSharedPtr<UIGeometry> geometry = nullptr;
	virtual void UpdateGeometry(const bool& parentLayoutChanged)override final;

	virtual void WidthChanged()override;
	virtual void HeightChanged()override;
	virtual void PivotChanged()override;

	virtual void UpdateBasePrevData()override;
	virtual void UpdateCachedData()override;
	virtual void UpdateCachedDataBeforeGeometry()override;
	virtual void MarkAllDirtyRecursive()override;
public:
	void MarkVertexPositionDirty();
	void MarkUVDirty();
public:
	/** game thread function. setup anything you need on game thread before rendering, like create RenderTargetTexture */
	virtual void OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView) {};
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenImage				the full screen render image
	 * @param	ViewProjectionMatrix	for vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList, 
		FTextureRHIRef ScreenImage, 
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix
	) {};
private:
	/** local vertex position changed */
	uint8 bLocalVertexPositionChanged : 1;
	/** vertex's uv change */
	uint8 bUVChanged : 1;

	uint8 cacheForThisUpdate_LocalVertexPositionChanged : 1, cacheForThisUpdate_UVChanged : 1;
protected:
	/** create ui geometry */
	virtual void OnCreateGeometry()PURE_VIRTUAL(UUIRenderable::OnCreateGeometry, );
	/** update ui geometry */
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)PURE_VIRTUAL(UUIRenderable::OnUpdateGeometry, );
private:
	void CreateGeometry();
};
