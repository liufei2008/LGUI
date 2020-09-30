// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
#include "Rendering/Texture2DResource.h"

UUIBackgroundBlur::UUIBackgroundBlur(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBackgroundBlur::BeginPlay()
{
	Super::BeginPlay();

	inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
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
	}
}

void UUIBackgroundBlur::SetApplyAlphaToBlur(bool newValue)
{
	if (applyAlphaToBlur != newValue)
	{
		applyAlphaToBlur = newValue;
	}
}

void UUIBackgroundBlur::SetMaxDownSampleLevel(int newValue)
{
	if (maxDownSampleLevel != newValue)
	{
		maxDownSampleLevel = newValue;
		inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
	}
}

void UUIBackgroundBlur::SetStrengthTexture(UTexture2D* newValue)
{
	if (strengthTexture != newValue)
	{
		strengthTexture = newValue;
	}
}

void UUIBackgroundBlur::SetMaskTexture(UTexture2D* newValue)
{
	if (maskTexture != newValue)
	{
		maskTexture = newValue;
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

void UUIBackgroundBlur::OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	auto blurStrengthWithAlpha = GetBlurStrengthInternal();
	if (blurStrengthWithAlpha <= 0.0f)return;
	auto& vertices = geometry->vertices;
	if (vertices.Num() <= 0)return;
	if (!IsValid(blurEffectRenderTarget1))
	{
		float width = widget.width;
		float height = widget.height;
		if (width >= 1.0f && height >= 1.0f)
		{
			blurEffectRenderTarget1 = NewObject<UTextureRenderTarget2D>(this);
			blurEffectRenderTarget1->InitAutoFormat((int)width, (int)height);

			blurEffectRenderTarget2 = NewObject<UTextureRenderTarget2D>(this);
			blurEffectRenderTarget2->InitAutoFormat((int)width, (int)height);

			inv_TextureSize.X = 1.0f / width;
			inv_TextureSize.Y = 1.0f / height;
		}
	}
	else
	{
		if (blurEffectRenderTarget1->SizeX != widget.width || blurEffectRenderTarget1->SizeY != widget.height)
		{
			float width = widget.width;
			float height = widget.height;
			width = FMath::Max(width, 1.0f);
			height = FMath::Max(height, 1.0f);

			blurEffectRenderTarget1->ResizeTarget((int)width, (int)height);
			blurEffectRenderTarget2->ResizeTarget((int)width, (int)height);

			inv_TextureSize.X = 1.0f / width;
			inv_TextureSize.Y = 1.0f / height;
		}
	}


#if WITH_EDITOR
	inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;//only execute in edit mode, because it's already calculated in BeginPlay.
#endif

	if (copyRegionVertexArray.Num() == 0)
	{
		//full screen vertex position
		copyRegionVertexArray =
		{
			FLGUIPostProcessVertex(FVector(-1, -1, 0), FVector2D(0.0f, 0.0f)),
			FLGUIPostProcessVertex(FVector(1, -1, 0), FVector2D(1.0f, 0.0f)),
			FLGUIPostProcessVertex(FVector(-1, 1, 0), FVector2D(0.0f, 1.0f)),
			FLGUIPostProcessVertex(FVector(1, 1, 0), FVector2D(1.0f, 1.0f))
		};
	}
	objectToWorldMatrix = this->GetRenderCanvas()->CheckAndGetUIItem()->GetComponentTransform().ToMatrixWithScale();
	auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
	{
		FScopeLock scopeLock(&mutex);

		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = copyRegionVertexArray[i];
			//convert vertex postition to screen, and use as texture coordinate
			auto clipSpacePos = modelViewPrjectionMatrix.TransformPosition(vertices[i].Position);
			float inv_W = 1.0f / clipSpacePos.W;
			copyVert.TextureCoordinate0 = FVector2D(clipSpacePos.X * inv_W, clipSpacePos.Y * inv_W) * 0.5f + FVector2D(0.5f, 0.5f);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundBlur"), STAT_BackgroundBlur, STATGROUP_LGUI);

void UUIBackgroundBlur::OnRenderPostProcess_RenderThread(
	FRHICommandListImmediate& RHICmdList, 
	FTexture2DRHIRef ScreenImage, 
	FGlobalShaderMap* GlobalShaderMap,
	const FMatrix& ViewProjectionMatrix,  
	const TFunction<void()>& DrawPrimitive
)
{
	SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
	auto blurStrengthWithAlpha = GetBlurStrengthInternal();
	if (blurStrengthWithAlpha <= 0.0f)return;
	if (!IsValid(blurEffectRenderTarget1))return;
	if (!IsValid(blurEffectRenderTarget2))return;

	auto Resource1 = blurEffectRenderTarget1->GetRenderTargetResource();
	auto Resource2 = blurEffectRenderTarget2->GetRenderTargetResource();
	if (Resource1 == nullptr || Resource2 == nullptr)return;
	FTexture2DRHIRef BlurEffectRenderTexture1 = Resource1->GetRenderTargetTexture();
	FTexture2DRHIRef BlurEffectRenderTexture2 = Resource2->GetRenderTargetTexture();
	//copy rect area from screen image to a render target, so we can just process this area
	TArray<FLGUIPostProcessVertex> tempCopyRegion;
	{
		FScopeLock scopeLock(&mutex);
		tempCopyRegion = copyRegionVertexArray;
	}
	FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, BlurEffectRenderTexture1, true, tempCopyRegion);//mesh's uv.y is flipped
	//do the blur process on the area
	{
		if (IsValid(strengthTexture))//use mask texture to control blur strength
		{
			TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			VertexShader->SetParameters(RHICmdList, false);
			TShaderMapRef<FLGUIPostProcessGaussianBlurWithStrengthTexturePS> PixelShader(GlobalShaderMap);
			PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);
			PixelShader->SetStrengthTexture(RHICmdList, ((FTexture2DResource*)strengthTexture->Resource)->GetTexture2DRHI(), strengthTexture->Resource->SamplerStateRHI);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
			
			auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			blurStrengthWithAlpha = FMath::Pow(blurStrengthWithAlpha * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more linear
			float calculatedBlurStrength = blurStrengthWithAlpha * inv_SampleLevelInterval;
			float calculatedBlurStrength2 = 1.0f;
			int sampleCount = (int)calculatedBlurStrength + 1;
			for (int i = 0; i < sampleCount; i++)
			{
				if (i + 1 == sampleCount)
				{
					float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
					fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more linear
					PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2 * fracValue);
				}
				else
				{
					PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2);
				}
				//render vertical
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture2, ERenderTargetActions::Load_DontStore), TEXT("Vertical"));
				PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
				PixelShader->SetHorizontalOrVertical(RHICmdList, true);
				FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
				//render horizontal
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_DontStore), TEXT("Horizontal"));
				PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
				PixelShader->SetHorizontalOrVertical(RHICmdList, false);
				FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
				calculatedBlurStrength2 *= 2;
			}
		}
		else
		{
			TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
			VertexShader->SetParameters(RHICmdList, false);
			TShaderMapRef<FLGUIPostProcessGaussianBlurPS> PixelShader(GlobalShaderMap);
			PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);

			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
			blurStrengthWithAlpha = FMath::Pow(blurStrengthWithAlpha * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more linear
			float calculatedBlurStrength = blurStrengthWithAlpha * inv_SampleLevelInterval;
			float calculatedBlurStrength2 = 1.0f;
			int sampleCount = (int)calculatedBlurStrength + 1;
			for (int i = 0; i < sampleCount; i++)
			{
				if (i + 1 == sampleCount)
				{
					float fracValue = (calculatedBlurStrength - (int)calculatedBlurStrength);
					fracValue = FMath::FastAsin(fracValue * 2.0f - 1.0f) * INV_PI + 0.5f;//another thing to make the blur transition feel more linear
					PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2 * fracValue);
				}
				else
				{
					PixelShader->SetBlurStrength(RHICmdList, calculatedBlurStrength2);
				}
				//render vertical
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture2, ERenderTargetActions::Load_DontStore), TEXT("Vertical"));
				PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
				PixelShader->SetHorizontalOrVertical(RHICmdList, true);
				FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
				//render horizontal
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_DontStore), TEXT("Horizontal"));
				PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture2, samplerState);
				PixelShader->SetHorizontalOrVertical(RHICmdList, false);
				FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
				RHICmdList.EndRenderPass();
				calculatedBlurStrength2 *= 2;
			}
		}
	}
	//after blur process, copy the area back to screen image
	{
		if (IsValid(maskTexture))
		{
			FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, BlurEffectRenderTexture2, true, tempCopyRegion);//mesh's uv.y is flipped

			RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenImage, ERenderTargetActions::Load_DontStore), TEXT("CopyAreaToScreen"));
			TShaderMapRef<FLGUIMeshPostProcessVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIMeshCopyTargetWithMaskPS> PixelShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
			VertexShader->SetParameters(RHICmdList, objectToWorldMatrix, ViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1, BlurEffectRenderTexture2, ((FTexture2DResource*)maskTexture->Resource)->GetTexture2DRHI()
				, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
				, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
				, maskTexture->Resource->SamplerStateRHI
				);
		}
		else
		{
			RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenImage, ERenderTargetActions::Load_DontStore), TEXT("CopyAreaToScreen"));
			TShaderMapRef<FLGUIMeshPostProcessVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FLGUIMeshCopyTargetPS> PixelShader(GlobalShaderMap);
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIVertexDeclaration();
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
			VertexShader->SetParameters(RHICmdList, objectToWorldMatrix, ViewProjectionMatrix);
			PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1);
		}
		DrawPrimitive();
		RHICmdList.EndRenderPass();
		RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
	}
}