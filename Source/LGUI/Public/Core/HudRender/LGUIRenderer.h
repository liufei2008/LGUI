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
class FGlobalShaderMap;

class LGUI_API FLGUIViewExtension : public FSceneViewExtensionBase
{
public:
	FLGUIViewExtension(const FAutoRegister&, ULGUICanvas* InLGUICanvas, UTextureRenderTarget2D* InCustomRenderTarget);
	virtual ~FLGUIViewExtension();

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
	void SortRenderPriority();

	//
	void AddHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);
	void RemoveHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);

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
		const FMatrix& MVP
	);
	void DrawFullScreenQuad(
		FRHICommandListImmediate& RHICmdList
	);
	uint16 GetMultiSampleCount()const { return MultiSampleCount; }
private:
	void SetGraphicPipelineStateFromMaterial(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material);
	void SortRenderPriority_RenderThread();
	TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	TWeakObjectPtr<ULGUICanvas> UICanvas;
	TWeakObjectPtr<UTextureRenderTarget2D> CustomRenderTarget;
	TWeakObjectPtr<UWorld> World;
	uint16 MultiSampleCount = 0;

	FVector ViewLocation;
	FMatrix ViewRotationMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;
public:
#if WITH_EDITORONLY_DATA
	static uint32 EditorPreview_ViewKey;
	bool IsEditorPreview = false;
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
