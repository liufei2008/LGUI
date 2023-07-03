// Copyright 2019-Present LexLiu. All Rights Reserved.

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
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/LGUISettings.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ClearQuad.h"
#if WITH_EDITOR
#include "Core/HudRender/LGUIHelperLineShaders.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITORONLY_DATA
uint32 FLGUIHudRenderer::EditorPreview_ViewKey = 0;
#endif
FLGUIHudRenderer::FLGUIHudRenderer(const FAutoRegister& AutoRegister, UWorld* InWorld)
	:FSceneViewExtensionBase(AutoRegister)
{
	World = InWorld;

#if WITH_EDITORONLY_DATA
	bIsEditorPreview = !World->IsGameWorld();
#endif
}
FLGUIHudRenderer::~FLGUIHudRenderer()
{
	
}

void FLGUIHudRenderer::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!World.IsValid())return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
	if (!bIsRenderToRenderTarget)
	{

	}
	//world space
	{

	}
	//screen space
	if (ScreenSpaceRenderParameter.RenderCanvas.IsValid())
	{
		//@todo: these parameters should use ENQUE_RENDER_COMMAND to pass to render thread
		auto ViewLocation = ScreenSpaceRenderParameter.RenderCanvas->GetViewLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(ScreenSpaceRenderParameter.RenderCanvas->GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetProjectionMatrix();
		auto ViewProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetViewProjectionMatrix();

		ScreenSpaceRenderParameter.ViewOrigin = ViewLocation;
		ScreenSpaceRenderParameter.ViewRotationMatrix = ViewRotationMatrix;
		ScreenSpaceRenderParameter.ProjectionMatrix = ProjectionMatrix;
		ScreenSpaceRenderParameter.ViewProjectionMatrix = FMatrix44f(ViewProjectionMatrix);
		ScreenSpaceRenderParameter.bEnableDepthTest = ScreenSpaceRenderParameter.RenderCanvas->GetEnableDepthTest();
	}
}
void FLGUIHudRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{

}
void FLGUIHudRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}
void FLGUIHudRenderer::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{

}
void FLGUIHudRenderer::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}
void FLGUIHudRenderer::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& InView)
{
	RenderLGUI_RenderThread(GraphBuilder, InView);
}
int32 FLGUIHudRenderer::GetPriority() const
{
#if WITH_EDITOR
	auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#else
	static auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#endif
	return Priority;
}
bool FLGUIHudRenderer::IsActiveThisFrame_Internal(const FSceneViewExtensionContext& Context) const
{
#if WITH_EDITOR
	if (GEngine == nullptr) return false;
	bCanRenderScreenSpace = true;
	bIsPlaying = ALGUIManagerActor::GetIsPlaying(World.Get());
	//check if simulation
	if (UEditorEngine* editor = Cast<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)bCanRenderScreenSpace = false;
	}

	if (bIsPlaying == bIsEditorPreview)bCanRenderScreenSpace = false;
#endif


	if (!World.IsValid())return false;
	if (World.Get() != Context.GetWorld())return false;//only render self world
	return true;
}
void FLGUIHudRenderer::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	
}
void FLGUIHudRenderer::PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

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
			FMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
			RHIUnlockBuffer(VertexBufferRHI);

			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);

			VertexBufferRHI.SafeRelease();
		});
}

