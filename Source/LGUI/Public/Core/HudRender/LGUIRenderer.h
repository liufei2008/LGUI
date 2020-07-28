// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
		FTexture2DRHIRef Src, FTexture2DRHIRef Dst, bool FlipY,
		const FVector4& PositionScaleAndOffset = FVector4(1, 1, 0, 0), const FVector4& UVScaleAndOffset = FVector4(1, 1, 0, 0)
	);
	static void CopyRenderTargetOnMeshRegion(
		FRHICommandListImmediate& RHICmdList,
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		FTexture2DRHIRef Src, FTexture2DRHIRef Dst, bool FlipY,
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
