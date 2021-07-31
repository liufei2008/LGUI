// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UICustomDepthStencilMask.h"
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
#include "PostProcess/SceneRenderTargets.h"
#include "RenderGraphBuilder.h"

UUICustomDepthStencilMask::UUICustomDepthStencilMask(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUICustomDepthStencilMask::BeginPlay()
{
	Super::BeginPlay();
}

void UUICustomDepthStencilMask::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUICustomDepthStencilMask::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		
	}
}
#endif
void UUICustomDepthStencilMask::MarkAllDirtyRecursive()
{
	Super::MarkAllDirtyRecursive();

	if (this->RenderCanvas.IsValid())
	{
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
	}
}

#if PLATFORM_ANDROID || PLATFORM_IOS
#define PLATFORM_MOBILE 1
#endif

DECLARE_CYCLE_STAT(TEXT("PostProcess_CustomDepthStencilMask"), STAT_CustomDepthStencilMask, STATGROUP_LGUI);
class FUICustomDepthStencilMaskRenderProxy :public FUIPostProcessRenderProxy
{
public:
	float maskStrength = 0.0f;
	EUICustomDepthStencilMaskSourceType sourceType = EUICustomDepthStencilMaskSourceType::CustomDepth;
	int stencilValue = 0.0f;
	bool bFullScreen = false;
public:
	FUICustomDepthStencilMaskRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return FUIPostProcessRenderProxy::CanRender() && maskStrength > 0.0f;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenTargetTexture,
		FTextureRHIRef ScreenTargetImage,
		FTextureRHIRef ScreenTargetResolveImage,
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix
	)override
	{
		SCOPE_CYCLE_COUNTER(STAT_CustomDepthStencilMask);
		if (maskStrength <= 0.0f)return;

		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
		if (sourceType == EUICustomDepthStencilMaskSourceType::CustomDepth)
		{
#if !PLATFORM_MOBILE
			if (!SceneContext.CustomDepth.IsValid())
#else
			if (!SceneContext.MobileCustomDepth.IsValid())
#endif
			{
				return;
			}
		}
		else
		{
#if !PLATFORM_MOBILE
			if (!SceneContext.CustomStencilSRV.IsValid())
#else
			if (!SceneContext.MobileCustomStencil.IsValid())
#endif
			{
				return;
			}
		}

		if (bFullScreen)
		{
			//get render target
			TRefCountPtr<IPooledRenderTarget> ScreenTarget_ProcessRenderTarget;
			TRefCountPtr<IPooledRenderTarget> OriginScreenTarget_ProcessRenderTarget;
			TRefCountPtr<IPooledRenderTarget> DepthTexture_ProcessRenderTarget;

			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y), ScreenTargetImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = 1;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget1"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget1"));
			if (!ScreenTarget_ProcessRenderTarget.IsValid())return;
			if (!OriginScreenTarget_ProcessRenderTarget.IsValid())return;
			auto ScreenTarget_ProcessRenderTargetTexture = ScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto OriginScreenTarget_ProcessRenderTargetTexture = OriginScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
#if PLATFORM_MOBILE
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, DepthTexture_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget3"));
			if (!DepthTexture_ProcessRenderTarget.IsValid())return;
			auto DepthTexture_ProcessRenderTargetTexture = DepthTexture_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			RHICmdList.CopyToResolveTarget(SceneContext.MobileCustomDepth->GetRenderTargetItem().TargetableTexture, DepthTexture_ProcessRenderTargetTexture, FResolveParams());
#endif

			//copy render target
			if (ScreenTargetImage->IsMultisampled())
			{
				RHICmdList.CopyToResolveTarget(ScreenTargetImage, ScreenTargetResolveImage, FResolveParams());
				Renderer->CopyRenderTarget(RHICmdList, GlobalShaderMap, ScreenTargetResolveImage, ScreenTarget_ProcessRenderTargetTexture);
			}
			else
			{
				Renderer->CopyRenderTarget(RHICmdList, GlobalShaderMap, ScreenTargetImage, ScreenTarget_ProcessRenderTargetTexture);
			}
			RHICmdList.CopyToResolveTarget(OriginScreenTargetTexture, OriginScreenTarget_ProcessRenderTargetTexture, FResolveParams());
			//@todo: dont't konw why below two line is needed; if remove them, then OriginScreenTargetTexture seems black
			{
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetImage, ERenderTargetActions::Load_Store), TEXT("LGUICopyRenderTarget"));
				RHICmdList.EndRenderPass();
			}
			//do depth mask
			{
				if (sourceType == EUICustomDepthStencilMaskSourceType::CustomDepth)
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIPostProcessCustomDepthMaskPS> PixelShader(GlobalShaderMap);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTarget_ProcessRenderTargetTexture, samplerState
#if !PLATFORM_MOBILE
						, SceneContext.CustomDepth->GetRenderTargetItem().TargetableTexture, samplerState
#else
						, DepthTexture_ProcessRenderTargetTexture, samplerState
#endif
						, maskStrength
					);
				}
				else
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
#if !PLATFORM_MOBILE
					TShaderMapRef<FLGUIPostProcessCustomDepthStencilMaskPS> PixelShader(GlobalShaderMap);
