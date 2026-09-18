// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackBufferCopy.h"

#include "LGUI.h"
#include "Core/LexUIRender/LexUIPostProcessShaders.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualPostProcessRenderProxy.h"

ULexBackBufferCopy::ULexBackBufferCopy(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	
}

#if WITH_EDITOR
void ULexBackBufferCopy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
	}
}
#endif


void ULexBackBufferCopy::MarkAllDirty()
{
	Super::MarkAllDirty();

	SendRegionVertexDataToRenderProxy();
	SendMaskTextureToRenderProxy();
	SendOthersDataToRenderProxy();
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackBufferCopy"), STAT_BackBufferCopy, STATGROUP_LGUI);
class FUIBackBufferCopyRenderProxy : public FLexVisualPostProcessRenderProxy
{
public:
	FUIBackBufferCopyRenderProxy()
		:FLexVisualPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return RenderTargetResource != nullptr;
	}
	virtual void OnRenderPostProcess_RenderThread(
		FRDGBuilder& GraphBuilder,
		const FMinimalSceneTextures& SceneTextures,
		FLexUIRenderer* Renderer,
		FTextureRHIRef ScreenTargetTexture,
		FGlobalShaderMap* GlobalShaderMap,
		const FMatrix44f& ViewProjectionMatrix,
		bool bIsWorldSpace,
		bool bIsRenderTarget,
		float BlendDepthForWorld,
		int DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	) override
	{
		SCOPE_CYCLE_COUNTER(STAT_BackBufferCopy);

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedRenderTarget;
		uint8 NumSamples = ScreenTargetTexture->GetNumSamples();
		auto ScreenSize = ScreenTargetTexture->GetSizeXY();
		if (NumSamples > 1)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(ScreenSize, ScreenTargetTexture->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			GRenderTargetPool.FindFreeElement(GraphBuilder.RHICmdList, desc, ScreenResolvedRenderTarget, TEXT("LexUIBackBufferCopyResolveTarget"));
			if (!ScreenResolvedRenderTarget.IsValid())
				return;
			auto ResolveSrc = RegisterExternalTexture(GraphBuilder, ScreenTargetTexture, TEXT("LexUIBackBufferCopyResolveSource"));
			auto ResolveDst = RegisterExternalTexture(GraphBuilder, ScreenResolvedRenderTarget->GetRHI(), TEXT("LexUIBackBufferCopyResolveTarget"));
			Renderer->AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
		}
		
		auto RenderTargetRHITexture = RenderTargetResource->GetRenderTargetTexture();
		auto RenderTargetTextureRDG = RegisterExternalTexture(GraphBuilder, RenderTargetRHITexture, TEXT("LexUIBackBufferCopy_RDG"));
		//clear the whole target first so the area not covered by the mesh region is deterministic.
#if 0
		{
			auto* ClearParameters = GraphBuilder.AllocParameters<FRenderTargetParameters>();
			ClearParameters->RenderTargets[0] = FRenderTargetBinding(RenderTargetTextureRDG, ERenderTargetLoadAction::EClear);
			GraphBuilder.AddPass(RDG_EVENT_NAME("LexUIBackBufferCopy_ClearRegionTarget"), ClearParameters, ERDGPassFlags::Raster, [](FRHICommandListImmediate&) {});
		}
#endif
		Renderer->CopyRenderTargetOnMeshRegion(GraphBuilder
			, RenderTargetTextureRDG
			, NumSamples > 1 ? ScreenResolvedRenderTarget->GetRHI() : ScreenTargetTexture.GetReference()
			, GlobalShaderMap
			, RenderScreenToMeshRegionVertexArray
			, bIsRenderTarget
			, FIntRect(0, 0, RenderTargetRHITexture->GetSizeXYZ().X, RenderTargetRHITexture->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
			, true
		);

		//after filter, copy back to render target
		{
			// Renderer->CopyRenderTarget_ColorCorrect(GraphBuilder, GlobalShaderMap, RenderTargetRHITexture, RenderTargetResource->GetRenderTargetTexture());
		}

		//Defer releasing the pooled render targets until the graph executes
		GraphBuilder.AddPass(
			RDG_EVENT_NAME("LexUIBackBufferCopy_ReleaseRenderTargets"),
			ERDGPassFlags::None,
			[ScreenResolvedRenderTarget](FRHICommandListImmediate& RHICmdList) mutable
			{
				if (ScreenResolvedRenderTarget.IsValid()) ScreenResolvedRenderTarget.SafeRelease();
			});
	}
};


void ULexBackBufferCopy::SendOthersDataToRenderProxy()
{
	if (RenderProxy != nullptr)
	{
		
	}
}

FLexVisualPostProcessRenderProxy* ULexBackBufferCopy::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackBufferCopyRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendMaskTextureToRenderProxy();
		SendRenderTargetToRenderProxy();
		SendOthersDataToRenderProxy();
	}
	return RenderProxy;
}

void ULexBackBufferCopy::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	if (RenderProxy != nullptr)
	{
		
	}
}
