// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"

class UUIPostProcessRenderable;

/**
 * UIPostProcessRenderProxy is an render agent for UIPostProcessRenderable in render thread, act as a SceneProxy.
 */
class LGUI_API FUIPostProcessRenderProxy: public ILGUIHudPrimitive
{
public:
	FUIPostProcessRenderProxy()
	{
		
	}
	virtual~FUIPostProcessRenderProxy()
	{
		
	}
private:
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	int32 RenderPriority = 0;
	bool bIsVisible = true;
	bool bIsWorld = false;//is world space or screen space
	void* RenderCanvasPtr = nullptr;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);
	void SetVisibility(bool value);
	void AddToLGUIScreenSpaceRenderer(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIWorldSpaceRenderer(void* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIScreenSpaceRenderer_RenderThread(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIWorldSpaceRenderer_RenderThread(void* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void RemoveFromLGUIRenderer();
	void RemoveFromLGUIRenderer_RenderThread(bool isWorld);

	//begin ILGUIHudPrimitive interface
	virtual bool CanRender() const { return bIsVisible; };
	virtual int GetRenderPriority() const { return RenderPriority; };

	virtual void GetMeshElements(const FSceneViewFamily& ViewFamilyclass, class FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& Result) {};

	virtual bool GetIsPostProcess() { return true; };
	//end ILGUIHudPrimitive interface
private:
	void SetUITranslucentSortPriority_RenderThread(int value)
	{
		RenderPriority = value;
	}
	void SetVisibility_RenderThread(bool value)
	{
		bIsVisible = value;
	}

public:
	ELGUICanvasClipType clipType;

	FVector4 rectClipOffsetAndSize;
	FVector4 rectClipFeather;

	FTexture2DResource* clipTexture;
	FVector4 textureClipOffsetAndSize;

	FMatrix objectToWorldMatrix = FMatrix::Identity;
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	FVector2D RectSize;
	FTexture2DResource* maskTexture = nullptr;

	void RenderMeshOnScreen_RenderThread(
#if ENGINE_MAJOR_VERSION >= 5
		FRDGBuilder& GraphBuilder
#else
		FRHICommandListImmediate& RHICmdList
#endif
		, FTextureRHIRef ScreenTargetTexture
		, FGlobalShaderMap* GlobalShaderMap
		, FTextureRHIRef ResultTexture
		, const FMatrix & ModelViewProjectionMatrix
		, bool IsWorldSpace
		, float BlendDepthForWorld
		, const FVector4& DepthTextureScaleOffset
		, const FIntRect& ViewRect
		, FRHISamplerState* ResultTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	);
};
