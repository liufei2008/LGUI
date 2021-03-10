// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	UpdateRegionVertex();
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
	UpdateRegionVertex();
}
void UUIBackgroundBlur::WidthChanged()
{
	Super::WidthChanged();
}
void UUIBackgroundBlur::HeightChanged()
{
	Super::HeightChanged();
}
void UUIBackgroundBlur::MarkAllDirtyRecursive()
{
	Super::MarkAllDirtyRecursive();

	UpdateVertexData();
	UpdateStrengthTexture();
	UpdateMaskTexture();
	UpdateOthersData();
}
void UUIBackgroundBlur::UpdateRegionVertex()
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
			copyVert.TextureCoordinate0 = vertices[i].TextureCoordinate[0];
		}
	}
	UpdateVertexData();
}


DECLARE_CYCLE_STAT(TEXT("PostProcess_BackgroundBlur"), STAT_BackgroundBlur, STATGROUP_LGUI);
class FUIBackgroundBlurRenderProxy : public FUIPostProcessRenderProxy
{
public:
	TArray<FLGUIPostProcessVertex> renderScreenToMeshRegionVertexArray;
	TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
	FTexture2DResource* strengthTexture = nullptr;
	FTexture2DResource* maskTexture = nullptr;
	FUIWidget widget;
	float inv_SampleLevelInterval = 0.0f;
	float maxDownSampleLevel = 0.0f;
	float blurStrength = 0.0f;
public:
	FUIBackgroundBlurRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual void OnRenderPostProcess_RenderThread(
		FRHICommandListImmediate& RHICmdList,
		FTextureRHIRef ScreenImage,
		TShaderMap<FGlobalShaderType>* GlobalShaderMap,
		const FMatrix& ViewProjectionMatrix
	) override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackgroundBlur);
		if (blurStrength <= 0.0f)return;

		float width = widget.width;
		float height = widget.height;
		width = FMath::Max(width, 1.0f);
		height = FMath::Max(height, 1.0f);
		FVector2D inv_TextureSize(1.0f / width, 1.0f / height);
#if WITH_EDITOR
		inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;//only execute in edit mode, because it's already calculated in BeginPlay.
#endif
		//get render target
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget1;
		TRefCountPtr<IPooledRenderTarget> BlurEffectRenderTarget2;
		{
			auto MultiSampleCount = (uint16)ULGUISettings::GetAntiAliasingSampleCount();
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(FIntPoint(width, height), ScreenImage->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = MultiSampleCount;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget1, TEXT("LGUIBlurEffectRenderTarget1"));
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, BlurEffectRenderTarget2, TEXT("LGUIBlurEffectRenderTarget2"));
			if (!BlurEffectRenderTarget1.IsValid())
				return;
			if (!BlurEffectRenderTarget2.IsValid())
				return;
		}
		auto BlurEffectRenderTexture1 = BlurEffectRenderTarget1->GetRenderTargetItem().TargetableTexture;
		auto BlurEffectRenderTexture2 = BlurEffectRenderTarget2->GetRenderTargetItem().TargetableTexture;

		FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, BlurEffectRenderTexture1, renderScreenToMeshRegionVertexArray);
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
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);
				PixelShader->SetStrengthTexture(RHICmdList, strengthTexture->TextureRHI, strengthTexture->SamplerStateRHI);

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more linear
				calculatedBlurStrength = calculatedBlurStrength * inv_SampleLevelInterval;
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
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture2->GetSizeXYZ().X, BlurEffectRenderTexture2->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, true);
					FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					//render horizontal
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_DontStore), TEXT("Horizontal"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture1->GetSizeXYZ().X, BlurEffectRenderTexture1->GetSizeXYZ().Y, 1.0f);
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
				TShaderMapRef<FLGUIPostProcessGaussianBlurPS> PixelShader(GlobalShaderMap);

				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetInverseTextureSize(RHICmdList, inv_TextureSize);

				auto samplerState = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
				float calculatedBlurStrength = FMath::Pow(blurStrength * INV_MAX_BlurStrength, 0.5f) * MAX_BlurStrength;//this can make the blur effect transition feel more linear
				calculatedBlurStrength = blurStrength * inv_SampleLevelInterval;
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
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture2->GetSizeXYZ().X, BlurEffectRenderTexture2->GetSizeXYZ().Y, 1.0f);
					PixelShader->SetMainTexture(RHICmdList, BlurEffectRenderTexture1, samplerState);
					PixelShader->SetHorizontalOrVertical(RHICmdList, true);
					FLGUIViewExtension::DrawFullScreenQuad(RHICmdList);
					RHICmdList.EndRenderPass();
					//render horizontal
					RHICmdList.BeginRenderPass(FRHIRenderPassInfo(BlurEffectRenderTexture1, ERenderTargetActions::Load_DontStore), TEXT("Horizontal"));
					RHICmdList.SetViewport(0, 0, 0.0f, BlurEffectRenderTexture1->GetSizeXYZ().X, BlurEffectRenderTexture1->GetSizeXYZ().Y, 1.0f);
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
			if (maskTexture != nullptr)
			{
				FLGUIViewExtension::CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, ScreenImage, BlurEffectRenderTexture2, renderScreenToMeshRegionVertexArray);

				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenImage, ERenderTargetActions::Load_DontStore), TEXT("CopyAreaToScreen"));
				RHICmdList.SetViewport(0, 0, 0.0f, ScreenImage->GetSizeXYZ().X, ScreenImage->GetSizeXYZ().Y, 1.0f);
				TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUISimpleCopyTargetWithMaskPS> PixelShader(GlobalShaderMap);
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1, BlurEffectRenderTexture2, maskTexture->TextureRHI
					, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
					, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
					, maskTexture->SamplerStateRHI
				);
			}
			else
			{
				RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenImage, ERenderTargetActions::Load_DontStore), TEXT("CopyAreaToScreen"));
				RHICmdList.SetViewport(0, 0, 0.0f, ScreenImage->GetSizeXYZ().X, ScreenImage->GetSizeXYZ().Y, 1.0f);
				TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FLGUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
				VertexShader->SetParameters(RHICmdList);
				PixelShader->SetParameters(RHICmdList, BlurEffectRenderTexture1);
			}

			uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
			FRHIResourceCreateInfo CreateInfo;
			FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
			void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
			FPlatformMemory::Memcpy(VoidPtr, renderMeshRegionToScreenVertexArray.GetData(), VertexBufferSize);
			RHIUnlockVertexBuffer(VertexBufferRHI);
			RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
			VertexBufferRHI.SafeRelease();

			RHICmdList.EndRenderPass();
		}

		//release render target
		BlurEffectRenderTarget1.SafeRelease();
		BlurEffectRenderTarget2.SafeRelease();
	}
};


