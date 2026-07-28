// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIRender/LexUIRenderer.h"
#include "Core/LexUIRender/LexUIShaders.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "Core/LexUIRender/LexUIResolveShaders.h"
#include "LGUI.h"
#include "SceneView.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/LexUIRender/ILexUIRendererPrimitive.h"
#include "Core/Components/LexCanvas.h"
#include "MeshPassProcessor.inl"
#include "ScenePrivate.h"
#include "TextureResource.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "SceneTextures.h"
#if WITH_EDITOR
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Core/LexUISettings.h"
#include "ClearQuad.h"
#include "RHIResourceUtils.h"
#include "Core/LexUIMeshVertex.h"
#include "Core/LexUIMesh/LexUIGizmoMesh.h"
#include "Core/LexUIRender/LexUIPostProcessVertex.h"


#if WITH_EDITORONLY_DATA
#endif
FLexUIRenderer::FLexUIRenderer(const FAutoRegister& AutoRegister, UWorld* InWorld, ELexUIRendererType InRendererType)
	:FSceneViewExtensionBase(AutoRegister)
{
	World = InWorld;
	RendererType = InRendererType;

#if WITH_EDITORONLY_DATA
	bIsEditorPreview = !World->IsGameWorld();
#endif
}
FLexUIRenderer::~FLexUIRenderer()
{
	
}

void FLexUIRenderer::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!World.IsValid())return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
	
	if (ScreenSpaceRenderParameter.RootCanvas.IsValid())
	{
		//@todo: these parameters should use ENQUEUE_RENDER_COMMAND to pass to render thread
		auto ViewLocation = ScreenSpaceRenderParameter.RootCanvas->GetViewLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(ScreenSpaceRenderParameter.RootCanvas->GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ProjectionMatrix = ScreenSpaceRenderParameter.RootCanvas->GetProjectionMatrix();
		auto ViewProjectionMatrix = FMatrix44f(FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix);

		ScreenSpaceRenderParameter.ViewOrigin = ViewLocation;
		ScreenSpaceRenderParameter.ViewRotationMatrix = ViewRotationMatrix;
		ScreenSpaceRenderParameter.ProjectionMatrix = ProjectionMatrix;
		ScreenSpaceRenderParameter.ViewProjectionMatrix = FMatrix44f(ViewProjectionMatrix);
		ScreenSpaceRenderParameter.bEnableDepthTest = ScreenSpaceRenderParameter.RootCanvas->GetEnableDepthTest();
	}

	if (auto LexUISettings = GetDefault<ULexUISettings>())
	{
		NumSamples_MSAA = LexUISettings->AntiAliasingMethod == ELexUIRendererAntiAliasingMethod::MSAA ? (uint8)LexUISettings->MSAASampleCount : 1;
		bFrustumCulling = LexUISettings->bFrustumCulling;
	}
	else
	{
		NumSamples_MSAA = 1;
	}
}
void FLexUIRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{

}
void FLexUIRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}
void FLexUIRenderer::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{

}
void FLexUIRenderer::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	RenderLexUI_RenderThread(GraphBuilder, InView);
}
int32 FLexUIRenderer::GetPriority() const
{
#if WITH_EDITOR
	auto Priority = ULexUISettings::GetPriorityInSceneViewExtension();
#else
	static auto Priority = ULexUISettings::GetPriorityInSceneViewExtension();
#endif
	return Priority;
}
bool FLexUIRenderer::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
	if (!World.IsValid())return false;
#if WITH_EDITOR
	if (GEngine == nullptr) return false;
	bCanRenderScreenSpace = true;
	bIsPlaying = World.Get()->IsGameWorld();
	//check if simulation
	if (UEditorEngine* editor = Cast<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)bCanRenderScreenSpace = false;
	}

	if (bIsPlaying == bIsEditorPreview)bCanRenderScreenSpace = false;
#endif

	if (World.Get() != Context.GetWorld())return false;//only render self world
	return true;
}
void FLexUIRenderer::PreRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	
}
void FLexUIRenderer::PostRenderBasePassDeferred_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView, const FRenderTargetBindingSlots& RenderTargets, TRDGUniformBufferRef<FSceneTextureUniformParameters> SceneTextures)
{

}

void FLexUIRenderer::CopyRenderTarget(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst
	, FRHISamplerState* SrcTextureSamplerState
)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst, TEXT("LexUICopyRenderTarget")), ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUICopyRenderTarget"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, GlobalShaderMap, Src, Dst, SrcTextureSamplerState](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

			TShaderMapRef<FLexUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			TShaderMapRef<FLexUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
			PixelShader->SetParameters(RHICmdList, Src, SrcTextureSamplerState);
			VertexShader->SetParameters(RHICmdList);

			DrawFullScreenQuad(RHICmdList);
		});
}

void FLexUIRenderer::CopyRenderTarget_ColorCorrect(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap,
	FTextureRHIRef Src, FTextureRHIRef Dst, FRHISamplerState* SrcTextureSamplerState)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst, TEXT("LexUICopyRenderTarget_ColorCorrect")), ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUICopyRenderTarget_ColorCorrect"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, GlobalShaderMap, Src, Dst, SrcTextureSamplerState](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

			TShaderMapRef<FLexUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			TShaderMapRef<FLexUISimpleCopyTargetPS_ColorCorrect> PixelShader(GlobalShaderMap);
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
			PixelShader->SetParameters(RHICmdList, Src, SrcTextureSamplerState);
			VertexShader->SetParameters(RHICmdList);

			DrawFullScreenQuad(RHICmdList);
		});
}

void FLexUIRenderer::CopyRenderTarget_BlendAlpha(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap,
                                                 FTextureRHIRef Src, FTextureRHIRef Dst, float BlendAlpha, FRHISamplerState* SrcTextureSamplerState)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst, TEXT("LexUICopyRenderTarget_BlendAlpha")), ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUICopyRenderTarget_BlendAlpha"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, GlobalShaderMap, Src, Dst, SrcTextureSamplerState, BlendAlpha](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);


			TShaderMapRef<FLexUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();

			TShaderMapRef<FLexUISimpleCopyTargetPS_BlendAlpha> PixelShader(GlobalShaderMap);
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
			PixelShader->SetParameters(RHICmdList, Src, SrcTextureSamplerState);
			PixelShader->SetBlendAlpha(RHICmdList, BlendAlpha);

			VertexShader->SetParameters(RHICmdList);

			DrawFullScreenQuad(RHICmdList);
		});
}

