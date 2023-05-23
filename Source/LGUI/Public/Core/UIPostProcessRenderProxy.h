// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"
#include "Core/HudRender/LGUIHudVertex.h"

class ULGUICanvas;
class UUIPostProcessRenderable;
enum class ELGUICanvasClipType :uint8;
enum class EUIPostProcessMaskTextureType :uint8;

/**
 * UIPostProcessRenderProxy is an render agent for UIPostProcessRenderable in render thread, act as a SceneProxy.
 */
class LGUI_API FUIPostProcessRenderProxy: public ILGUIHudPrimitive
{
public:
	FUIPostProcessRenderProxy();
	virtual~FUIPostProcessRenderProxy()
	{
		
	}
private:
	TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	int32 RenderPriority = 0;
	bool bIsVisible = true;
	bool bIsWorld = false;//is world space or screen space
	ULGUICanvas* RenderCanvasPtr = nullptr;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);
	void SetVisibility(bool value);
	void AddToLGUIScreenSpaceRenderer(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIWorldSpaceRenderer(ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIScreenSpaceRenderer_RenderThread(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void AddToLGUIWorldSpaceRenderer_RenderThread(ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer);
	void RemoveFromLGUIRenderer();
	void RemoveFromLGUIRenderer_RenderThread(bool isWorld);

	//begin ILGUIHudPrimitive interface
	virtual bool CanRender() const override { return bIsVisible; };
	virtual int GetRenderPriority() const override { return RenderPriority; };
	virtual ULGUICanvas* GetCanvas()const override { return RenderCanvasPtr; }

	virtual void GetMeshElements(const FSceneViewFamily& ViewFamilyclass, class FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& Result) override {};

	virtual ELGUIHudPrimitiveType GetPrimitiveType()const override { return ELGUIHudPrimitiveType::PostProcess; };
	virtual FPrimitiveComponentId GetMeshPrimitiveComponentId() const override { return FPrimitiveComponentId(); };
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

	FVector4f rectClipOffsetAndSize;
	FVector4f rectClipFeather;

	FTexture2DResource* clipTexture;
	FVector4f textureClipOffsetAndSize;

	FMatrix44f objectToWorldMatrix = FMatrix44f::Identity;
	TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	FVector2f RectSize;
	FTexture2DResource* maskTexture = nullptr;
	EUIPostProcessMaskTextureType MaskTextureType;

	/**
	 * Use a mesh to render the MeshRegionTexture to ScreenTargetTexture
	 */
	void RenderMeshOnScreen_RenderThread(
		FRDGBuilder& GraphBuilder
		, FTextureRHIRef ScreenTargetTexture
		, FGlobalShaderMap* GlobalShaderMap
		, FTextureRHIRef MeshRegionTexture
		, const FMatrix44f & ModelViewProjectionMatrix
		, bool IsWorldSpace
		, float BlendDepthForWorld
		, float DepthFadeForWorld
		, const FVector4f& DepthTextureScaleOffset
		, const FIntRect& ViewRect
		, FRHISamplerState* ResultTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	);
};
