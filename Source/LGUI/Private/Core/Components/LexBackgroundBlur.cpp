// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackgroundBlur.h"

#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "PipelineStateCache.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "RHIStaticStates.h"

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
public:
	FUIBackgroundBlurRenderProxy()
		:FLexVisualPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return BlurStrength > 0.0f;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FLexUIRenderer* Renderer,
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
		if (BlurStrength <= 0.0f && RenderTargetResource == nullptr)return;

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
		auto ScreenSize = ScreenTargetTexture->GetSizeXY();
		if (NumSamples > 1)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenResolvedRenderTarget, TEXT("LexUIBlurEffectResolveTarget"));
			if (!ScreenResolvedRenderTarget.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LexUIBlurEffectResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedRenderTarget->GetRHI(), TEXT("LexUIBlurEffectResolveTarget"));
			Renderer->AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
		}
		
		//get render target
		{
			float RectWidth = RectSize.X;
			float RectHeight = RectSize.Y;
			RectWidth = FMath::Max(RectWidth, 1.0f);
			RectHeight = FMath::Max(RectHeight, 1.0f);
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(RectWidth, RectHeight), ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			if (RenderTargetResource == nullptr)
			{
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget, TEXT("LexUIBlurEffectRenderTarget1"));
				if (!BlurEffectRenderTarget.IsValid())
				{
					ReleaseRenderTarget();
					return;
				}
			}
			else
			{
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget, TEXT("LexUIBlurEffectRenderTarget1"));
				if (!BlurEffectRenderTarget.IsValid())
				{
					ReleaseRenderTarget();
					return;
				}
			}
		}
		FRHITexture* BlurEffectRHITexture = nullptr;
		if (RenderTargetResource == nullptr)
		{
			BlurEffectRHITexture = BlurEffectRenderTarget->GetRHI();
		}
		else
		{
			BlurEffectRHITexture = BlurEffectRenderTarget->GetRHI();
		}

		auto ModelViewProjectionMatrix = ObjectToWorldMatrix * ViewProjectionMatrix;
		auto BlurEffectRDGTextureRef = RegisterExternalTexture(GraphBuilder, BlurEffectRHITexture, TEXT("LexUIBlurEffectRenderTexture_ExternalTexture"));
		//clear the whole target first so the area not covered by the mesh region is deterministic.
#if 0
		{
			auto* ClearParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			ClearParameters->RenderTargets[0] = FRenderTargetBinding(BlurEffectRDGTextureRef, ERenderTargetLoadAction::EClear);
			GraphBuilder.AddPass(RDG_EVENT_NAME("LexUIBackgroundBlur_ClearRegionTarget"), ClearParameters, ERDGPassFlags::Raster, [](FRHICommandListImmediate&) {});
		}