void FLexUIRenderer::CopyRenderTargetOnMeshRegion(
	FRDGBuilder& GraphBuilder
	, FRDGTextureRef Dst
	, FTextureRHIRef Src
	, FGlobalShaderMap* GlobalShaderMap
	, const TArray<FLexUIPostProcessCopyMeshRegionVertex>& RegionVertexData
	, const FMatrix44f& MVP
	, bool bIsRenderTarget
	, const FIntRect& ViewRect
	, const FVector4f& SrcTextureScaleOffset
	, bool ColorCorrect
)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(Dst, ERenderTargetLoadAction::ELoad);
	auto NumSamples = Dst->Desc.NumSamples;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUICopyRenderTargetOnMeshRegion"),
		PassParameters,
		ERDGPassFlags::Raster,
		[Src, GlobalShaderMap, RegionVertexData, MVP, bIsRenderTarget, ViewRect, SrcTextureScaleOffset, NumSamples, ColorCorrect](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			TShaderMapRef<FLexUICopyMeshRegionVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessCopyMeshRegionVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = NumSamples;
			if (ColorCorrect)
			{
				TShaderMapRef<FLexUICopyMeshRegionPS_ColorCorrect> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

				PixelShader->SetParameters(RHICmdList, MVP, bIsRenderTarget, Src, SrcTextureScaleOffset);
			}
			else
			{
				TShaderMapRef<FLexUICopyMeshRegionPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

				PixelShader->SetParameters(RHICmdList, MVP, bIsRenderTarget, Src, SrcTextureScaleOffset);
			}
			
			FBufferRHIRef VertexBufferRHI = UE::RHIResourceUtils::CreateVertexBufferFromArray(
			RHICmdList, TEXT("CopyRenderTargetOnMeshRegion"), EBufferUsageFlags::Volatile, MakeConstArrayView(RegionVertexData)
			);

			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GLexUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);

			VertexBufferRHI.SafeRelease();
		});
}

void FLexUIRenderer::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	RHICmdList.SetStreamSource(0, GLexUIFullScreenQuadVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLexUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
}
void FLexUIRenderer::SetGraphicPipelineState_BlendDepthStencilRasterize(ERHIFeatureLevel::Type FeatureLevel, FGraphicsPipelineStateInitializer& GraphicsPSOInit, EBlendMode BlendMode
	, bool bIsWireFrame, bool bIsTwoSided, bool bDisableDepthTestForTransparent, bool bIsDepthValid, bool bReverseCulling
) 
{
	switch (BlendMode)
	{
	default:
	case BLEND_Opaque:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Masked:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Translucent:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
		break;
	case BLEND_Additive:
		// Add to the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		break;
	case BLEND_Modulate:
		// Modulate with the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_Zero, BF_SourceColor>::GetRHI();
		break;
	case BLEND_AlphaComposite:
		// Blend with existing scene color. New color is already pre-multiplied by alpha.
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		break;
	case BLEND_AlphaHoldout:
		// Blend by holding out the matte shape of the source alpha
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		break;
	};

	if (bIsDepthValid)
	{
		if (BlendMode == BLEND_Opaque || BlendMode == BLEND_Masked)
		{
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<true, ECompareFunction::CF_GreaterEqual>::GetRHI();
		}
		else
		{
			if (bDisableDepthTestForTransparent)
			{
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_GreaterEqual>::GetRHI();
			}
		}
	}
	else
	{
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	}
	
#if PLATFORM_ANDROID
	auto ShaderPlatform = GShaderPlatformForFeatureLevel[FeatureLevel];
	if (ShaderPlatform == EShaderPlatform::SP_OPENGL_ES3_1_ANDROID)
	{
		bReverseCulling = !bReverseCulling;//android gles is flipped
	}
#endif
	if (!bIsWireFrame)
	{
		if (bIsTwoSided)
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
		}
		else
		{
			if (bReverseCulling)
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CCW>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CW>::GetRHI();
			}
		}
	}
	else
	{
		if (bIsTwoSided)
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_None>::GetRHI();
		}
		else
		{
			if (bReverseCulling)
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_CCW>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_CW>::GetRHI();
			}
		}
	}
}

#ifndef LGUI_ENABLE_SCENETEXTURES
#define LGUI_ENABLE_SCENETEXTURES 0//not render in clean project, so disable it
#endif

