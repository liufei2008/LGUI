// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "LexUIRender/ILexUIRendererPrimitive.h"
#include "LexUIRender/LexUIPostProcessVertex.h"
#include "RHIStaticStates.h"
#include "SceneTextures.h"

class ULexCanvas;
class ULexVisualPostProcess;

/**
 * LexVisualPostProcessRenderProxy is a render-agent for LexVisualPostProcess in render thread, just like a SceneProxy for PrimitiveComponent.
 */
class LGUI_API FLexVisualPostProcessRenderProxy
{
public:
	FLexVisualPostProcessRenderProxy();
	virtual~FLexVisualPostProcessRenderProxy()
	{
		
	}
private:
	TWeakPtr<FLexUIRenderer, ESPMode::ThreadSafe> LexRenderer;
	bool bIsWorld = false;//is world space or screen space
public:
	virtual bool CanRender() const = 0;
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenTargetTexture				The full screen render target
	 * @param	ViewProjectionMatrix			For vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FLexUIRenderer* Renderer,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix44f& ViewProjectionMatrix,
		bool bIsWorldSpace,
		bool bIsRenderTarget,
		float BlendDepthForWorld,
		int DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	) = 0;
public:
	FTexture2DDynamicResource* ClipDataTexture = nullptr;
	
	FMatrix44f ObjectToWorldMatrix = FMatrix44f::Identity;
	TArray<FLexUIPostProcessCopyMeshRegionVertex> RenderScreenToMeshRegionVertexArray;
	TArray<FLexUIPostProcessVertex> RenderMeshRegionToScreenVertexArray;
	FVector2f RectSize;
	FTexture2DResource* MaskTexture = nullptr;
	FBox BoundingBox;
	//output target
	FTextureRenderTargetResource* RenderTargetResource = nullptr;

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
		, const FMatrix44f & ModelMatrix
		, bool IsWorldSpace
		, float BlendDepthForWorld
		, int DepthFadeForWorld
		, const FVector4f& DepthTextureScaleOffset
		, const FIntRect& ViewRect
		, FRHISamplerState* ResultTextureSamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	);
};

BEGIN_SHADER_PARAMETER_STRUCT(FLexUIRenderMeshOnScreenPSParameter, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTex)
	SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, MeshRegionTexture)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
