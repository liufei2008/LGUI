// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIRenderer.h"
#include "Core/LGUIRender/LGUIShaders.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "Core/LGUIRender/LGUIPostProcessShaders.h"
#include "Core/LGUIRender/LGUIResolveShaders.h"
#include "Modules/ModuleManager.h"
#include "LGUI.h"
#include "SceneView.h"
#include "Widgets/SWindow.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/LGUIRender/ILGUIRendererPrimitive.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "MeshPassProcessor.inl"
#include "ScenePrivate.h"
#include "Core/UIPostProcessRenderProxy.h"
#if WITH_EDITOR
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Slate/SceneViewport.h"
#include "Core/Actor/LGUIManager.h"
#include "Core/LGUISettings.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ClearQuad.h"
#if WITH_EDITOR
#include "Core/LGUIRender/LGUIHelperLineShaders.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITORONLY_DATA
uint32 FLGUIRenderer::EditorPreview_ViewKey = 0;
#endif
FLGUIRenderer::FLGUIRenderer(const FAutoRegister& AutoRegister, UWorld* InWorld, ELGUIRendererType InRendererType)
	:FSceneViewExtensionBase(AutoRegister)
{
	World = InWorld;
	RendererType = InRendererType;

#if WITH_EDITORONLY_DATA
	bIsEditorPreview = !World->IsGameWorld();
#endif
}
FLGUIRenderer::~FLGUIRenderer()
{
	
}

void FLGUIRenderer::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!World.IsValid())return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
	
	if (ScreenSpaceRenderParameter.RootCanvas.IsValid())
	{
		//@todo: these parameters should use ENQUE_RENDER_COMMAND to pass to render thread
		auto ViewLocation = ScreenSpaceRenderParameter.RootCanvas->GetViewLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(ScreenSpaceRenderParameter.RootCanvas->GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ProjectionMatrix = ScreenSpaceRenderParameter.RootCanvas->GetProjectionMatrix();
		auto ViewProjectionMatrix = ScreenSpaceRenderParameter.RootCanvas->GetViewProjectionMatrix();

		ScreenSpaceRenderParameter.ViewOrigin = ViewLocation;
		ScreenSpaceRenderParameter.ViewRotationMatrix = ViewRotationMatrix;
		ScreenSpaceRenderParameter.ProjectionMatrix = ProjectionMatrix;
		ScreenSpaceRenderParameter.ViewProjectionMatrix = FMatrix44f(ViewProjectionMatrix);
		ScreenSpaceRenderParameter.bEnableDepthTest = ScreenSpaceRenderParameter.RootCanvas->GetEnableDepthTest();
	}

	if (auto LGUISettings = GetDefault<ULGUISettings>())
	{
		NumSamples_MSAA = LGUISettings->AntiAliasingMothod == ELGUIRendererAntiAliasingMethod::MSAA ? (uint8)LGUISettings->MSAASampleCount : 1;
	}
	else
	{
		NumSamples_MSAA = 1;
	}
}
void FLGUIRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{

}
void FLGUIRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}
void FLGUIRenderer::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{

}
void FLGUIRenderer::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}
void FLGUIRenderer::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	RenderLGUI_RenderThread(GraphBuilder, InView);
}
int32 FLGUIRenderer::GetPriority() const
{
#if WITH_EDITOR
	auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#else
	static auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#endif
	return Priority;
}
bool FLGUIRenderer::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
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
void FLGUIRenderer::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	
}
void FLGUIRenderer::PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIRenderer::CopyRenderTarget(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst
	, bool ColorCorrect
)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst, TEXT("LGUICopyRenderTarget")), ERenderTargetLoadAction::ELoad);
	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LGUICopyRenderTarget"),
		PassParameters,
		ERDGPassFlags::Raster,
		[this, GlobalShaderMap, Src, Dst, ColorCorrect](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

			TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			if (ColorCorrect)
			{
				TShaderMapRef<FLGUISimpleCopyTargetPS_ColorCorrect> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				PixelShader->SetParameters(RHICmdList, Src);
			}
			else
			{
				TShaderMapRef<FLGUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				PixelShader->SetParameters(RHICmdList, Src);
			}
			VertexShader->SetParameters(RHICmdList);

			DrawFullScreenQuad(RHICmdList);
		});
}