DECLARE_CYCLE_STAT(TEXT("LexUI RHIRenderMesh"), STAT_LGUI_RHIRenderMesh, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LexUI RHIRenderPostProcess"), STAT_LGUI_RHIRenderPostProcess, STATGROUP_LGUI);
void FLexUIRenderer::RenderLexUI_RenderThread(
	FRDGBuilder& GraphBuilder
	, FSceneView& InView)
{
	if (ScreenSpaceRenderParameter.PrimitiveArray.Num() <= 0 && WorldSpaceRenderCanvasParameterArray.Num() <= 0
#if WITH_EDITOR
		&& ScreenSpaceGizmoMeshArray.Num() <= 0
		&& WorldSpaceGizmoMeshArray.Num() <= 0
#endif
		)return;//nothing to render
	bool bIsMainViewport = !(InView.bIsSceneCapture || InView.bIsReflectionCapture || InView.bIsPlanarReflection || InView.bIsVirtualTexture);
	
	bool bRenderWireframe = InView.Family->ViewMode == VMI_Wireframe || InView.Family->ViewMode == VMI_Lit_Wireframe;
	bool bRenderLit = InView.Family->ViewMode != VMI_Wireframe;
	FMaterialRenderProxy* WireframeMaterialInstance = NULL;
	if (bRenderWireframe)
	{
		WireframeMaterialInstance = GEngine->WireframeMaterial->GetRenderProxy();
	}

	FTextureRHIRef OrignScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	//msaa render target
	TRefCountPtr<IPooledRenderTarget> MSAARenderTarget = nullptr;

	uint8 NumSamples = NumSamples_MSAA;
	FIntRect ViewRect;
	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
	FVector4f DepthTextureScaleOffset;
	FVector4f ColorTextureScaleOffset;
	if (RendererType == ELexUIRendererType::RenderTarget)//render-target mode
	{
		if (!bIsMainViewport)//render to scene capture (or other capture)
		{
			return;
		}
		if (RenderTargetResource != nullptr && RenderTargetResource->GetRenderTargetTexture() != nullptr)
		{
			ScreenColorRenderTargetTexture = RenderTargetResource->GetRenderTargetTexture();
			if (ScreenColorRenderTargetTexture == nullptr)return;//invalid render target

			if (NumSamples > 1)
			{
				//get msaa render target
				FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenColorRenderTargetTexture->GetSizeXY(), ScreenColorRenderTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
				desc.NumSamples = NumSamples;
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, MSAARenderTarget, TEXT("LexUI_MSAA_RenderTarget"));
				if (!MSAARenderTarget.IsValid())
					return;

				OrignScreenColorRenderTargetTexture = ScreenColorRenderTargetTexture;
				ScreenColorRenderTargetTexture = MSAARenderTarget->GetRHI();
			}

			//clear render target
			{
				auto Parameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
				Parameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LexUIRender_ClearRenderTarget")), ERenderTargetLoadAction::ENoAction);
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LexUIRender_ClearRenderTarget"),
					Parameters,
					ERDGPassFlags::Raster,
					[ClearColor = RenderTargetClearColor](FRHICommandListImmediate& RHICmdList)
					{
						DrawClearQuad(RHICmdList, FLinearColor(ClearColor));
					}
				);
			}
			RenderTargetResource = nullptr;

			ViewRect = FIntRect(0, 0, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y);
		}
		else
		{
			return;
		}

		ColorTextureScaleOffset = DepthTextureScaleOffset = FVector4f(1, 1, 0, 0);
	}
	else//world space or screen space mode
	{
		ScreenColorRenderTargetTexture = InView.Family->RenderTarget->GetRenderTargetTexture();
		if (ScreenColorRenderTargetTexture == nullptr)return;//invalid render target

		if (NumSamples > 1)
		{
			//get msaa render target
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenColorRenderTargetTexture->GetSizeXY(), ScreenColorRenderTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = NumSamples;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, MSAARenderTarget, TEXT("LexUI_MSAA_RenderTarget"));
			if (!MSAARenderTarget.IsValid())
				return;

			CopyRenderTarget(GraphBuilder, GetGlobalShaderMap(InView.GetFeatureLevel()), ScreenColorRenderTargetTexture, MSAARenderTarget->GetRHI());
			OrignScreenColorRenderTargetTexture = ScreenColorRenderTargetTexture;
			ScreenColorRenderTargetTexture = MSAARenderTarget->GetRHI();
		}

		ViewRect = InView.UnscaledViewRect;
		float ScreenPercentage = 1.0f;//this can affect scale on depth texture
		if (InView.bIsViewInfo)
		{
			auto& ViewInfo = static_cast<FViewInfo&>(InView);
			ScreenPercentage = (float)ViewInfo.ViewRect.Width() / ViewRect.Width();
		}
		else
		{
			ScreenPercentage = 1.0f;
		}
		const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)InView.Family)->GetSceneTextures();
		switch (InView.StereoPass)
		{
		case EStereoscopicPass::eSSP_FULL:
		{
			DepthTextureScaleOffset = FVector4f(
				(float)ScreenColorRenderTargetTexture->GetSizeXYZ().X / SceneTextures.Depth.Resolve->Desc.GetSize().X,
				(float)ScreenColorRenderTargetTexture->GetSizeXYZ().Y / SceneTextures.Depth.Resolve->Desc.GetSize().Y,
				0, 0
			);
			DepthTextureScaleOffset = DepthTextureScaleOffset * ScreenPercentage;
			ColorTextureScaleOffset = FVector4f(1, 1, 0, 0);
		}
		break;
		case EStereoscopicPass::eSSP_PRIMARY:
		{
			DepthTextureScaleOffset = FVector4f(
				(float)ViewRect.Width() / SceneTextures.Depth.Resolve->Desc.GetSize().X,//normally ViewRect.Width is half of screen size
				(float)ViewRect.Height() / SceneTextures.Depth.Resolve->Desc.GetSize().Y,
				0, 0
			);
			DepthTextureScaleOffset = DepthTextureScaleOffset * ScreenPercentage;
			ColorTextureScaleOffset = FVector4f(0.5f, 1, 0, 0);
		}
		break;
		case EStereoscopicPass::eSSP_SECONDARY:
		{
			DepthTextureScaleOffset = FVector4f(
				(float)ViewRect.Width() / SceneTextures.Depth.Resolve->Desc.GetSize().X,
				(float)ViewRect.Height() / SceneTextures.Depth.Resolve->Desc.GetSize().Y,
				0, 0
			);
			DepthTextureScaleOffset = DepthTextureScaleOffset * ScreenPercentage;
			DepthTextureScaleOffset.Z = 0.5f;//right eye offset 0.5
			ColorTextureScaleOffset = FVector4f(0.5f, 1, 0.5f, 0);
		}
		break;
		}
	}

	FRDGTextureRef RenderTargetTexture = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LexUIRendererTargetTexture"));
	const float EngineGamma = GEngine ? GEngine->GetDisplayGamma() : 2.2f;
	float GammaValue =
		(RendererType == ELexUIRendererType::RenderTarget || !bIsMainViewport) ? 1.0f : EngineGamma;

	//Render world space
	if (WorldSpaceRenderCanvasParameterArray.Num() > 0
#if WITH_EDITOR
		|| WorldSpaceGizmoMeshArray.Num() > 0
#endif
		)
	{
		//collect render primitive to a sequence
		struct FWorldSpaceRenderParameterSequence
		{
			TArray<FLexUIPrimitiveDataContainer> RenderDataArray;
			//blend depth, 0-occlude by depth, 1-all visible
			float BlendDepth = 0.0f;
			//depth fade effect
			int DepthFade = 0;

			//for sort translucent
			FVector3f WorldPosition;
			//distance to camera (square)
			float DistToCamera = 0;
			int RenderPriority = 0;
		};
		TArray<FWorldSpaceRenderParameterSequence> RenderSequenceArray;
		for (auto& WorldRenderParameter : WorldSpaceRenderCanvasParameterArray)
		{
			if (WorldRenderParameter.Primitive->LexUI_CanRender())
			{
				bool bIsPrimitiveVisible = false;//default is not visible
				if (InView.ShowOnlyPrimitives.IsSet())
				{
					bIsPrimitiveVisible = InView.ShowOnlyPrimitives.GetValue().Contains(WorldRenderParameter.Primitive->LexUI_GetPrimitiveComponentId());
				}
				else
				{
					bIsPrimitiveVisible = !InView.HiddenPrimitives.Contains(WorldRenderParameter.Primitive->LexUI_GetPrimitiveComponentId());
				}
				if (bIsPrimitiveVisible)
				{
					auto WorldBounds = WorldRenderParameter.Primitive->LexUI_GetWorldBounds();
					if (!bFrustumCulling 
						|| (bFrustumCulling && InView.GetCullingFrustum().IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent))//simple View Frustum Culling
						)
					{
						FWorldSpaceRenderParameterSequence Item;
						WorldRenderParameter.Primitive->LexUI_CollectRenderData(Item.RenderDataArray);
						if (Item.RenderDataArray.Num() > 0)
						{
							Item.BlendDepth = WorldRenderParameter.BlendDepth;
							Item.DepthFade = WorldRenderParameter.DepthFade;
							Item.WorldPosition = WorldRenderParameter.Primitive->LexUI_GetWorldPositionForSortTranslucent();
							Item.RenderPriority = WorldRenderParameter.Primitive->LexUI_GetRenderPriority();
							RenderSequenceArray.Add(Item);
						}
					}
				}
			}
		}
		if (RenderSequenceArray.Num() > 0
#if WITH_EDITOR
			|| WorldSpaceGizmoMeshArray.Num() > 0
#endif
			)
		{
			//use a copied view. 
			//NOTE!!! world-space and screen-space must use different 'RenderView' (actually different ViewUniformBuffer), because RDG is async. 
			//if use same one, after world-space when modify 'RenderView' for screen-space, the screen-space ViewUniformBuffer will be applyed to world-space
			FSceneView* RenderView = new FSceneView(InView);
			auto GlobalShaderMap = GetGlobalShaderMap(RenderView->GetFeatureLevel());

			const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)InView.Family)->GetSceneTextures();

			RenderView->ViewMatrices = InView.ViewMatrices;
			RenderView->ViewMatrices.HackRemoveTemporalAAProjectionJitter();
			auto ViewProjectionMatrix = FMatrix44f(RenderView->ViewMatrices.GetViewProjectionMatrix());

			FViewUniformShaderParameters ViewUniformShaderParameters;
			RenderView->SetupCommonViewUniformBufferParameters(
				ViewUniformShaderParameters,
				ViewRect.Size(),
				1,
				ViewRect,
				RenderView->ViewMatrices,
				FViewMatrices()
			);

			RenderView->ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

			//if (bNeedSortWorldSpaceRenderCanvas)//@todo: mark dirty when need to
			{
				bNeedSortWorldSpaceRenderCanvas = false;

				auto InViewPosition = FVector3f(RenderView->ViewMatrices.GetViewOrigin());
				for (auto& Item : RenderSequenceArray)
				{
					Item.DistToCamera = FVector3f::DistSquared(InViewPosition, Item.WorldPosition);
				}
				RenderSequenceArray.Sort([InViewPosition](const FWorldSpaceRenderParameterSequence& A, const FWorldSpaceRenderParameterSequence& B) {
					if (A.RenderPriority == B.RenderPriority)
					{
						return A.DistToCamera > B.DistToCamera;
					}
					else
					{
						return A.RenderPriority < B.RenderPriority;
					}
					});
			}

			for (auto& RenderSequenceItem : RenderSequenceArray)
			{
				for (auto& RenderPrimitiveItem : RenderSequenceItem.RenderDataArray)
				{
					switch (RenderPrimitiveItem.Type)
					{
					case ELexUIRendererPrimitiveType::PostProcess://render post process
						{
							for (int i = 0; i < RenderPrimitiveItem.Sections.Num(); i++)
							{
								if (auto Primitive = RenderPrimitiveItem.Primitive->LexUI_GetPostProcessElement(RenderPrimitiveItem.Sections[i].SectionPointer))
								{
									SCOPE_CYCLE_COUNTER(STAT_LGUI_RHIRenderPostProcess);
									Primitive->OnRenderPostProcess_RenderThread(
										GraphBuilder,
										SceneTextures,
										this,
										ScreenColorRenderTargetTexture,
										GlobalShaderMap,
										ViewProjectionMatrix,
										true,
										false,
										RenderSequenceItem.BlendDepth,
										RenderSequenceItem.DepthFade,
										ViewRect,
										DepthTextureScaleOffset,
										ColorTextureScaleOffset
									);
								}
							}
						}
						break;
					case ELexUIRendererPrimitiveType::Mesh://render mesh
						{
							auto* PassParameters = GraphBuilder.AllocParameters<FLexUIWorldRenderPSParameter>();
							PassParameters->SceneDepthTex = SceneTextures.Depth.Resolve;
							PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);

							GraphBuilder.AddPass(
								RDG_EVENT_NAME("LexUIRender_WorldSpace"),
								PassParameters,
								ERDGPassFlags::Raster,
								[this, DepthFade = RenderSequenceItem.DepthFade, BlendDepth = RenderSequenceItem.BlendDepth
									, RenderPrimitiveItem, RenderView, ViewRect, PassParameters
									, SceneDepthTexST = DepthTextureScaleOffset, NumSamples, GammaValue
									, bRenderWireframe, bRenderLit, WireframeMaterialInstance](FRHICommandListImmediate& RHICmdList)
								{
									SCOPE_CYCLE_COUNTER(STAT_LGUI_RHIRenderMesh);
									FGraphicsPipelineStateInitializer GraphicsPSOInit;
									RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
									RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

									MeshBatchArray.Reset();
									FSceneRenderingBulkObjectAllocator Allocator;
									FLexUIMeshElementCollector MeshCollector(RenderView->GetFeatureLevel(), Allocator, RHICmdList);
									RenderPrimitiveItem.Primitive->LexUI_GetMeshElements(*RenderView->Family, MeshCollector, RenderPrimitiveItem, MeshBatchArray);
									for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
									{
										auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
										const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
		#if LGUI_ENABLE_SCENETEXTURES
										FRHIUniformBuffer* SceneTextureUniformBuffer = GetSceneTextureExtracts().GetUniformBuffer();
										if (!SceneTextureUniformBuffer)return;
										const FUniformBufferStaticBindings StaticUniformBuffers(SceneTextureUniformBuffer);
										SCOPED_UNIFORM_BUFFER_STATIC_BINDINGS(RHICmdList, StaticUniformBuffers);
		#endif

										auto DoRender = [&](bool bWireframe)
										{
											auto MaterialRenderProxy = (bWireframe ? WireframeMaterialInstance : Mesh.MaterialRenderProxy);
											auto Material = MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material can't render with LexUIRenderer
											if (!Material)return;
											
											if (DepthFade <= 0)
											{
												FMaterialShaderTypes ShaderTypes;
												ShaderTypes.AddShaderType<FLexUIScreenRenderVS>();
												ShaderTypes.AddShaderType<FLexUIWorldRenderPS>();
												FMaterialShaders Shaders;
												if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
												{
													TShaderRef<FLexUIScreenRenderVS> VertexShader;
													TShaderRef<FLexUIWorldRenderPS> PixelShader;
													Shaders.TryGetVertexShader(VertexShader);
													Shaders.TryGetPixelShader(PixelShader);

													FLexUIRenderer::SetGraphicPipelineState_BlendDepthStencilRasterize(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
													, Material->IsWireframe() || bWireframe, Material->IsTwoSided(), Material->ShouldDisableDepthTest(), false, Mesh.ReverseCulling
													);
													GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIMeshVertexDeclaration();
													GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
													GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
													GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
													GraphicsPSOInit.NumSamples = NumSamples;
													SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

													VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
													PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
													PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
													PixelShader->SetGammaValue(RHICmdList, GammaValue);

													RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
													RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
												}
											}
											else
											{
												FMaterialShaderTypes ShaderTypes;
												ShaderTypes.AddShaderType<FLexUIScreenRenderVS>();
												ShaderTypes.AddShaderType<FLexUIWorldRenderDepthFadePS>();
												FMaterialShaders Shaders;
												if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
												{
													TShaderRef<FLexUIScreenRenderVS> VertexShader;
													TShaderRef<FLexUIWorldRenderDepthFadePS> PixelShader;
													Shaders.TryGetVertexShader(VertexShader);
													Shaders.TryGetPixelShader(PixelShader);

													FLexUIRenderer::SetGraphicPipelineState_BlendDepthStencilRasterize(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
													, Material->IsWireframe() || bWireframe, Material->IsTwoSided(), Material->ShouldDisableDepthTest(), false, Mesh.ReverseCulling
													);
													GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIMeshVertexDeclaration();
													GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
													GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
													GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
													GraphicsPSOInit.NumSamples = NumSamples;
													SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

													VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
													PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
													PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
													PixelShader->SetDepthFadeParameter(RHICmdList, DepthFade);
													PixelShader->SetGammaValue(RHICmdList, GammaValue);

													RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
													RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
												}
											}
										};
										if (bRenderLit)
										{
											DoRender(false);
										}
										if (bRenderWireframe)
										{
											DoRender(true);
										}
									}
								});
						}break;
					}
				}
			}
