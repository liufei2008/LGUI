// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UICustomDepthMask.h"
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

UUICustomDepthMask::UUICustomDepthMask(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUICustomDepthMask::BeginPlay()
{
	Super::BeginPlay();
}

void UUICustomDepthMask::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUICustomDepthMask::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		
	}
}
#endif
void UUICustomDepthMask::MarkAllDirtyRecursive()
{
	Super::MarkAllDirtyRecursive();

	if (this->RenderCanvas.IsValid())
	{
		auto objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
		auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
		SendRegionVertexDataToRenderProxy(modelViewPrjectionMatrix);
		SendMaskTextureToRenderProxy();
	}
}



DECLARE_CYCLE_STAT(TEXT("PostProcess_CustomDepthmask"), STAT_CustomDepthmask, STATGROUP_LGUI);
class FUICustomDepthMaskRenderProxy :public FUIPostProcessRenderProxy
{
public:
	float maskStrength = 0.0f;
	EUICustomDepthMaskSourceType sourceType = EUICustomDepthMaskSourceType::CustomDepth;
	int stencilValue = 0.0f;
	bool bFullScreen = false;
public:
	FUICustomDepthMaskRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FLGUIHudRenderer* Renderer,
		FTextureRHIRef OriginScreenTargetTexture,
		FTextureRHIRef ScreenTargetImage,
		FTextureRHIRef ScreenTargetResolveImage,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix
	)override
	{
		SCOPE_CYCLE_COUNTER(STAT_CustomDepthmask);
		if (maskStrength <= 0.0f)return;

		FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
		if (sourceType == EUICustomDepthMaskSourceType::CustomDepth)
		{
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
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
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
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
			{
				FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y), ScreenTargetImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
				desc.NumSamples = 1;
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthMaskRenderTarget1"));
				if (!ScreenTarget_ProcessRenderTarget.IsValid())return;
			}
			auto ScreenTarget_ProcessRenderTargetTexture = ScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;

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
			//@todo: dont't konw why below two line is needed; if remove them, then OriginScreenTargetTexture seems black
			{
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetImage, ERenderTargetActions::Load_Store), TEXT("LGUICopyRenderTarget"));
				RHICmdList.EndRenderPass();
			}
			//do depth mask
			{
				if (sourceType == EUICustomDepthMaskSourceType::CustomDepth)
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIPostProcessCustomDepthMaskPS> PixelShader(GlobalShaderMap);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTargetTexture, samplerState
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
						, SceneContext.CustomDepth->GetRenderTargetItem().TargetableTexture, samplerState
#else
						, SceneContext.MobileCustomDepth->GetRenderTargetItem().TargetableTexture, samplerState
#endif
						, maskStrength
					);
				}
				else
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
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
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTargetTexture, samplerState
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
						, SceneContext.CustomStencilSRV
						, stencilValue
						, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y
#else
						, SceneContext.MobileCustomStencil->GetRenderTargetItem().TargetableTexture, samplerState
						, stencilValue
#endif
					);
				}

				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenTargetImage, ERenderTargetActions::Load_DontStore), TEXT("LGUIPostProcessCustomDepthMask"));
				RHICmdList.SetViewport(0, 0, 0.0f, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y, 1.0f);
				Renderer->DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
			}
			//release render target
			if (ScreenTarget_ProcessRenderTarget.IsValid())
				ScreenTarget_ProcessRenderTarget.SafeRelease();
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
			{
				FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenTargetImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
				desc.NumSamples = 1;
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthMaskRenderTarget1"));
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenTarget_ProcessRenderTarget, TEXT("LGUICustomDepthMaskRenderTarget2"));
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, DepthTexture_ProcessRenderTarget, TEXT("LGUICustomDepthMaskRenderTarget3"));
				GRenderTargetPool.FindFreeElement(RHICmdList, desc, Result_ProcessRenderTarget, TEXT("LGUICustomDepthMaskRenderTarget4"));
				if (!ScreenTarget_ProcessRenderTarget.IsValid())return;
				if (!OriginScreenTarget_ProcessRenderTarget.IsValid())return;
				if (!DepthTexture_ProcessRenderTarget.IsValid())return;
				if (!Result_ProcessRenderTarget.IsValid())return;
			}
			auto ScreenTarget_ProcessRenderTargetTexture = ScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto OriginScreenTarget_ProcessRenderTargetTexture = OriginScreenTarget_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto DepthTexture_ProcessRenderTargetTexture = DepthTexture_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;
			auto Result_ProcessRenderTargetTexture = Result_ProcessRenderTarget->GetRenderTargetItem().TargetableTexture;

			//copy rect area from screen image to render target, so we can just process this area
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
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
				, SceneContext.CustomDepth->GetRenderTargetItem().TargetableTexture
