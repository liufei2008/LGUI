// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexVisualPostProcessRenderProxy.h"
#include "LGUI.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "Core/LexUIRender/LexUIVertex.h"
#include "Rendering/Texture2DResource.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "Core/Components/LexVisualPostProcess.h"

FLexVisualPostProcessRenderProxy::FLexVisualPostProcessRenderProxy()
{
	
}

#define SET_PIPELINE_STATE_FOR_CLIP()\
FGraphicsPipelineStateInitializer GraphicsPSOInit;\
RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);\
GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();\
GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();\
GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();\
GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();\
GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();\
GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();\
GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;\
GraphicsPSOInit.NumSamples = NumSamples;\
SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::ForceApply);

void FLexVisualPostProcessRenderProxy::RenderMeshOnScreen_RenderThread(
	FRDGBuilder& GraphBuilder
	, const FMinimalSceneTextures& SceneTextures
	, FTextureRHIRef ScreenTargetTexture
	, FGlobalShaderMap* GlobalShaderMap
	, FTextureRHIRef MeshRegionTexture
	, const FMatrix44f& ModelViewProjectionMatrix
	, const FMatrix44f& ModelMatrix
	, bool IsWorldSpace
	, float BlendDepthForWorld
	, int DepthFadeForWorld
	, const FVector4f& DepthTextureScaleOffset
	, const FIntRect& ViewRect
	, FRHISamplerState* ResultTextureSamplerState
)
{
	uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
	auto PSShaderParameters = GraphBuilder.AllocParameters<FLexUIWorldRenderPSParameter>();
	PSShaderParameters->SceneDepthTex = SceneTextures.Depth.Resolve;
	PSShaderParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LexUIRendererTargetTexture")), ERenderTargetLoadAction::ELoad);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("UIPostProcess_RenderMeshToScreen"),
		PSShaderParameters,
		ERDGPassFlags::Raster,
		[this, PSShaderParameters, GlobalShaderMap, MeshRegionTexture, ModelViewProjectionMatrix, ModelMatrix, IsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect, ResultTextureSamplerState, NumSamples](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			FBufferRHIRef IndexBuffer = nullptr;
			int32 TriangleCount = 2;
			if (MaskTexture != nullptr)
			{
				if (IsWorldSpace)
				{
					if (DepthFadeForWorld <= 0.0f)
					{
						TShaderMapRef<FLexUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
						TShaderMapRef<FLexUIRenderMeshWithMaskWorldPS_Clip> PixelShader(GlobalShaderMap);
						SET_PIPELINE_STATE_FOR_CLIP();
						VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
						PixelShader->SetParameters(RHICmdList, MeshRegionTexture, MaskTexture->TextureRHI
							, ResultTextureSamplerState
							, MaskTexture->SamplerStateRHI
						);
						if (ClipDataTexture != nullptr)
						{
							PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI, ClipDataTexture->SamplerStateRHI);
						}
						PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
					}
					else
					{
						TShaderMapRef<FLexUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
						TShaderMapRef<FLexUIRenderMeshWithMaskWorldDepthFadePS_Clip> PixelShader(GlobalShaderMap);
						SET_PIPELINE_STATE_FOR_CLIP();
						VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
						PixelShader->SetParameters(RHICmdList, MeshRegionTexture, MaskTexture->TextureRHI
							, ResultTextureSamplerState
							, MaskTexture->SamplerStateRHI
						);
						if (ClipDataTexture != nullptr)
						{
							PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI, ClipDataTexture->SamplerStateRHI);
						}
						PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
						PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld, FVector2f(1.0f / ViewRect.Width(), 1.0f / ViewRect.Height()));
					}
				}
				else
				{
					TShaderMapRef<FLexUIRenderMeshVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLexUIRenderMeshWithMaskPS_Clip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, MaskTexture->TextureRHI
						, ResultTextureSamplerState
						, MaskTexture->SamplerStateRHI
					);
					if (ClipDataTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI, ClipDataTexture->SamplerStateRHI);
					}
				}
				IndexBuffer = GLexUIFullScreenQuadIndexBuffer.IndexBufferRHI;
			}
			else
			{
				if (IsWorldSpace)
				{
					if (DepthFadeForWorld <= 0.0f)
					{
						TShaderMapRef<FLexUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
						TShaderMapRef<FLexUIRenderMeshWorldPS_Clip> PixelShader(GlobalShaderMap);
						SET_PIPELINE_STATE_FOR_CLIP();
						VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
						PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
						if (ClipDataTexture != nullptr)
						{
							PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI);
						}
						PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
					}
					else
					{
						TShaderMapRef<FLexUIRenderMeshWorldVS> VertexShader(GlobalShaderMap);
						TShaderMapRef<FLexUIRenderMeshWorldDepthFadePS_Clip> PixelShader(GlobalShaderMap);
						SET_PIPELINE_STATE_FOR_CLIP();
						VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
						PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
						if (ClipDataTexture != nullptr)
						{
							PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI);
						}
						PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepthForWorld, DepthTextureScaleOffset, PSShaderParameters->SceneDepthTex->GetRHI());
						PixelShader->SetDepthFadeParameter(RHICmdList, DepthFadeForWorld, FVector2f(1.0f / ViewRect.Width(), 1.0f / ViewRect.Height()));
					}
				}
				else
				{
					TShaderMapRef<FLexUIRenderMeshVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLexUIRenderMeshPS_Clip> PixelShader(GlobalShaderMap);
					SET_PIPELINE_STATE_FOR_CLIP();
					VertexShader->SetParameters(RHICmdList, ModelViewProjectionMatrix, ModelMatrix);
					PixelShader->SetParameters(RHICmdList, MeshRegionTexture, ResultTextureSamplerState);
					if (ClipDataTexture != nullptr)
					{
						PixelShader->SetClipParameters(RHICmdList, ModelMatrix.Inverse(), ClipDataTexture->TextureRHI);
					}
				}
				IndexBuffer = GLexUIFullScreenQuadIndexBuffer.IndexBufferRHI;
			}

			uint32 VertexBufferSize = RenderMeshRegionToScreenVertexArray.Num() * sizeof(FLexUIPostProcessVertex);
			FRHIResourceCreateInfo CreateInfo(TEXT("RenderMeshOnScreen"));
			FBufferRHIRef VertexBufferRHI = RHICmdList.CreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
			void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
			FPlatformMemory::Memcpy(VoidPtr, RenderMeshRegionToScreenVertexArray.GetData(), VertexBufferSize);
			RHICmdList.UnlockBuffer(VertexBufferRHI);
			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(IndexBuffer, 0, 0, RenderMeshRegionToScreenVertexArray.Num(), 0, TriangleCount, 1);
			VertexBufferRHI.SafeRelease();
		});
}