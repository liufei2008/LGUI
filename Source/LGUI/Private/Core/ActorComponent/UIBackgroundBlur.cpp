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
#include "Core/ActorComponent/LGUICanvas.h"

UUIBackgroundBlur::UUIBackgroundBlur(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBackgroundBlur::BeginPlay()
{
	Super::BeginPlay();

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
		if (Property->GetFName() == TEXT("blurStrength") || Property->GetFName() == TEXT("applyAlphaToBlur"))
		{
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
		bNeedToResize = true;
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
	auto calculatedBlurStrength = GetBlurStrengthInternal();
	if (calculatedBlurStrength <= 0.0f)return;
	if (!IsValid(blurEffectRenderTarget1))
	{
		float width = widget.width;
		float height = widget.height;
		if (calculatedBlurStrength >= downSampleThreshold)
		{
			float downSample = calculatedBlurStrength / downSampleThreshold;
			width /= downSample;
			height /= downSample;
		}

		blurEffectRenderTarget1 = NewObject<UTextureRenderTarget2D>(this);
		blurEffectRenderTarget1->InitAutoFormat((int)width, (int)height);
		blurEffectRenderTarget1->UpdateResource();

		blurEffectRenderTarget2 = NewObject<UTextureRenderTarget2D>(this);
		blurEffectRenderTarget2->InitAutoFormat((int)width, (int)height);
		blurEffectRenderTarget2->UpdateResource();
	}
	else
	{
		if (blurEffectRenderTarget1->SizeX != widget.width || blurEffectRenderTarget1->SizeY != widget.height || bNeedToResize)
		{
			float width = widget.width;
			float height = widget.width;
			if (calculatedBlurStrength >= downSampleThreshold)
			{
				float downSample = calculatedBlurStrength / downSampleThreshold;
				width /= downSample;
				height /= downSample;
			}
			blurEffectRenderTarget1->ResizeTarget((int)width, (int)height);
			blurEffectRenderTarget2->ResizeTarget((int)width, (int)height);
		}
	}

	auto modelViewPrjectionMatrix = RenderCanvas->GetRootCanvas()->GetViewProjectionMatrix();
	auto& vertices = geometry->vertices;
	{
		FScopeLock scopeLock(&mutex);

		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = copyRegionVertexArray[i];
			//convert vertex postition to screen, and use as texture coordinate
			auto clipSpacePos = modelViewPrjectionMatrix.TransformPosition(vertices[i].Position);
			copyVert.TextureCoordinate0 = FVector2D(clipSpacePos.X / clipSpacePos.W, clipSpacePos.Y / clipSpacePos.W) * 0.5f + FVector2D(0.5f, 0.5f);
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
	auto calculatedBlurStrength = GetBlurStrengthInternal();
	if (calculatedBlurStrength <= 0.0f)return;
	if (!IsValid(blurEffectRenderTarget1))return;
	if (!IsValid(blurEffectRenderTarget2))return;

	FTexture2DRHIRef BlurEffectRenderTexture1 = blurEffectRenderTarget1->GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef BlurEffectRenderTexture2 = blurEffectRenderTarget2->GetRenderTargetResource()->GetRenderTargetTexture();
	//copy rect area from screen image to a render target, so we can just process this area
	{
		TArray<FLGUIPostProcessVertex> tempCopyRegion;
		{
			FScopeLock scopeLock(&mutex);
			tempCopyRegion = copyRegionVertexArray;
		}
		FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, GraphicsPSOInit, ScreenImage, BlurEffectRenderTexture1, false, tempCopyRegion);
	}
	//do the blur process on the area
	{
		TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FLGUIBlurShaderPS> PixelShader(GlobalShaderMap);
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		int iterateCount = 1;
		if (calculatedBlurStrength >= 1.0f)
		{
			iterateCount = FMath::CeilToInt(calculatedBlurStrength);
			iterateCount = FMath::Min(iterateCount, 5);//max iterate count is 5
		}
		for (int i = 0; i < iterateCount; i++)
		{
			//render vertical
			SetRenderTarget(RHICmdList, BlurEffectRenderTexture2, FTextureRHIRef());
			PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1, calculatedBlurStrength, false);
			FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
			//render horizontal
			SetRenderTarget(RHICmdList, BlurEffectRenderTexture1, FTextureRHIRef());
			PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture2, calculatedBlurStrength, true);
			FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
		}
	}
	//after blur process, copy the area back to screen image
	{
		TShaderMapRef<FLGUIMeshPostProcessVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FLGUICopyTargetMeshPS> PixelShader(GlobalShaderMap);
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIVertexDeclaration();
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
		GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		SetRenderTarget(RHICmdList, ScreenImage, FTextureRHIRef());
		VertexShader->SetParameters(RHICmdList, ViewProjectionMatrix);
		PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1);
		DrawPrimitive();
	}
}