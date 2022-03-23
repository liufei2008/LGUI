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

struct FLGUIMeshBatchContainer
{
	FMeshBatch Mesh;
#if ENGINE_MAJOR_VERSION >= 5
	FBufferRHIRef VertexBufferRHI;
#else
	FVertexBufferRHIRef VertexBufferRHI;
#endif
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

	//begin mesh interface
	virtual void GetMeshElements(const FSceneViewFamily& ViewFamilyclass, FMeshElementCollector* Collector, TArray<FLGUIMeshBatchContainer>& ResultArray) = 0;
	//end mesh interface

	//begin post process interface
	virtual ELGUIHudPrimitiveType GetPrimitiveType()const = 0;
	/**
	 * render thread function that will do the post process draw
	 * @param	ScreenImage				the full screen render image
	 * @param	ViewProjectionMatrix	for vertex shader to convert vertex to screen space. vertex position is already transformed to world space, so we dont need model matrix
	 */
	virtual void OnRenderPostProcess_RenderThread(
#if ENGINE_MAJOR_VERSION >= 5
		FRDGBuilder& GraphBuilder,
#else
		FRHICommandListImmediate& RHICmdList,
#endif
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenColorTexture,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		const FIntRect& ViewRect,
		const FVector4& DepthTextureScaleOffset, 
		const FVector4& ViewTextureScaleOffset
	) {};
	//end post process interface
};
