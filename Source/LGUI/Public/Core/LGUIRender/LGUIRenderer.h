// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "StaticMeshVertexData.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "Core/LGUIRender/ILGUIRendererPrimitive.h"

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
struct FLGUIHelperLineRenderParameter
{
public:
	FLGUIHelperLineRenderParameter(const TArray<FLGUIHelperLineVertex>& InLinePoints)
	{
		LinePoints = InLinePoints;
	}
	TArray<FLGUIHelperLineVertex> LinePoints;
};
#endif

class LGUI_API FLGUIRenderer : public FSceneViewExtensionBase
{
public:
	FLGUIRenderer(const FAutoRegister&, UWorld* InWorld);
	virtual ~FLGUIRenderer();

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
	void AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIRendererPrimitive* InPrimitive);
	void RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIRendererPrimitive* InPrimitive);

	void AddScreenSpacePrimitive_RenderThread(ILGUIRendererPrimitive* InPrimitive);
	void RemoveScreenSpacePrimitive_RenderThread(ILGUIRendererPrimitive* InPrimitive);

	void MarkNeedToSortScreenSpacePrimitiveRenderPriority();
	void MarkNeedToSortWorldSpacePrimitiveRenderPriority();
	void SetRenderCanvasDepthParameter(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade);

	void SetScreenSpaceRootCanvas(ULGUICanvas* InCanvas);
	void ClearScreenSpaceRootCanvas();

	void SetRenderToRenderTarget(bool InValue);
	void UpdateRenderTargetRenderer(UTextureRenderTarget2D* InRenderTarget);

	TWeakObjectPtr<UWorld> GetWorld() { return World; }

	void CopyRenderTarget(
		FRDGBuilder& GraphBuilder,
		FGlobalShaderMap* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst,
		bool ColorCorrect = false
	);
	void CopyRenderTargetOnMeshRegion(
		FRDGBuilder& GraphBuilder,
		FRDGTextureRef Dst,
		FTextureRHIRef Src,
		FGlobalShaderMap* GlobalShaderMap,
		const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData,
		const FMatrix44f& MVP,
		const FIntRect& ViewRect,
		const FVector4f& SrcTextureScaleOffset,
		bool ColorCorrect = false
	);
	void DrawFullScreenQuad(
		FRHICommandListImmediate& RHICmdList
	);
	void AddResolvePass(
		FRDGBuilder& GraphBuilder
		, FRDGTextureMSAA SceneColor
		, bool bIsInstancedStereoPass
		, float InstancedStereoWidth
		, const FIntRect& ViewRect
		, uint8 NumSamples
		, FGlobalShaderMap* GlobalShaderMap
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

		ILGUIRendererPrimitive* Primitive = nullptr;
	};
	struct FScreenSpaceRenderParameter
	{
		FVector ViewOrigin = FVector::ZeroVector;
		FMatrix ViewRotationMatrix = FMatrix::Identity;
		FMatrix ProjectionMatrix = FMatrix::Identity;
		FMatrix44f ViewProjectionMatrix = FMatrix44f::Identity;
		bool bEnableDepthTest = false;
		bool bNeedSortRenderPriority = true;

		TWeakObjectPtr<ULGUICanvas> RootCanvas = nullptr;
		TArray<ILGUIRendererPrimitive*> PrimitiveArray;
	};
	TArray<FWorldSpaceRenderParameter> WorldSpaceRenderCanvasParameterArray;
	TMap<ULGUICanvas*, bool> WorldSpaceCanvasVisibilityMap;
	bool bNeedSortWorldSpaceRenderCanvas = true;
	FScreenSpaceRenderParameter ScreenSpaceRenderParameter;
	TWeakObjectPtr<UWorld> World;
	TArray<FLGUIMeshBatchContainer> MeshBatchArray;
	//if 'bIsRenderToRenderTarget' is true then we need a render target
	FTextureRenderTargetResource* RenderTargetResource = nullptr;
	void SortScreenSpacePrimitiveRenderPriority_RenderThread();
	void SetRenderCanvasDepthFade_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade);
	//is render to a custom render target? or just render to screen
	bool bIsRenderToRenderTarget = false;
	//render thread sample count for MSAA
	uint8 NumSamples_MSAA = 1;

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
	TArray<FLGUIHelperLineRenderParameter> HelperLineRenderParameterArray;
public:
	void AddLineRender(const FLGUIHelperLineRenderParameter& InLineParameter);
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
