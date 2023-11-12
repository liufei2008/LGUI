// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/UIPostProcessRenderProxy.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/LGUIRender/LGUIPostProcessShaders.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Rendering/Texture2DResource.h"
#include "PostProcess/SceneRenderTargets.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"

FUIPostProcessRenderProxy::FUIPostProcessRenderProxy()
{
	clipType = ELGUICanvasClipType::None;
	MaskTextureType = EUIPostProcessMaskTextureType::Simple;
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
SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::ForceApply);

void FUIPostProcessRenderProxy::RenderMeshOnScreen_RenderThread(
	FRDGBuilder& GraphBuilder
	, const FMinimalSceneTextures& SceneTextures
	, FTextureRHIRef ScreenTargetTexture
	, FGlobalShaderMap* GlobalShaderMap
	, FTextureRHIRef MeshRegionTexture
	, const FMatrix44f& ModelViewProjectionMatrix
	, bool IsWorldSpace
	, float BlendDepthForWorld
	, float DepthFadeForWorld
	, const FVector4f& DepthTextureScaleOffset
	, const FIntRect& ViewRect
	, FRHISamplerState* ResultTextureSamplerState
)
{
	uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
	FLGUIWorldRenderPSParameter* PSShaderParameters = GraphBuilder.AllocParameters<FLGUIWorldRenderPSParameter>();
	PSShaderParameters->SceneDepthTex = SceneTextures.Depth.Resolve;
	PSShaderParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LGUIRendererTargetTexture")), ERenderTargetLoadAction::ELoad);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("UIPostProcess_RenderMeshToScreen"),
		PSShaderParameters,
		ERDGPassFlags::Raster,
		[this, PSShaderParameters, GlobalShaderMap, MeshRegionTexture, ModelViewProjectionMatrix, IsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect, ResultTextureSamplerState, NumSamples](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			FBufferRHIRef IndexBuffer = nullptr;
			int32 TriangleCount = 2;
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
						}
						else
						{
							TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
							TShaderMapRef<FLGUIRenderMeshWorldDepthFadePS> PixelShader(GlobalShaderMap);
							SET_PIPELINE_STATE_FOR_CLIP();
							VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
							PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
						}
						else
						{
							TShaderMapRef<FLGUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
							TShaderMapRef<FLGUIRenderMeshWorldDepthFadePS_RectClip> PixelShader(GlobalShaderMap);
							SET_PIPELINE_STATE_FOR_CLIP();
							VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix);
							PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
							PixelShader->SetClipParameters(RHICmdList, rectClipOffsetAndSize, rectClipFeather);
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
							PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
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
			FRHIResourceCreateInfo CreateInfo(TEXT("RenderMeshOnScreen"));
			FBufferRHIRef VertexBufferRHI = RHICmdList.CreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
			void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
			FPlatformMemory::Memcpy(VoidPtr, renderMeshRegionToScreenVertexArray.GetData(), VertexBufferSize);
			RHICmdList.UnlockBuffer(VertexBufferRHI);
			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, renderMeshRegionToScreenVertexArray.Num(), 0, TriangleCount, 1);
			VertexBufferRHI.SafeRelease();
		});
}