void FLGUIRenderer::CopyRenderTargetOnMeshRegion(
	FRDGBuilder& GraphBuilder
	, FRDGTextureRef Dst
	, FTextureRHIRef Src
	, FGlobalShaderMap* GlobalShaderMap
	, const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData
	, const FMatrix44f& MVP
	, const FIntRect& ViewRect
	, const FVector4f& SrcTextureScaleOffset
	, bool ColorCorrect
)
{
	auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
	PassParameters->RenderTargets[0] = FRenderTargetBinding(Dst, ERenderTargetLoadAction::ELoad);
	auto NumSamples = Dst->Desc.NumSamples;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LGUICopyRenderTargetOnMeshRegion"),
		PassParameters,
		ERDGPassFlags::Raster,
		[Src, GlobalShaderMap, RegionVertexData, MVP, ViewRect, SrcTextureScaleOffset, NumSamples, ColorCorrect](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

			TShaderMapRef<FLGUICopyMeshRegionVS> VertexShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessCopyMeshRegionVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			GraphicsPSOInit.NumSamples = NumSamples;
			if (ColorCorrect)
			{
				TShaderMapRef<FLGUICopyMeshRegionPS_ColorCorrect> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

				PixelShader->SetParameters(RHICmdList, MVP, SrcTextureScaleOffset, Src);
			}
			else
			{
				TShaderMapRef<FLGUICopyMeshRegionPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

				PixelShader->SetParameters(RHICmdList, MVP, SrcTextureScaleOffset, Src);
			}

			uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessCopyMeshRegionVertex);
			FRHIResourceCreateInfo CreateInfo(TEXT("CopyRenderTargetOnMeshRegion"));
			FBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
			void* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
			FMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
			RHIUnlockBuffer(VertexBufferRHI);

			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);

			VertexBufferRHI.SafeRelease();
		});
}

