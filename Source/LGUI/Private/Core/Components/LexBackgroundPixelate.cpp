// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackgroundPixelate.h"
#include "LGUI.h"
#include "Core/LexUIGeometry.h"
#include "PipelineStateCache.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "RHIStaticStates.h"
#include "Core/Components/LexWidget.h"

ULexBackgroundPixelate::ULexBackgroundPixelate(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	
}

void ULexBackgroundPixelate::BeginPlay()
{
	Super::BeginPlay();
}

#if WITH_EDITOR
void ULexBackgroundPixelate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		
	}
}
#endif
void ULexBackgroundPixelate::MarkAllDirty()
{
	Super::MarkAllDirty();

	SendRegionVertexDataToRenderProxy();
	SendMaskTextureToRenderProxy();
}



void ULexBackgroundPixelate::SetPixelateStrength(float Value)
{
	if (PixelateStrength != Value)
	{
		PixelateStrength = Value;
		SendOthersDataToRenderProxy();
	}
}

void ULexBackgroundPixelate::SetApplyAlphaToStrength(bool Value)
{
	if (ApplyAlphaToStrength != Value)
	{
		ApplyAlphaToStrength = Value;
		SendOthersDataToRenderProxy();
	}
}

float ULexBackgroundPixelate::GetStrengthInternal()
{
	if (ApplyAlphaToStrength)
	{
		return GetFinalAlpha01() * PixelateStrength;
	}
	return PixelateStrength;
}


#define MAX_PixelateStrength 100.0f
#define INV_MAX_PixelateStrength 0.01f

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundPixelate"), STAT_BackgroundPixelate, STATGROUP_LGUI);
class FUIBackgroundPixelateRenderProxy :public FLexVisualPostProcessRenderProxy
{
public:
	float PixelateStrength = 0.0f;
public:
	FUIBackgroundPixelateRenderProxy()
		:FLexVisualPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return PixelateStrength > 0.0f;
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
	)override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundPixelate);
		if (PixelateStrength <= 0.0f)return;

		auto& RHICmdList = GraphBuilder.RHICmdList;

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedTexture;
		TRefCountPtr<IPooledRenderTarget> PixelateEffectRenderTarget;
		auto ReleaseRenderTarget = [&] {
			if (ScreenResolvedTexture.IsValid())
			{
				ScreenResolvedTexture.SafeRelease();
			}
			if (PixelateEffectRenderTarget.IsValid())
			{
				PixelateEffectRenderTarget.SafeRelease();
			}
		};

		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		auto ScreenSize = ScreenTargetTexture->GetSizeXY();
		if (NumSamples > 1)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenResolvedTexture, TEXT("LGUIBlurEffectResolveTarget"));
			if (!ScreenResolvedTexture.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LGUIBlurEffectResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedTexture->GetRHI(), TEXT("LGUIBlurEffectResolveTarget"));
			Renderer->AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
		}

		float calculatedStrength = FMath::Pow(PixelateStrength * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
		calculatedStrength = FMath::Clamp(calculatedStrength, 0.0f, 100.0f);
		calculatedStrength += 1;

		auto width = (int)(RectSize.X / calculatedStrength);
		auto height = (int)(RectSize.Y / calculatedStrength);
		width = FMath::Clamp(width, 1, (int)RectSize.X);
		height = FMath::Clamp(height, 1, (int)RectSize.Y);
		auto TextureSize = FIntPoint(width, height);
		bool bFullScreen = TextureSize == ScreenSize;

		//get render target
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, PixelateEffectRenderTarget, TEXT("LexUIPixelateEffectRenderTarget"));
			if (!PixelateEffectRenderTarget.IsValid())
			{
				ReleaseRenderTarget();
				return;
			}
		}
		auto PixelateEffectRenderTargetTexture = PixelateEffectRenderTarget->GetRHI();

		//copy rect area from screen image to a render target, so we can just process this area
		auto ModelViewProjectionMatrix = ObjectToWorldMatrix * ViewProjectionMatrix;
		if (!bFullScreen)
		{
			Renderer->CopyRenderTargetOnMeshRegion(GraphBuilder
				, RegisterExternalTexture(GraphBuilder, PixelateEffectRenderTargetTexture, TEXT("LexUI_PixelateEffectRenderTargetTexture"))
				, NumSamples > 1 ? ScreenResolvedTexture->GetRHI() : ScreenTargetTexture.GetReference()
				, GlobalShaderMap
				, RenderScreenToMeshRegionVertexArray
				, ModelViewProjectionMatrix
				, bIsRenderTarget
				, FIntRect(0, 0, PixelateEffectRenderTargetTexture->GetSizeXYZ().X, PixelateEffectRenderTargetTexture->GetSizeXYZ().Y)
				, ViewTextureScaleOffset
			);
		}
		else
		{
			Renderer->CopyRenderTarget(GraphBuilder, GlobalShaderMap, NumSamples > 1 ? ScreenResolvedTexture->GetRHI() : ScreenTargetTexture.GetReference()
				, PixelateEffectRenderTargetTexture);
		}

		if (RenderTargetResource == nullptr)
		{
			//after pixelate process, copy the area back to screen image
			if (!bFullScreen)
			{
				RenderMeshOnScreen_RenderThread(GraphBuilder, SceneTextures, ScreenTargetTexture, GlobalShaderMap, PixelateEffectRenderTargetTexture, ModelViewProjectionMatrix, ObjectToWorldMatrix, bIsWorldSpace, BlendDepthForWorld, BlendDepthForWorld, DepthTextureScaleOffset, ViewRect
					, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
			}
			else
			{
				Renderer->CopyRenderTarget(GraphBuilder, GlobalShaderMap, PixelateEffectRenderTargetTexture, ScreenTargetTexture
					, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
			}
		}
		else
		{
			Renderer->CopyRenderTarget_ColorCorrect(GraphBuilder, GlobalShaderMap, PixelateEffectRenderTargetTexture, RenderTargetResource->GetRenderTargetTexture()
					, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
		}

		//release render target
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUI_Clean"),
			ERDGPassFlags::None,
			[ScreenResolvedTexture, PixelateEffectRenderTarget](FRHICommandListImmediate& RHICmdList)mutable 
			{
				if (ScreenResolvedTexture.IsValid())
					ScreenResolvedTexture.SafeRelease();
				if (PixelateEffectRenderTarget.IsValid())
					PixelateEffectRenderTarget.SafeRelease();
			});
	}
};

void ULexBackgroundPixelate::SendOthersDataToRenderProxy()
{
	if (RenderProxy != nullptr)
	{
		auto TempRenderProxy = (FUIBackgroundPixelateRenderProxy*)RenderProxy;
		float pixelateStrengthWidthAlpha = this->GetStrengthInternal();
		ENQUEUE_RENDER_COMMAND(FLexBackgroundPixelate_UpdateData)
			([TempRenderProxy, pixelateStrengthWidthAlpha](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->PixelateStrength = pixelateStrengthWidthAlpha;
				});
	}
}

FLexVisualPostProcessRenderProxy* ULexBackgroundPixelate::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackgroundPixelateRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
	}
	return RenderProxy;
}

void ULexBackgroundPixelate::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	SendOthersDataToRenderProxy();
}
