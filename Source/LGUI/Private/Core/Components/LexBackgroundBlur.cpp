// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackgroundBlur.h"

#include "LGUI.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "PipelineStateCache.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualBackBufferRenderProxy.h"
#include "RHIStaticStates.h"
#include "Core/Components/LexWidget.h"

ULexBackgroundBlur::ULexBackgroundBlur(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	
}

#if WITH_EDITOR
void ULexBackgroundBlur::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(ULexBackgroundBlur, MaxDownSampleLevel))
		{
			MaxDownSampleLevel += 1;//just make it work
			SetMaxDownSampleLevel(MaxDownSampleLevel - 1);
		}
	}
}
#endif


void ULexBackgroundBlur::MarkAllDirty()
{
	Super::MarkAllDirty();

	SendRegionVertexDataToRenderProxy();
	SendMaskTextureToRenderProxy();
	SendOthersDataToRenderProxy();
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundBlur"), STAT_BackgroundBlur, STATGROUP_LGUI);
class FUIBackgroundBlurRenderProxy : public FLexVisualPostProcessRenderProxy
{
public:
	int MaxDownSampleLevel = 0;
	float BlurStrength = 0.0f;
	ELexBackGroundBlurType BlurType = ELexBackGroundBlurType::Gaussian;
	const float MagicNumber = 1.0f / 2.2f;//this is a magic number which can make blur transition feel smooth
public:
	FUIBackgroundBlurRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return BlurStrength > 0.0f;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix44f& ViewProjectionMatrix,
		bool bIsWorldSpace,
		bool bIsRenderTarget,
		float BlendDepthForWorld,
		int DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	) override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
		if (BlurStrength <= 0.0f)return;

		auto& RHICmdList = GraphBuilder.RHICmdList;

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedRenderTarget;
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget;
		auto ReleaseRenderTarget = [&] {
			if (ScreenResolvedRenderTarget.IsValid())
			{
				ScreenResolvedRenderTarget.SafeRelease();
			}
			if (BlurEffectRenderTarget.IsValid())
			{
				BlurEffectRenderTarget.SafeRelease();
			}
		};

		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		if (NumSamples > 1)
		{
			auto ScreenSize = ScreenTargetTexture->GetSizeXY();
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenResolvedRenderTarget, TEXT("LexUIBlurEffectResolveTarget"));
			if (!ScreenResolvedRenderTarget.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LexUIBlurEffectResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedRenderTarget->GetRHI(), TEXT("LexUIBlurEffectResolveTarget"));
			FLexUIRenderer::AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
		}
		
		//get render target
		{
			auto RectSize = MeshRectInScreen.GetSize();
			auto RectWidth = FMath::Max(RectSize.X, 1.0f);
			auto RectHeight = FMath::Max(RectSize.Y, 1.0f);
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(RectWidth, RectHeight), ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget, TEXT("LexUIBlurEffectRenderTarget1"));
			if (!BlurEffectRenderTarget.IsValid())
			{
				ReleaseRenderTarget();
				return;
			}
		}
		FRHITexture* BlurEffectRHITexture = BlurEffectRenderTarget->GetRHI();

		auto BlurEffectRDGTextureRef = RegisterExternalTexture(GraphBuilder, BlurEffectRHITexture, TEXT("LexUIBlurEffectRenderTexture_ExternalTexture"));
		//clear the whole target first so the area not covered by the mesh region is deterministic.
#if 0
		{
			auto* ClearParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			ClearParameters->RenderTargets[0] = FRenderTargetBinding(BlurEffectRDGTextureRef, ERenderTargetLoadAction::EClear);
			GraphBuilder.AddPass(RDG_EVENT_NAME("LexUIBackgroundBlur_ClearRegionTarget"), ClearParameters, ERDGPassFlags::Raster, [](FRHICommandListImmediate&) {});
		}