#if WITH_EDITOR
			RenderGizmoMesh_RenderThread(WorldSpaceGizmoMeshArray, GraphBuilder, RenderView, ViewRect, NumSamples, RenderTargetTexture);
#endif
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("LexUI_RenderWorld_Clean"),
				ERDGPassFlags::None,
				[RenderView](FRHICommandListImmediate& RHICmdList)
				{
					RenderView->ViewUniformBuffer.SafeRelease();
					delete RenderView;
				});
		}
	}

	//Render screen space
	if ((ScreenSpaceRenderParameter.PrimitiveArray.Num() > 0
#if WITH_EDITOR
		|| ScreenSpaceGizmoMeshArray.Num() > 0
#endif
		)
		&& bIsMainViewport
		)
	{
		if (ScreenSpaceRenderParameter.bNeedSortRenderPriority)
		{
			ScreenSpaceRenderParameter.bNeedSortRenderPriority = false;
			SortScreenSpacePrimitiveRenderPriority_RenderThread();
		}
#if WITH_EDITOR
		if (RendererType == ELexUIRendererType::RenderTarget)
		{

		}
		else
		{
			if (!bCanRenderScreenSpace)goto END_LEXUI_RENDER;
			if (bIsPlaying)
			{
				if (!InView.bIsGameView)goto END_LEXUI_RENDER;
			}
			else
			{
				goto END_LEXUI_RENDER;
			}
		}
#endif
		TRefCountPtr<IPooledRenderTarget> LexUIScreenSpaceDepthTexture = nullptr;
		FRDGTextureRef LexUIScreenSpaceDepthRDGTexture = nullptr;
		if (ScreenSpaceRenderParameter.bEnableDepthTest)
		{
			// Allow UAV depth?
			const ETextureCreateFlags textureUAVCreateFlags = GRHISupportsDepthUAV ? TexCreate_UAV : TexCreate_None;

			// Create a texture to store the resolved scene depth, and a render-targetable surface to hold the unresolved scene depth.
			FPooledRenderTargetDesc Desc(FPooledRenderTargetDesc::Create2DDesc(
				InView.Family->RenderTarget->GetRenderTargetTexture()->GetSizeXY()
				, EPixelFormat::PF_DepthStencil
				, FClearValueBinding::DepthFar
				, TexCreate_None, TexCreate_DepthStencilTargetable | textureUAVCreateFlags
				, false
			));
			Desc.NumSamples = NumSamples;
			Desc.ArraySize = 1;
			Desc.Flags |= TexCreate_Memoryless;

			GRenderTargetPool.FindFreeElement(RHICmdList, Desc, LexUIScreenSpaceDepthTexture, TEXT("LexUIScreenSpaceDepthTexture"));
			LexUIScreenSpaceDepthRDGTexture = RegisterExternalTexture(GraphBuilder, LexUIScreenSpaceDepthTexture->GetRHI(), TEXT("LexUIRendererTargetTexture"));
		}

		//use a copied view. 
		//NOTE!!! world-space and screen-space must use different 'RenderView' (actually different ViewUniformBuffer), because RDG is not immediately execute. 
		//if use same one, after world-space when modify 'RenderView' for screen-space, the screen-space ViewUniformBuffer will be applyed to world-space
		FSceneView* RenderView = new FSceneView(InView);
		auto GlobalShaderMap = GetGlobalShaderMap(RenderView->GetFeatureLevel());

		RenderView->SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewOrigin;
		RenderView->SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
		RenderView->SceneViewInitOptions.ProjectionMatrix = ScreenSpaceRenderParameter.ProjectionMatrix;
		RenderView->ViewMatrices = FViewMatrices(RenderView->SceneViewInitOptions);
		if (bFrustumCulling)
		{
			RenderView->UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);//this is mainly for ViewFrustum
		}

		FViewUniformShaderParameters ViewUniformShaderParameters;
		RenderView->SetupCommonViewUniformBufferParameters(
			ViewUniformShaderParameters,
			ViewRect.Size(),
			1,
			ViewRect,
			RenderView->ViewMatrices,
			FViewMatrices()
		);

		RenderView->ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);
		
		//collect render primitive to a sequence
		TArray<FLexUIPrimitiveDataContainer> RenderSequenceArray;
		for (auto Primitive : ScreenSpaceRenderParameter.PrimitiveArray)
		{
			if (Primitive->LexUI_CanRender())
			{
				auto WorldBounds = Primitive->LexUI_GetWorldBounds();
				if (!bFrustumCulling 
					|| (bFrustumCulling && RenderView->GetCullingFrustum().IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent))//simple View Frustum Culling
					)
				{
					Primitive->LexUI_CollectRenderData(RenderSequenceArray);
				}
			}
		}
		
		const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)InView.Family)->GetSceneTextures();
		bool bIsDepthStencilCleared = false;
		bool bIsRenderTarget = RendererType == ELexUIRendererType::RenderTarget;
		for (auto& RenderSequenceItem : RenderSequenceArray)
		{
			switch (RenderSequenceItem.Type)
			{
			case ELexUIRendererPrimitiveType::PostProcess://render post process
			{
				for (int i = 0; i < RenderSequenceItem.Sections.Num(); i++)
				{
					if (auto Primitive = RenderSequenceItem.Primitive->LexUI_GetPostProcessElement(RenderSequenceItem.Sections[i].SectionPointer))
					{
						SCOPE_CYCLE_COUNTER(STAT_LGUI_RHIRenderPostProcess);
						Primitive->OnRenderPostProcess_RenderThread(
							GraphBuilder,
							SceneTextures,
							this,
							ScreenColorRenderTargetTexture,
							GlobalShaderMap,
							ScreenSpaceRenderParameter.ViewProjectionMatrix,
							/*IsWorldSpace*/false,
							/*IsRenderToRenderTarget*/bIsRenderTarget,
							/*BlendDepthForWorld*/0.0f,//actually this value will not work because 'IsWorldSpace' is false
							/*BlendDepthForWorld*/0.0f,//actually this value will not work because 'IsWorldSpace' is false
							ViewRect,
							DepthTextureScaleOffset,
							ColorTextureScaleOffset
						);
					}
				}
			}
			break;
			case ELexUIRendererPrimitiveType::Mesh:
			{
				auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
				PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
				if (LexUIScreenSpaceDepthRDGTexture != nullptr)
				{
					if (bIsDepthStencilCleared)
					{
						PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(LexUIScreenSpaceDepthRDGTexture, ERenderTargetLoadAction::ENoAction, ERenderTargetLoadAction::ENoAction, FExclusiveDepthStencil::DepthWrite_StencilWrite);
					}
					else
					{
						bIsDepthStencilCleared = true;//only clear depth stencil when first use depth texture
						PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(LexUIScreenSpaceDepthRDGTexture, ERenderTargetLoadAction::EClear, ERenderTargetLoadAction::EClear, FExclusiveDepthStencil::DepthWrite_StencilWrite);
					}
				}
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LexUIRender_ScreenSpace"),
					PassParameters,
					ERDGPassFlags::Raster,
					[this, RenderSequenceItem, RenderView, ViewRect, SceneDepthTexST = DepthTextureScaleOffset
						, NumSamples, ValidDepth = LexUIScreenSpaceDepthRDGTexture != nullptr, GammaValue
						, bRenderLit, bRenderWireframe, WireframeMaterialInstance](FRHICommandListImmediate& RHICmdList)
					{
						SCOPE_CYCLE_COUNTER(STAT_LGUI_RHIRenderMesh);
						FGraphicsPipelineStateInitializer GraphicsPSOInit;
						RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
						RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);
						MeshBatchArray.Reset();
						FSceneRenderingBulkObjectAllocator Allocator;
						FLexUIMeshElementCollector MeshCollector(RenderView->GetFeatureLevel(), Allocator, RHICmdList);
						RenderSequenceItem.Primitive->LexUI_GetMeshElements(*RenderView->Family, MeshCollector,
						RenderSequenceItem, MeshBatchArray);

						for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
						{
							auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
							const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
							
#if LGUI_ENABLE_SCENETEXTURES
							FRHIUniformBuffer* SceneTextureUniformBuffer = GetSceneTextureExtracts().GetUniformBuffer();
							if (!SceneTextureUniformBuffer)return;
							const FUniformBufferStaticBindings StaticUniformBuffers(SceneTextureUniformBuffer);
							SCOPED_UNIFORM_BUFFER_STATIC_BINDINGS(RHICmdList, StaticUniformBuffers);
#endif

							auto DoRender = [&](bool bWireframe)
							{
								auto MaterialRenderProxy = (bWireframe ? WireframeMaterialInstance : Mesh.MaterialRenderProxy);
								auto Material = MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material cann't render with LexUIRenderer
								if (!Material)return;
								
								FMaterialShaderTypes ShaderTypes;
								ShaderTypes.AddShaderType<FLexUIScreenRenderVS>();
								ShaderTypes.AddShaderType<FLexUIScreenRenderPS>();
								FMaterialShaders Shaders;
								if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
								{
									TShaderRef<FLexUIScreenRenderVS> VertexShader;
									TShaderRef<FLexUIScreenRenderPS> PixelShader;
									Shaders.TryGetVertexShader(VertexShader);
									Shaders.TryGetPixelShader(PixelShader);

									FLexUIRenderer::SetGraphicPipelineState_BlendDepthStencilRasterize(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
										, Material->IsWireframe() || bWireframe, Material->IsTwoSided(), Material->ShouldDisableDepthTest(), ValidDepth, Mesh.ReverseCulling
									);

									GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIMeshVertexDeclaration();
									GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
									GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
									GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
									GraphicsPSOInit.NumSamples = NumSamples;
									SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

									VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
									PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, Mesh.Elements[0].PrimitiveUniformBufferResource);
									PixelShader->SetGammaValue(RHICmdList, GammaValue);

									RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
									RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.Elements[0].NumPrimitives, Mesh.Elements[0].NumInstances);
								}
							};
							if (bRenderLit)
							{
								DoRender(false);
							}
							if (bRenderWireframe)
							{
								DoRender(true);
							}
						}
					});
			}break;
			}
		}

