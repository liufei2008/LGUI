// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneManagement.h"
#include "MeshBatch.h"
#include "RHIResources.h"
#include "GlobalShader.h"

class FLGUIHudRenderer;
class UUIPostProcessRenderable;
class FSceneViewFamily;
enum class ELGUICanvasDepthMode :uint8;

struct FLGUIMeshBatchContainer
{
	FMeshBatch Mesh;
	FVertexBufferRHIRef VertexBufferRHI;
	int32 NumVerts = 0;

	FLGUIMeshBatchContainer() {}
};

enum class ELGUIHudPrimitiveType :uint8
{
	Mesh,
	PostProcess,
};

class ILGUIHudPrimitive
{
public:
	virtual ~ILGUIHudPrimitive() {}

	virtual bool CanRender() const = 0;
	virtual int GetRenderPriority() const = 0;
	virtual ELGUIHudPrimitiveType GetPrimitiveType()const = 0;

	//begin mesh interface
	virtual void GetMeshElements(const FSceneViewFamily& ViewFamilyclass, FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& ResultArray) = 0;
	//end mesh interface

	//begin post process interface
	virtual bool PostProcessRequireOriginScreenColorTexture()const = 0;
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenTargetTexture				The full screen render target
	 * @param	OriginScreenColorTexture		Origin screen color texture. PostProcessNeedOriginScreenColorTexture must return true for this to work.
	 * @param	ViewProjectionMatrix			For vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenColorTexture,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		float DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4& DepthTextureScaleOffset, 
		const FVector4& ViewTextureScaleOffset
	) {};
	//end post process interface
};
