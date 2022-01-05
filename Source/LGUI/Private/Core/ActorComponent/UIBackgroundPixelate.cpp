// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBackgroundPixelate.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/LGUISpriteData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "PipelineStateCache.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISettings.h"
#include "RenderTargetPool.h"
#include "Core/UIPostProcessRenderProxy.h"

UUIBackgroundPixelate::UUIBackgroundPixelate(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBackgroundPixelate::BeginPlay()
{
	Super::BeginPlay();
}

void UUIBackgroundPixelate::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIBackgroundPixelate::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		
	}
}
#endif
void UUIBackgroundPixelate::MarkAllDirtyRecursive()
{
	Super::MarkAllDirtyRecursive();

	if (this->RenderCanvas.IsValid())
	{
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
	}
}



void UUIBackgroundPixelate::SetPixelateStrength(float newValue)
{
	if (pixelateStrength != newValue)
	{
		pixelateStrength = newValue;
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundPixelate::SetApplyAlphaToStrength(bool newValue)
{
	if (applyAlphaToStrength != newValue)
	{
		applyAlphaToStrength = newValue;
		SendOthersDataToRenderProxy();
	}
}

float UUIBackgroundPixelate::GetStrengthInternal()
{
	if (applyAlphaToStrength)
	{
		return GetFinalAlpha01() * pixelateStrength;
	}
	return pixelateStrength;
}


#define MAX_PixelateStrength 100.0f
#define INV_MAX_PixelateStrength 0.01f

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundPixelate"), STAT_BackgroundPixelate, STATGROUP_LGUI);
class FUIBackgroundPixelateRenderProxy :public FUIPostProcessRenderProxy
{
public:
	float pixelateStrength = 0.0f;
public:
	FUIBackgroundPixelateRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return FUIPostProcessRenderProxy::CanRender() && pixelateStrength > 0.0f;
	}
	virtual void OnRenderPostProcess_RenderThread(
#if ENGINE_MAJOR_VERSION >= 5
		FRDGBuilder& GraphBuilder,
#else
		FRHICommandListImmediate& RHICmdList,
#endif
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenTargetTexture,
		FTextureRHIRef ScreenTargetImage,
		FTextureRHIRef ScreenTargetResolveImage,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		const FIntRect& ViewRect,
		const FVector4& DepthTextureScaleOffset,
		const FVector4& ViewTextureScaleOffset
	)override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundPixelate);
		if (pixelateStrength <= 0.0f)return;
#if ENGINE_MAJOR_VERSION >= 5
		FRHICommandListImmediate& RHICmdList = GraphBuilder.RHICmdList;
#endif
		float calculatedStrength = FMath::Pow(pixelateStrength * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
		calculatedStrength = FMath::Clamp(calculatedStrength, 0.0f, 100.0f);
		calculatedStrength += 1;

		auto width = (int)(RectSize.X / calculatedStrength);
		auto height = (int)(RectSize.Y / calculatedStrength);
		width = FMath::Clamp(width, 1, (int)RectSize.X);
		height = FMath::Clamp(height, 1, (int)RectSize.Y);

		//get render target
		TRefCountPtr<IPooledRenderTarget> PixelateEffectRenderTarget;
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenTargetImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = 1;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, PixelateEffectRenderTarget, TEXT("LGUIPixelateEffectRenderTarget"));
			if (!PixelateEffectRenderTarget.IsValid())
				return;
		}
		auto PixelateEffectRenderTargetTexture = PixelateEffectRenderTarget->GetRenderTargetItem().TargetableTexture;

		//copy rect area from screen image to a render target, so we can just process this area
		auto modelViewProjectionMatrix = objectToWorldMatrix * ViewProjectionMatrix;
		if (ScreenTargetImage->IsMultisampled())
		{
			RHICmdList.CopyToResolveTarget(ScreenTargetImage, ScreenTargetResolveImage, FResolveParams());
			Renderer->CopyRenderTargetOnMeshRegion(RHICmdList
				, GlobalShaderMap
				, ScreenTargetResolveImage
				, PixelateEffectRenderTargetTexture
				, renderScreenToMeshRegionVertexArray
				, modelViewProjectionMatrix
				, FIntRect(0, 0, PixelateEffectRenderTargetTexture->GetSizeXYZ().X, PixelateEffectRenderTargetTexture->GetSizeXYZ().Y)
				, ViewTextureScaleOffset
			);
		}
		else
		{
			Renderer->CopyRenderTargetOnMeshRegion(RHICmdList
				, GlobalShaderMap
				, ScreenTargetImage
				, PixelateEffectRenderTargetTexture
				, renderScreenToMeshRegionVertexArray
				, modelViewProjectionMatrix
				, FIntRect(0, 0, PixelateEffectRenderTargetTexture->GetSizeXYZ().X, PixelateEffectRenderTargetTexture->GetSizeXYZ().Y)
				, ViewTextureScaleOffset
			);
		}
		//after pixelate process, copy the area back to screen image
		RenderMeshOnScreen_RenderThread(
#if ENGINE_MAJOR_VERSION >= 5
			GraphBuilder
#else
			RHICmdList
#endif
			, ScreenTargetImage, GlobalShaderMap, PixelateEffectRenderTargetTexture, modelViewProjectionMatrix, IsWorldSpace, BlendDepthForWorld, DepthTextureScaleOffset, ViewRect, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());

		//release render target
		PixelateEffectRenderTarget.SafeRelease();
	}
};

void UUIBackgroundPixelate::SendOthersDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = (FUIBackgroundPixelateRenderProxy*)(RenderProxy.Get());
		float pixelateStrengthWidthAlpha = this->GetStrengthInternal();
		ENQUEUE_RENDER_COMMAND(FUIBackgroundPixelate_UpdateData)
			([TempRenderProxy, pixelateStrengthWidthAlpha](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->pixelateStrength = pixelateStrengthWidthAlpha;
				});
	}
}

TSharedPtr<FUIPostProcessRenderProxy> UUIBackgroundPixelate::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUIBackgroundPixelateRenderProxy>(new FUIBackgroundPixelateRenderProxy());
		if (this->RenderCanvas.IsValid())
		{
			SendRegionVertexDataToRenderProxy();
			SendMaskTextureToRenderProxy();
		}
	}
	return RenderProxy;
}

void UUIBackgroundPixelate::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	SendOthersDataToRenderProxy();
}
