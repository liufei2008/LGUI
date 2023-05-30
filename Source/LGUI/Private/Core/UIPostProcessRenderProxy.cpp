// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/UIPostProcessRenderProxy.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Rendering/Texture2DResource.h"
#include "PostProcess/SceneRenderTargets.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"

FUIPostProcessRenderProxy::FUIPostProcessRenderProxy()
{
	clipType = ELGUICanvasClipType::None;
	MaskTextureType = EUIPostProcessMaskTextureType::Simple;
}
void FUIPostProcessRenderProxy::AddToLGUIScreenSpaceRenderer(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer)
{
	this->bIsWorld = false;
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_AddToLGUIScreenSpaceRenderer)(
		[renderProxy, InLGUIRenderer](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->AddToLGUIScreenSpaceRenderer_RenderThread(InLGUIRenderer);
		});
}
void FUIPostProcessRenderProxy::AddToLGUIWorldSpaceRenderer(ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer)
{
	this->bIsWorld = true;
	this->RenderCanvasPtr = InCanvasPtr;
	auto renderProxy = this;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_AddToLGUIScreenSpaceRenderer)(
		[renderProxy, InCanvasPtr, InCanvasSortOrder, InLGUIRenderer](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->AddToLGUIWorldSpaceRenderer_RenderThread(InCanvasPtr, InCanvasSortOrder, InLGUIRenderer);
		});
}
void FUIPostProcessRenderProxy::AddToLGUIScreenSpaceRenderer_RenderThread(TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer)
{
	LGUIRenderer = InLGUIRenderer;
	if (LGUIRenderer.IsValid())
	{
		LGUIRenderer.Pin()->AddScreenSpacePrimitive_RenderThread(this);
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d Trying add to LGUIRenderer but the LGUIRenderer is not valid."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FUIPostProcessRenderProxy::AddToLGUIWorldSpaceRenderer_RenderThread(ULGUICanvas* InCanvasPtr, int32 InCanvasSortOrder, TWeakPtr<FLGUIHudRenderer, ESPMode::ThreadSafe> InLGUIRenderer)
{
	LGUIRenderer = InLGUIRenderer;
	if (LGUIRenderer.IsValid())
	{
		LGUIRenderer.Pin()->AddWorldSpacePrimitive_RenderThread((ULGUICanvas*)InCanvasPtr, this);
	}
	else
	{
		UE_LOG(LGUI, Log, TEXT("[%s].%d Trying add to LGUIRenderer but the LGUIRenderer is not valid."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FUIPostProcessRenderProxy::RemoveFromLGUIRenderer()
{
	auto renderProxy = this;
	bool isWorld = this->bIsWorld;
	ENQUEUE_RENDER_COMMAND(FUIPostProcessRenderProxy_RemoveFromHudRenderer)(
		[renderProxy, isWorld](FRHICommandListImmediate& RHICmdList)
		{
			renderProxy->RemoveFromLGUIRenderer_RenderThread(isWorld);
		});
}
void FUIPostProcessRenderProxy::RemoveFromLGUIRenderer_RenderThread(bool isWorld)
{
	if (!LGUIRenderer.IsValid())
	{
		return;
	}
	if (LGUIRenderer.IsValid())
	{
		if (isWorld)
		{
			LGUIRenderer.Pin()->RemoveWorldSpacePrimitive_RenderThread((ULGUICanvas*)RenderCanvasPtr, this);
		}
		else
		{
			LGUIRenderer.Pin()->RemoveScreenSpacePrimitive_RenderThread(this);
		}
	}
	LGUIRenderer.Reset();
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
GraphicsPSOInit.NumSamples = NumSamples;\
SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

void FUIPostProcessRenderProxy::RenderMeshOnScreen_RenderThread(
	FRHICommandListImmediate& RHICmdList
	, FTextureRHIRef ScreenTargetTexture
	, FGlobalShaderMap* GlobalShaderMap
	, FTextureRHIRef MeshRegionTexture
	, const FMatrix& ModelViewProjectionMatrix
	, bool IsWorldSpace
	, float BlendDepthForWorld
	, float DepthFadeForWorld
	, const FVector4& DepthTextureScaleOffset
	, const FIntRect& ViewRect
	, FRHISamplerState* ResultTextureSamplerState
)
{
	uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	FIndexBufferRHIRef IndexBuffer = nullptr;
	int32 TriangleCount = 2;

	auto RPInfo = FRHIRenderPassInfo(ScreenTargetTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("UIPostProcess_RenderMeshToScreen"));
	RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

	if (maskTexture != nullptr)
	{
		switch (clipType)
		{
		default:
		case ELGUICanvasClipType::None:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldPS> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldDepthFadePS> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshWithMaskPS> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
					, ResultTextureSamplerState
					, maskTexture->SamplerStateRHI
				);
			}
		}
		break;
		case ELGUICanvasClipType::Rect:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldPS_RectClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshWithMaskPS_RectClip> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
					, ResultTextureSamplerState
					, maskTexture->SamplerStateRHI
				);
				PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
			}
		}
		break;
		case ELGUICanvasClipType::Texture:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldPS_TextureClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					if (clipTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
					}
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
						, ResultTextureSamplerState
						, maskTexture->SamplerStateRHI
					);
					if (clipTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
					}
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshWithMaskPS_TextureClip> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, maskTexture->TextureRHI
					, ResultTextureSamplerState
					, maskTexture->SamplerStateRHI
				);
				if (clipTexture != nullptr)
				{
					PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
				}
			}
		}
		break;
		}
		switch (MaskTextureType)
		{
		default:
		case EUIPostProcessMaskTextureType::Simple:
		{
			IndexBuffer = GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI;
		}
		break;
		case EUIPostProcessMaskTextureType::Sliced:
		{
			IndexBuffer = GLGUIFullScreenSlicedQuadIndexBuffer.IndexBufferRHI;
			TriangleCount = 18;
		}
		break;
		}
	}
	else
	{
		switch (clipType)
		{
		default:
		case ELGUICanvasClipType::None:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldPS> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldDepthFadePS> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshPS> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
			}
		}
		break;
		case ELGUICanvasClipType::Rect:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldPS_RectClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldDepthFadePS_RectClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshPS_RectClip> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
				PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
			}
		}
		break;
		case ELGUICanvasClipType::Texture:
		{
			if (IsWorldSpace)
			{
				if (DepthFadeForWorld <= 0.0f)
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldPS_TextureClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					if (clipTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
					}
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
				}
				else
				{
					TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIRenderMeshWorldDepthFadePS_TextureClip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					if (clipTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
					}
					PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, SceneContext.GetSceneDepthTexture());
					PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld);
				}
			}
			else
			{
				TShaderMapRef<FLGUIRenderMeshVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIRenderMeshPS_TextureClip> PixelShader(GlobalShaderMap);
				SET_PIPELINE_STATE_FOR_CLIP();
				VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
				PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
				if (clipTexture != nullptr)
				{
					PixelShader->SetClipParameters(RHICmdList, textureClipOffsetAndSize, clipTexture->TextureRHI, clipTexture->SamplerStateRHI);
				}
			}
		}
		break;
		}
		IndexBuffer = GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI;
	}

	uint32 VertexBufferSize = renderMeshRegionToScreenVertexArray.Num() * sizeof(FLGUIPostProcessVertex);
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, renderMeshRegionToScreenVertexArray.GetData(), VertexBufferSize);
	RHIUnlockVertexBuffer(VertexBufferRHI);
	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, renderMeshRegionToScreenVertexArray.Num(), 0, TriangleCount, 1);
	VertexBufferRHI.SafeRelease();

	RHICmdList.EndRenderPass();
}