void FLGUIHudRenderer::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	RHICmdList.SetStreamSource(0, GLGUIFullScreenQuadVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
}
void FLGUIHudRenderer::SetGraphicPipelineState(ERHIFeatureLevel::Type FeatureLevel, FGraphicsPipelineStateInitializer& GraphicsPSOInit, EBlendMode BlendMode
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

DECLARE_CYCLE_STAT(TEXT("Hud RHIRender"), STAT_Hud_RHIRender, STATGROUP_LGUI);
void FLGUIHudRenderer::RenderLGUI_RenderThread(
	FRDGBuilder& GraphBuilder
	, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() <= 0 && WorldSpaceRenderCanvasParameterArray.Num() <= 0)return;//nothing to render
	bool bIsMainViewport = !(InView.bIsSceneCapture || InView.bIsReflectionCapture || InView.bIsPlanarReflection || InView.bIsVirtualTexture);

	//create render target
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef OriginScreenColorTexture = nullptr;//@todo: can use a normal texture here?
	TRefCountPtr<IPooledRenderTarget> OriginScreenColorRenderTarget = nullptr;

	uint8 NumSamples = 1;
	FIntRect ViewRect;
	FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
	FVector4f DepthTextureScaleOffset;
	FVector4f ViewTextureScaleOffset;
	if (bIsRenderToRenderTarget)
	{
		if (RenderTargetResource != nullptr && RenderTargetResource->GetRenderTargetTexture() != nullptr)
		{
			ScreenColorRenderTargetTexture = (FTextureRHIRef)RenderTargetResource->GetRenderTargetTexture();
			if (ScreenColorRenderTargetTexture == nullptr)return;//invalid render target
			NumSamples = ScreenColorRenderTargetTexture->GetNumSamples();
			//clear render target
			{
				auto Parameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
				Parameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIHudRender_ClearRenderTarget")), ERenderTargetLoadAction::ENoAction);
				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LGUIHudRender_ClearRenderTarget"),
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

		ViewTextureScaleOffset = DepthTextureScaleOffset = FVector4f(1, 1, 0, 0);
	}
	else
	{
		ScreenColorRenderTargetTexture = (FTextureRHIRef)InView.Family->RenderTarget->GetRenderTargetTexture();
		if (ScreenColorRenderTargetTexture == nullptr)return;//invalid render target
		NumSamples = ScreenColorRenderTargetTexture->GetNumSamples();

		if (bNeedOriginScreenColorTextureOnPostProcess)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(InView.Family->RenderTarget->GetRenderTargetTexture()->GetSizeXY(), InView.Family->RenderTarget->GetRenderTargetTexture()->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_ShaderResource, false));
			desc.NumSamples = NumSamples;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenColorRenderTarget, TEXT("LGUISceneColorRenderTarget"));
			if (!OriginScreenColorRenderTarget.IsValid())
				return;
			OriginScreenColorTexture = OriginScreenColorRenderTarget->GetRHI();
			RHICmdList.CopyTexture(ScreenColorRenderTargetTexture, OriginScreenColorTexture, FRHICopyTextureInfo());
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
			ViewTextureScaleOffset = FVector4f(1, 1, 0, 0);
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
			ViewTextureScaleOffset = FVector4f(0.5f, 1, 0, 0);
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
			ViewTextureScaleOffset = FVector4f(0.5f, 1, 0.5f, 0);
		}
		break;
		}
	}

	FRDGTextureRef RenderTargetTexture = RegisterExternalTexture(GraphBuilder, ScreenColorRenderTargetTexture, TEXT("LGUIRendererTargetTexture"));
	float ColorCorrectionValue =
#if PLATFORM_ANDROID || PLATFORM_IOS
		IsMobileHDR() ? 0.45454545f : 1.0f;
#else
		(bIsRenderToRenderTarget || !bIsMainViewport) ? 1.0f : 0.45454545f;