void FLGUIRenderer::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	RHICmdList.SetStreamSource(0, GLGUIFullScreenQuadVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
}
void FLGUIRenderer::SetGraphicPipelineState(ERHIFeatureLevel::Type FeatureLevel, FGraphicsPipelineStateInitializer& GraphicsPSOInit, EBlendMode BlendMode
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
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
		}
		else
		{
			if (bReverseCulling)
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CCW, false>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CW, false>::GetRHI();
			}
		}
	}
	else
	{
		if (bIsTwoSided)
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_None, true>::GetRHI();
		}
		else
		{
			if (bReverseCulling)
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_CCW, true>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_CW, true>::GetRHI();
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("LGUI RHIRender"), STAT_LGUI_RHIRender, STATGROUP_LGUI);
void FLGUIRenderer::RenderLGUI_RenderThread(
	FRDGBuilder& GraphBuilder
	, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_LGUI_RHIRender);
	if (ScreenSpaceRenderParameter.PrimitiveArray.Num() <= 0 && WorldSpaceRenderCanvasParameterArray.Num() <= 0)return;//nothing to render
	bool bIsMainViewport = !(InView.bIsSceneCapture || InView.bIsReflectionCapture || InView.bIsPlanarReflection || InView.bIsVirtualTexture);

	FTextureRHIRef OrignScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	//msaa render target
	TRefCountPtr<IPooledRenderTarget> MSAARenderTarget = nullptr;

	uint8 NumSamples = NumSamples_MSAA;
	FIntRect ViewRect;
	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
	FVector4f DepthTextureScaleOffset;
	FVector4f ColorTextureScaleOffset;
	float InstancedStereoWidth = 0;
	if (RendererType == ELGUIRendererType::RenderTarget)//rendertarget mode
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
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, MSAARenderTarget, TEXT("LGUI_MSAA_RenderTarget"));
				if (!MSAARenderTarget.IsValid())
					return;

				OrignScreenColorRenderTargetTexture = ScreenColorRenderTargetTexture;
				ScreenColorRenderTargetTexture = MSAARenderTarget->GetRHI();
			}

			//clear render target
			{
				auto Parameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
				Parameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIRender_ClearRenderTarget")), ERenderTargetLoadAction::ENoAction);
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LGUIRender_ClearRenderTarget"),
					Parameters,
					ERDGPassFlags::Raster,
					[](FRHICommandListImmediate& RHICmdList)
					{
						DrawClearQuad(RHICmdList, FLinearColor(0, 0, 0, 0));
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
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, MSAARenderTarget, TEXT("LGUI_MSAA_RenderTarget"));
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
			InstancedStereoWidth = ViewInfo.InstancedStereoWidth;
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

	FRDGTextureRef RenderTargetTexture = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIRendererTargetTexture"));
	const float EngineGamma = GEngine ? GEngine->GetDisplayGamma() : 2.2f;
	float GammaValue =
		(RendererType == ELGUIRendererType::RenderTarget || !bIsMainViewport) ? 1.0f : EngineGamma;

	auto CurrentWorldTime = InView.Family->Time.GetWorldTimeSeconds();
	//Render world space
	if (WorldSpaceRenderCanvasParameterArray.Num() > 0)
	{
		//collect render primitive to a sequence
		struct FWorldSpaceRenderParameterSequence
		{
			TArray<FLGUIPrimitiveDataContainer> RenderDataArray;
			//blend depth, 0-occlude by depth, 1-all visible
			float BlendDepth = 0.0f;
			//depth fade effect
			float DepthFade = 0.0f;

			//for sort translucent
			FVector3f WorldPosition;
			//distance to camera (sqare)
			float DistToCamera = 0;
			int RenderPriority = 0;
		};
		TArray<FWorldSpaceRenderParameterSequence> RenderSequenceArray;
		for (auto& WorldRenderParameter : WorldSpaceRenderCanvasParameterArray)
		{
			if (WorldRenderParameter.Primitive->CanRender())
			{
				bool bIsPrimitiveVisible = false;//default is not visible
				if (InView.ShowOnlyPrimitives.IsSet())
				{
					bIsPrimitiveVisible = InView.ShowOnlyPrimitives.GetValue().Contains(WorldRenderParameter.Primitive->GetPrimitiveComponentId());
				}
				else
				{
					bIsPrimitiveVisible = !InView.HiddenPrimitives.Contains(WorldRenderParameter.Primitive->GetPrimitiveComponentId());
				}
				if (bIsPrimitiveVisible)
				{
					auto WorldBounds = WorldRenderParameter.Primitive->GetWorldBounds();
					if (InView.CullingFrustum.IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent))//simple View Frustum Culling
					{
						FWorldSpaceRenderParameterSequence Item;
						WorldRenderParameter.Primitive->CollectRenderData(Item.RenderDataArray, CurrentWorldTime);
						if (Item.RenderDataArray.Num() > 0)
						{
							Item.BlendDepth = WorldRenderParameter.BlendDepth;
							Item.DepthFade = WorldRenderParameter.DepthFade;
							Item.WorldPosition = WorldRenderParameter.Primitive->GetWorldPositionForSortTranslucent();
							Item.RenderPriority = WorldRenderParameter.Primitive->GetRenderPriority();
							RenderSequenceArray.Add(Item);
						}
					}
				}
			}
		}
		if (RenderSequenceArray.Num() <= 0)
		{
			return;
		}

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

			auto InViewPosition = (FVector3f)RenderView->ViewMatrices.GetViewOrigin();
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
				case ELGUIRendererPrimitiveType::PostProcess://render post process
				{
					for (int i = 0; i < RenderPrimitiveItem.Sections.Num(); i++)
					{
						if (auto Primitive = RenderPrimitiveItem.Primitive->GetPostProcessElement(RenderPrimitiveItem.Sections[i].SectionPointer))
						{
							Primitive->OnRenderPostProcess_RenderThread(
								GraphBuilder,
								SceneTextures,
								this,
								ScreenColorRenderTargetTexture,
								GlobalShaderMap,
								ViewProjectionMatrix,
								true,
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
				case ELGUIRendererPrimitiveType::Mesh://render mesh
				{
					auto* PassParameters = GraphBuilder.AllocParameters<FLGUIWorldRenderPSParameter>();
					PassParameters->SceneDepthTex = SceneTextures.Depth.Resolve;
					PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);

					GraphBuilder.AddPass(
						RDG_EVENT_NAME("LGUIRender_WorldSpace"),
						PassParameters,
						ERDGPassFlags::Raster,
						[this, DepthFade = RenderSequenceItem.DepthFade, BlendDepth = RenderSequenceItem.BlendDepth, RenderPrimitiveItem, RenderView, ViewRect, PassParameters, SceneDepthTexST = DepthTextureScaleOffset, NumSamples, GammaValue](FRHICommandListImmediate& RHICmdList)
						{
							FGraphicsPipelineStateInitializer GraphicsPSOInit;
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
							RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

							MeshBatchArray.Reset();
							FSceneRenderingBulkObjectAllocator Allocator;
							FLGUIMeshElementCollector meshCollector(RenderView->GetFeatureLevel(), Allocator);
							RenderPrimitiveItem.Primitive->GetMeshElements(*RenderView->Family, (FMeshElementCollector*)&meshCollector, RenderPrimitiveItem, MeshBatchArray);
							for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
							{
								auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
								const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
								auto Material = Mesh.MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material cann't render with LGUIRenderer
								if (!Material)return;

								FLGUIRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
									, Material->IsWireframe(), Material->IsTwoSided(), Material->ShouldDisableDepthTest(), false, Mesh.ReverseCulling
								);

								if (DepthFade <= 0.0f)
								{
									TShaderRef<FLGUIScreenRenderVS> VertexShader;
									TShaderRef<FLGUIWorldRenderPS> PixelShader;
									FMaterialShaderTypes ShaderTypes;
									ShaderTypes.AddShaderType<FLGUIScreenRenderVS>();
									ShaderTypes.AddShaderType<FLGUIWorldRenderPS>();
									FMaterialShaders Shaders;
									if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
									{
										Shaders.TryGetVertexShader(VertexShader);
										Shaders.TryGetPixelShader(PixelShader);

										GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIMeshVertexDeclaration();
										GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
										GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
										GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
										GraphicsPSOInit.NumSamples = NumSamples;
										SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

										VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
										PixelShader->SetGammaValue(RHICmdList, GammaValue);

										RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
										RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
									}
								}
								else
								{
									TShaderRef<FLGUIScreenRenderVS> VertexShader;
									TShaderRef<FLGUIWorldRenderDepthFadePS> PixelShader;
									FMaterialShaderTypes ShaderTypes;
									ShaderTypes.AddShaderType<FLGUIScreenRenderVS>();
									ShaderTypes.AddShaderType<FLGUIWorldRenderDepthFadePS>();
									FMaterialShaders Shaders;
									if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
									{
										Shaders.TryGetVertexShader(VertexShader);
										Shaders.TryGetPixelShader(PixelShader);

										GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIMeshVertexDeclaration();
										GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
										GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
										GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
										GraphicsPSOInit.NumSamples = NumSamples;
										SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

										VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
										PixelShader->SetDepthBlendParameter(RHICmdList, BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
										PixelShader->SetDepthFadeParameter(RHICmdList, DepthFade);
										PixelShader->SetGammaValue(RHICmdList, GammaValue);

										RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
										RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
									}
								}
							}
						});
				}break;
				}
			}
		}
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LGUI_RenderWorld_Clean"),
			ERDGPassFlags::None,
			[RenderView](FRHICommandListImmediate& RHICmdList)
			{
				RenderView->ViewUniformBuffer.SafeRelease();
				delete RenderView;
			});
	}

	//Render screen space
	if (ScreenSpaceRenderParameter.PrimitiveArray.Num() > 0
		&& bIsMainViewport
		)
	{
		if (ScreenSpaceRenderParameter.bNeedSortRenderPriority)
		{
			ScreenSpaceRenderParameter.bNeedSortRenderPriority = false;
			SortScreenSpacePrimitiveRenderPriority_RenderThread();
		}
#if WITH_EDITOR
		if (RendererType == ELGUIRendererType::RenderTarget)
		{

		}
		else
		{
			if (!bCanRenderScreenSpace)goto END_LGUI_RENDER;
			if (bIsPlaying)
			{
				if (!InView.bIsGameView)goto END_LGUI_RENDER;
			}
			else//editor viewport preview
			{
				if (InView.GetViewKey() != EditorPreview_ViewKey)goto END_LGUI_RENDER;//only preview in specific viewport in editor
			}
		}
#endif
		TRefCountPtr<IPooledRenderTarget> LGUIScreenSpaceDepthTexture = nullptr;
		FRDGTextureRef LGUIScreenSpaceDepthRDGTexture = nullptr;
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

			GRenderTargetPool.FindFreeElement(RHICmdList, Desc, LGUIScreenSpaceDepthTexture, TEXT("LGUIScreenSpaceDepthTexture"));
			LGUIScreenSpaceDepthRDGTexture = RegisterExternalTexture(GraphBuilder, LGUIScreenSpaceDepthTexture->GetRHI(), TEXT("LGUIRendererTargetTexture"));
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
		RenderView->UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);//this is mainly for ViewFrustum

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
		TArray<FLGUIPrimitiveDataContainer> RenderSequenceArray;
		for (auto Primitive : ScreenSpaceRenderParameter.PrimitiveArray)
		{
			if (Primitive->CanRender())
			{
				auto WorldBounds = Primitive->GetWorldBounds();
				if (RenderView->CullingFrustum.IntersectBox(WorldBounds.Origin, WorldBounds.BoxExtent))//simple View Frustum Culling
				{
					Primitive->CollectRenderData(RenderSequenceArray, CurrentWorldTime);
				}
			}
		}

		const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)InView.Family)->GetSceneTextures();
		bool bIsDepthStencilCleared = false;
		for (auto& RenderSequenceItem : RenderSequenceArray)
		{
			switch (RenderSequenceItem.Type)
			{
			case ELGUIRendererPrimitiveType::PostProcess://render post process
			{
				for (int i = 0; i < RenderSequenceItem.Sections.Num(); i++)
				{
					if (auto Primitive = RenderSequenceItem.Primitive->GetPostProcessElement(RenderSequenceItem.Sections[i].SectionPointer))
					{
						Primitive->OnRenderPostProcess_RenderThread(
							GraphBuilder,
							SceneTextures,
							this,
							ScreenColorRenderTargetTexture,
							GlobalShaderMap,
							ScreenSpaceRenderParameter.ViewProjectionMatrix,
							/*IsWorldSpace*/false,
							/*BlendDepthForWorld*/0.0f,//actually this value will no work because 'IsWorldSpace' is false
							/*BlendDepthForWorld*/0.0f,//actually this value will no work because 'IsWorldSpace' is false
							ViewRect,
							DepthTextureScaleOffset,
							ColorTextureScaleOffset
						);
					}
				}
			}
			break;
			case ELGUIRendererPrimitiveType::Mesh:
			{
				auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
				PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
				if (LGUIScreenSpaceDepthRDGTexture != nullptr)
				{
					if (bIsDepthStencilCleared)
					{
						PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(LGUIScreenSpaceDepthRDGTexture, ERenderTargetLoadAction::ENoAction, ERenderTargetLoadAction::ENoAction, FExclusiveDepthStencil::DepthWrite_StencilWrite);
					}
					else
					{
						bIsDepthStencilCleared = true;//only clear depth stencil when first use depth texture
						PassParameters->RenderTargets.DepthStencil = FDepthStencilBinding(LGUIScreenSpaceDepthRDGTexture, ERenderTargetLoadAction::EClear, ERenderTargetLoadAction::EClear, FExclusiveDepthStencil::DepthWrite_StencilWrite);
					}
				}
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LGUIRender_ScreenSpace"),
					PassParameters,
					ERDGPassFlags::Raster,
					[this, RenderSequenceItem, RenderView, ViewRect, SceneDepthTexST = DepthTextureScaleOffset, NumSamples, ValidDepth = LGUIScreenSpaceDepthRDGTexture != nullptr, GammaValue](FRHICommandListImmediate& RHICmdList)
					{
						MeshBatchArray.Reset();
						FSceneRenderingBulkObjectAllocator Allocator;
						FLGUIMeshElementCollector meshCollector(RenderView->GetFeatureLevel(), Allocator);
						RenderSequenceItem.Primitive->GetMeshElements(*RenderView->Family, (FMeshElementCollector*)&meshCollector, RenderSequenceItem, MeshBatchArray);
						for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
						{
							auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
							const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
							auto Material = Mesh.MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material cann't render with LGUIRenderer
							if (!Material)return;

							TShaderRef<FLGUIScreenRenderVS> VertexShader;
							TShaderRef<FLGUIScreenRenderPS> PixelShader;
							FMaterialShaderTypes ShaderTypes;
							ShaderTypes.AddShaderType<FLGUIScreenRenderVS>();
							ShaderTypes.AddShaderType<FLGUIScreenRenderPS>();
							FMaterialShaders Shaders;
							if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
							{
								Shaders.TryGetVertexShader(VertexShader);
								Shaders.TryGetPixelShader(PixelShader);

								RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

								FGraphicsPipelineStateInitializer GraphicsPSOInit;
								RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

								FLGUIRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
									, Material->IsWireframe(), Material->IsTwoSided(), Material->ShouldDisableDepthTest(), ValidDepth, Mesh.ReverseCulling
								);

								GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIMeshVertexDeclaration();
								GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
								GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
								GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
								GraphicsPSOInit.NumSamples = NumSamples;
								SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

								VertexShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
								PixelShader->SetMaterialShaderParameters(RHICmdList, *RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
								PixelShader->SetGammaValue(RHICmdList, GammaValue);

								RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
								RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.Elements[0].NumPrimitives, Mesh.Elements[0].NumInstances);
							}
						}
					});
			}break;
			}
		}

