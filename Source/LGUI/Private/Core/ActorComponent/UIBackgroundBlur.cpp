// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBackgroundBlur.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/LGUISpriteData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Core/Render/LGUIHudShaders.h"
#include "Core/Render/LGUIHudVertex.h"
#include "PipelineStateCache.h"
#include "Core/Render/LGUIRenderer.h"

UUIBackgroundBlur::UUIBackgroundBlur(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBackgroundBlur::BeginPlay()
{
	Super::BeginPlay();
}

void UUIBackgroundBlur::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIBackgroundBlur::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		if (Property->GetFName() == TEXT("blurStrength"))
		{
			MarkTriangleDirty();//mark dirty to recreate drawcall
			bNeedToResize = true;
		}
	}
}
#endif

void UUIBackgroundBlur::OnCreateGeometry()
{
	UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, FLGUISpriteInfo(), RenderCanvas, this);
}
void UUIBackgroundBlur::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexPositionChanged)
	{
		UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, RenderCanvas, this);
	}
	if (InVertexUVChanged)
	{
		UIGeometry::UpdateUIRectSimpleUV(geometry, FLGUISpriteInfo());
	}
	if (InVertexColorChanged)
	{
		UIGeometry::UpdateUIColor(geometry, GetFinalColor());
	}
}

void UUIBackgroundBlur::SetBlurStrength(float newValue)
{
	if (blurStrength != newValue)
	{
		blurStrength = newValue;
		bNeedToResize = true;
	}
}
void UUIBackgroundBlur::OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (blurStrength <= 0.0f)return;
	if (!IsValid(BlurEffectRenderTarget))
	{
		BlurEffectRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		BlurEffectRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGB10A2;
		float width = InView.UnscaledViewRect.Width();
		float height = InView.UnscaledViewRect.Height();
		if (blurStrength > 1.0f)
		{
			float downSample = FMath::Sqrt(blurStrength);
			width /= downSample;
			height /= downSample;
		}
		BlurEffectRenderTarget->InitAutoFormat((int)width, (int)height);
		BlurEffectRenderTarget->UpdateResource();
	}
	else
	{
		if (BlurEffectRenderTarget->SizeX != InView.UnscaledViewRect.Width() || BlurEffectRenderTarget->SizeY != InView.UnscaledViewRect.Height() || bNeedToResize)
		{
			float width = InView.UnscaledViewRect.Width();
			float height = InView.UnscaledViewRect.Height();
			if (blurStrength > 1.0f)
			{
				float downSample = FMath::Sqrt(blurStrength);
				width /= downSample;
				height /= downSample;
			}
			BlurEffectRenderTarget->ResizeTarget(width, height);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundBlur"), STAT_BackgroundBlur, STATGROUP_LGUI);

void UUIBackgroundBlur::OnRenderPostProcess_RenderThread(
	FRHICommandListImmediate& RHICmdList, 
	FTexture2DRHIRef ScreenImage, 
	TShaderMap<FGlobalShaderType>* GlobalShaderMap, 
	const FMatrix& ViewProjectionMatrix, 
	FGraphicsPipelineStateInitializer& GraphicsPSOInit, 
	const TFunction<void()>& DrawPrimitive
)
{
	SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
	if (blurStrength <= 0.0f)return;
	if (!IsValid(BlurEffectRenderTarget))return;

	FTexture2DRHIRef BlurEffectRenderTexture1 = BlurEffectRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();

	FLGUIViewExtension::CopyRenderTarget(RHICmdList, GlobalShaderMap, GraphicsPSOInit, ScreenImage, BlurEffectRenderTexture1, false);
	TShaderMapRef<FLGUIMeshPostProcessVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FLGUIBlurShaderPS> PixelShader(GlobalShaderMap);
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GLGUIVertexDeclaration.VertexDeclarationRHI;
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
	int iterateCount = FMath::CeilToInt(blurStrength);
	for(int i = 0; i < iterateCount; i++)
	{
		//render vertical
		SetRenderTarget(RHICmdList, BlurEffectRenderTexture1, FTextureRHIRef());
		VertexShader->SetParameters(RHICmdList, ViewProjectionMatrix);
		PixelShader->SetParameters(RHICmdList, ScreenImage, blurStrength, false);
		DrawPrimitive();
		//render horizontal
		SetRenderTarget(RHICmdList, ScreenImage, FTextureRHIRef());
		VertexShader->SetParameters(RHICmdList, ViewProjectionMatrix);
		PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1, blurStrength, true);
		DrawPrimitive();
	}
	//change render target back to origin screen image
	//SetRenderTarget(RHICmdList, ScreenImage, FTextureRHIRef());//if current render target is not ScreenImage then we need to set it back, because other draw command depend on ScreenImage
}