#endif
	//Render world space
	if (WorldSpaceRenderCanvasParameterArray.Num() > 0)
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

		//process visibility. Can only tell visibility from mesh by PrimitiveComponentId, so what about postprocess? Just add an invisible mesh (sprite, texture, text)
		TMap<ULGUICanvas*, bool> CanvasVisibilityMap;
		for (auto& WorldRenderParameter : WorldSpaceRenderCanvasParameterArray)
		{
			auto HudPrimitive = WorldRenderParameter.HudPrimitive;
			if (HudPrimitive != nullptr)
			{
				if (HudPrimitive->GetPrimitiveType() == ELGUIHudPrimitiveType::Mesh)
				{
					if (!CanvasVisibilityMap.Contains(WorldRenderParameter.RenderCanvas))
					{
						bool bIsPrimitiveVisible = true;
						if (InView.ShowOnlyPrimitives.IsSet())
						{
							bIsPrimitiveVisible = InView.ShowOnlyPrimitives.GetValue().Contains(HudPrimitive->GetMeshPrimitiveComponentId());
						}
						else
						{
							bIsPrimitiveVisible = !InView.HiddenPrimitives.Contains(HudPrimitive->GetMeshPrimitiveComponentId());
						}
						CanvasVisibilityMap.Add(WorldRenderParameter.RenderCanvas, bIsPrimitiveVisible);
					}
				}
			}
		}

		//preprocess these data as seqence render, to reduce GraphBuilder.AddPass count
		TArray<TArray<FWorldSpaceRenderParameter>> RenderSequenceArray;
		ELGUIHudPrimitiveType PrevPrimitiveType = ELGUIHudPrimitiveType::Mesh;
		bool bIsFirst = true;
		for (auto& WorldRenderParameter : WorldSpaceRenderCanvasParameterArray)
		{
			auto HudPrimitive = WorldRenderParameter.HudPrimitive;
			if (HudPrimitive != nullptr)
			{
				if (HudPrimitive->CanRender())
				{
					bool bIsPrimitiveVisible = false;//default is not visible
					if (auto VisibilityPtr = CanvasVisibilityMap.Find(WorldRenderParameter.RenderCanvas))
					{
						bIsPrimitiveVisible = *VisibilityPtr;
					}
					if (bIsPrimitiveVisible)
					{
						if (bIsFirst)
						{
							bIsFirst = false;
							PrevPrimitiveType = HudPrimitive->GetPrimitiveType();
							TArray<FWorldSpaceRenderParameter> Sequence;
							Sequence.Add(WorldRenderParameter);
							RenderSequenceArray.Add(Sequence);
						}
						else
						{
							if (PrevPrimitiveType != HudPrimitive->GetPrimitiveType())
							{
								PrevPrimitiveType = HudPrimitive->GetPrimitiveType();
								TArray<FWorldSpaceRenderParameter> Sequence;
								Sequence.Add(WorldRenderParameter);
								RenderSequenceArray.Add(Sequence);
							}
							else
							{
								auto& Sequence = RenderSequenceArray[RenderSequenceArray.Num() - 1];
								Sequence.Add(WorldRenderParameter);
							}
						}
					}
				}
			}
		}

		for (auto& RenderSequenceItem : RenderSequenceArray)
		{
			switch (RenderSequenceItem[0].HudPrimitive->GetPrimitiveType())
			{
			case ELGUIHudPrimitiveType::PostProcess://render post process
			{
				for (auto& Primitive : RenderSequenceItem)
				{
					Primitive.HudPrimitive->OnRenderPostProcess_RenderThread(
						GraphBuilder,
						SceneTextures,
						this,
						OriginScreenColorTexture,
						ScreenColorRenderTargetTexture,
						GlobalShaderMap,
						ViewProjectionMatrix,
						true,
						Primitive.BlendDepth,
						Primitive.DepthFade,
						ViewRect,
						DepthTextureScaleOffset,
						ViewTextureScaleOffset
					);
				}
			}break;
			case ELGUIHudPrimitiveType::Mesh://render mesh
			{
				auto* PassParameters = GraphBuilder.AllocParameters<FLGUIWorldRenderPSParameter>();
				PassParameters->SceneDepthTex = SceneTextures.Depth.Resolve;
				PassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture, ERenderTargetLoadAction::ELoad);

				GraphBuilder.AddPass(
					RDG_EVENT_NAME("LGUIHudRender_WorldSpace"),
					PassParameters,
					ERDGPassFlags::Raster,
					[this, RenderSequenceItem, RenderView, ViewRect, PassParameters, SceneDepthTexST = DepthTextureScaleOffset, NumSamples, ColorCorrectionValue](FRHICommandListImmediate& RHICmdList)
					{
						FGraphicsPipelineStateInitializer GraphicsPSOInit;
						RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
						RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

						for (auto& Primitive : RenderSequenceItem)
						{
							MeshBatchArray.Reset();
							FLGUIMeshElementCollector meshCollector(RenderView->GetFeatureLevel(), Allocator);
							Primitive.HudPrimitive->GetMeshElements(*RenderView->Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
							for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
							{
								auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
								const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
								auto Material = Mesh.MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material cann't render with LGUIRenderer
								if (!Material)return;

								FLGUIHudRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
									, Material->IsWireframe(), Material->IsTwoSided(), Material->ShouldDisableDepthTest(), false, Mesh.ReverseCulling
								);

								if (Primitive.DepthFade <= 0.0f)
								{
									TShaderRef<FLGUIHudRenderVS> VertexShader;
									TShaderRef<FLGUIWorldRenderPS> PixelShader;
									FMaterialShaderTypes ShaderTypes;
									ShaderTypes.AddShaderType<FLGUIHudRenderVS>();
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
										PixelShader->SetDepthBlendParameter(RHICmdList, Primitive.BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
										PixelShader->SetColorCorrectionValue(RHICmdList, ColorCorrectionValue);

										RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
										RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
									}
								}
								else
								{
									TShaderRef<FLGUIHudRenderVS> VertexShader;
									TShaderRef<FLGUIWorldRenderDepthFadePS> PixelShader;
									FMaterialShaderTypes ShaderTypes;
									ShaderTypes.AddShaderType<FLGUIHudRenderVS>();
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
										PixelShader->SetDepthBlendParameter(RHICmdList, Primitive.BlendDepth, SceneDepthTexST, PassParameters->SceneDepthTex->GetRHI());
										PixelShader->SetDepthFadeParameter(RHICmdList, Primitive.DepthFade);
										PixelShader->SetColorCorrectionValue(RHICmdList, ColorCorrectionValue);

										RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
										RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
									}
								}
							}
						}
					});
			}break;
			}
		}
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LGUI_RenderWorld_Clean"),
			ERDGPassFlags::None,
			[RenderView](FRHICommandListImmediate& RHICmdList)
			{
				delete RenderView;
			});
	}
	//Render screen space
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() > 0
		&& bIsMainViewport
		)
	{
#if WITH_EDITOR
		if (bIsRenderToRenderTarget)
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
		//NOTE!!! world-space and screen-space must use different 'RenderView' (actually different ViewUniformBuffer), because RDG is async. 
		//if use same one, after world-space when modify 'RenderView' for screen-space, the screen-space ViewUniformBuffer will be applyed to world-space
		FSceneView* RenderView = new FSceneView(InView);
		auto GlobalShaderMap = GetGlobalShaderMap(RenderView->GetFeatureLevel());

		RenderView->SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewOrigin;
		RenderView->SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
		RenderView->SceneViewInitOptions.ProjectionMatrix = ScreenSpaceRenderParameter.ProjectionMatrix;
		RenderView->ViewMatrices = FViewMatrices(RenderView->SceneViewInitOptions);

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

		//preprocess these data as seqence render, to reduce GraphBuilder.AddPass count
		TArray<TArray<ILGUIHudPrimitive*>> RenderSequenceArray;
		ELGUIHudPrimitiveType PrevPrimitiveType = ELGUIHudPrimitiveType::Mesh;
		bool bIsFirst = true;
		for (int PrimitiveIndex = 0; PrimitiveIndex < ScreenSpaceRenderParameter.HudPrimitiveArray.Num(); PrimitiveIndex++)
		{
			auto HudPrimitive = ScreenSpaceRenderParameter.HudPrimitiveArray[PrimitiveIndex];
			if (HudPrimitive != nullptr)
			{
				if (HudPrimitive->CanRender())
				{
					if (bIsFirst)
					{
						bIsFirst = false;
						PrevPrimitiveType = HudPrimitive->GetPrimitiveType();
						TArray<ILGUIHudPrimitive*> Sequence;
						Sequence.Add(HudPrimitive);
						RenderSequenceArray.Add(Sequence);
					}
					else
					{
						if (PrevPrimitiveType != HudPrimitive->GetPrimitiveType())
						{
							PrevPrimitiveType = HudPrimitive->GetPrimitiveType();
							TArray<ILGUIHudPrimitive*> Sequence;
							Sequence.Add(HudPrimitive);
							RenderSequenceArray.Add(Sequence);
						}
						else
						{
							auto& Sequence = RenderSequenceArray[RenderSequenceArray.Num() - 1];
							Sequence.Add(HudPrimitive);
						}
					}
				}
			}
		}


		const FMinimalSceneTextures& SceneTextures = ((FViewFamilyInfo*)InView.Family)->GetSceneTextures();
		bool bIsDepthStencilCleared = false;
		for (auto& RenderSequenceItem : RenderSequenceArray)
		{
			switch (RenderSequenceItem[0]->GetPrimitiveType())
			{
			case ELGUIHudPrimitiveType::PostProcess://render post process
			{
				for (auto& Primitive : RenderSequenceItem)
				{
					Primitive->OnRenderPostProcess_RenderThread(
						GraphBuilder,
						SceneTextures,
						this,
						OriginScreenColorTexture,
						ScreenColorRenderTargetTexture,
						GlobalShaderMap,
						ScreenSpaceRenderParameter.ViewProjectionMatrix,
						/*IsWorldSpace*/false,
						/*BlendDepthForWorld*/0.0f,//actually this value will no work because 'IsWorldSpace' is false
						/*BlendDepthForWorld*/0.0f,//actually this value will no work because 'IsWorldSpace' is false
						ViewRect,
						DepthTextureScaleOffset,
						ViewTextureScaleOffset
					);
				}
			}break;//render mesh
			case ELGUIHudPrimitiveType::Mesh:
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
					RDG_EVENT_NAME("LGUIHudRender_ScreenSpace"),
					PassParameters,
					ERDGPassFlags::Raster,
					[this, RenderSequenceItem, RenderView, ViewRect, SceneDepthTexST = DepthTextureScaleOffset, NumSamples, ValidDepth = LGUIScreenSpaceDepthRDGTexture != nullptr, ColorCorrectionValue](FRHICommandListImmediate& RHICmdList)
					{
						for (auto& Primitive : RenderSequenceItem)
						{
							MeshBatchArray.Reset();
							FLGUIMeshElementCollector meshCollector(RenderView->GetFeatureLevel(), Allocator);
							Primitive->GetMeshElements(*RenderView->Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
							for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
							{
								auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
								const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
								auto Material = Mesh.MaterialRenderProxy->GetMaterialNoFallback(RenderView->GetFeatureLevel());//why not use "GetIncompleteMaterialWithFallback" here? because fallback material cann't render with LGUIRenderer
								if (!Material)return;

								TShaderRef<FLGUIHudRenderVS> VertexShader;
								TShaderRef<FLGUIHudRenderPS> PixelShader;
								FMaterialShaderTypes ShaderTypes;
								ShaderTypes.AddShaderType<FLGUIHudRenderVS>();
								ShaderTypes.AddShaderType<FLGUIHudRenderPS>();
								FMaterialShaders Shaders;
								if (Material->TryGetShaders(ShaderTypes, nullptr, Shaders))
								{
									Shaders.TryGetVertexShader(VertexShader);
									Shaders.TryGetPixelShader(PixelShader);

									RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

									FGraphicsPipelineStateInitializer GraphicsPSOInit;
									RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

									FLGUIHudRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, Material->GetBlendMode()
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
									PixelShader->SetColorCorrectionValue(RHICmdList, ColorCorrectionValue);

									RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
									RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.Elements[0].NumPrimitives, Mesh.Elements[0].NumInstances);
								}
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
					FLGUIHudRenderer::SetGraphicPipelineState(RenderView->GetFeatureLevel(), GraphicsPSOInit, EBlendMode::BLEND_Opaque, false, true, true, false, false);

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
				delete RenderView;
			});

		if (LGUIScreenSpaceDepthTexture.IsValid())
		{
			LGUIScreenSpaceDepthTexture.SafeRelease();
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

void FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FWorldSpaceRenderParameter RenderParameter;
		RenderParameter.BlendDepth = InCanvas->GetActualBlendDepth();
		RenderParameter.DepthFade = InCanvas->GetActualDepthFade();
		RenderParameter.RenderCanvas = InCanvas;
		RenderParameter.HudPrimitive = InPrimitive;

		WorldSpaceRenderCanvasParameterArray.Add(RenderParameter);
		SortWorldSpacePrimitiveRenderPriority_RenderThread();
		//check if we have postprocess
		CheckContainsPostProcess_RenderThread();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		int existIndex = WorldSpaceRenderCanvasParameterArray.IndexOfByPredicate([InPrimitive](const FWorldSpaceRenderParameter& item) {
			return item.HudPrimitive == InPrimitive;
			});
		if (existIndex == INDEX_NONE)
		{
			UE_LOG(LGUI, Log, TEXT("[FLGUIHudRenderer::RemoveHudPrimitive]Canvas already removed."));
		}
		else
		{
			WorldSpaceRenderCanvasParameterArray.RemoveAt(existIndex);
			//check if we have postprocess
			CheckContainsPostProcess_RenderThread();
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::RemoveWorldSpacePrimitive_RenderThread]Remove nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::AddScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.HudPrimitiveArray.AddUnique(InPrimitive);
		SortScreenSpacePrimitiveRenderPriority_RenderThread();
		CheckContainsPostProcess_RenderThread();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::AddScreenSpacePrimitive_RenderThread]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::RemoveScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.HudPrimitiveArray.RemoveSingle(InPrimitive);
		CheckContainsPostProcess_RenderThread();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::RemoveScreenSpacePrimitive_RenderThread]Remove nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::SortScreenSpacePrimitiveRenderPriority_RenderThread()
{
	ScreenSpaceRenderParameter.HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
		{
			return A.GetRenderPriority() < B.GetRenderPriority();
		});
}
void FLGUIHudRenderer::SortWorldSpacePrimitiveRenderPriority_RenderThread()
{
	WorldSpaceRenderCanvasParameterArray.Sort([](const FWorldSpaceRenderParameter& A, const FWorldSpaceRenderParameter& B) {
			return A.HudPrimitive->GetRenderPriority() < B.HudPrimitive->GetRenderPriority();
		});
}

void FLGUIHudRenderer::SortScreenAndWorldSpacePrimitiveRenderPriority()
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortScreenAndWorldSpacePrimitiveRenderPriority)(
		[viewExtension](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SortWorldSpacePrimitiveRenderPriority_RenderThread();
			viewExtension->SortScreenSpacePrimitiveRenderPriority_RenderThread();
		}
	);
}
void FLGUIHudRenderer::SetRenderCanvasDepthParameter(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade)
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension, InRenderCanvas, InBlendDepth, InDepthFade](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SetRenderCanvasDepthFade_RenderThread(InRenderCanvas, InBlendDepth, InDepthFade);
		}
	);
}

