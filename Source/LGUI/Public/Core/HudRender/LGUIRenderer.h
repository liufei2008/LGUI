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
	void AddHudPrimitive(ILGUIHudPrimitive* InPrimitive);
	void RemoveHudPrimitive(ILGUIHudPrimitive* InPrimitive);

	static void CopyRenderTarget(
		FRHICommandListImmediate& RHICmdList, 
		TShaderMap<FGlobalShaderType>* GlobalShaderMap, 
		FTextureRHIRef Src, FTextureRHIRef Dst
	);
	static void CopyRenderTargetOnMeshRegion(
		FRHICommandListImmediate& RHICmdList,
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		FTextureRHIRef Src, FTextureRHIRef Dst,
		const TArray<FLGUIPostProcessVertex>& RegionVertexData
	);
	static void DrawFullScreenQuad(
		FRHICommandListImmediate& RHICmdList
	);
private:
	void AddHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive);
	void MarkSortRenderPriority_RenderThread();
	TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	TWeakObjectPtr<ULGUICanvas> UICanvas;
	TWeakObjectPtr<UTextureRenderTarget2D> CustomRenderTarget;
	TWeakObjectPtr<UWorld> World;
	TRefCountPtr<IPooledRenderTarget> SceneColorRenderTarget;
	uint16 MultiSampleCount = 0;

	FVector ViewLocation;
	FMatrix ViewRotationMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;
	FCriticalSection Mutex;//for lock HudPrimitiveArray, incase gamethread and renderthread
public:
#if WITH_EDITORONLY_DATA
	static uint32 EditorPreview_ViewKey;
	bool IsEditorPreview = false;
#endif
};

class FLGUIFullScreenQuadVertexBuffer :public FVertexBuffer
{
public:
	void InitRHI()override;
};
class FLGUIFullScreenQuadIndexBuffer :public FIndexBuffer
{
public:
	void InitRHI()override;
};
static TGlobalResource<FLGUIFullScreenQuadVertexBuffer> GLGUIFullScreenQuadVertexBuffer;
static TGlobalResource<FLGUIFullScreenQuadIndexBuffer> GLGUIFullScreenQuadIndexBuffer;
