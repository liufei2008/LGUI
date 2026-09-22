// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackBufferCopy.h"

#include "LGUI.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualBackBufferRenderProxy.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "Engine/TextureRenderTarget2D.h"

ULexBackBufferCopy::ULexBackBufferCopy(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	
}

#if WITH_EDITOR
void ULexBackBufferCopy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(ULexBackBufferCopy, BackBufferCopyFilter))
		{
			SendFilterToRenderProxy();
		}
	}
	SendRenderTargetToRenderProxy();
}
#endif


void ULexBackBufferCopy::MarkAllDirty()
{
	Super::MarkAllDirty();

	SendRegionVertexDataToRenderProxy();
	SendRenderTargetToRenderProxy();
	SendFilterToRenderProxy();
}

void ULexBackBufferCopy::ClearMaterialsUsingThisBackBuffer()
{
	MaterialsUsingThisBackBuffer.Reset();
}

void ULexBackBufferCopy::RegisterMaterialsUsingThisBackBuffer(UMaterialInstanceDynamic* InMaterialInstanceDynamic)
{
	MaterialsUsingThisBackBuffer.Add(InMaterialInstanceDynamic);
	InMaterialInstanceDynamic->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, RenderTarget);
	InMaterialInstanceDynamic->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackBufferCopy"), STAT_BackBufferCopy, STATGROUP_LGUI);
class FUIBackBufferCopyRenderProxy : public FLexVisualBackBufferRenderProxy
{
public:
	FLexBackBufferCopyFilterRenderProxy* BackBufferCopyFilter = nullptr;
	//output target
	FTextureRenderTargetResource* RenderTargetResource = nullptr;
	
	FUIBackBufferCopyRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return RenderTargetResource != nullptr;
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
		SCOPE_CYCLE_COUNTER(STAT_BackBufferCopy);

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedRenderTarget;
		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		auto ScreenSize = ScreenTargetTexture->GetSizeXY();
		if (NumSamples > 1)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, desc, ScreenResolvedRenderTarget, TEXT("LexUIBackBufferCopyResolveTarget"));
			if (!ScreenResolvedRenderTarget.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LexUIBackBufferCopyResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedRenderTarget->GetRHI(), TEXT("LexUIBackBufferCopyResolveTarget"));
			FLexUIRenderer::AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
		}
		
		auto RenderTargetRHITexture = RenderTargetResource->GetRenderTargetTexture();
		auto RenderTargetTextureRDG = RegisterExternalTexture(GraphBuilder, RenderTargetRHITexture, TEXT("LexUIBackBufferCopy_RDG"));
		//clear the whole target first so the area not covered by the mesh region is deterministic.
#if 0
		{
			auto* ClearParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			ClearParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTextureRDG, ERenderTargetLoadAction::EClear);
			GraphBuilder.AddPass(RDG_EVENT_NAME("LexUIBackBufferCopy_ClearRegionTarget"), ClearParameters, ERDGPassFlags::Raster, [](FRHICommandListImmediate&) {});
		}
#endif
		FLexUIRenderer::CopyRenderTargetOnMeshRegion(GraphBuilder
			, RenderTargetTextureRDG
			, NumSamples > 1 ? ScreenResolvedRenderTarget->GetRHI() : ScreenTargetTexture.GetReference()
			, GlobalShaderMap
			, RenderScreenToMeshRegionVertexArray
			, bIsRenderTarget
			, FIntRect(0, 0, RenderTargetRHITexture->GetSizeXYZ().X, RenderTargetRHITexture->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
			, true//linearize it, so material sample will get correct color
		);

		if (BackBufferCopyFilter != nullptr)
		{
			BackBufferCopyFilter->DoFilter(GraphBuilder, GlobalShaderMap, RenderTargetRHITexture);
		}

		//Defer releasing the pooled render targets until the graph executes
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackBufferCopy_ReleaseRenderTargets"),
			ERDGPassFlags::None,
			[ScreenResolvedRenderTarget](FRHICommandListImmediate& RHICmdList) mutable
			{
				if (ScreenResolvedRenderTarget.IsValid()) ScreenResolvedRenderTarget.SafeRelease();
			});
	}
};

