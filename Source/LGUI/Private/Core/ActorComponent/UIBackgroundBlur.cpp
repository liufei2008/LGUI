﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBackgroundBlur.h"
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
#include "Rendering/Texture2DResource.h"

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
		if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UUIBackgroundBlur, maxDownSampleLevel))
		{
			maxDownSampleLevel += 1;//just make it work
			SetMaxDownSampleLevel(maxDownSampleLevel - 1);
		}
	}
}
#endif


void UUIBackgroundBlur::MarkAllDirty()
{
	Super::MarkAllDirty();

	if (this->RenderCanvas.IsValid())
	{
		SendRegionVertexDataToRenderProxy();
		SendStrengthTextureToRenderProxy();
		SendMaskTextureToRenderProxy();
		SendOthersDataToRenderProxy();
	}
}

#define MAX_BlurStrength 100.0f
#define INV_MAX_BlurStrength 0.01f

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundBlur"), STAT_BackgroundBlur, STATGROUP_LGUI);
class FUIBackgroundBlurRenderProxy : public FUIPostProcessRenderProxy
{
public:
	FTexture2DResource* strengthTexture = nullptr;
	float inv_SampleLevelInterval = 0.0f;
	float maxDownSampleLevel = 0.0f;
	float blurStrength = 0.0f;
public:
	FUIBackgroundBlurRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return FUIPostProcessRenderProxy::CanRender() && blurStrength > 0.0f;
	}
	virtual bool PostProcessRequireOriginScreenColorTexture()const override
	{
		return false;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenColorTexture,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		float DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4& DepthTextureScaleOffset,
		const FVector4& ViewTextureScaleOffset
	) override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
		if (blurStrength <= 0.0f)return;

		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		float width = RectSize.X;
		float height = RectSize.Y;
		width = FMath::Max(width, 1.0f);
		height = FMath::Max(height, 1.0f);
		FVector2D inv_TextureSize(1.0f / width, 1.0f / height);
		//get render target
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget1;
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget2;
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = NumSamples;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget1, TEXT("LGUIBlurEffectRenderTarget1"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget2, TEXT("LGUIBlurEffectRenderTarget2"));
			if (!BlurEffectRenderTarget1.IsValid())
				return;
			if (!BlurEffectRenderTarget2.IsValid())
				return;
		}
		auto BlurEffectRenderTexture1 = BlurEffectRenderTarget1->GetRenderTargetItem().TargetableTexture;
		auto BlurEffectRenderTexture2 = BlurEffectRenderTarget2->GetRenderTargetItem().TargetableTexture;

		auto modelViewProjectionMatrix = objectToWorldMatrix * ViewProjectionMatrix;
		Renderer->CopyRenderTargetOnMeshRegion(RHICmdList
			, BlurEffectRenderTexture1
			, ScreenTargetTexture
			, GlobalShaderMap
			, renderScreenToMeshRegionVertexArray
			, modelViewProjectionMatrix
			, FIntRect(0, 0, BlurEffectRenderTexture1->GetSizeXYZ().X, BlurEffectRenderTexture1->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
		);
		//do the blur process on the area
		{
			if (strengthTexture != nullptr)//use mask texture to control blur strength
			{
				TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIPostProcessGaussianBlurWithStrengthTexturePS> PixelShader(GlobalShaderMap);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				GraphicsPSOInit.NumSamples = NumSamples;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);
				PixelShader->SetStrengthTexture(RHICmdList, strengthTexture->TextureRHI, strengthTexture->SamplerStateRHI);

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more smooth
				calculatedBlurStrength = calculatedBlurStrength * inv_SampleLevelInterval;
				float calculatedBlurStrength2 = 1.0f;
				int sampleCount = (int)calculatedBlurStrength + 1;
				for (int i = 0; i < sampleCount; i++)
				{
					if (i + 1 == sampleCount)
					{
						float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
						fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more smooth
						PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2 * fracValue);
					}
					else
					{
						PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2);
					}
					//render vertical
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture2, ERenderTargetActions::Load_Store), TEXT("Vertical"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture2->GetSizeXYZ().X, BlurEffectRenderTexture2->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, true);
					Renderer->DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					//render horizontal
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_Store), TEXT("Horizontal"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture1->GetSizeXYZ().X, BlurEffectRenderTexture1->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, false);
					Renderer->DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					calculatedBlurStrength2 *= 2;
				}
			}
			else
			{
				TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIPostProcessGaussianBlurPS> PixelShader(GlobalShaderMap);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				GraphicsPSOInit.NumSamples = NumSamples;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more smooth
				calculatedBlurStrength = calculatedBlurStrength * inv_SampleLevelInterval;
				float calculatedBlurStrength2 = 1.0f;
				int sampleCount = (int)calculatedBlurStrength + 1;
				for (int i = 0; i < sampleCount; i++)
				{
					if (i + 1 == sampleCount)
					{
						float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
						fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more smooth
						PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2 * fracValue);
					}
					else
					{
						PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2);
					}
					//render vertical
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture2, ERenderTargetActions::Load_Store), TEXT("Vertical"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture2->GetSizeXYZ().X, BlurEffectRenderTexture2->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, true);
					Renderer->DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					//render horizontal
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_Store), TEXT("Horizontal"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture1->GetSizeXYZ().X, BlurEffectRenderTexture1->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, false);
					Renderer->DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					calculatedBlurStrength2 *= 2;
				}
			}
		}

		//after blur process, copy the area back to screen image
		RenderMeshOnScreen_RenderThread(RHICmdList, ScreenTargetTexture, GlobalShaderMap, BlurEffectRenderTexture1, modelViewProjectionMatrix, IsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect);

		//release render target
		BlurEffectRenderTarget1.SafeRelease();
		BlurEffectRenderTarget2.SafeRelease();
	}
};


