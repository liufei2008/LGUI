// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "StaticMeshVertexData.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"

class ULGUICanvas;
struct FLGUIPostProcessVertex;
struct FLGUIPostProcessCopyMeshRegionVertex;
class FGlobalShaderMap;

class LGUI_API FLGUIHudRenderer : public FSceneViewExtensionBase
{
public:
	FLGUIHudRenderer(const FAutoRegister&, UWorld* InWorld, UTextureRenderTarget2D* InCustomRenderTarget);
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
	virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)override {};

	virtual int32 GetPriority() const override;
	virtual bool IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const override;
	//end ISceneViewExtension interfaces

	//
	void AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, int32 InRenderCanvasSortOrder, ILGUIHudPrimitive* InPrimitive);
	void RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive);

	void AddScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);
	void RemoveScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);

	void SetRenderCanvasSortOrder(ULGUICanvas* InRenderCanvas, int32 InSortOrder);
	void SortPrimitiveRenderPriority();
	void SetRenderCanvasBlendDepth(ULGUICanvas* InRenderCanvas, float InBlendDepth);

	void AddWorldSpaceRenderCanvas(ULGUICanvas* InCanvas);
	void RemoveWorldSpaceRenderCanvas(ULGUICanvas* InCanvas);

	void SetScreenSpaceRenderCanvas(ULGUICanvas* InCanvas);
	void ClearScreenSpaceRenderCanvas();

	TWeakObjectPtr<UWorld> GetWorld() { return World; }

	void CopyRenderTarget(
		FRHICommandListImmediate& RHICmdList, 
		FGlobalShaderMap* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst
	);
	void CopyRenderTargetOnMeshRegion(
		FRHICommandListImmediate& RHICmdList,
		FGlobalShaderMap* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst,
		const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData,
		const FMatrix& MVP,
		const FIntRect& ViewRect,
		const FVector4& SrcTextureScaleOffset
	);
	void DrawFullScreenQuad(
		FRHICommandListImmediate& RHICmdList
	);
	uint16 GetMultiSampleCount()const { return MultiSampleCount; }
private:
	void SetGraphicPipelineStateFromMaterial(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material);
	struct FRenderCanvasParameter
	{
		/*
		 * CAUTION! use this uobject pointer only in game-thread!
		 * I use it in render-thread just as a pointer or a key, so it is safe here.
		 */
		ULGUICanvas* RenderCanvas;

		int32 RenderCanvasSortOrder;
		//blend depth, 0-occlude by depth, 1-all visible
		float BlendDepth;

		TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	};
	struct FScreenSpaceRenderParameter
	{
		FVector ViewOrigin = FVector::ZeroVector;
		FMatrix ViewRotationMatrix = FMatrix::Identity;
		FMatrix ProjectionMatrix = FMatrix::Identity;
		FMatrix ViewProjectionMatrix = FMatrix::Identity;

		TWeakObjectPtr<ULGUICanvas> RenderCanvas = nullptr;
		TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	};
	TArray<FRenderCanvasParameter> WorldSpaceRenderCanvasParameterArray;
	FScreenSpaceRenderParameter ScreenSpaceRenderParameter;
	TWeakObjectPtr<UTextureRenderTarget2D> CustomRenderTarget;
	TWeakObjectPtr<UWorld> World;
	TArray<FLGUIMeshBatchContainer> MeshBatchArray;
	uint16 MultiSampleCount = 0;
	bool bContainsPostProcess = false;
	void CheckContainsPostProcess_RenderThread();	
	void AddWorldSpaceRenderCanvas_RenderThread(FRenderCanvasParameter InCanvasParameter);
	void RemoveWorldSpaceRenderCanvas_RenderThread(ULGUICanvas* InCanvas);
	void SortScreenSpacePrimitiveRenderPriority_RenderThread();
	void SortPrimitiveRenderPriority_RenderThread();
	void SetRenderCanvasSortOrder_RenderThread(ULGUICanvas* InRenderCanvas, int32 InSortOrder);
	void SetRenderCanvasBlendDepth_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth);

	void RenderLGUI_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView);
public:
#if WITH_EDITORONLY_DATA
	static uint32 EditorPreview_ViewKey;
	bool bIsEditorPreview = false;
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
static TGlobalResource<FLGUIFullScreenQuadVertexBuffer> GLGUIFullScreenQuadVertexBuffer;
static TGlobalResource<FLGUIFullScreenQuadIndexBuffer> GLGUIFullScreenQuadIndexBuffer;
