// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexBackBufferCopy.h"

#include "LGUI.h"
#include "Core/LexUIRender/LexUIRenderer.h"
#include "RenderTargetPool.h"
#include "Core/LexVisualBackBufferRenderProxy.h"
#include "Core/Components/LexCanvas.h"
#include "Engine/TextureRenderTarget2D.h"

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
	SendRenderTargetToRenderProxy();
}
#endif


void ULexBackBufferCopy::MarkAllDirty()
{
	Super::MarkAllDirty();

	SendRegionVertexDataToRenderProxy();
	SendRenderTargetToRenderProxy();
}

void ULexBackBufferCopy::ClearMaterialsUsingThisBackBuffer()
{
	MaterialsUsingThisBackBuffer.Reset();
}

void ULexBackBufferCopy::RegisterMaterialsUsingThisBackBuffer(UMaterialInstanceDynamic* InMaterialInstanceDynamic)
{
	MaterialsUsingThisBackBuffer.Add(InMaterialInstanceDynamic);
	InMaterialInstanceDynamic->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, RenderTarget);
	InMaterialInstanceDynamic->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_BackBufferCopy"), STAT_BackBufferCopy, STATGROUP_LGUI);
class FUIBackBufferCopyRenderProxy : public FLexVisualBackBufferRenderProxy
{
public:
	//output target
	FTextureRenderTargetResource* RenderTargetResource = nullptr;
	
	FUIBackBufferCopyRenderProxy()
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
			FLexUIRenderer::AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), FIntRect(0, 0, ScreenSize.X, ScreenSize.Y), NumSamples, GlobalShaderMap);
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
		FLexUIRenderer::CopyRenderTargetOnMeshRegion(GraphBuilder
			, RenderTargetTextureRDG
			, NumSamples > 1 ? ScreenResolvedRenderTarget->GetRHI() : ScreenTargetTexture.GetReference()
			, GlobalShaderMap
			, RenderScreenToMeshRegionVertexArray
			, bIsRenderTarget
			, FIntRect(0, 0, RenderTargetRHITexture->GetSizeXYZ().X, RenderTargetRHITexture->GetSizeXYZ().Y)
			, ViewTextureScaleOffset
			, true//linearize it, so material sample will get correct color
		);

		//after filter, copy back to render target
		{
			// auto ModelViewProjectionMatrix = ObjectToWorldMatrix * ViewProjectionMatrix;
			// RenderMeshOnScreen_RenderThread(GraphBuilder, SceneTextures, ScreenTargetTexture, GlobalShaderMap, RenderTargetRHITexture, ModelViewProjectionMatrix, ObjectToWorldMatrix, bIsWorldSpace, BlendDepthForWorld, DepthFadeForWorld, DepthTextureScaleOffset, ViewRect);
			// Renderer->CopyRenderTarget_LinearizeColor(GraphBuilder, GlobalShaderMap, RenderTargetRHITexture, RenderTargetResource->GetRenderTargetTexture());
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

void ULexBackBufferCopy::OnRegister()
{
	Super::OnRegister();
}

void ULexBackBufferCopy::OnUnregister()
{
	Super::OnUnregister();
	OnRenderTargetChanged.Broadcast(nullptr);
}

void ULexBackBufferCopy::PostUpdateDrawCall()
{
	Super::PostUpdateDrawCall();
	UpdateRenderTarget();
	for (auto& MID : MaterialsUsingThisBackBuffer)
	{
		if (!MID.IsValid())continue;
		MID->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
	}
}

FLexVisualBackBufferRenderProxy* ULexBackBufferCopy::GetRenderProxy()
{
	if (RenderProxy == nullptr)
	{
		RenderProxy = new FUIBackBufferCopyRenderProxy();
		SendRegionVertexDataToRenderProxy();
		SendRenderTargetToRenderProxy();
	}
	return RenderProxy;
}

void ULexBackBufferCopy::SendRenderTargetToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FUIBackBufferCopyRenderProxy*)RenderProxy;
		FTextureRenderTargetResource* RenderTargetResource = nullptr;
		if (IsValid(RenderTarget))
		{
			RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
		}
		else
		{
			RenderTargetResource = nullptr;
		}
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, RenderTargetResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->RenderTargetResource = RenderTargetResource;
				});
	}
}

void ULexBackBufferCopy::SetRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (RenderTarget != InRenderTarget)
	{
		RenderTarget = InRenderTarget;
		UpdateRenderTarget();
		OnRenderTargetChanged.Broadcast(RenderTarget);
	}
}

void ULexBackBufferCopy::UpdateRenderTarget()
{
	auto DesiredRenderTargetSize = MeshRectInScreen.GetSize();
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X < 1 || DesiredRenderTargetSize.Y < 1)
	{
		return;
	}
	DesiredRenderTargetSize.X = FMath::Min(DesiredRenderTargetSize.X, MaxAllowedDrawSize);
	DesiredRenderTargetSize.Y = FMath::Min(DesiredRenderTargetSize.Y, MaxAllowedDrawSize);

	if (RenderTarget == nullptr)
	{
		RenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, EObjectFlags::RF_Transient);
		RenderTarget->AddressX = TextureAddress::TA_Clamp;
		RenderTarget->AddressY = TextureAddress::TA_Clamp;
		RenderTarget->ClearColor = FLinearColor::Transparent;
		RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
		SendRenderTargetToRenderProxy();
		OnRenderTargetChanged.Broadcast(RenderTarget);
		//update material's texture, because OutputRenderTarget could be null when register
		for (auto& MID : MaterialsUsingThisBackBuffer)
		{
			if (!MID.IsValid())continue;
			MID->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, RenderTarget);
		}
	}
	else
	{
		if (RenderTarget->SizeX != DesiredRenderTargetSize.X || RenderTarget->SizeY != DesiredRenderTargetSize.Y)
		{
			RenderTarget->ClearColor = FLinearColor::Transparent;
			RenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
			RenderTarget->UpdateResourceImmediate();
#if WITH_EDITOR
			RenderTarget->Modify();
#endif
			SendRenderTargetToRenderProxy();
		}
	}

#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		if (!RenderTarget->GameThread_GetRenderTargetResource())
		{
			RenderTarget->InitCustomFormat(RenderTarget->SizeX, RenderTarget->SizeY, EPixelFormat::PF_B8G8R8A8, false);
			SendRenderTargetToRenderProxy();
		}
	}
#endif
}
