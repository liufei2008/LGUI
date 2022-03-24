// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/HudRender/LGUIRenderer.h"
#include "Core/HudRender/LGUIHudShaders.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "Modules/ModuleManager.h"
#include "LGUI.h"
#include "SceneView.h"
#include "Widgets/SWindow.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "MeshPassProcessor.inl"
#include "ScenePrivate.h"
#if WITH_EDITOR
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Slate/SceneViewport.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/LGUISettings.h"
#include "Engine/TextureRenderTarget2D.h"



#if ENGINE_MAJOR_VERSION >= 5

void FLGUIHudRenderer::CopyRenderTarget(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst, TEXT("LGUICopyRenderTarget")), ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LGUICopyRenderTarget"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, GlobalShaderMap, Src, Dst](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

			TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

			VertexShader->SetParameters(RHICmdList);
			PixelShader->SetParameters(RHICmdList, Src);

			DrawFullScreenQuad(RHICmdList);
		});
}

void FLGUIHudRenderer::CopyRenderTargetOnMeshRegion(
	FRDGBuilder& GraphBuilder
	, FRDGTextureRef Dst
	, FTextureRHIRef Src
	, FGlobalShaderMap* GlobalShaderMap
	, const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData
	, const FMatrix44f& MVP
	, const FIntRect& ViewRect
	, const FVector4f& SrcTextureScaleOffset
)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(Dst, ERenderTargetLoadAction::ELoad);
	auto NumSamples = Dst->Desc.NumSamples;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LGUICopyRenderTargetOnMeshRegion"),
		PassParameters,
		ERDGPassFlags::Raster,
		[Src, GlobalShaderMap, RegionVertexData, MVP, ViewRect, SrcTextureScaleOffset, NumSamples](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			TShaderMapRef<FLGUICopyMeshRegionVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUICopyMeshRegionPS> PixelShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessCopyMeshRegionVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = NumSamples;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

			PixelShader->SetParameters(RHICmdList, MVP, SrcTextureScaleOffset, Src);

			uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessCopyMeshRegionVertex);
			FRHIResourceCreateInfo CreateInfo(TEXT("CopyRenderTargetOnMeshRegion"));
			FBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
			void* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
			FPlatformMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
			RHIUnlockBuffer(VertexBufferRHI);

			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);

			VertexBufferRHI.SafeRelease();
		});
}