#if WITH_EDITOR
		RenderGizmoMesh_RenderThread(ScreenSpaceGizmoMeshArray, GraphBuilder, RenderView, ViewRect, NumSamples, RenderTargetTexture);
#endif

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUI_RenderScreen_Clean"),
			ERDGPassFlags::None,
			[RenderView](FRHICommandListImmediate& RHICmdList)
			{
				RenderView->ViewUniformBuffer.SafeRelease();
				delete RenderView;
			});

		if (LexUIScreenSpaceDepthTexture.IsValid())
		{
			LexUIScreenSpaceDepthTexture.SafeRelease();
		}
	}

#if WITH_EDITOR
	END_LEXUI_RENDER :
	;
#endif

	if (NumSamples > 1)
	{
		auto Src = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LexUIResolveSrc"));
		auto Dst = RegisterExternalTexture(GraphBuilder, OrignScreenColorRenderTargetTexture, TEXT("LexUIResolveDst"));

		AddResolvePass(GraphBuilder, FRDGTextureMSAA(Src, Dst), ViewRect, NumSamples, GetGlobalShaderMap(InView.GetFeatureLevel()));
	}

	if (MSAARenderTarget.IsValid())
	{
		MSAARenderTarget.SafeRelease();
	}
}


class FLexUIDummySceneColorResolveBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		const int32 NumDummyVerts = 3;
		const uint32 Size = sizeof(FVector4f) * NumDummyVerts;
		const FRHIBufferCreateDesc CreateDesc =
			FRHIBufferCreateDesc::CreateVertex(TEXT("FLexUIDummySceneColorResolveBuffer"), Size)
			.AddUsage(EBufferUsageFlags::Static)
			.DetermineInitialState();

		VertexBufferRHI = RHICmdList.CreateBuffer(CreateDesc);
	}
};

