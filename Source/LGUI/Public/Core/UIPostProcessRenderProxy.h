// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"

class UUIPostProcess;

/**
 * UIPostProcessRenderProxy is an render agent for UIPostProcess in render thread, act as a SceneProxy.
 */
class LGUI_API FUIPostProcessRenderProxy: public ILGUIHudPrimitive
{
public:
	FUIPostProcessRenderProxy()
	{
		
	}
	virtual~FUIPostProcessRenderProxy()
	{
		
	}
private:
	TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> LGUIHudRenderer;
	int32 RenderPriority = 0;
	bool bIsVisible = true;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority);
	void SetVisibility(bool value);
	void AddToHudRenderer(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> InLGUIHudRenderer);
	void AddToHudRenderer_RenderThread(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> InLGUIHudRenderer);
	void RemoveFromHudRenderer();
	void RemoveFromHudRenderer_RenderThread();

	//begin ILGUIHudPrimitive interface
	virtual bool CanRender() const { return bIsVisible; };
	virtual int GetRenderPriority() const { return RenderPriority; };

	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) { return FMeshBatch(); };
	virtual FRHIVertexBuffer* GetVertexBufferRHI() { return nullptr; };
	virtual uint32 GetNumVerts() { return 0; };

	virtual bool GetIsPostProcess() { return true; };
	//end ILGUIHudPrimitive interface
private:
	void SetUITranslucentSortPriority_RenderThread(int value)
	{
		RenderPriority = value;
	}
	void SetVisibility_RenderThread(bool value)
	{
		bIsVisible = value;
	}
};