#if WITH_EDITOR
		if (HelperLineRenderParameterArray.Num() > 0)
		{
			auto* PassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);
			GraphBuilder.AddPass(
				RDG_EVENT_NAME("LGUI_RenderHelperLine"),
				PassParameters,
				ERDGPassFlags::Raster,
				[this, RenderView, ViewRect, NumSamples, GlobalShaderMap](FRHICommandListImmediate& RHICmdList)
				{
					TShaderMapRef<FLGUIHelperLineShaderVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIHelperLineShaderPS> PixelShader(GlobalShaderMap);

					RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					FLGUIRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, EBlendMode::BLEND_Opaque, false, true, true, false, false);

					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHelperLineVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_LineList;
					GraphicsPSOInit.NumSamples = NumSamples;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);

					VertexShader->SetParameters(RHICmdList, ScreenSpaceRenderParameter.ViewProjectionMatrix);
					
					for (auto& LineRenderParameter : HelperLineRenderParameterArray)
					{
						FRHIResourceCreateInfo CreateInfo(TEXT("LGUIHelperLineRenderVertexBuffer"));
						FBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(sizeof(FLGUIHelperLineVertex) * LineRenderParameter.LinePoints.Num(), BUF_Volatile, CreateInfo);
						auto* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, sizeof(FLGUIHelperLineVertex) * LineRenderParameter.LinePoints.Num(), RLM_WriteOnly);
						FMemory::Memcpy(VoidPtr, LineRenderParameter.LinePoints.GetData(), sizeof(FLGUIHelperLineVertex) * LineRenderParameter.LinePoints.Num());
						RHIUnlockBuffer(VertexBufferRHI);

						RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);

						int32 MaxVerticesAllowed = ((GDrawUPVertexCheckCount / sizeof(FSimpleElementVertex)) / 2) * 2;
						/*
						hack to avoid a crash when trying to render large numbers of line segments.
						*/
						MaxVerticesAllowed = FMath::Min(MaxVerticesAllowed, 64 * 1024);

						int32 MinVertex = 0;
						int32 TotalVerts = (LineRenderParameter.LinePoints.Num() / 2) * 2;
						while (MinVertex < TotalVerts)
						{
							int32 NumLinePrims = FMath::Min(MaxVerticesAllowed, TotalVerts - MinVertex) / 2;
							RHICmdList.DrawPrimitive(MinVertex, NumLinePrims, 1);
							MinVertex += NumLinePrims * 2;
						}
						VertexBufferRHI.SafeRelease();
					}
					HelperLineRenderParameterArray.Reset();
				});
		}