TGlobalResource<FLexUIDummySceneColorResolveBuffer> GLexUIResolveDummyVertexBuffer;

BEGIN_SHADER_PARAMETER_STRUCT(FLexUIResolveParameters, )
RDG_TEXTURE_ACCESS(MainTex, ERHIAccess::SRVGraphics)
RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

//reference from SceneRendering.cpp::AddResolveSceneColorPass
void FLexUIRenderer::AddResolvePass(
	FRDGBuilder& GraphBuilder
	, FRDGTextureMSAA SceneColor
	, const FIntRect& ViewRect
	, uint8 NumSamples
	, FGlobalShaderMap* GlobalShaderMap
)
{
	FLexUIResolveParameters* PassParameters = GraphBuilder.AllocParameters<FLexUIResolveParameters>();
	PassParameters->MainTex = SceneColor.Target;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(SceneColor.Resolve, SceneColor.Resolve->HasBeenProduced() ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction);

	FRDGTextureRef SceneColorTargetable = SceneColor.Target;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUIResolveColor"),
		PassParameters,
		ERDGPassFlags::Raster,
		[ViewRect, SceneColorTargetable, NumSamples, GlobalShaderMap](FRHICommandList& RHICmdList)
		{
			FRHITexture* SceneColorTargetableRHI = SceneColorTargetable->GetRHI();

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

			const FIntPoint SceneColorExtent = SceneColorTargetable->Desc.Extent;

			// Resolve views individually. In the case of adaptive resolution, the view family will be much larger than the views individually.
			RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, SceneColorExtent.X, SceneColorExtent.Y, 1.0f);
			RHICmdList.SetScissorRect(true, ViewRect.Min.X, ViewRect.Min.Y, ViewRect.Max.X, ViewRect.Max.Y);

			TShaderMapRef<FLexUIResolveShaderVS> VertexShader(GlobalShaderMap);
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			if (NumSamples == 2)
			{
				TShaderMapRef<FLexUIResolveShader2xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}
			else if (NumSamples == 4)
			{
				TShaderMapRef<FLexUIResolveShader4xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}
			else if (NumSamples == 8)
			{
				TShaderMapRef<FLexUIResolveShader8xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}

			RHICmdList.SetStreamSource(0, GLexUIResolveDummyVertexBuffer.VertexBufferRHI, 0);
			RHICmdList.DrawPrimitive(0, 1, 1);
			RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
		}
	);
}

