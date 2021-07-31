// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "Widgets/Layout/SBox.h"
#include "RenderResource.h"
#include "StaticMeshVertexData.h"

class ILGUIHudPrimitive;
class ULGUICanvas;
struct FLGUIPostProcessVertex;
struct FLGUIPostProcessCopyMeshRegionVertex;

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
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily)override {};
	virtual void PreRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)override {};
	virtual void PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)override;

	virtual void PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)override;
	virtual void PostRenderViewFamily_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneViewFamily& InViewFamily)override {};
	//end ISceneViewExtension interfaces

	//
	void AddHudPrimitive_RenderThread(ULGUICanvas* InCanvas, int32 InRenderCanvasSortOrder, ILGUIHudPrimitive* InPrimitive);
	void RemoveHudPrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive);
	void SetRenderCanvasSortOrder_RenderThread(ULGUICanvas* InRenderCanvas, int32 InSortOrder);

	void AddRenderCanvas(ULGUICanvas* InCanvas);
	void RemoveRenderCanvas(ULGUICanvas* InCanvas);

	void CopyRenderTarget(
		FRHICommandListImmediate& RHICmdList, 
		TShaderMap<FGlobalShaderType>* GlobalShaderMap, 
		FTextureRHIRef Src, FTextureRHIRef Dst
	);
	void CopyRenderTargetOnMeshRegion(
		FRHICommandListImmediate& RHICmdList,
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst,
		const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData,
		const FMatrix& MVP
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

		FVector ViewLocation;
		FMatrix ViewRotationMatrix;
		FMatrix ProjectionMatrix;
		FMatrix ViewProjectionMatrix;

		TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	};
	TArray<FRenderCanvasParameter> RenderCanvasParameterArray;
	TWeakObjectPtr<UTextureRenderTarget2D> CustomRenderTarget;
	TWeakObjectPtr<UWorld> World;
	uint16 MultiSampleCount = 0;
	bool bContainsPostProcess = false;
	FCriticalSection RenderCanvasParameterArray_Mutex;//Lock for RenderCanvasParameterArray
	void CheckContainsPostProcess_RenderThread();	
	void AddRootCanvas_RenderThread(FRenderCanvasParameter InCanvasParameter);
	void RemoveRootCanvas_RenderThread(ULGUICanvas* InCanvas);
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