#endif

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LGUI_RenderScreen_Clean"),
			ERDGPassFlags::None,
			[RenderView](FRHICommandListImmediate& RHICmdList)
			{
				RenderView->ViewUniformBuffer.SafeRelease();
				delete RenderView;
			});

		if (LGUIScreenSpaceDepthTexture.IsValid())
		{
			LGUIScreenSpaceDepthTexture.SafeRelease();
		}
	}

	if (NumSamples > 1)
	{
		auto Src = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIResolveSrc"));
		auto Dst = RegisterExternalTexture(GraphBuilder, OrignScreenColorRenderTargetTexture, TEXT("LGUIResolveDst"));

		AddResolvePass(GraphBuilder, FRDGTextureMSAA(Src, Dst), InView.IsInstancedStereoPass(), InstancedStereoWidth, ViewRect, NumSamples, GetGlobalShaderMap(InView.GetFeatureLevel()));
	}

#if WITH_EDITOR
	END_LGUI_RENDER :
	;
#endif
	if (MSAARenderTarget.IsValid())
	{
		MSAARenderTarget.SafeRelease();
	}
}


class FLGUIDummySceneColorResolveBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI() override
	{
		const int32 NumDummyVerts = 3;
		const uint32 Size = sizeof(FVector4f) * NumDummyVerts;
		FRHIResourceCreateInfo CreateInfo(TEXT("FLGUIDummySceneColorResolveBuffer"));
		VertexBufferRHI = RHICreateBuffer(Size, BUF_Static | BUF_VertexBuffer, 0, ERHIAccess::VertexOrIndexBuffer, CreateInfo);
		void* BufferData = RHILockBuffer(VertexBufferRHI, 0, Size, RLM_WriteOnly);
		FMemory::Memset(BufferData, 0, Size);
		RHIUnlockBuffer(VertexBufferRHI);
	}
};