void FLexUIRenderer::AddWorldSpacePrimitive_RenderThread(void* InCanvasPtr, float InBlendDepth, int InDepthFade, ILexUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FWorldSpaceRenderParameter RenderParameter;
		RenderParameter.BlendDepth = InBlendDepth;
		RenderParameter.DepthFade = InDepthFade;
		RenderParameter.RenderCanvasPtr = InCanvasPtr;
		RenderParameter.Primitive = InPrimitive;

		WorldSpaceRenderCanvasParameterArray.Add(RenderParameter);
		bNeedSortWorldSpaceRenderCanvas = true;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Add nullptr as ILexUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLexUIRenderer::RemoveWorldSpacePrimitive_RenderThread(ILexUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		int existIndex = WorldSpaceRenderCanvasParameterArray.IndexOfByPredicate([InPrimitive](const FWorldSpaceRenderParameter& item) {
			return item.Primitive == InPrimitive;
			});
		if (existIndex == INDEX_NONE)
		{
			UE_LOG(LGUI, Log, TEXT("[%s].%d Canvas already removed."), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		}
		else
		{
			WorldSpaceRenderCanvasParameterArray.RemoveAt(existIndex);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Remove nullptr as ILexUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLexUIRenderer::AddScreenSpacePrimitive_RenderThread(ILexUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.PrimitiveArray.AddUnique(InPrimitive);
		ScreenSpaceRenderParameter.bNeedSortRenderPriority = true;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Add nullptr as ILexUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLexUIRenderer::RemoveScreenSpacePrimitive_RenderThread(ILexUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.PrimitiveArray.RemoveSingle(InPrimitive);
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Remove nullptr as ILexUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLexUIRenderer::SortScreenSpacePrimitiveRenderPriority_RenderThread()
{
	ScreenSpaceRenderParameter.PrimitiveArray.Sort([](ILexUIRendererPrimitive& A, ILexUIRendererPrimitive& B)
		{
			return A.LexUI_GetRenderPriority() < B.LexUI_GetRenderPriority();
		});
}

void FLexUIRenderer::MarkNeedToSortScreenSpacePrimitiveRenderPriority()
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLexUIRender_SortRenderPriority)(
		[ViewExtension](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->ScreenSpaceRenderParameter.bNeedSortRenderPriority = true;
		}
	);
}
void FLexUIRenderer::MarkNeedToSortWorldSpacePrimitiveRenderPriority()
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLexUIRender_SortRenderPriority)(
		[ViewExtension](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->bNeedSortWorldSpaceRenderCanvas = true;
		}
	);
}

void FLexUIRenderer::SetRenderCanvasDepthParameter(ULexCanvas* InRenderCanvas, float InBlendDepth, int InDepthFade)
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLexUIRender_SortRenderPriority)(
		[viewExtension, InRenderCanvas, InBlendDepth, InDepthFade](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SetRenderCanvasDepthFade_RenderThread(InRenderCanvas, InBlendDepth, InDepthFade);
		}
	);
}

void FLexUIRenderer::SetRenderCanvasDepthFade_RenderThread(ULexCanvas* InRenderCanvas, float InBlendDepth, int InDepthFade)
{
	for (auto& RenderParameter : WorldSpaceRenderCanvasParameterArray)
	{
		if (RenderParameter.RenderCanvasPtr == InRenderCanvas)
		{
			RenderParameter.BlendDepth = InBlendDepth;
			RenderParameter.DepthFade = InDepthFade;
		}
	}
}

void FLexUIRenderer::SetScreenSpaceRootCanvas(ULexCanvas* InCanvas)
{
	ScreenSpaceRenderParameter.RootCanvas = InCanvas;
}
void FLexUIRenderer::ClearScreenSpaceRootCanvas()
{
	ScreenSpaceRenderParameter.RootCanvas = nullptr;
}

void FLexUIRenderer::UpdateRenderTargetRenderer(UTextureRenderTarget2D* InRenderTarget, FColor InClearColor)
{
	auto Resource = InRenderTarget->GameThread_GetRenderTargetResource();
	if (Resource)
	{
		auto ViewExtension = this;
		ENQUEUE_RENDER_COMMAND(FLexUIRender_UpdateRenderTargetRenderer)(
			[ViewExtension, Resource, InClearColor](FRHICommandListImmediate& RHICmdList)
			{
				ViewExtension->RenderTargetResource = Resource;
				ViewExtension->RenderTargetClearColor = InClearColor;
			}
		);
	}
}

