// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "StaticMeshVertexData.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"

class ULGUICanvas;
struct FLGUIPostProcessVertex;
struct FLGUIPostProcessCopyMeshRegionVertex;
class FGlobalShaderMap;

class FLGUIMeshElementCollector : FMeshElementCollector//why use a custom collector? because default FMeshElementCollector have no public constructor
{
public:
	FLGUIMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel, FSceneRenderingBulkObjectAllocator& Allocator)
		:FMeshElementCollector(InFeatureLevel, Allocator)
	{

	}
};

#if WITH_EDITOR
/** Parameters for render editor helper line */
struct FHelperLineRenderParameter
{
public:
	FHelperLineRenderParameter(const TArray<FLGUIHelperLineVertex>& InLinePoints)
	{
		LinePoints = InLinePoints;
	}
	TArray<FLGUIHelperLineVertex> LinePoints;
};
#endif

class LGUI_API FLGUIHudRenderer : public FSceneViewExtensionBase
{
public:
	FLGUIHudRenderer(const FAutoRegister&, UWorld* InWorld);
	virtual ~FLGUIHudRenderer();

	//begin ISceneViewExtension interfaces
	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily)override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)override;
	virtual void SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)override;
	virtual void SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)override;
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily)override;

	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)override {};
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)override;

	virtual void PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)override;
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)override {};
	virtual void SubscribeToPostProcessingPass(EPostProcessingPass Pass, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)override {};

	virtual void PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)override;
	virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)override;
	virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)override {};

	virtual int32 GetPriority() const override;
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
	//end ISceneViewExtension interfaces

	//
	void AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive);
	void RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive);

	void AddScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);
	void RemoveScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);

	void SortScreenAndWorldSpacePrimitiveRenderPriority();
	void SetRenderCanvasDepthParameter(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade);

	void SetScreenSpaceRenderCanvas(ULGUICanvas* InCanvas);
	void ClearScreenSpaceRenderCanvas();

	void SetRenderToRenderTarget(bool InValue);
	void UpdateRenderTargetRenderer(UTextureRenderTarget2D* InRenderTarget);

	TWeakObjectPtr<UWorld> GetWorld() { return World; }

	void CopyRenderTarget(
		FRDGBuilder& GraphBuilder,
		FGlobalShaderMap* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst
	);
	void CopyRenderTargetOnMeshRegion(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef Dst,
		FTextureRHIRef Src,
		FGlobalShaderMap* GlobalShaderMap,
		const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData,
		const FMatrix44f& MVP,
		const FIntRect& ViewRect,
		const FVector4f& SrcTextureScaleOffset
	);
	void DrawFullScreenQuad(
		FRHICommandListImmediate& RHICmdList
	);
private:
	static void SetGraphicPipelineState(ERHIFeatureLevel::Type FeatureLevel, FGraphicsPipelineStateInitializer& GraphicsPSOInit, EBlendMode BlendMode
		, bool bIsWireFrame, bool bIsTwoSided, bool bDisableDepthTestForTransparent, bool bIsDepthValid, bool bReverseCulling
	);
	struct FWorldSpaceRenderParameter
	{
		/*
		 * CAUTION! use this uobject pointer only in game-thread!
		 * I use it in render-thread just as a pointer or a key, so it is safe here.
		 */
		ULGUICanvas* RenderCanvas = nullptr;
		//blend depth, 0-occlude by depth, 1-all visible
		float BlendDepth = 0.0f;
		//depth fade effect
		float DepthFade = 0.0f;

		ILGUIHudPrimitive* HudPrimitive;
	};
	struct FScreenSpaceRenderParameter
	{
		FVector ViewOrigin = FVector::ZeroVector;
		FMatrix ViewRotationMatrix = FMatrix::Identity;
		FMatrix ProjectionMatrix = FMatrix::Identity;
		FMatrix44f ViewProjectionMatrix = FMatrix44f::Identity;
		bool bEnableDepthTest = false;

		TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
		TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	};
	TArray<FWorldSpaceRenderParameter> WorldSpaceRenderCanvasParameterArray;
	FScreenSpaceRenderParameter ScreenSpaceRenderParameter;
	TWeakObjectPtr<UWorld> World;
	TArray<FLGUIMeshBatchContainer> MeshBatchArray;
	//if 'bIsRenderToRenderTarget' is true then we need a render target
	class FTextureRenderTargetResource* RenderTargetResource = nullptr;
	//some post process component need to blend origin screen image without UI element
	bool bNeedOriginScreenColorTextureOnPostProcess = false;
	void CheckContainsPostProcess_RenderThread();	
	void SortScreenSpacePrimitiveRenderPriority_RenderThread();
	void SortWorldSpacePrimitiveRenderPriority_RenderThread();
	void SetRenderCanvasDepthFade_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade);
	//is render to a custom render target? or just render to screen
	bool bIsRenderToRenderTarget = false;
	FSceneRenderingBulkObjectAllocator Allocator;

	void RenderLGUI_RenderThread(
		FRDGBuilder& GraphBuilder
		, FSceneView& InView);
#if WITH_EDITORONLY_DATA
public:
	static uint32 EditorPreview_ViewKey;
private:
	bool bIsEditorPreview = false;
	mutable bool bCanRenderScreenSpace = true;
	mutable bool bIsPlaying = false;
#endif
#if WITH_EDITOR
private:
	TArray<FHelperLineRenderParameter> HelperLineRenderParameterArray;
public:
	void AddLineRender(const FHelperLineRenderParameter& InLineParameter);
#endif
};

class LGUI_API FLGUIFullScreenQuadVertexBuffer :public FVertexBuffer
{
public:
	void InitRHI()override;
};
class LGUI_API FLGUIFullScreenQuadIndexBuffer :public FIndexBuffer
{
public:
	void InitRHI()override;
};
class LGUI_API FLGUIFullScreenSlicedQuadIndexBuffer :public FIndexBuffer
{
public:
	void InitRHI()override;
};
static TGlobalResource<FLGUIFullScreenQuadVertexBuffer> GLGUIFullScreenQuadVertexBuffer;
static TGlobalResource<FLGUIFullScreenQuadIndexBuffer> GLGUIFullScreenQuadIndexBuffer;
static TGlobalResource<FLGUIFullScreenSlicedQuadIndexBuffer> GLGUIFullScreenSlicedQuadIndexBuffer;
BEGIN_SHADER_PARAMETER_STRUCT(FLGUIWorldRenderPSParameter, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneDepthTex)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