DECLARE_CYCLE_STAT(TEXT("Hud RHIRender"), STAT_Hud_RHIRender, STATGROUP_LGUI);
void FLGUIHudRenderer::RenderLGUI_RenderThread(
	FRDGBuilder& GraphBuilder
	, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() <= 0 && WorldSpaceRenderCanvasParameterArray.Num() <= 0)return;//nothing to render

	FSceneView RenderView(InView);//use a copied view
	auto GlobalShaderMap = GetGlobalShaderMap(RenderView.GetFeatureLevel());

	//create render target
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef OriginScreenColorTexture = nullptr;//@todo: can use a normal texture here?
	TRefCountPtr<IPooledRenderTarget> OriginScreenColorRenderTarget = nullptr;

	uint8 NumSamples = 1;
	auto ViewRect = RenderView.UnscaledViewRect;
	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
	if (CustomRenderTarget.IsValid())
	{
		ScreenColorRenderTargetTexture = (FTextureRHIRef)CustomRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
		NumSamples = ScreenColorRenderTargetTexture->GetNumSamples();
		//clear render target;
		RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::Clear_DontStore), TEXT("LGUIHudRender_ClearRenderTarget"));
		RHICmdList.EndRenderPass();

		ViewRect = FIntRect(0, 0, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y);
	}
	else
	{
		ScreenColorRenderTargetTexture = (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture();
		NumSamples = RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetNumSamples();

		if (bNeedOriginScreenColorTextureOnPostProcess)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetSizeXY(), RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_ShaderResource, false));
			desc.NumSamples = NumSamples;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenColorRenderTarget, TEXT("LGUISceneColorRenderTarget"));
			if (!OriginScreenColorRenderTarget.IsValid())
				return;
			OriginScreenColorTexture = OriginScreenColorRenderTarget->GetRenderTargetItem().ShaderResourceTexture;
			RHICmdList.CopyTexture(ScreenColorRenderTargetTexture, OriginScreenColorTexture, FRHICopyTextureInfo());
		}
	}
	FRDGTextureRef RenderTargetTexture = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIRendererTargetTexture"));

	//auto RPInfo = FRHIRenderPassInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::Load_DontStore);
	const FMinimalSceneTextures& SceneTextures = FSceneTextures::Get(GraphBuilder);
	FVector4f DepthTextureScaleOffset;
	FVector4f ViewTextureScaleOffset;
	switch (RenderView.StereoPass)
	{
	case EStereoscopicPass::eSSP_FULL:
	{
		DepthTextureScaleOffset = FVector4f(
			(float)ScreenColorRenderTargetTexture->GetSizeXYZ().X / SceneTextures.Depth.Target->Desc.GetSize().X,
			(float)ScreenColorRenderTargetTexture->GetSizeXYZ().Y / SceneTextures.Depth.Target->Desc.GetSize().Y,
			0, 0
		);
		ViewTextureScaleOffset = FVector4f(1, 1, 0, 0);
	}
	break;
	case EStereoscopicPass::eSSP_PRIMARY:
	{
		DepthTextureScaleOffset = FVector4f(
			//(float)ViewRect.Width() / SceneContext.GetSceneDepthSurface()->GetSizeX(),
			//(float)ViewRect.Height() / SceneContext.GetSceneDepthSurface()->GetSizeY(),
			0.5f, 1.0f,
			0, 0
		);
		ViewTextureScaleOffset = DepthTextureScaleOffset;
	}
	break;
	case EStereoscopicPass::eSSP_SECONDARY:
	{
		DepthTextureScaleOffset = FVector4f(
			//(float)ViewRect.Width() / SceneContext.GetSceneDepthSurface()->GetSizeX(),
			//(float)ViewRect.Height() / SceneContext.GetSceneDepthSurface()->GetSizeY(),
			0.5f, 1.0f,
			0.5f, 0
		);
		ViewTextureScaleOffset = DepthTextureScaleOffset;
	}
	break;
	}

	//Render world space
	if (WorldSpaceRenderCanvasParameterArray.Num() > 0)
	{
		auto ViewLocation = RenderView.ViewLocation;
		auto ViewRotationMatrix = FInverseRotationMatrix(RenderView.ViewRotation) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		
		auto ProjectionMatrix = RenderView.SceneViewInitOptions.ProjectionMatrix;
		auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * (ViewRotationMatrix)*ProjectionMatrix;

		RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ProjectionMatrix);

		FViewUniformShaderParameters ViewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			ViewUniformShaderParameters,
			ViewRect.Size(),
			1,
			ViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

		for (int canvasIndex = 0; canvasIndex < WorldSpaceRenderCanvasParameterArray.Num(); canvasIndex++)
		{
			auto& canvasParamItem = WorldSpaceRenderCanvasParameterArray[canvasIndex];

			for (int primitiveIndex = 0; primitiveIndex < canvasParamItem.HudPrimitiveArray.Num(); primitiveIndex++)
			{
				auto hudPrimitive = canvasParamItem.HudPrimitiveArray[primitiveIndex];
				if (hudPrimitive != nullptr)
				{
					if (hudPrimitive->CanRender())
					{
						switch (hudPrimitive->GetPrimitiveType())
						{
						case ELGUIHudPrimitiveType::PostProcess://render post process
						{
							hudPrimitive->OnRenderPostProcess_RenderThread(
								GraphBuilder,
								this,
								OriginScreenColorTexture,
								ScreenColorRenderTargetTexture,
								GlobalShaderMap,
								FMatrix44f(ViewProjectionMatrix),
								true,
								canvasParamItem.BlendDepth,
								ViewRect,
								DepthTextureScaleOffset,
								ViewTextureScaleOffset
							);
						}break;
						case ELGUIHudPrimitiveType::Mesh:
						{
							FLGUIWorldRenderPSParameter* PSShaderParameters = GraphBuilder.AllocParameters<FLGUIWorldRenderPSParameter>();
							PSShaderParameters->SceneDepthTex = SceneTextures.Depth.Target;
							PSShaderParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);

							GraphBuilder.AddPass(
								RDG_EVENT_NAME("LGUI_RenderWorld"),
								PSShaderParameters,
								ERDGPassFlags::Raster,
								[this, hudPrimitive, &InView, ViewRect, PSShaderParameters, SceneDepthTexST = DepthTextureScaleOffset, SceneDepthBlend = canvasParamItem.BlendDepth, NumSamples](FRHICommandListImmediate& RHICmdList)
							{
								FSceneView RenderView(InView);//use a copied view
								auto ViewLocation = RenderView.ViewLocation;
								auto ViewRotationMatrix = FInverseRotationMatrix(RenderView.ViewRotation) * FMatrix(
									FPlane(0, 0, 1, 0),
									FPlane(1, 0, 0, 0),
									FPlane(0, 1, 0, 0),
									FPlane(0, 0, 0, 1));

								auto ProjectionMatrix = RenderView.SceneViewInitOptions.ProjectionMatrix;
								auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * (ViewRotationMatrix)*ProjectionMatrix;

								RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
								RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
								RenderView.UpdateProjectionMatrix(ProjectionMatrix);

								FViewUniformShaderParameters ViewUniformShaderParameters;
								RenderView.SetupCommonViewUniformBufferParameters(
									ViewUniformShaderParameters,
									ViewRect.Size(),
									1,
									ViewRect,
									RenderView.ViewMatrices,
									FViewMatrices()
								);

								RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

								MeshBatchArray.Reset();
								FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
								hudPrimitive->GetMeshElements(*RenderView.Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
								for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
								{
									auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
									const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
									auto Material = &Mesh.MaterialRenderProxy->GetIncompleteMaterialWithFallback(RenderView.GetFeatureLevel());
									const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
									auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();
									auto PixelShader = MaterialShaderMap->GetShader<FLGUIWorldRenderPS>();

									if (VertexShader.IsValid() && PixelShader.IsValid())
									{
										RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

										FGraphicsPipelineStateInitializer GraphicsPSOInit;
										RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

										FLGUIHudRenderer::SetGraphicPipelineState(GraphicsPSOInit, Material->GetBlendMode(), Material->IsWireframe(), Material->IsTwoSided());

										GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
										GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
										GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
										GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
										GraphicsPSOInit.NumSamples = NumSamples;
										SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

										VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetDepthBlendParameter(RHICmdList, SceneDepthBlend, SceneDepthTexST, PSShaderParameters->SceneDepthTex->GetRHI());

										RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
										RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
									}
								}
							});
						}break;
						}
					}
				}
			}
		}
	}
	//Render screen space
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() > 0)
	{
#if WITH_EDITOR
		if (!bCanRenderScreenSpace)goto END_LGUI_RENDER;
		if (bIsPlaying)
		{
			if (!InView.bIsGameView)goto END_LGUI_RENDER;
		}
		else//editor viewport preview
		{
			if (InView.GetViewKey() != EditorPreview_ViewKey)goto END_LGUI_RENDER;//only preview in specific viewport in editor
		}
#endif

		RenderView.SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewOrigin;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);

		FViewUniformShaderParameters ViewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			ViewUniformShaderParameters,
			ViewRect.Size(),
			1,
			ViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.NumSamples = NumSamples;

		for (int primitiveIndex = 0; primitiveIndex < ScreenSpaceRenderParameter.HudPrimitiveArray.Num(); primitiveIndex++)
		{
			auto hudPrimitive = ScreenSpaceRenderParameter.HudPrimitiveArray[primitiveIndex];
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->CanRender())
				{
					switch (hudPrimitive->GetPrimitiveType())
					{
					case ELGUIHudPrimitiveType::PostProcess://render post process
					{
						hudPrimitive->OnRenderPostProcess_RenderThread(
							GraphBuilder,
							this,
							OriginScreenColorTexture,
							ScreenColorRenderTargetTexture,
							GlobalShaderMap,
							FMatrix44f(ScreenSpaceRenderParameter.ViewProjectionMatrix),
							false,
							0.0f,
							ViewRect,
							DepthTextureScaleOffset,
							ViewTextureScaleOffset
						);
					}break;//render mesh
					case ELGUIHudPrimitiveType::Mesh:
					{
						auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
						PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
						GraphBuilder.AddPass(
							RDG_EVENT_NAME("LGUI_RenderScreen"),
							PassParameters,
							ERDGPassFlags::Raster,
							[this, hudPrimitive, &InView, ViewRect, SceneDepthTexST = DepthTextureScaleOffset, NumSamples](FRHICommandListImmediate& RHICmdList)
						{
							FSceneView RenderView(InView);//use a copied view
							RenderView.SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewOrigin;
							RenderView.SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
							RenderView.UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);

							FViewUniformShaderParameters ViewUniformShaderParameters;
							RenderView.SetupCommonViewUniformBufferParameters(
								ViewUniformShaderParameters,
								ViewRect.Size(),
								1,
								ViewRect,
								RenderView.ViewMatrices,
								FViewMatrices()
							);

							RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

							MeshBatchArray.Reset();
							FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
							hudPrimitive->GetMeshElements(*RenderView.Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
							for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
							{
								auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
								const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
								auto Material = &Mesh.MaterialRenderProxy->GetIncompleteMaterialWithFallback(RenderView.GetFeatureLevel());
								const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
								auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();
								auto PixelShader = MaterialShaderMap->GetShader<FLGUIHudRenderPS>();

								if (VertexShader.IsValid() && PixelShader.IsValid())
								{
									RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);
									
									FGraphicsPipelineStateInitializer GraphicsPSOInit;
									RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

									FLGUIHudRenderer::SetGraphicPipelineState(GraphicsPSOInit, Material->GetBlendMode(), Material->IsWireframe(), Material->IsTwoSided());

									GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
									GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
									GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
									GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
									GraphicsPSOInit.NumSamples = NumSamples;
									SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

									VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
									PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);

									RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
									RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
								}
							}
						});
					}break;
					}
				}
			}
		}
	}
#if WITH_EDITOR
	END_LGUI_RENDER :
#endif

	if (OriginScreenColorRenderTarget.IsValid())
	{
		OriginScreenColorRenderTarget.SafeRelease();
	}
}
#endif