void ULexBackBufferCopy::OnRegister()
{
	Super::OnRegister();
}

void ULexBackBufferCopy::OnUnregister()
{
	Super::OnUnregister();
	OnRenderTargetChanged.Broadcast(nullptr);
}

void ULexBackBufferCopy::PostUpdateDrawCall()
{
	Super::PostUpdateDrawCall();
	UpdateRenderTarget();
	for (auto& MID : MaterialsUsingThisBackBuffer)
	{
		if (!MID.IsValid())continue;
		MID->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
	}
}

FLexVisualBackBufferRenderProxy* ULexBackBufferCopy::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackBufferCopyRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendRenderTargetToRenderProxy();
		SendFilterToRenderProxy();
	}
	return RenderProxy;
}

void ULexBackBufferCopy::SendRenderTargetToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FUIBackBufferCopyRenderProxy*)RenderProxy;
		FTextureRenderTargetResource* RenderTargetResource = nullptr;
		if (IsValid(RenderTarget))
		{
			RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
		}
		else
		{
			RenderTargetResource = nullptr;
		}
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, RenderTargetResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->RenderTargetResource = RenderTargetResource;
				});
	}
}

void ULexBackBufferCopy::SendFilterToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FUIBackBufferCopyRenderProxy*)RenderProxy;
		auto Filter = IsValid(BackBufferCopyFilter) ? BackBufferCopyFilter->GetRenderProxy() : nullptr;
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, Filter](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->BackBufferCopyFilter = Filter;
				});
	}
}

void ULexBackBufferCopy::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (RenderTarget != InRenderTarget)
	{
		RenderTarget = InRenderTarget;
		UpdateRenderTarget();
		OnRenderTargetChanged.Broadcast(RenderTarget);
	}
}

void ULexBackBufferCopy::UpdateRenderTarget()
{
	auto DesiredRenderTargetSize = MeshRectInScreen.GetSize();
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X < 1 || DesiredRenderTargetSize.Y < 1)
	{
		return;
	}
	DesiredRenderTargetSize.X = FMath::Min(DesiredRenderTargetSize.X, MaxAllowedDrawSize);
	DesiredRenderTargetSize.Y = FMath::Min(DesiredRenderTargetSize.Y, MaxAllowedDrawSize);

	if (RenderTarget == nullptr)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, EObjectFlags::RF_Transient);
		RenderTarget->AddressX = TextureAddress::TA_Clamp;
		RenderTarget->AddressY = TextureAddress::TA_Clamp;
		RenderTarget->ClearColor = FLinearColor::Transparent;
		RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
		SendRenderTargetToRenderProxy();
		OnRenderTargetChanged.Broadcast(RenderTarget);
		//update material's texture, because OutputRenderTarget could be null when register
		for (auto& MID : MaterialsUsingThisBackBuffer)
		{
			if (!MID.IsValid())continue;
			MID->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, RenderTarget);
		}
	}
	else
	{
		if (RenderTarget->SizeX != DesiredRenderTargetSize.X || RenderTarget->SizeY != DesiredRenderTargetSize.Y)
		{
			RenderTarget->ClearColor = FLinearColor::Transparent;
			RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
			RenderTarget->UpdateResourceImmediate();
#if WITH_EDITOR
			RenderTarget->Modify();
#endif
			SendRenderTargetToRenderProxy();
		}
	}

#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		if (!RenderTarget->GameThread_GetRenderTargetResource())
		{
			RenderTarget->InitCustomFormat(RenderTarget->SizeX, RenderTarget->SizeY, EPixelFormat::PF_B8G8R8A8, false);
			SendRenderTargetToRenderProxy();
		}
	}
#endif
}

void ULexBackBufferCopyFilter::BeginDestroy()
{
	ENQUEUE_RENDER_COMMAND(ULexBackBufferCopyFilter_ReleaseRenderProxy)
			([RenderProxyPtr = RenderProxy](FRHICommandListImmediate& RHICmdList)
				{
					delete RenderProxyPtr;
				});
	Super::BeginDestroy();
}