#endif
		FLexUIRenderer::CopyRenderTargetOnMeshRegion(GraphBuilder
			, BlurEffectRDGTextureRef
			, NumSamples > 1 ? ScreenResolvedRenderTarget->GetRHI() : ScreenTargetTexture.GetReference()
			, GlobalShaderMap
			, RenderScreenToMeshRegionVertexArray
			, bIsRenderTarget
			, FIntRect(0, 0, BlurEffectRHITexture->GetSizeXYZ().X, BlurEffectRHITexture->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
		);

		uint32 SourceWidth = BlurEffectRHITexture->GetSizeX();
		uint32 SourceHeight = BlurEffectRHITexture->GetSizeY();
		auto MaxDownSampleCount = FMath::Min3(FMath::FloorLog2(SourceWidth), FMath::FloorLog2(SourceHeight), static_cast<uint32>(MaxDownSampleLevel));
		float FilteredBlurStrength = FMath::Pow(BlurStrength, MagicNumber) * MaxDownSampleCount;//convert BlurStrength from 0~1 to 0~Count, with adjusted curvature
		FRHITexture* PrevRT = BlurEffectRHITexture;
		SourceWidth = BlurEffectRHITexture->GetSizeX();
		SourceHeight = BlurEffectRHITexture->GetSizeY();
		TArray<TRefCountPtr<IPooledRenderTarget>> DownSampleRenderTargetArray;//store rt from big to small
		for (int i = MaxDownSampleCount; i >= 1; i--)
		{
			if (FilteredBlurStrength >= i)
			{
				SourceWidth >>= 1;
				SourceHeight >>= 1;
				TRefCountPtr<IPooledRenderTarget> DownSampleRT;
				FPooledRenderTargetDesc RenderTargetDesc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(SourceWidth, SourceHeight)
					, BlurEffectRHITexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
				GRenderTargetPool.FindFreeElement(RHICmdList, RenderTargetDesc, DownSampleRT, *FString::Printf(TEXT("LexUI_DownsampleRT_%d"), i));
				DownSampleRenderTargetArray.Add(DownSampleRT);
				if (BlurType == ELexBackGroundBlurType::Gaussian)
				{
#if 0
					FLexUIRenderer::CopyRenderTarget(GraphBuilder, GlobalShaderMap, PrevRT, DownSampleRT->GetRHI());
#else//use DualKawaseDownSample can solve aliasing flickering, but cost more
					DualKawaseDownSample(GraphBuilder, GlobalShaderMap, 1, PrevRT, DownSampleRT->GetRHI());
#endif
				}
				else
				{
					DualKawaseDownSample(GraphBuilder, GlobalShaderMap, 1, PrevRT, DownSampleRT->GetRHI());
				}
			
				PrevRT = DownSampleRT->GetRHI();
			}
		}
		if (BlurType == ELexBackGroundBlurType::Gaussian)
		{
			for (int i = MaxDownSampleCount; i >= 1; i--)
			{
				if (FilteredBlurStrength >= i)
				{
					auto RenderTarget = DownSampleRenderTargetArray[i - 1];
					DoGaussianBlur(RenderTarget->GetRHI(), FilteredBlurStrength - i, GraphBuilder, GlobalShaderMap);
					auto NextRT = i == 1 ? BlurEffectRHITexture : DownSampleRenderTargetArray[i - 2]->GetRHI();
					if (FilteredBlurStrength >= i + 1)
					{
						FLexUIRenderer::CopyRenderTarget(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT);
					}
					else
					{
						auto BlendValue = FMath::Clamp(FilteredBlurStrength - i, 0.0f, 1.0f);
						BlendValue = FMath::Pow(BlendValue, MagicNumber);
						FLexUIRenderer::CopyRenderTarget_BlendAlpha(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT, BlendValue);
					}
				}
			}
			DoGaussianBlur(BlurEffectRHITexture, FilteredBlurStrength, GraphBuilder, GlobalShaderMap);
		}
		else
		{
			for (int i = MaxDownSampleCount; i >= 1; i--)
			{
				if (FilteredBlurStrength >= i)
				{
					auto RenderTarget = DownSampleRenderTargetArray[i - 1];
					auto NextRT = i == 1 ? BlurEffectRHITexture : DownSampleRenderTargetArray[i - 2]->GetRHI();
					if (FilteredBlurStrength >= i + 1)
					{
						DualKawaseUpSample(GraphBuilder, GlobalShaderMap, FilteredBlurStrength - i, RenderTarget->GetRHI(), NextRT);
					}
					else
					{
						DualKawaseUpSample(GraphBuilder, GlobalShaderMap, 1, RenderTarget->GetRHI(), NextRT);
						auto BlendValue = FMath::Clamp(FilteredBlurStrength - i, 0.0f, 1.0f);
						BlendValue = FMath::Pow(BlendValue, MagicNumber);
						FLexUIRenderer::CopyRenderTarget_BlendAlpha(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT, 1-BlendValue);
					}
				}
			}
		}

		//after blur process, copy the blur result image back to screen image of the area
		if (bFullViewport)
		{
			//copy full viewport
			FLexUIRenderer::CopyRenderTarget(GraphBuilder, GlobalShaderMap, BlurEffectRHITexture, ScreenTargetTexture);
		}
		else
		{
			//copy on mesh region
			auto ModelViewProjectionMatrix = ObjectToWorldMatrix * ViewProjectionMatrix;
			RenderMeshOnScreen_RenderThread(GraphBuilder, SceneTextures, ScreenTargetTexture, GlobalShaderMap, BlurEffectRHITexture, ModelViewProjectionMatrix, ObjectToWorldMatrix, bIsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect);
		}

		//Defer releasing the pooled render targets until the graph executes
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_ReleaseRenderTargets"),
			ERDGPassFlags::None,
			[ScreenResolvedRenderTarget, BlurEffectRenderTarget, DownSampleRenderTargetArray](FRHICommandListImmediate& RHICmdList) mutable
			{
				if (ScreenResolvedRenderTarget.IsValid()) ScreenResolvedRenderTarget.SafeRelease();
				if (BlurEffectRenderTarget.IsValid()) BlurEffectRenderTarget.SafeRelease();
				for (auto& RenderTarget : DownSampleRenderTargetArray)
				{
					if (RenderTarget.IsValid()) RenderTarget.SafeRelease();
				}
			});
	}
	void DoGaussianBlur(FRHITexture* RenderTargetTexture
		, float BlurAmount
		, FRDGBuilder& GraphBuilder
		, FGlobalShaderMap* GlobalShaderMap
		)
	{
		FRDGTextureRef InputTexture = RegisterExternalTexture(GraphBuilder, RenderTargetTexture, TEXT("LexUIBlurInputTexture"));
		
		TRefCountPtr<IPooledRenderTarget> DownSampleRT_Blur;
		FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(RenderTargetTexture->GetSizeX(), RenderTargetTexture->GetSizeY())
			, RenderTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
		GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, desc, DownSampleRT_Blur, *FString::Printf(TEXT("LexUI_DownsampleRT_Blur")));
		auto RenderTargetTexture_Blur = DownSampleRT_Blur->GetRHI();
		
		TShaderMapRef<FLexUISimplePostProcessVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FLexUIPostProcessGaussianBlurPS> PixelShader(GlobalShaderMap);
		auto SamplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

		if (BlurAmount < 1)//if BlurAmount is 0~1 then use MagicNumber to make blur feel smooth
		{
			BlurAmount = FMath::Pow(BlurAmount, MagicNumber);
		}
		else
		{
			BlurAmount = 1;//clamp to 1, use full offset value
		}
		auto BlurStrength2 = FVector2f(1.0f / RenderTargetTexture->GetSizeX(), 1.0f / RenderTargetTexture->GetSizeY()) * BlurAmount;
				
		// Use the pooled overload (member) so RDG keeps the temp render target alive until the graph executes, no manual SafeRelease is needed afterwards.
		auto RenderTargetTexture_Blur_RDG = GraphBuilder.RegisterExternalTexture(DownSampleRT_Blur, TEXT("LexUIBlurTempTexture"), ERDGTextureFlags::None);
		
		auto* HorizontalPassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		HorizontalPassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(InputTexture));
		HorizontalPassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture_Blur_RDG, ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_Pass_Horizontal"),
			HorizontalPassParameters,
			ERDGPassFlags::Raster,
			[this, VertexShader, PixelShader, MainTexture = RenderTargetTexture, SamplerState, BlurStrength2](FRHICommandListImmediate& RHICmdList)
			{
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				VertexShader->SetParameters(RHICmdList);
				//render horizontal
				RHICmdList.SetViewport(0, 0, 0.0f, MainTexture->GetSizeX(), MainTexture->GetSizeY(), 1.0f);
				PixelShader->SetMainTexture(RHICmdList, MainTexture, SamplerState);
				PixelShader->SetBlurStrength(RHICmdList, FVector2f(BlurStrength2.X, 0));
				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});

		auto* VerticalPassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		VerticalPassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(RenderTargetTexture_Blur_RDG));
		VerticalPassParameters->RenderTargets[0] = FRenderTargetBinding(InputTexture, ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_Pass_Vertical"),
			VerticalPassParameters,
			ERDGPassFlags::Raster,
			[this, VertexShader, PixelShader, MainTexture = RenderTargetTexture_Blur, SamplerState, BlurStrength2](FRHICommandListImmediate& RHICmdList)
			{
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLexUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				VertexShader->SetParameters(RHICmdList);
				//render vertical
				RHICmdList.SetViewport(0, 0, 0.0f, MainTexture->GetSizeX(), MainTexture->GetSizeY(), 1.0f);
				PixelShader->SetMainTexture(RHICmdList, MainTexture, SamplerState);
				PixelShader->SetBlurStrength(RHICmdList, FVector2f(0, BlurStrength2.Y));
				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});
	}
	void DualKawaseDownSample(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap
		, float BlurAmount
		, FTextureRHIRef Src, FTextureRHIRef Dst
	)
	{
		if (BlurAmount < 1)//if BlurAmount is 0~1 then use MagicNumber to make blur feel smooth
		{
			BlurAmount = FMath::Pow(BlurAmount, MagicNumber);
		}
		else
		{
			BlurAmount = 1;//clamp to 1, use full offset value
		}
		auto BlurStrength2 = FVector2f(1.0f / Src->GetSizeX(), 1.0f / Src->GetSizeY()) * BlurAmount;
		
		auto SrcTexture = RegisterExternalTexture(GraphBuilder, Src.GetReference(), TEXT("LexUIDualKawaseDownSampleSrc"));
		auto* PassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		PassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(SrcTexture));
		PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst.GetReference(), TEXT("LexUIDualKawaseDownSampleRenderTarget")), ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIDualKawaseDownSamplePass"),
			PassParameters,
			ERDGPassFlags::Raster,
			[this, GlobalShaderMap, Src, Dst, BlurStrength2](FRHICommandListImmediate& RHICmdList)
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
				TShaderMapRef<FLexUIPostProcessDualKawaseBlurDownSamplePS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				PixelShader->SetMainTexture(RHICmdList, Src, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
				PixelShader->SetBlurStrength(RHICmdList, BlurStrength2);
				VertexShader->SetParameters(RHICmdList);

				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});
	}
	void DualKawaseUpSample(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap
		, float BlurAmount
		, FTextureRHIRef Src, FTextureRHIRef Dst
	)
	{
		if (BlurAmount < 1)//if BlurAmount is 0~1 then use MagicNumber to make blur feel smooth
		{
			BlurAmount = FMath::Pow(BlurAmount, MagicNumber);
		}
		else
		{
			BlurAmount = 1;//clamp to 1, use full offset value
		}
		auto BlurStrength2 = FVector2f(1.0f / Src->GetSizeX(), 1.0f / Src->GetSizeY()) * BlurAmount;
		
		auto SrcTexture = RegisterExternalTexture(GraphBuilder, Src.GetReference(), TEXT("LexUIDualKawaseUpSampleSrc"));
		auto* PassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		PassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(SrcTexture));
		PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst.GetReference(), TEXT("LexUIDualKawaseUpSampleRenderTarget")), ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIDualKawaseUpSamplePass"),
			PassParameters,
			ERDGPassFlags::Raster,
			[this, GlobalShaderMap, Src, Dst, BlurStrength2](FRHICommandListImmediate& RHICmdList)
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
				TShaderMapRef<FLexUIPostProcessDualKawaseBlurUpSamplePS> PixelShader(GlobalShaderMap);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
				PixelShader->SetMainTexture(RHICmdList, Src, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
				PixelShader->SetBlurStrength(RHICmdList, BlurStrength2);
				VertexShader->SetParameters(RHICmdList);

				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});
	}
};