#endif
		//@todo: should use screen-space region
		Renderer->CopyRenderTargetOnMeshRegion(GraphBuilder
			, BlurEffectRDGTextureRef
			, NumSamples > 1 ? ScreenResolvedRenderTarget->GetRHI() : ScreenTargetTexture.GetReference()
			, GlobalShaderMap
			, RenderScreenToMeshRegionVertexArray
			, ModelViewProjectionMatrix
			, bIsRenderTarget
			, FIntRect(0, 0, BlurEffectRHITexture->GetSizeXYZ().X, BlurEffectRHITexture->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
		);

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
				GRenderTargetPool.FindFreeElement(RHICmdList, RenderTargetDesc, DownSampleRT, *FString::Printf(TEXT("LexUI_DownsampleRT_%d"), i));
				DownSampleRenderTargetArray.Add(DownSampleRT);
				Renderer->CopyRenderTarget(GraphBuilder, GlobalShaderMap, PrevRT, DownSampleRT->GetRHI());
			
				PrevRT = DownSampleRT->GetRHI();
			}
		}
		for (int i = MaxDownSampleCount; i >= 1; i--)
		{
			if (FilteredBlurStrength >= i)
			{
				auto RenderTarget = DownSampleRenderTargetArray[i - 1];
				DoBlur(RenderTarget->GetRHI(), FilteredBlurStrength - i, MagicNumber, GraphBuilder, Renderer, GlobalShaderMap);
				auto NextRT = i == 1 ? BlurEffectRHITexture : DownSampleRenderTargetArray[i - 2]->GetRHI();
				if (FilteredBlurStrength >= i + 1)
				{
					Renderer->CopyRenderTarget(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT);
				}
				else
				{
					auto BlendValue = FMath::Clamp(FilteredBlurStrength - i, 0.0f, 1.0f);
					BlendValue = FMath::Pow(BlendValue, MagicNumber);
					Renderer->CopyRenderTarget_BlendAlpha(GraphBuilder, GlobalShaderMap, RenderTarget->GetRHI(), NextRT, BlendValue);
				}
			}
		}
		DoBlur(BlurEffectRHITexture, FilteredBlurStrength, MagicNumber, GraphBuilder, Renderer, GlobalShaderMap);

		if (RenderTargetResource == nullptr)
		{
			//after blur process, copy the blur result image back to screen image of the area
			//copy on mesh region
			RenderMeshOnScreen_RenderThread(GraphBuilder, SceneTextures, ScreenTargetTexture, GlobalShaderMap, BlurEffectRHITexture, ModelViewProjectionMatrix, ObjectToWorldMatrix, bIsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect);
		}
		else
		{
			Renderer->CopyRenderTarget_ColorCorrect(GraphBuilder, GlobalShaderMap, BlurEffectRHITexture, RenderTargetResource->GetRenderTargetTexture());
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
	void DoBlur(FRHITexture* RenderTargetTexture
		, float BlurAmount
		, float MagicNumber
		, FRDGBuilder& GraphBuilder
		, FLexUIRenderer* Renderer
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

		BlurAmount = FMath::Clamp(BlurAmount, 0.0f, 1.0f);
		BlurAmount = FMath::Pow(BlurAmount, MagicNumber);
				
		// Use the pooled overload (member) so RDG keeps the temp render target alive until the graph executes, no manual SafeRelease is needed afterwards.
		auto RenderTargetTexture_Blur_RDG = GraphBuilder.RegisterExternalTexture(DownSampleRT_Blur, TEXT("LexUIBlurTempTexture"), ERDGTextureFlags::None);
		
		auto* HorizontalPassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		HorizontalPassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(InputTexture));
		HorizontalPassParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTexture_Blur_RDG, ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_Pass_Horizontal"),
			HorizontalPassParameters,
			ERDGPassFlags::Raster,
			[this, VertexShader, PixelShader, Renderer, MainTexture = RenderTargetTexture, SamplerState, BlurAmount](FRHICommandListImmediate& RHICmdList)
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
				PixelShader->SetBlurStrength(RHICmdList, FVector2f(1.0f / MainTexture->GetSizeX() * BlurAmount, 0));
				Renderer->DrawFullScreenQuad(RHICmdList);
			});

		auto* VerticalPassParameters = GraphBuilder.AllocParameters<FLexUIPostProcessCopyParameters>();
		VerticalPassParameters->InputTexture = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(RenderTargetTexture_Blur_RDG));
		VerticalPassParameters->RenderTargets[0] = FRenderTargetBinding(InputTexture, ERenderTargetLoadAction::ELoad);
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackgroundBlur_Pass_Vertical"),
			VerticalPassParameters,
			ERDGPassFlags::Raster,
			[this, VertexShader, PixelShader, Renderer, MainTexture = RenderTargetTexture_Blur, SamplerState, BlurAmount](FRHICommandListImmediate& RHICmdList)
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
				PixelShader->SetBlurStrength(RHICmdList, FVector2f(0, 1.0f / MainTexture->GetSizeY() * BlurAmount));
				Renderer->DrawFullScreenQuad(RHICmdList);
			});
	}
};


void ULexBackgroundBlur::SendOthersDataToRenderProxy()
{
	if (RenderProxy != nullptr)
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)RenderProxy;
		struct FUIBackgroundBlurUpdateOthersData
		{
			float BlurStrengthWithAlpha;
			float MaxDownSampleLevel;
		};
		auto updateData = new FUIBackgroundBlurUpdateOthersData();
		updateData->BlurStrengthWithAlpha = this->GetBlurStrengthInternal();
		updateData->MaxDownSampleLevel = this->MaxDownSampleLevel;
		ENQUEUE_RENDER_COMMAND(FLexBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
			{
				BackgroundBlurRenderProxy->MaxDownSampleLevel = updateData->MaxDownSampleLevel;
				BackgroundBlurRenderProxy->BlurStrength = updateData->BlurStrengthWithAlpha;
				delete updateData;
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

void ULexBackgroundBlur::SetApplyAlphaToBlur(bool Value)
{
	if (ApplyAlphaToBlur != Value)
	{
		ApplyAlphaToBlur = Value;
		SendOthersDataToRenderProxy();
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
	if (ApplyAlphaToBlur)
	{
		return GetFinalAlpha01() * BlurStrength;
	}
	return BlurStrength;
}

FLexVisualPostProcessRenderProxy* ULexBackgroundBlur::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackgroundBlurRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
		SendRenderTargetToRenderProxy();
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