struct FLexBackBufferCopyFilterRenderProxy_Blur : public FLexBackBufferCopyFilterRenderProxy
{
	void DoGaussianBlur(FRHITexture* RenderTargetTexture
		, float BlurAmount
		, float MagicNumber
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
		, FTextureRHIRef Src, FTextureRHIRef Dst
	)
	{
		auto BlurStrength = FVector2f(1.0f / Src->GetSizeX(), 1.0f / Src->GetSizeY());
		
		auto SrcTexture = RegisterExternalTexture(GraphBuilder, Src.GetReference(), TEXT("LexUIDualKawaseDownSampleSrc"));
		auto* PassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		PassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(SrcTexture));
		PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst.GetReference(), TEXT("LexUIDualKawaseDownSampleRenderTarget")), ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIDualKawaseDownSamplePass"),
			PassParameters,
			ERDGPassFlags::Raster,
			[this, GlobalShaderMap, Src, Dst, BlurStrength](FRHICommandListImmediate& RHICmdList)
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
				PixelShader->SetBlurStrength(RHICmdList, BlurStrength);
				VertexShader->SetParameters(RHICmdList);

				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});
	}
	void DualKawaseUpSample(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap
		, FTextureRHIRef Src, FTextureRHIRef Dst
	)
	{
		auto BlurStrength = FVector2f(1.0f / Src->GetSizeX(), 1.0f / Src->GetSizeY());
		
		auto SrcTexture = RegisterExternalTexture(GraphBuilder, Src.GetReference(), TEXT("LexUIDualKawaseUpSampleSrc"));
		auto* PassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		PassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(SrcTexture));
		PassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, Dst.GetReference(), TEXT("LexUIDualKawaseUpSampleRenderTarget")), ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIDualKawaseUpSamplePass"),
			PassParameters,
			ERDGPassFlags::Raster,
			[this, GlobalShaderMap, Src, Dst, BlurStrength](FRHICommandListImmediate& RHICmdList)
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
				PixelShader->SetBlurStrength(RHICmdList, BlurStrength);
				VertexShader->SetParameters(RHICmdList);

				FLexUIRenderer::DrawFullScreenQuad(RHICmdList);
			});
	}
};

struct FLexBackBufferCopyFilterRenderProxy_GaussianBlur : public FLexBackBufferCopyFilterRenderProxy_Blur
{
	float BlurStrength = 0.2f;
	int MaxDownSampleLevel = 7;
	
	virtual void DoFilter(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef RenderTargetRHITexture)override
	{
		auto BlurEffectRHITexture = RenderTargetRHITexture.GetReference();
		float MagicNumber = 1.0f / 2.2f;//this is a magic number which can make blur transition feel smooth
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
				GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, RenderTargetDesc, DownSampleRT, *FString::Printf(TEXT("LexUI_DownsampleRT_%d"), i));
				DownSampleRenderTargetArray.Add(DownSampleRT);
#if 0
				FLexUIRenderer::CopyRenderTarget(GraphBuilder, GlobalShaderMap, PrevRT, DownSampleRT->GetRHI());
#else//use DualKawaseDownSample can solve aliasing flickering, but cost more
				DualKawaseDownSample(GraphBuilder, GlobalShaderMap, PrevRT, DownSampleRT->GetRHI());
#endif
				PrevRT = DownSampleRT->GetRHI();
			}
		}

		for (int i = MaxDownSampleCount; i >= 1; i--)
		{
			if (FilteredBlurStrength >= i)
			{
				auto RenderTarget = DownSampleRenderTargetArray[i - 1];
				DoGaussianBlur(RenderTarget->GetRHI(), FilteredBlurStrength - i, MagicNumber, GraphBuilder, GlobalShaderMap);
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
		DoGaussianBlur(BlurEffectRHITexture, FilteredBlurStrength, MagicNumber, GraphBuilder, GlobalShaderMap);


		//Defer releasing the pooled render targets until the graph executes
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_ReleaseRenderTargets"),
			ERDGPassFlags::None,
			[DownSampleRenderTargetArray](FRHICommandListImmediate& RHICmdList) mutable
			{
				for (auto& RenderTarget : DownSampleRenderTargetArray)
				{
					if (RenderTarget.IsValid()) RenderTarget.SafeRelease();
				}
			});
	}
};
struct FLexBackBufferCopyFilterRenderProxy_DualKawaseBlur : public FLexBackBufferCopyFilterRenderProxy_Blur
{
	int DownSampleLevel = 7;
	