void UUIBackgroundBlur::SendOthersDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)(RenderProxy.Get());
		struct FUIBackgroundBlurUpdateOthersData
		{
			float blurStrengthWithAlpha;
			float inv_SampleLevelInterval;
			int maxDownSampleLevel;
		};
		auto updateData = new FUIBackgroundBlurUpdateOthersData();
		updateData->blurStrengthWithAlpha = this->GetBlurStrengthInternal();
		updateData->inv_SampleLevelInterval = this->inv_SampleLevelInterval;
		updateData->maxDownSampleLevel = this->maxDownSampleLevel;
		ENQUEUE_RENDER_COMMAND(FUIBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->inv_SampleLevelInterval = updateData->inv_SampleLevelInterval;
					BackgroundBlurRenderProxy->maxDownSampleLevel = updateData->maxDownSampleLevel;
					BackgroundBlurRenderProxy->blurStrength = updateData->blurStrengthWithAlpha;
					delete updateData;
				});
	}
}
void UUIBackgroundBlur::SendStrengthTextureToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)(RenderProxy.Get());
		FTexture2DResource* strengthTextureResource = nullptr;
		if (IsValid(this->strengthTexture) && this->strengthTexture->Resource != nullptr)
		{
			strengthTextureResource = (FTexture2DResource*)this->strengthTexture->Resource;
		}
		ENQUEUE_RENDER_COMMAND(FUIBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, strengthTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->strengthTexture = strengthTextureResource;
				});
	}
}

void UUIBackgroundBlur::SetBlurStrength(float newValue)
{
	if (blurStrength != newValue)
	{
		blurStrength = newValue;
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetApplyAlphaToBlur(bool newValue)
{
	if (applyAlphaToBlur != newValue)
	{
		applyAlphaToBlur = newValue;
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetMaxDownSampleLevel(int newValue)
{
	if (maxDownSampleLevel != newValue)
	{
		maxDownSampleLevel = newValue;
		inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetStrengthTexture(UTexture2D* newValue)
{
	if (strengthTexture != newValue)
	{
		strengthTexture = newValue;
		SendStrengthTextureToRenderProxy();
	}
}

float UUIBackgroundBlur::GetBlurStrengthInternal()
{
	if (applyAlphaToBlur)
	{
		return GetFinalAlpha01() * blurStrength;
	}
	return blurStrength;
}

TSharedPtr<FUIPostProcessRenderProxy> UUIBackgroundBlur::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUIBackgroundBlurRenderProxy>(new FUIBackgroundBlurRenderProxy());
		if (this->RenderCanvas.IsValid())
		{
			inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
			SendRegionVertexDataToRenderProxy();
			SendStrengthTextureToRenderProxy();
			SendMaskTextureToRenderProxy();
			SendOthersDataToRenderProxy();
		}
	}
	return RenderProxy;
}

void UUIBackgroundBlur::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	if (RenderProxy.IsValid())
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)(RenderProxy.Get());
		auto blurStrengthWithAlpha = this->GetBlurStrengthInternal();
		ENQUEUE_RENDER_COMMAND(FUIBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, blurStrengthWithAlpha](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->blurStrength = blurStrengthWithAlpha;
				});
	}
}