TGlobalResource<FLGUIDummySceneColorResolveBuffer> GLGUIResolveDummyVertexBuffer;

BEGIN_SHADER_PARAMETER_STRUCT(FLGUIResolveParameters, )
RDG_TEXTURE_ACCESS(MainTex, ERHIAccess::SRVGraphics)
RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()

//reference from SceneRendering.cpp::AddResolveSceneColorPass
void FLGUIRenderer::AddResolvePass(
	FRDGBuilder& GraphBuilder
	, FRDGTextureMSAA SceneColor
	, bool bIsInstancedStereoPass
	, float InstancedStereoWidth
	, const FIntRect& ViewRect
	, uint8 NumSamples
	, FGlobalShaderMap* GlobalShaderMap
)
{
	FLGUIResolveParameters* PassParameters = GraphBuilder.AllocParameters<FLGUIResolveParameters>();
	PassParameters->MainTex = SceneColor.Target;
	PassParameters->RenderTargets[0] = FRenderTargetBinding(SceneColor.Resolve, SceneColor.Resolve->HasBeenProduced() ? ERenderTargetLoadAction::ELoad : ERenderTargetLoadAction::ENoAction);

	FRDGTextureRef SceneColorTargetable = SceneColor.Target;

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("LGUIResolveColor"),
		PassParameters,
		ERDGPassFlags::Raster,
		[bIsInstancedStereoPass, ViewRect, InstancedStereoWidth, SceneColorTargetable, NumSamples, GlobalShaderMap](FRHICommandList& RHICmdList)
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
			RHICmdList.SetScissorRect(true, bIsInstancedStereoPass ? 0 : ViewRect.Min.X, ViewRect.Min.Y,
				bIsInstancedStereoPass ? InstancedStereoWidth : ViewRect.Max.X, ViewRect.Max.Y);

			TShaderMapRef<FLGUIResolveShaderVS> VertexShader(GlobalShaderMap);
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.PrimitiveType = PT_TriangleList;
			if (NumSamples == 2)
			{
				TShaderMapRef<FLGUIResolveShader2xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}
			else if (NumSamples == 4)
			{
				TShaderMapRef<FLGUIResolveShader4xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}
			else if (NumSamples == 8)
			{
				TShaderMapRef<FLGUIResolveShader8xPS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);
				PixelShader->SetParameters(RHICmdList, SceneColorTargetableRHI);
			}

			RHICmdList.SetStreamSource(0, GLGUIResolveDummyVertexBuffer.VertexBufferRHI, 0);
			RHICmdList.DrawPrimitive(0, 1, 1);
			RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
		}
	);
}

