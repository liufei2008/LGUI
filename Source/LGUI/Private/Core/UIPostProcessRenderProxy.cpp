﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/UIPostProcessRenderProxy.h"
#include "LGUI.h"

void FUIPostProcessRenderProxy::AddToHudRenderer(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> InLGUIHudRenderer)
{
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_AddToHudRenderer)(
		[renderProxy, InLGUIHudRenderer](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->AddToHudRenderer_RenderThread(InLGUIHudRenderer);
		});
}
void FUIPostProcessRenderProxy::AddToHudRenderer_RenderThread(TWeakPtr<FLGUIViewExtension, ESPMode::ThreadSafe> InLGUIHudRenderer)
{
	if (LGUIHudRenderer.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("[FUIPostProcessRenderProxy::AddToHudRenderer]Already added to LGUIRenderer!"));
		return;
	}
	LGUIHudRenderer = InLGUIHudRenderer;
	if (LGUIHudRenderer.IsValid())
	{
		LGUIHudRenderer.Pin()->AddHudPrimitive_RenderThread(this);
	}
}
void FUIPostProcessRenderProxy::RemoveFromHudRenderer()
{
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_RemoveFromHudRenderer)(
		[renderProxy](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->RemoveFromHudRenderer_RenderThread();
		});
}
void FUIPostProcessRenderProxy::RemoveFromHudRenderer_RenderThread()
{
	if (!LGUIHudRenderer.IsValid())
	{
		return;
	}
	if (LGUIHudRenderer.IsValid())
	{
		LGUIHudRenderer.Pin()->RemoveHudPrimitive_RenderThread(this);
	}
	LGUIHudRenderer.Reset();
}
void FUIPostProcessRenderProxy::SetUITranslucentSortPriority(int32 NewTranslucentSortPriority)
{
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcess_SetSortPriority)(
		[NewTranslucentSortPriority, renderProxy](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->SetUITranslucentSortPriority_RenderThread(NewTranslucentSortPriority);
		});
}
void FUIPostProcessRenderProxy::SetVisibility(bool value)
{
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcess_SetSortPriority)(
		[value, renderProxy](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->SetVisibility_RenderThread(value);
		});
}