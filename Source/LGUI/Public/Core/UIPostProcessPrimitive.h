// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "UIGeometry.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"

class UUIPostProcess;

/**
 * PostProcessPrimitive is an agent for UIPostProcess, act as a UIDrawcallMesh.
 */
class LGUI_API UUIPostProcessPrimitive: public ILGUIHudPrimitive
{
public:
	UUIPostProcessPrimitive(TWeakObjectPtr<UUIPostProcess> InPostProcessObject, TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> InLGUIHudRenderer)
	{
		PostProcessObject = InPostProcessObject;
		LGUIHudRenderer = InLGUIHudRenderer;
		if (LGUIHudRenderer.IsValid())
		{
			LGUIHudRenderer.Pin()->AddHudPrimitive(this);
		}
	}
	~UUIPostProcessPrimitive()
	{
		if (LGUIHudRenderer.IsValid())
		{
			LGUIHudRenderer.Pin()->RemoveHudPrimitive_RenderThread(this);
			LGUIHudRenderer.Reset();
		}
	}
private:
	TWeakObjectPtr<UUIPostProcess> PostProcessObject;
	TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> LGUIHudRenderer;
	int32 RenderPriority = 0;
	bool bIsVisible = true;
public:
	void SetUITranslucentSortPriority(int32 NewTranslucentSortPriority)
	{
		RenderPriority = NewTranslucentSortPriority;
	}
	void SetVisibility(bool value)
	{
		bIsVisible = value;
	}

	//begin ILGUIHudPrimitive interface
	virtual bool CanRender() const { return PostProcessObject.IsValid() && bIsVisible; };
	virtual int GetRenderPriority() const { return RenderPriority; };

	virtual FMeshBatch GetMeshElement(class FMeshElementCollector* Collector) { return FMeshBatch(); };
	virtual FRHIVertexBuffer* GetVertexBufferRHI() { return nullptr; };
	virtual uint32 GetNumVerts() { return 0; };

	virtual bool GetIsPostProcess() { return true; };
	virtual TWeakObjectPtr<UUIPostProcess> GetPostProcessObject()
	{
		return PostProcessObject;
	}
	//end ILGUIHudPrimitive interface
};