	virtual void DoFilter(FRDGBuilder& GraphBuilder, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef RenderTargetRHITexture)override
	{
		auto BlurEffectRHITexture = RenderTargetRHITexture.GetReference();
		uint32 SourceWidth = BlurEffectRHITexture->GetSizeX();
		uint32 SourceHeight = BlurEffectRHITexture->GetSizeY();
		FRHITexture* PrevRT = BlurEffectRHITexture;
		TArray<TRefCountPtr<IPooledRenderTarget>> DownSampleRenderTargetArray;//store rt from big to small
		for (int i = DownSampleLevel; i >= 1; i--)
		{
			SourceWidth >>= 1;
			SourceHeight >>= 1;
			if (SourceWidth <= 2 || SourceHeight <= 2)break;
			TRefCountPtr<IPooledRenderTarget> DownSampleRT;
			FPooledRenderTargetDesc RenderTargetDesc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(SourceWidth, SourceHeight)
				, BlurEffectRHITexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, RenderTargetDesc, DownSampleRT, *FString::Printf(TEXT("LexUI_DownsampleRT_%d"), i));
			DownSampleRenderTargetArray.Add(DownSampleRT);
			DualKawaseDownSample(GraphBuilder, GlobalShaderMap, PrevRT, DownSampleRT->GetRHI());
			PrevRT = DownSampleRT->GetRHI();
		}

		for (int i = DownSampleRenderTargetArray.Num(); i >= 1; i--)
		{
			auto RenderTarget = DownSampleRenderTargetArray[i - 1];
			auto NextRT = i == 1 ? BlurEffectRHITexture : DownSampleRenderTargetArray[i - 2]->GetRHI();
			DualKawaseUpSample(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT);
		}


		//Defer releasing the pooled render targets until the graph executes
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_ReleaseRenderTargets"),
			ERDGPassFlags::None,
			[DownSampleRenderTargetArray](FRHICommandListImmediate& RHICmdList) mutable
			{
				for (auto& RenderTarget : DownSampleRenderTargetArray)
				{
					if (RenderTarget.IsValid()) RenderTarget.SafeRelease();
				}
			});
	}
};

void ULexBackBufferCopyFilter_GaussianBlur::SendDataToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FLexBackBufferCopyFilterRenderProxy_GaussianBlur*)RenderProxy;
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, BlurStrength = BlurStrength, MaxDownSampleLevel = MaxDownSampleLevel](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->BlurStrength = BlurStrength;
					TempRenderProxy->MaxDownSampleLevel = MaxDownSampleLevel;
				});
	}
}

#if WITH_EDITOR
void ULexBackBufferCopyFilter_GaussianBlur::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	SendDataToRenderProxy();
}
#endif

FLexBackBufferCopyFilterRenderProxy* ULexBackBufferCopyFilter_GaussianBlur::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FLexBackBufferCopyFilterRenderProxy_GaussianBlur();
		SendDataToRenderProxy();
	}
	return RenderProxy;
}


void ULexBackBufferCopyFilter_DualKawaseBlur::SendDataToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FLexBackBufferCopyFilterRenderProxy_DualKawaseBlur*)RenderProxy;
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, DownSampleLevel = FMath::Clamp(DownSampleLevel, 0, 10)](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->DownSampleLevel = DownSampleLevel;
				});
	}
}
#if WITH_EDITOR
void ULexBackBufferCopyFilter_DualKawaseBlur::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	DownSampleLevel = FMath::Clamp(DownSampleLevel, 0, 10);
	SendDataToRenderProxy();
}
#endif

FLexBackBufferCopyFilterRenderProxy* ULexBackBufferCopyFilter_DualKawaseBlur::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FLexBackBufferCopyFilterRenderProxy_DualKawaseBlur();
		SendDataToRenderProxy();
	}
	return RenderProxy;
}