void FLGUIRenderer::AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FWorldSpaceRenderParameter RenderParameter;
		RenderParameter.BlendDepth = InCanvas->GetActualBlendDepth();
		RenderParameter.DepthFade = InCanvas->GetActualDepthFade();
		RenderParameter.RenderCanvas = InCanvas;
		RenderParameter.Primitive = InPrimitive;

		WorldSpaceRenderCanvasParameterArray.Add(RenderParameter);
		bNeedSortWorldSpaceRenderCanvas = true;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Add nullptr as ILGUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLGUIRenderer::RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIRendererPrimitive* InPrimitive)
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
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Remove nullptr as ILGUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLGUIRenderer::AddScreenSpacePrimitive_RenderThread(ILGUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.PrimitiveArray.AddUnique(InPrimitive);
		ScreenSpaceRenderParameter.bNeedSortRenderPriority = true;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Add nullptr as ILGUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLGUIRenderer::RemoveScreenSpacePrimitive_RenderThread(ILGUIRendererPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.PrimitiveArray.RemoveSingle(InPrimitive);
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d Remove nullptr as ILGUIRendererPrimitive!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
	}
}
void FLGUIRenderer::SortScreenSpacePrimitiveRenderPriority_RenderThread()
{
	ScreenSpaceRenderParameter.PrimitiveArray.Sort([](ILGUIRendererPrimitive& A, ILGUIRendererPrimitive& B)
		{
			return A.GetRenderPriority() < B.GetRenderPriority();
		});
}

