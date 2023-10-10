// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
#include "Core/LGUISpriteData_BaseObject.h"
#include "Core/LGUIRender/ILGUIRendererPrimitive.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "RHIStaticStates.h"

class ULGUICanvas;
class UUIPostProcessRenderable;
enum class ELGUICanvasClipType :uint8;
enum class EUIPostProcessMaskTextureType :uint8;

/**
 * UIPostProcessRenderProxy is an render agent for UIPostProcessRenderable in render thread, act as a SceneProxy.
 */
class LGUI_API FUIPostProcessRenderProxy
{
public:
	FUIPostProcessRenderProxy();
	virtual~FUIPostProcessRenderProxy()
	{
		
	}
private:
	TWeakPtr<FLGUIRenderer, ESPMode::ThreadSafe> LGUIRenderer;
	bool bIsVisible = true;
	bool bIsWorld = false;//is world space or screen space
	ULGUICanvas* RenderCanvasPtr = nullptr;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);
	void SetVisibility(bool value);

	virtual bool CanRender() const { return bIsVisible; };
	virtual bool PostProcessRequireOriginScreenColorTexture()const = 0;
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenTargetTexture				The full screen render target
	 * @param	OriginScreenColorTexture		Origin screen color texture. PostProcessNeedOriginScreenColorTexture must return true for this to work.
	 * @param	ViewProjectionMatrix			For vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FLGUIRenderer* Renderer,
		FTextureRHIRef OriginScreenColorTexture,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix44f& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		float DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	) = 0;
private:
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
		, const FMinimalSceneTextures& SceneTextures
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
