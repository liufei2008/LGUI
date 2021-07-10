// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIHudRenderer;
	int32 RenderPriority = 0;
	bool bIsVisible = true;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);
	void SetVisibility(bool value);
	void AddToHudRenderer(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIHudRenderer);
	void AddToHudRenderer_RenderThread(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIHudRenderer);
	void RemoveFromHudRenderer();
	void RemoveFromHudRenderer_RenderThread();

	//begin ILGUIHudPrimitive interface
	virtual bool CanRender() const { return bIsVisible; };
	virtual int GetRenderPriority() const { return RenderPriority; };

	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) { return FMeshBatch(); };
	virtual FRHIBuffer* GetVertexBufferRHI() { return nullptr; };
	virtual uint32 GetNumVerts() { return 0; };

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

	FMatrix modelViewProjectionMatrix = FMatrix::Identity;
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	FUIWidget widget;
	FTexture2DResource* maskTexture = nullptr;

	void RenderMeshOnScreen_RenderThread(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef ScreenTargetTexture
		, FGlobalShaderMap* GlobalShaderMap
		, FTextureRHIRef ResultTexture
		, FRHISamplerState* ResultTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	);
};
