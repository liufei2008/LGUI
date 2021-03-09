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
	UpdateRegionVertex();
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
	UpdateRegionVertex();
}
void UUIBackgroundPixelate::UpdateRegionVertex()
{
	auto& vertices = geometry->vertices;
	if (vertices.Num() <= 0)return;
	if (renderScreenToMeshRegionVertexArray.Num() == 0)
	{
		//full screen vertex position
		renderScreenToMeshRegionVertexArray =
		{
			FLGUIPostProcessVertex(FVector(-1, -1, 0), FVector2D(0.0f, 0.0f)),
			FLGUIPostProcessVertex(FVector(1, -1, 0), FVector2D(1.0f, 0.0f)),
			FLGUIPostProcessVertex(FVector(-1, 1, 0), FVector2D(0.0f, 1.0f)),
			FLGUIPostProcessVertex(FVector(1, 1, 0), FVector2D(1.0f, 1.0f))
		};
	}
	if (renderMeshRegionToScreenVertexArray.Num() == 0)
	{
		renderMeshRegionToScreenVertexArray.AddUninitialized(4);
	}
	auto objectToWorldMatrix = this->GetRenderCanvas()->CheckAndGetUIItem()->GetComponentTransform().ToMatrixWithScale();
	auto modelViewPrjectionMatrix = objectToWorldMatrix * RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
	{
		FScopeLock scopeLock(&mutex);

		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = renderScreenToMeshRegionVertexArray[i];
			//convert vertex postition to screen, and use as texture coordinate
			auto clipSpacePos = modelViewPrjectionMatrix.TransformPosition(vertices[i].Position);
			float inv_W = 1.0f / clipSpacePos.W;
			copyVert.TextureCoordinate0 = FVector2D(clipSpacePos.X * inv_W, clipSpacePos.Y * inv_W) * 0.5f + FVector2D(0.5f, 0.5f);
			copyVert.TextureCoordinate0.Y = 1.0f - copyVert.TextureCoordinate0.Y;
		}

		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = renderMeshRegionToScreenVertexArray[i];
			auto clipSpacePos = modelViewPrjectionMatrix.TransformPosition(vertices[i].Position);
			float inv_W = 1.0f / clipSpacePos.W;
			copyVert.Position = FVector(clipSpacePos.X, clipSpacePos.Y, clipSpacePos.Z) * inv_W;
			copyVert.TextureCoordinate0 = geometry->vertices[i].TextureCoordinate[0];
		}
	}
}

void UUIBackgroundPixelate::SetPixelateStrength(float newValue)
{
	if (pixelateStrength != newValue)
	{
		pixelateStrength = newValue;
	}
}

void UUIBackgroundPixelate::SetApplyAlphaToStrength(bool newValue)
{
	if (applyAlphaToStrength != newValue)
	{
		applyAlphaToStrength = newValue;
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

}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundPixelate"), STAT_BackgroundPixelate, STATGROUP_LGUI);

void UUIBackgroundPixelate::OnRenderPostProcess_RenderThread(
	FRHICommandListImmediate& RHICmdList, 
	FTextureRHIRef ScreenImage, 
	FGlobalShaderMap* GlobalShaderMap,
	const FMatrix& ViewProjectionMatrix
)
{
	SCOPE_CYCLE_COUNTER(STAT_BackgroundPixelate);
	auto pixelateStrengthWidthAlpha = GetStrengthInternal();
	if (pixelateStrengthWidthAlpha <= 0.0f)return;
	pixelateStrengthWidthAlpha = FMath::Pow(pixelateStrengthWidthAlpha * INV_MAX_PixelateStrength, 2) * MAX_PixelateStrength;//this can make the pixelate effect transition feel more linear
	pixelateStrengthWidthAlpha = FMath::Clamp(pixelateStrengthWidthAlpha, 0.0f, 100.0f);
	pixelateStrengthWidthAlpha += 1;

	auto width = (int)(widget.width / pixelateStrengthWidthAlpha);
	auto height = (int)(widget.height / pixelateStrengthWidthAlpha);
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
	{
		TArray<FLGUIPostProcessVertex> tempCopyRegion;
		{
			FScopeLock scopeLock(&mutex);
			tempCopyRegion = renderScreenToMeshRegionVertexArray;
		}
		FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, PixelateEffectRenderTargetTexture, tempCopyRegion);
	}
	//copy the area back to screen image
	{
		RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenImage, ERenderTargetActions::Load_DontStore), TEXT("CopyAreaToScreen"));
		TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FLGUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
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
		VertexShader->SetParameters(RHICmdList);
		PixelShader->SetParameters(RHICmdList, PixelateEffectRenderTargetTexture, TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
		
		TArray<FLGUIPostProcessVertex> tempRenderRegion;
		{
			FScopeLock scopeLock(&mutex);
			tempRenderRegion = renderMeshRegionToScreenVertexArray;
		}
		uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
		FRHIResourceCreateInfo CreateInfo;
		FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
		void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
		FPlatformMemory::Memcpy(VoidPtr, tempRenderRegion.GetData(), VertexBufferSize);
		RHIUnlockVertexBuffer(VertexBufferRHI);
		RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
		RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
		VertexBufferRHI.SafeRelease();

		RHICmdList.EndRenderPass();
	}

	//release render target
	PixelateEffectRenderTarget.SafeRelease();
}