#else
					TShaderMapRef<FLGUIPostProcessMobileCustomDepthStencilMaskPS> PixelShader(GlobalShaderMap);
#endif

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTarget_ProcessRenderTargetTexture, samplerState
#if !PLATFORM_MOBILE
						, SceneContext.CustomStencilSRV
						, stencilValue
						, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y
#else
						, SceneContext.MobileCustomStencil->GetRenderTargetItem().TargetableTexture, samplerState
						, stencilValue
#endif
						, maskStrength
					);
				}

				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetImage, ERenderTargetActions::Load_DontStore), TEXT("LGUIPostProcessCustomDepthMask"));
				RHICmdList.SetViewport(0, 0, 0.0f, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y, 1.0f);
				Renderer->DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
			}
			//release render target
			if (ScreenTarget_ProcessRenderTarget.IsValid())ScreenTarget_ProcessRenderTarget.SafeRelease();
			if (OriginScreenTarget_ProcessRenderTarget.IsValid())OriginScreenTarget_ProcessRenderTarget.SafeRelease();
			if (DepthTexture_ProcessRenderTarget.IsValid())DepthTexture_ProcessRenderTarget.SafeRelease();
		}
		else
		{
			float width = widget.width;
			float height = widget.height;
			width = FMath::Max(width, 1.0f);
			height = FMath::Max(height, 1.0f);

			//get render target
			TRefCountPtr<IPooledRenderTarget> ScreenTarget_ProcessRenderTarget;
			TRefCountPtr<IPooledRenderTarget> OriginScreenTarget_ProcessRenderTarget;
			TRefCountPtr<IPooledRenderTarget> DepthTexture_ProcessRenderTarget;
			TRefCountPtr<IPooledRenderTarget> Result_ProcessRenderTarget;

			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenTargetImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = 1;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget1"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget2"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, DepthTexture_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget3"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, Result_ProcessRenderTarget, TEXT("LGUICustomDepthStencilMaskRenderTarget4"));
			if (!ScreenTarget_ProcessRenderTarget.IsValid())return;
			if (!OriginScreenTarget_ProcessRenderTarget.IsValid())return;
			if (!DepthTexture_ProcessRenderTarget.IsValid())return;
			if (!Result_ProcessRenderTarget.IsValid())return;

			auto ScreenTarget_ProcessRenderTargetTexture = ScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto OriginScreenTarget_ProcessRenderTargetTexture = OriginScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto DepthTexture_ProcessRenderTargetTexture = DepthTexture_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto Result_ProcessRenderTargetTexture = Result_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;

			//copy rect area from screen image to render target, so we can just process this area
			auto modelViewProjectionMatrix = objectToWorldMatrix * ViewProjectionMatrix;
			if (ScreenTargetImage->IsMultisampled())
			{
				RHICmdList.CopyToResolveTarget(ScreenTargetImage, ScreenTargetResolveImage, FResolveParams());
				Renderer->CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenTargetResolveImage, ScreenTarget_ProcessRenderTargetTexture, renderScreenToMeshRegionVertexArray, modelViewProjectionMatrix);
			}
			else
			{
				Renderer->CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenTargetImage, ScreenTarget_ProcessRenderTargetTexture, renderScreenToMeshRegionVertexArray, modelViewProjectionMatrix);
			}
			Renderer->CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, OriginScreenTargetTexture, OriginScreenTarget_ProcessRenderTargetTexture, renderScreenToMeshRegionVertexArray, modelViewProjectionMatrix);
			Renderer->CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap
#if !PLATFORM_MOBILE
				, SceneContext.CustomDepth->GetRenderTargetItem().TargetableTexture
#else
				, SceneContext.MobileCustomDepth->GetRenderTargetItem().TargetableTexture
