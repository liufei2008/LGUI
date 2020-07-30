// Copyright 2019-2020 LexLiu. All Rights Reserved.

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

void UUIBackgroundPixelate::OnCreateGeometry()
{
	UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, FLGUISpriteInfo(), RenderCanvas, this);
}
void UUIBackgroundPixelate::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
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

void UUIBackgroundPixelate::SetPixelateStrength(float newValue)
{
	if (pixelateStrength != newValue)
	{
		pixelateStrength = newValue;
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

void UUIBackgroundPixelate::OnBeforeRenderPostProcess_GameThread(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	auto pixelateStrengthWidthAlpha = GetStrengthInternal();
	if (pixelateStrengthWidthAlpha <= 0.0f)return;
	auto& vertices = geometry->vertices;
	if (vertices.Num() <= 0)return;
	if (!IsValid(helperRenderTarget))
	{
		pixelateStrengthWidthAlpha = FMath::Pow(pixelateStrengthWidthAlpha * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
		pixelateStrengthWidthAlpha = FMath::Clamp(pixelateStrengthWidthAlpha, 0.0f, 100.0f);
		pixelateStrengthWidthAlpha += 1;

		auto width = (int)(widget.width / pixelateStrengthWidthAlpha);
		auto height = (int)(widget.height / pixelateStrengthWidthAlpha);
		width = FMath::Clamp(width, 1, (int)widget.width);
		height = FMath::Clamp(height, 1, (int)widget.height);

		helperRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		helperRenderTarget->InitAutoFormat((int)width, (int)height);
	}
	else
	{
		pixelateStrengthWidthAlpha = FMath::Pow(pixelateStrengthWidthAlpha * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
		pixelateStrengthWidthAlpha = FMath::Clamp(pixelateStrengthWidthAlpha, 0.0f, 100.0f);
		pixelateStrengthWidthAlpha += 1;

		auto width = (int)(widget.width / pixelateStrengthWidthAlpha);
		auto height = (int)(widget.height / pixelateStrengthWidthAlpha);
		width = FMath::Clamp(width, 1, (int)widget.width);
		height = FMath::Clamp(height, 1, (int)widget.height);

		if (helperRenderTarget->SizeX != width || helperRenderTarget->SizeY != height)
		{
			helperRenderTarget->ResizeTarget((int)width, (int)height);
		}
	}


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

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundPixelate"), STAT_BackgroundPixelate, STATGROUP_LGUI);

void UUIBackgroundPixelate::OnRenderPostProcess_RenderThread(
	FRHICommandListImmediate& RHICmdList, 
	FTexture2DRHIRef ScreenImage, 
	FGlobalShaderMap* GlobalShaderMap,
	const FMatrix& ViewProjectionMatrix,  
	const TFunction<void()>& DrawPrimitive
)
{
	SCOPE_CYCLE_COUNTER(STAT_BackgroundPixelate);
	auto pixelateStrengthWidthAlpha = GetStrengthInternal();
	if (pixelateStrengthWidthAlpha <= 0.0f)return;
	if (!IsValid(helperRenderTarget))return;

	auto helperRenderTargetResource = helperRenderTarget->GetRenderTargetResource();
	if (helperRenderTargetResource == nullptr)return;
	FTexture2DRHIRef helperRenderTargetTexture = helperRenderTargetResource->GetRenderTargetTexture();
	//copy rect area from screen image to a render target, so we can just process this area
	{
		TArray<FLGUIPostProcessVertex> tempCopyRegion;
		{
			FScopeLock scopeLock(&mutex);
			tempCopyRegion = copyRegionVertexArray;
		}
		FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, helperRenderTargetTexture, true, tempCopyRegion);//mesh's uv.y is flipped
	}
	//copy the area back to screen image
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
		PixelShader->SetParameters(RHICmdList, helperRenderTargetTexture, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
		DrawPrimitive();
		RHICmdList.EndRenderPass();
		RHICmdList.ImmediateFlush(EImmediateFlushType::FlushRHIThread);
	}
}