void FLGUIRenderer::MarkNeedToSortScreenSpacePrimitiveRenderPriority()
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[ViewExtension](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->ScreenSpaceRenderParameter.bNeedSortRenderPriority = true;
		}
	);
}
void FLGUIRenderer::MarkNeedToSortWorldSpacePrimitiveRenderPriority()
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[ViewExtension](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->bNeedSortWorldSpaceRenderCanvas = true;
		}
	);
}

void FLGUIRenderer::SetRenderCanvasDepthParameter(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade)
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension, InRenderCanvas, InBlendDepth, InDepthFade](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SetRenderCanvasDepthFade_RenderThread(InRenderCanvas, InBlendDepth, InDepthFade);
		}
	);
}

void FLGUIRenderer::SetRenderCanvasDepthFade_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade)
{
	for (auto& RenderParameter : WorldSpaceRenderCanvasParameterArray)
	{
		if (RenderParameter.RenderCanvas == InRenderCanvas)
		{
			RenderParameter.BlendDepth = InBlendDepth;
			RenderParameter.DepthFade = InDepthFade;
		}
	}
}

void FLGUIRenderer::SetScreenSpaceRootCanvas(ULGUICanvas* InCanvas)
{
	ScreenSpaceRenderParameter.RootCanvas = InCanvas;
}
void FLGUIRenderer::ClearScreenSpaceRootCanvas()
{
	ScreenSpaceRenderParameter.RootCanvas = nullptr;
}

void FLGUIRenderer::UpdateRenderTargetRenderer(UTextureRenderTarget2D* InRenderTarget)
{
	auto Resource = InRenderTarget->GameThread_GetRenderTargetResource();
	if (Resource)
	{
		auto ViewExtension = this;
		ENQUEUE_RENDER_COMMAND(FLGUIRender_UpdateRenderTargetRenderer)(
			[ViewExtension, Resource](FRHICommandListImmediate& RHICmdList)
			{
				ViewExtension->RenderTargetResource = Resource;
			}
		);
	}
}

#if WITH_EDITOR
void FLGUIRenderer::AddLineRender(const FLGUIHelperLineRenderParameter& InLineParameter)
{
	auto Buffer = new FLGUIHelperLineRenderParameter(InLineParameter);
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_AddLineRender)(
		[ViewExtension, Buffer](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->HelperLineRenderParameterArray.Add(*Buffer);
			delete Buffer;
		}
	);
}
#endif

void FLGUIFullScreenQuadVertexBuffer::InitRHI()
{
	TResourceArray<FLGUIPostProcessVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
	Vertices.SetNumUninitialized(4);

	Vertices[0] = FLGUIPostProcessVertex(FVector3f(-1, -1, 0), FVector2f(0.0f, 1.0f));
	Vertices[1] = FLGUIPostProcessVertex(FVector3f(1, -1, 0), FVector2f(1.0f, 1.0f));
	Vertices[2] = FLGUIPostProcessVertex(FVector3f(-1, 1, 0), FVector2f(0.0f, 0.0f));
	Vertices[3] = FLGUIPostProcessVertex(FVector3f(1, 1, 0), FVector2f(1.0f, 0.0f));

	FRHIResourceCreateInfo CreateInfo(TEXT("LGUIFullScreenQuadVertexBuffer"), &Vertices);
	VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
}
void FLGUIFullScreenQuadIndexBuffer::InitRHI()
{
	const uint16 Indices[] =
	{
		0, 2, 3,
		0, 3, 1
	};

	TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
	uint32 NumIndices = UE_ARRAY_COUNT(Indices);
	IndexBuffer.AddUninitialized(NumIndices);
	FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

	FRHIResourceCreateInfo CreateInfo(TEXT("LGUIFullScreenQuadIndexBuffer"), &IndexBuffer);
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}
void FLGUIFullScreenSlicedQuadIndexBuffer::InitRHI()
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

	TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
	uint32 NumIndices = UE_ARRAY_COUNT(Indices);
	IndexBuffer.AddUninitialized(NumIndices);
	FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

	FRHIResourceCreateInfo CreateInfo(TEXT("LGUIFullScreenSlicedQuadIndexBuffer"), &IndexBuffer);
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
