// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBackgroundBlur.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/LGUISpriteData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Core/LGUIRender/LGUIPostProcessShaders.h"
#include "Core/LGUIRender/LGUIVertex.h"
#include "PipelineStateCache.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISettings.h"
#include "RenderTargetPool.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "Rendering/Texture2DResource.h"
#include "RHIStaticStates.h"

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
		return blurStrength > 0.0f;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FLGUIRenderer* Renderer,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix44f& ViewProjectionMatrix,
		bool IsWorldSpace,
		float BlendDepthForWorld,
		int DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	) override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
		if (blurStrength <= 0.0f)return;

		auto& RHICmdList = GraphBuilder.RHICmdList;

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedTexture;
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget1;
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget2;
		auto ReleaseRenderTarget = [&] {
			if (ScreenResolvedTexture.IsValid())
			{
				ScreenResolvedTexture.SafeRelease();
			}
			if (BlurEffectRenderTarget1.IsValid())
			{
				BlurEffectRenderTarget1.SafeRelease();
			}
			if (BlurEffectRenderTarget2.IsValid())
			{
				BlurEffectRenderTarget2.SafeRelease();
			}
		};

		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		if (NumSamples > 1)
		{
			auto Size = ScreenTargetTexture->GetSizeXY();
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(Size, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenResolvedTexture, TEXT("LGUIBlurEffectResolveTarget"));
			if (!ScreenResolvedTexture.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LGUIBlurEffectResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedTexture->GetRHI(), TEXT("LGUIBlurEffectResolveTarget"));
			Renderer->AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, Size.X, Size.Y), NumSamples, GlobalShaderMap);
		}

		float width = RectSize.X;
		float height = RectSize.Y;
		width = FMath::Max(width, 1.0f);
		height = FMath::Max(height, 1.0f);
		FVector2f inv_TextureSize(1.0f / width, 1.0f / height);
		FIntPoint TextureSize(width, height);
		//get render target
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(TextureSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget1, TEXT("LGUIBlurEffectRenderTarget1"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget2, TEXT("LGUIBlurEffectRenderTarget2"));
			if (!BlurEffectRenderTarget1.IsValid() || !BlurEffectRenderTarget2.IsValid())
			{
				ReleaseRenderTarget();
				return;
			}
		}
		auto BlurEffectRenderTexture1 = BlurEffectRenderTarget1->GetRHI();
		auto BlurEffectRenderTexture2 = BlurEffectRenderTarget2->GetRHI();

		auto modelViewProjectionMatrix = objectToWorldMatrix * ViewProjectionMatrix;
		Renderer->CopyRenderTargetOnMeshRegion(GraphBuilder
			, RegisterExternalTexture(GraphBuilder, BlurEffectRenderTexture1, TEXT("LGUI_BlurEffectRenderTexture1"))
			, NumSamples > 1 ? ScreenResolvedTexture->GetRHI() : ScreenTargetTexture.GetReference()
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

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more smooth
				calculatedBlurStrength = calculatedBlurStrength * inv_SampleLevelInterval;
				float calculatedBlurStrength2 = 1.0f;
				int sampleCount = (int)calculatedBlurStrength + 1;
				for (int i = 0; i < sampleCount; i++)
				{
					float tempBlurStrength = 0.0f;
					if (i + 1 == sampleCount)
					{
						float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
						fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more smooth
						tempBlurStrength = calculatedBlurStrength2 * fracValue;
					}
					else
					{
						tempBlurStrength = calculatedBlurStrength2;
					}

					auto* VerticalPassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
					VerticalPassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, BlurEffectRenderTexture2, TEXT("Vertical_BlurEffectRenderTexture2")), ERenderTargetLoadAction::ELoad);
					GraphBuilder.AddPass(
						RDG_EVENT_NAME("Vertical"),
						VerticalPassParameters,
						ERDGPassFlags::Raster,
						[this, VertexShader, PixelShader, Renderer, BlurEffectRenderTexture1, TextureSize, samplerState, inv_TextureSize, tempBlurStrength](FRHICommandListImmediate& RHICmdList)
						{
							FGraphicsPipelineStateInitializer GraphicsPSOInit;
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
							GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
							GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
							GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
							VertexShader->SetParameters(RHICmdList);
							PixelShader->SetStrengthTexture(RHICmdList, strengthTexture->TextureRHI, strengthTexture->SamplerStateRHI);
							//render vertical
							RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
							PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
							PixelShader->SetBlurStrength(RHICmdList, FVector2f(0, tempBlurStrength * inv_TextureSize.Y));
							Renderer->DrawFullScreenQuad(RHICmdList);
						});

					auto* HorizontalPassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
					HorizontalPassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, BlurEffectRenderTexture1, TEXT("Vertical_BlurEffectRenderTexture1")), ERenderTargetLoadAction::ELoad);
					GraphBuilder.AddPass(
						RDG_EVENT_NAME("Horizontal"),
						HorizontalPassParameters,
						ERDGPassFlags::Raster,
						[this, VertexShader, PixelShader, Renderer, BlurEffectRenderTexture2, TextureSize, samplerState, inv_TextureSize, tempBlurStrength](FRHICommandListImmediate& RHICmdList)
						{
							FGraphicsPipelineStateInitializer GraphicsPSOInit;
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
							GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
							GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
							GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
							VertexShader->SetParameters(RHICmdList);
							PixelShader->SetStrengthTexture(RHICmdList, strengthTexture->TextureRHI, strengthTexture->SamplerStateRHI);
							//render horizontal
							RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
							PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
							PixelShader->SetBlurStrength(RHICmdList, FVector2f(tempBlurStrength * inv_TextureSize.X, 0));
							Renderer->DrawFullScreenQuad(RHICmdList);
						});

					calculatedBlurStrength2 *= 2;
				}
			}
			else
			{
				TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUIPostProcessGaussianBlurPS> PixelShader(GlobalShaderMap);

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more smooth
				calculatedBlurStrength = calculatedBlurStrength * inv_SampleLevelInterval;
				float calculatedBlurStrength2 = 1.0f;
				int sampleCount = (int)calculatedBlurStrength + 1;
				for (int i = 0; i < sampleCount; i++)
				{
					float tempBlurStrength = 0.0f;
					if (i + 1 == sampleCount)
					{
						float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
						fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more smooth
						tempBlurStrength = calculatedBlurStrength2 * fracValue;
					}
					else
					{
						tempBlurStrength = calculatedBlurStrength2;
					}

					auto* VerticalPassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
					VerticalPassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, BlurEffectRenderTexture2, TEXT("Vertical_BlurEffectRenderTexture2")), ERenderTargetLoadAction::ELoad);
					GraphBuilder.AddPass(
						RDG_EVENT_NAME("Vertical"),
						VerticalPassParameters,
						ERDGPassFlags::Raster,
						[this, VertexShader, PixelShader, Renderer, BlurEffectRenderTexture1, TextureSize, samplerState, inv_TextureSize, tempBlurStrength](FRHICommandListImmediate& RHICmdList)
						{
							FGraphicsPipelineStateInitializer GraphicsPSOInit;
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
							GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
							GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
							GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
							VertexShader->SetParameters(RHICmdList);
							//render vertical
							RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
							PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
							PixelShader->SetBlurStrength(RHICmdList, FVector2f(0, tempBlurStrength * inv_TextureSize.Y));
							Renderer->DrawFullScreenQuad(RHICmdList);
						});

					auto* HorizontalPassParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
					HorizontalPassParameters->RenderTargets[0] = FRenderTargetBinding(RegisterExternalTexture(GraphBuilder, BlurEffectRenderTexture1, TEXT("Vertical_BlurEffectRenderTexture1")), ERenderTargetLoadAction::ELoad);
					GraphBuilder.AddPass(
						RDG_EVENT_NAME("Horizontal"),
						HorizontalPassParameters,
						ERDGPassFlags::Raster,
						[this, VertexShader, PixelShader, Renderer, BlurEffectRenderTexture2, TextureSize, samplerState, inv_TextureSize, tempBlurStrength](FRHICommandListImmediate& RHICmdList)
						{
							FGraphicsPipelineStateInitializer GraphicsPSOInit;
							RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
							GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
							GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
							GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0, EApplyRendertargetOption::CheckApply);
							VertexShader->SetParameters(RHICmdList);
							//render horizontal
							RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
							PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
							PixelShader->SetBlurStrength(RHICmdList, FVector2f(tempBlurStrength * inv_TextureSize.X, 0));
							Renderer->DrawFullScreenQuad(RHICmdList);
						});

					calculatedBlurStrength2 *= 2;
				}
			}
		}

		//after blur process, copy the area back to screen image
		RenderMeshOnScreen_RenderThread(GraphBuilder, SceneTextures, ScreenTargetTexture, GlobalShaderMap, BlurEffectRenderTexture1, modelViewProjectionMatrix, IsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect);

		//release render target
		ReleaseRenderTarget();
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
		if (IsValid(this->strengthTexture) && this->strengthTexture->GetResource() != nullptr)
		{
			strengthTextureResource = (FTexture2DResource*)this->strengthTexture->GetResource();
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
		MarkCanvasUpdate(false, false, false, false);
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetApplyAlphaToBlur(bool newValue)
{
	if (applyAlphaToBlur != newValue)
	{
		applyAlphaToBlur = newValue;
		MarkCanvasUpdate(false, false, false, false);
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetMaxDownSampleLevel(int newValue)
{
	if (maxDownSampleLevel != newValue)
	{
		maxDownSampleLevel = newValue;
		inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
		MarkCanvasUpdate(false, false, false, false);
		SendOthersDataToRenderProxy();
	}
}

void UUIBackgroundBlur::SetStrengthTexture(UTexture2D* newValue)
{
	if (strengthTexture != newValue)
	{
		strengthTexture = newValue;
		MarkCanvasUpdate(false, false, false, false);
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