#endif
				, DepthTexture_ProcessRenderTargetTexture, renderScreenToMeshRegionVertexArray, modelViewProjectionMatrix);

			//do depth mask
			{
				if (sourceType == EUICustomDepthStencilMaskSourceType::CustomDepth)
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIPostProcessCustomDepthMaskPS> PixelShader(GlobalShaderMap);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTarget_ProcessRenderTargetTexture, samplerState
						, DepthTexture_ProcessRenderTargetTexture, samplerState
						, maskStrength
					);
				}
				else
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
#if !PLATFORM_MOBILE
					TShaderMapRef<FLGUIPostProcessCustomDepthStencilMaskPS> PixelShader(GlobalShaderMap);
#else
					TShaderMapRef<FLGUIPostProcessMobileCustomDepthStencilMaskPS> PixelShader(GlobalShaderMap);
#endif

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTarget_ProcessRenderTargetTexture, samplerState
#if !PLATFORM_MOBILE
						, SceneContext.CustomStencilSRV
						, stencilValue
						, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y
#else
						, SceneContext.MobileCustomStencil->GetRenderTargetItem().TargetableTexture, samplerState
						, stencilValue
#endif
						, maskStrength
					);
				}

				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Result_ProcessRenderTargetTexture, ERenderTargetActions::Load_DontStore), TEXT("LGUIPostProcessCustomDepthMask"));
				RHICmdList.SetViewport(0, 0, 0.0f, Result_ProcessRenderTargetTexture->GetSizeXYZ().X, Result_ProcessRenderTargetTexture->GetSizeXYZ().Y, 1.0f);
				Renderer->DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
			}

			//after pixelate process, copy the area back to screen image
			RenderMeshOnScreen_RenderThread(RHICmdList, ScreenTargetImage, GlobalShaderMap, Result_ProcessRenderTargetTexture, modelViewProjectionMatrix);

			//release render target
			if (ScreenTarget_ProcessRenderTarget.IsValid())
				ScreenTarget_ProcessRenderTarget.SafeRelease();
			if (OriginScreenTarget_ProcessRenderTarget.IsValid())
				OriginScreenTarget_ProcessRenderTarget.SafeRelease();
			if (DepthTexture_ProcessRenderTarget.IsValid())
				DepthTexture_ProcessRenderTarget.SafeRelease();
			if (Result_ProcessRenderTarget.IsValid())
				Result_ProcessRenderTarget.SafeRelease();
		}
	}
};

void UUICustomDepthStencilMask::SendOthersDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = (FUICustomDepthStencilMaskRenderProxy*)(RenderProxy.Get());
		float tempMaskStrength = this->GetFinalAlpha01();
		bool tempFullScreen = this->bFullScreen;
		if (this->sourceType == EUICustomDepthStencilMaskSourceType::CustomStencil)tempFullScreen = true;
		int tempStencilValue = this->stencilValue;
		EUICustomDepthStencilMaskSourceType tempSourceType = this->sourceType;
		ENQUEUE_RENDER_COMMAND(FUICustomDepthStencilMask_UpdateData)
			([TempRenderProxy, tempMaskStrength, tempFullScreen, tempSourceType, tempStencilValue](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->maskStrength = tempMaskStrength;
					TempRenderProxy->bFullScreen = tempFullScreen;
					TempRenderProxy->sourceType = tempSourceType;
					TempRenderProxy->stencilValue = tempStencilValue;
				});
	}
}

TSharedPtr<FUIPostProcessRenderProxy> UUICustomDepthStencilMask::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUICustomDepthStencilMaskRenderProxy>(new FUICustomDepthStencilMaskRenderProxy());
		if (this->RenderCanvas.IsValid())
		{
			SendRegionVertexDataToRenderProxy();
			SendMaskTextureToRenderProxy();
		}
	}
	return RenderProxy;
}

void UUICustomDepthStencilMask::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	SendOthersDataToRenderProxy();
}

void UUICustomDepthStencilMask::SetFullScreen(bool value)
{
	if (bFullScreen != value)
	{
		bFullScreen = value;
		SendOthersDataToRenderProxy();
	}
}
void UUICustomDepthStencilMask::SetSourceType(EUICustomDepthStencilMaskSourceType value)
{
	if (sourceType != value)
	{
		sourceType = value;
		SendOthersDataToRenderProxy();
	}
}
void UUICustomDepthStencilMask::SetStencilValue(int value)
{
	if (stencilValue != value)
	{
		stencilValue = value;
		SendOthersDataToRenderProxy();
	}
}