void UUIBackgroundBlur::UpdateOthersData()
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
void UUIBackgroundBlur::UpdateVertexData()
{
	if (RenderProxy.IsValid())
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)(RenderProxy.Get());
		struct FUIBackgroundBlurUpdateVertexData
		{
			TArray<FLGUIPostProcessVertex> renderScreenToMeshRegionVertexArray;
			TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
			FUIWidget widget;
		};
		auto updateData = new FUIBackgroundBlurUpdateVertexData();
		updateData->renderMeshRegionToScreenVertexArray = this->renderMeshRegionToScreenVertexArray;
		updateData->renderScreenToMeshRegionVertexArray = this->renderScreenToMeshRegionVertexArray;
		updateData->widget = this->widget;
		ENQUEUE_RENDER_COMMAND(FUIBackgroundBlur_UpdateData)
			([BackgroundBlurRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->renderScreenToMeshRegionVertexArray = updateData->renderScreenToMeshRegionVertexArray;
					BackgroundBlurRenderProxy->renderMeshRegionToScreenVertexArray = updateData->renderMeshRegionToScreenVertexArray;
					BackgroundBlurRenderProxy->widget = updateData->widget;
					delete updateData;
				});
	}
}
void UUIBackgroundBlur::UpdateStrengthTexture()
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
void UUIBackgroundBlur::UpdateMaskTexture()
{
	if (RenderProxy.IsValid())
	{
		auto BackgroundBlurRenderProxy = (FUIBackgroundBlurRenderProxy*)(RenderProxy.Get());
		FTexture2DResource* maskTextureResource = nullptr;
		if (IsValid(this->maskTexture) && this->maskTexture->Resource != nullptr)
		{
			maskTextureResource = (FTexture2DResource*)this->maskTexture->Resource;
		}
		ENQUEUE_RENDER_COMMAND(FUIBackgroundBlur_UpdateData)
			([this, BackgroundBlurRenderProxy, maskTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					BackgroundBlurRenderProxy->maskTexture = maskTextureResource;
				});
	}
}

void UUIBackgroundBlur::SetBlurStrength(float newValue)
{
	if (blurStrength != newValue)
	{
		blurStrength = newValue;
		UpdateOthersData();
	}
}

void UUIBackgroundBlur::SetApplyAlphaToBlur(bool newValue)
{
	if (applyAlphaToBlur != newValue)
	{
		applyAlphaToBlur = newValue;
		UpdateOthersData();
	}
}

void UUIBackgroundBlur::SetMaxDownSampleLevel(int newValue)
{
	if (maxDownSampleLevel != newValue)
	{
		maxDownSampleLevel = newValue;
		inv_SampleLevelInterval = 1.0f / MAX_BlurStrength * maxDownSampleLevel;
		UpdateOthersData();
	}
}

void UUIBackgroundBlur::SetStrengthTexture(UTexture2D* newValue)
{
	if (strengthTexture != newValue)
	{
		strengthTexture = newValue;
		UpdateStrengthTexture();
	}
}

void UUIBackgroundBlur::SetMaskTexture(UTexture2D* newValue)
{
	if (maskTexture != newValue)
	{
		maskTexture = newValue;
		UpdateMaskTexture();
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

TWeakPtr<FUIPostProcessRenderProxy> UUIBackgroundBlur::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		RenderProxy = TSharedPtr<FUIBackgroundBlurRenderProxy>(new FUIBackgroundBlurRenderProxy());
		UpdateVertexData();
		UpdateStrengthTexture();
		UpdateMaskTexture();
		UpdateOthersData();
	}
	return RenderProxy;
}