#else
				, SceneContext.MobileCustomDepth->GetRenderTargetItem().TargetableTexture
#endif
				, DepthTexture_ProcessRenderTargetTexture, renderScreenToMeshRegionVertexArray, modelViewProjectionMatrix);

			//do depth mask
			{
				if (sourceType == EUICustomDepthMaskSourceType::CustomDepth)
				{
					TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
					TShaderMapRef<FLGUIPostProcessCustomDepthMaskPS> PixelShader(GlobalShaderMap);

					FGraphicsPipelineStateInitializer GraphicsPSOInit;
					RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
					GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
					GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
					GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
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
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
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
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					GraphicsPSOInit.NumSamples = 1;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
					VertexShader->SetParameters(RHICmdList);
					auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
					PixelShader->SetParameters(RHICmdList
						, ScreenTarget_ProcessRenderTargetTexture, samplerState
						, OriginScreenTarget_ProcessRenderTargetTexture, samplerState
#if !(PLATFORM_ANDROID || PLATFORM_IOS)
						, SceneContext.CustomStencilSRV
						, stencilValue
						, ScreenTargetImage->GetSizeXYZ().X, ScreenTargetImage->GetSizeXYZ().Y
#else
						, SceneContext.MobileCustomStencil->GetRenderTargetItem().TargetableTexture, samplerState
						, stencilValue
#endif
					);
				}

				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Result_ProcessRenderTargetTexture, ERenderTargetActions::Load_DontStore), TEXT("LGUIPostProcessCustomDepthMask"));
				RHICmdList.SetViewport(0, 0, 0.0f, Result_ProcessRenderTargetTexture->GetSizeXYZ().X, Result_ProcessRenderTargetTexture->GetSizeXYZ().Y, 1.0f);
				Renderer->DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
			}

			//after pixelate process, copy the area back to screen image
			RenderMeshOnScreen_RenderThread(RHICmdList, ScreenTargetImage, GlobalShaderMap, Result_ProcessRenderTargetTexture);

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

void UUICustomDepthMask::SendOthersDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = (FUICustomDepthMaskRenderProxy*)(RenderProxy.Get());
		float tempMaskStrength = this->GetFinalAlpha01();
		bool tempFullScreen = this->bFullScreen;
		if (this->sourceType == EUICustomDepthMaskSourceType::CustomStencil)tempFullScreen = true;
		int tempStencilValue = this->stencilValue;
		EUICustomDepthMaskSourceType tempSourceType = this->sourceType;
		ENQUEUE_RENDER_COMMAND(FUICustomDepthMask_UpdateData)
			([TempRenderProxy, tempMaskStrength, tempFullScreen, tempSourceType, tempStencilValue](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->maskStrength = tempMaskStrength;
					TempRenderProxy->bFullScreen = tempFullScreen;
					TempRenderProxy->sourceType = tempSourceType;
					TempRenderProxy->stencilValue = tempStencilValue;
				});
	}
}

TWeakPtr<FUIPostProcessRenderProxy> UUICustomDepthMask::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUICustomDepthMaskRenderProxy>(new FUICustomDepthMaskRenderProxy());
		if (this->RenderCanvas.IsValid())
		{
			auto objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
			auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
			SendRegionVertexDataToRenderProxy(modelViewPrjectionMatrix);
			SendMaskTextureToRenderProxy();
		}
	}
	return RenderProxy;
}

void UUICustomDepthMask::SendRegionVertexDataToRenderProxy(const FMatrix& InModelViewProjectionMatrix)
{
	Super::SendRegionVertexDataToRenderProxy(InModelViewProjectionMatrix);
	SendOthersDataToRenderProxy();
}

void UUICustomDepthMask::SetFullScreen(bool value)
{
	if (bFullScreen != value)
	{
		bFullScreen = value;
		SendOthersDataToRenderProxy();
	}
}
void UUICustomDepthMask::SetSourceType(EUICustomDepthMaskSourceType value)
{
	if (sourceType != value)
	{
		sourceType = value;
		SendOthersDataToRenderProxy();
	}
}
void UUICustomDepthMask::SetStencilValue(int value)
{
	if (stencilValue != value)
	{
		stencilValue = value;
		SendOthersDataToRenderProxy();
	}
}