#if WITH_EDITOR
void FLexUIRenderer::RenderGizmoMesh_RenderThread(TArray<TSharedPtr<FLexUIGizmoMesh>>& HelperGizmoDataMap,
	FRDGBuilder& GraphBuilder, FSceneView* RenderView, const FIntRect& ViewRect, uint8 NumSamples,
	FRDGTextureRef RenderTargetTexture)
{
	if (HelperGizmoDataMap.Num() <= 0)return;
	const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)RenderView->Family)->GetSceneTextures();
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LexUI_RenderHelperLine"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, &HelperGizmoDataMap, RenderView, ViewRect, NumSamples](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			FSceneRenderingBulkObjectAllocator Allocator;
			FLexUIMeshElementCollector MeshCollector(RenderView->GetFeatureLevel(), Allocator, RHICmdList);
			
			for (auto& RenderParameter : HelperGizmoDataMap)
			{
				if (!RenderParameter->Material.IsValid())continue;
				auto& LocalBounds = RenderParameter->LocalBounds;
				auto& LocalToWorldMatrix = RenderParameter->LocalToWorldMatrix;
				auto WorldBounds = LocalBounds.TransformBy(LocalToWorldMatrix);
				if (bFrustumCulling)
				{
					if (!RenderView->GetCullingFrustum().IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent))continue;
				}
				
				auto MaterialRenderProxy = RenderParameter->Material->GetRenderProxy();
				auto Material = MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());
				FMaterialShaderTypes ShaderTypes;
				ShaderTypes.AddShaderType<FLexUIScreenRenderVS>();
				ShaderTypes.AddShaderType<FLexUIScreenRenderPS>();
				FMaterialShaders Shaders;
				if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
				{
					uint32 NumPrimitives = 0;
					EPrimitiveType PrimitiveType = EPrimitiveType::PT_TriangleList;
					
					switch (RenderParameter->GetPrimitiveType())
					{
					case ELexUIGizmoMeshPrimitiveType::Line:
						{
							NumPrimitives = RenderParameter->GetIndexBuffer().Indices.Num() / 2;
							PrimitiveType = EPrimitiveType::PT_LineList;
						}
						break;
					case ELexUIGizmoMeshPrimitiveType::Triangle:
						{
							NumPrimitives = RenderParameter->GetIndexBuffer().Indices.Num() / 3;
							PrimitiveType = EPrimitiveType::PT_TriangleList;
						}
					break;
					}

					TShaderRef<FLexUIScreenRenderVS> VertexShader;
					TShaderRef<FLexUIScreenRenderPS> PixelShader;
					Shaders.TryGetVertexShader(VertexShader);
					Shaders.TryGetPixelShader(PixelShader);
					
					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer = MeshCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
					DynamicPrimitiveUniformBuffer.Set(RHICmdList, LocalToWorldMatrix, LocalToWorldMatrix
						, WorldBounds, LocalBounds, false, false, false);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					FLexUIRenderer::SetGraphicPipelineState_BlendDepthStencilRasterize(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
						, false, Material->IsTwoSided(), Material->ShouldDisableDepthTest(), false, false);

					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIMeshVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.NumSamples = NumSamples;
					GraphicsPSOInit.PrimitiveType = PrimitiveType;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
							
					VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, &DynamicPrimitiveUniformBuffer.UniformBuffer);
					PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, MaterialRenderProxy, Material, &DynamicPrimitiveUniformBuffer.UniformBuffer);
						
					RHICmdList.SetStreamSource(0, RenderParameter->GetVertexBuffer().GetRHI(), 0);
					RHICmdList.DrawIndexedPrimitive(RenderParameter->GetIndexBuffer().GetRHI(), 0, 0, RenderParameter->GetNumVertices(), 0, NumPrimitives, 1);
				}
			}
			HelperGizmoDataMap.Reset();
		});
}

void FLexUIRenderer::AddScreenSpaceGizmoMesh(TSharedPtr<FLexUIGizmoMesh> InMesh)
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLexUIRender_AddLineRender)(
		[ViewExtension, InMesh](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->ScreenSpaceGizmoMeshArray.Add(InMesh);
		}
	);
}

void FLexUIRenderer::AddWorldSpaceGizmoMesh(TSharedPtr<FLexUIGizmoMesh> InMesh)
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLexUIRender_AddLineRender)(
		[ViewExtension, InMesh](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->WorldSpaceGizmoMeshArray.Add(InMesh);
		}
	);
}
#endif

void FLexUIFullScreenQuadVertexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	TArray<FLexUIPostProcessVertex> Vertices;
	Vertices.SetNumUninitialized(4);

	Vertices[0] = FLexUIPostProcessVertex(FVector3f(-1, -1, 0), FVector2f(0.0f, 1.0f));
	Vertices[1] = FLexUIPostProcessVertex(FVector3f(1, -1, 0), FVector2f(1.0f, 1.0f));
	Vertices[2] = FLexUIPostProcessVertex(FVector3f(-1, 1, 0), FVector2f(0.0f, 0.0f));
	Vertices[3] = FLexUIPostProcessVertex(FVector3f(1, 1, 0), FVector2f(1.0f, 0.0f));

	VertexBufferRHI = UE::RHIResourceUtils::CreateVertexBufferFromArray(
		RHICmdList, TEXT("LexUIFullScreenQuadVertexBuffer"), EBufferUsageFlags::Static, MakeConstArrayView(Vertices)
	);
}
void FLexUIFullScreenQuadIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	const uint16 Indices[] =
	{
		0, 2, 3,
		0, 3, 1
	};
	
	IndexBufferRHI = UE::RHIResourceUtils::CreateIndexBufferFromArray(
		RHICmdList, TEXT("LexUIFullScreenQuadIndexBuffer"), EBufferUsageFlags::Static, MakeConstArrayView(Indices)
	);
}
void FLexUIFullScreenSlicedQuadIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	uint16 Indices[54];
	int wSeg = 3, hSeg = 3;
	int vStartIndex = 0;
	int triangleArrayIndex = 0;
	for (int h = 0; h < hSeg; h++)
	{
		for (int w = 0; w < wSeg; w++)
		{
			int vIndex = vStartIndex + w;
			Indices[triangleArrayIndex++] = vIndex;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 2;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 1;

			Indices[triangleArrayIndex++] = vIndex;
			Indices[triangleArrayIndex++] = vIndex + 1;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 2;
		}
		vStartIndex += wSeg + 1;
	}
	
	IndexBufferRHI = UE::RHIResourceUtils::CreateIndexBufferFromArray(
		RHICmdList, TEXT("LexUIFullScreenSlicedQuadIndexBuffer"), EBufferUsageFlags::Static, MakeConstArrayView(Indices)
	);
}


