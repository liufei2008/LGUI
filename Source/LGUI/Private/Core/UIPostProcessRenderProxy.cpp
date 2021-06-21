// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/UIPostProcessRenderProxy.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "Core/HudRender/LGUIHudVertex.h"

void FUIPostProcessRenderProxy::AddToHudRenderer(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIHudRenderer)
{
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_AddToHudRenderer)(
		[renderProxy, InLGUIHudRenderer](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->AddToHudRenderer_RenderThread(InLGUIHudRenderer);
		});
}
void FUIPostProcessRenderProxy::AddToHudRenderer_RenderThread(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIHudRenderer)
{
	if (LGUIHudRenderer == InLGUIHudRenderer)
	{
		if (!LGUIHudRenderer.IsValid())
		{
			UE_LOG(LGUI, Log, TEXT("[FUIPostProcessRenderProxy::AddToHudRenderer]0Trying add to LGUIRenderer but the LGUIRenderer is not valid."));
		}
		return;
	}
	else
	{
		//remove from old
		if (LGUIHudRenderer.IsValid())
		{
			LGUIHudRenderer.Pin()->RemoveHudPrimitive_RenderThread(this);
		}
	}
	LGUIHudRenderer = InLGUIHudRenderer;
	if (LGUIHudRenderer.IsValid())
	{
		LGUIHudRenderer.Pin()->AddHudPrimitive_RenderThread(this);
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[FUIPostProcessRenderProxy::AddToHudRenderer]1Trying add to LGUIRenderer but the LGUIRenderer is not valid."));
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

#define SET_PIPELINE_STATE_FOR_CLIP()\
FGraphicsPipelineStateInitializer GraphicsPSOInit;\
RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);\
GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();\
GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();\
GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();\
GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();\
GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();\
GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();\
GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;\
SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

void FUIPostProcessRenderProxy::RenderMeshOnScreen_RenderThread(FRHICommandListImmediate& RHICmdList
	, FTextureRHIRef ScreenTargetTexture
	, FGlobalShaderMap* GlobalShaderMap
	, FTextureRHIRef ResultTexture
	, FRHISamplerState* ResultTextureSamplerState
)
{
	if (maskTexture != nullptr)
	{
		RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetTexture, ERenderTargetActions::Load_DontStore), TEXT("RenderMeshToScreen"));
		RHICmdList.SetViewport(0, 0, 0.0f, ScreenTargetTexture->GetSizeXYZ().X, ScreenTargetTexture->GetSizeXYZ().Y, 1.0f);

		switch (clipType)
		{
		default:
		case ELGUICanvasClipType::None:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshWithMaskPS> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, maskTexture->TextureRHI
				, ResultTextureSamplerState
				, maskTexture->SamplerStateRHI
			);
		}
		break;
		case ELGUICanvasClipType::Rect:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshWithMaskPS_RectClip> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, maskTexture->TextureRHI
				, ResultTextureSamplerState
				, maskTexture->SamplerStateRHI
			);
			PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
		}
		break;
		case ELGUICanvasClipType::Texture:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshWithMaskPS_TextureClip> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, maskTexture->TextureRHI
				, ResultTextureSamplerState
				, maskTexture->SamplerStateRHI
			);
			if (clipTexture != nullptr)
			{
				PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
			}
		}
		break;
		}
	}
	else
	{
		RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetTexture, ERenderTargetActions::Load_DontStore), TEXT("RenderMeshToScreen"));
		RHICmdList.SetViewport(0, 0, 0.0f, ScreenTargetTexture->GetSizeXYZ().X, ScreenTargetTexture->GetSizeXYZ().Y, 1.0f);

		switch (clipType)
		{
		default:
		case ELGUICanvasClipType::None:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshPS> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, ResultTextureSamplerState);
		}
		break;
		case ELGUICanvasClipType::Rect:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshPS_RectClip> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, ResultTextureSamplerState);
			PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
		}
		break;
		case ELGUICanvasClipType::Texture:
		{
			TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIRenderMeshPS_TextureClip> PixelShader(GlobalShaderMap);
			SET_PIPELINE_STATE_FOR_CLIP();
			VertexShader->SetParameters(RHICmdList, modelViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, ResultTexture, ResultTextureSamplerState);
			if (clipTexture != nullptr)
			{
				PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
			}
		}
		break;
		}

	}

	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, renderMeshRegionToScreenVertexArray.GetData(), VertexBufferSize);
	RHIUnlockVertexBuffer(VertexBufferRHI);
	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
	VertexBufferRHI.SafeRelease();

	RHICmdList.EndRenderPass();
}