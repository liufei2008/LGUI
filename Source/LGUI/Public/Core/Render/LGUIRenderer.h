// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "Widgets/Layout/SBox.h"
#include "RenderResource.h"
#include "StaticMeshVertexData.h"

class ILGUIHudPrimitive;
class ULGUICanvas;

class LGUI_API FLGUIViewExtension : public FSceneViewExtensionBase
{
public:
	FLGUIViewExtension(const FAutoRegister&, ULGUICanvas* InLGUICanvas);
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
private:
	void MarkSortRenderPriority_RenderThread();
	bool NeedToSortPrimitive = true;
	TArray<ILGUIHudPrimitive*> HudPrimitiveArray;
	TWeakObjectPtr<ULGUICanvas> UICanvas;

	FVector ViewLocation;
	FRotator ViewRotation;
	FMatrix ViewRotationMatrix;
	FMatrix ProjectionMatrix;
};