void FLGUIHudRenderer::SetRenderCanvasDepthFade_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth, float InDepthFade)
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

void FLGUIHudRenderer::SetScreenSpaceRenderCanvas(ULGUICanvas* InCanvas)
{
	ScreenSpaceRenderParameter.RenderCanvas = InCanvas;
}
void FLGUIHudRenderer::ClearScreenSpaceRenderCanvas()
{
	ScreenSpaceRenderParameter.RenderCanvas = nullptr;
}

void FLGUIHudRenderer::SetRenderToRenderTarget(bool InValue)
{
	auto ViewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SetRenderToRenderTarget)(
		[ViewExtension, InValue](FRHICommandListImmediate& RHICmdList)
		{
			ViewExtension->bIsRenderToRenderTarget = InValue;
		}
	);
}
void FLGUIHudRenderer::UpdateRenderTargetRenderer(UTextureRenderTarget2D* InRenderTarget)
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

void FLGUIHudRenderer::CheckContainsPostProcess_RenderThread()
{
	bNeedOriginScreenColorTextureOnPostProcess = false;
	for (auto& item : ScreenSpaceRenderParameter.HudPrimitiveArray)
	{
		if (item->GetPrimitiveType() == ELGUIHudPrimitiveType::PostProcess
			&& item->PostProcessRequireOriginScreenColorTexture()
			)
		{
			bNeedOriginScreenColorTextureOnPostProcess = true;
			return;
		}
	}
	for (auto& renderItem : WorldSpaceRenderCanvasParameterArray)
	{
		if (renderItem.HudPrimitive->GetPrimitiveType() == ELGUIHudPrimitiveType::PostProcess
			&& renderItem.HudPrimitive->PostProcessRequireOriginScreenColorTexture()
			)
		{
			bNeedOriginScreenColorTextureOnPostProcess = true;
			return;
		}
	}
}

#if WITH_EDITOR
void FLGUIHudRenderer::AddLineRender(const FLGUIHelperLineRenderParameter& InLineParameter)
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