void ULexBackgroundBlur::SendOthersDataToRenderProxy()
{
	if (RenderProxy != nullptr)
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)RenderProxy;
		ENQUEUE_RENDER_COMMAND(FLexBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy
				, BlurStrengthWithAlpha = this->GetBlurStrengthInternal()
				, MaxDownSampleLevel = this->MaxDownSampleLevel
				, BlurType = this->BlurType
				](FRHICommandListImmediate& RHICmdList)
			{
				BackgroundBlurRenderProxy->BlurStrength = BlurStrengthWithAlpha;
				BackgroundBlurRenderProxy->MaxDownSampleLevel = MaxDownSampleLevel;
				BackgroundBlurRenderProxy->BlurType = BlurType;
			});
	}
}

void ULexBackgroundBlur::SetBlurStrength(float Value)
{
	if (BlurStrength != Value)
	{
		BlurStrength = Value;
		SendOthersDataToRenderProxy();
	}
}

void ULexBackgroundBlur::SetApplyOpacityToBlur(bool Value)
{
	if (ApplyOpacityToBlur != Value)
	{
		ApplyOpacityToBlur = Value;
		SendOthersDataToRenderProxy();
	}
}

void ULexBackgroundBlur::SetBlurType(ELexBackGroundBlurType Value)
{
	if (BlurType != Value)
	{
		BlurType = Value;
	}
}

void ULexBackgroundBlur::SetMaxDownSampleLevel(int Value)
{
	if (MaxDownSampleLevel != Value)
	{
		MaxDownSampleLevel = Value;
		SendOthersDataToRenderProxy();
	}
}

float ULexBackgroundBlur::GetBlurStrengthInternal()
{
	if (ApplyOpacityToBlur)
	{
		if (auto Widget = GetWidget())
		{
			return Widget->GetRenderOpacity() * BlurStrength;
		}
	}
	return BlurStrength;
}

FLexVisualBackBufferRenderProxy* ULexBackgroundBlur::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackgroundBlurRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
		SendOthersDataToRenderProxy();
	}
	return RenderProxy;
}

void ULexBackgroundBlur::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	if (RenderProxy != nullptr)
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)RenderProxy;
		auto blurStrengthWithAlpha = this->GetBlurStrengthInternal();
		ENQUEUE_RENDER_COMMAND(FLexBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, blurStrengthWithAlpha](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->BlurStrength = blurStrengthWithAlpha;
				});
	}
}
