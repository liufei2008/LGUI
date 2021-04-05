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

	auto objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
	auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
	SendRegionVertexDataToRenderProxy(modelViewPrjectionMatrix);
	SendMaskTextureToRenderProxy();
	SendOthersDataToRenderProxy();
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
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FTextureRHIRef ScreenImage,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix
	)override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundPixelate);
		if (pixelateStrength <= 0.0f)return;
		float calculatedStrength = FMath::Pow(pixelateStrength * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
		calculatedStrength = FMath::Clamp(calculatedStrength, 0.0f, 100.0f);
		calculatedStrength += 1;

		auto width = (int)(widget.width / calculatedStrength);
		auto height = (int)(widget.height / calculatedStrength);
		width = FMath::Clamp(width, 1, (int)widget.width);
		height = FMath::Clamp(height, 1, (int)widget.height);

		//get render target
		TRefCountPtr<IPooledRenderTarget> PixelateEffectRenderTarget;
		{
			auto MultiSampleCount = (uint16)ULGUISettings::GetAntiAliasingSampleCount();
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = MultiSampleCount;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, PixelateEffectRenderTarget, TEXT("LGUIPixelateEffectRenderTarget"));
			if (!PixelateEffectRenderTarget.IsValid())
				return;
		}
		auto PixelateEffectRenderTargetTexture = PixelateEffectRenderTarget->GetRenderTargetItem().TargetableTexture;

		//copy rect area from screen image to a render target, so we can just process this area
		FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, PixelateEffectRenderTargetTexture, renderScreenToMeshRegionVertexArray);
		//after pixelate process, copy the area back to screen image
		RenderMeshOnScreen_RenderThread(RHICmdList, ScreenImage, GlobalShaderMap, PixelateEffectRenderTargetTexture, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());

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

TWeakPtr<FUIPostProcessRenderProxy> UUIBackgroundPixelate::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUIBackgroundPixelateRenderProxy>(new FUIBackgroundPixelateRenderProxy());
		auto objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
		auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
		SendRegionVertexDataToRenderProxy(modelViewPrjectionMatrix);
		SendOthersDataToRenderProxy();
		SendMaskTextureToRenderProxy();
	}
	return RenderProxy;
}
