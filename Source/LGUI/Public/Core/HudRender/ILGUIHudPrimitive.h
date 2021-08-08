// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "Core/HudRender/LGUIRenderer.h"

class FLGUIHudRenderer;
class UUIPostProcessRenderable;

class ILGUIHudPrimitive
{
public:
	virtual ~ILGUIHudPrimitive() {}

	virtual bool CanRender() const = 0;
	virtual int GetRenderPriority() const = 0;

	//begin mesh interface
	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) = 0;
	virtual FRHIBuffer* GetVertexBufferRHI() = 0;
	virtual uint32 GetNumVerts() = 0;
	//end mesh interface

	//begin post process interface
	virtual bool GetIsPostProcess() = 0;
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenImage				the full screen render image
	 * @param	ViewProjectionMatrix	for vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenTargetTexture,
		FTextureRHIRef ScreenTargetTexture,
		FTextureRHIRef ScreenTargetResolveTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld
	) {};
	//end post process interface
};
