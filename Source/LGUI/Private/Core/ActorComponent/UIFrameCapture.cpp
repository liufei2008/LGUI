// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIFrameCapture.h"
#include "LGUI.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "GameFramework/PlayerController.h"
#include "RenderTargetPool.h"

UUIFrameCapture::UUIFrameCapture(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UUIFrameCapture::BeginPlay()
{
	Super::BeginPlay();
}

void UUIFrameCapture::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	if (bIsFrameReady)
	{
		bIsFrameReady = false;
		OnFrameReady.Broadcast(CapturedFrame);
		OnFrameReady.Clear();
		CapturedFrame = nullptr;
		UpdateRenderTarget();
		SendOthersDataToRenderProxy();
	}
}


#if WITH_EDITOR
void UUIFrameCapture::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{
		
	}
}
bool UUIFrameCapture::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUIFrameCapture, maskTexture))
		{
			return false;//mask texture not needed here
		}
	}
	return Super::CanEditChange(InProperty);
}
#endif
void UUIFrameCapture::MarkAllDirty()
{
	Super::MarkAllDirty();

	if (this->RenderCanvas.IsValid())
	{
		
	}
}

DECLARE_CYCLE_STAT(TEXT("PostProcess_UIFrameCapture"), STAT_FrameGrabber, STATGROUP_LGUI);
class FUIFrameCaptureRenderProxy :public FUIPostProcessRenderProxy
{
public:
	bool bCaptureFullScreen = true;
	FTextureRenderTargetResource* CapturedFrameResource = nullptr;
	bool bDoCapture = false;
	/**
	 * This is a pointer to UIFrameCapture's IsFrameReady.
	 * Why it is safe to use? Check PrimitiveSceneInfo.h OwnerLastRenderTime
	 */
	bool* OwnerIsFrameReady = nullptr;
public:
	FUIFrameCaptureRenderProxy()
		:FUIPostProcessRenderProxy()
	{

	}
	virtual bool CanRender()const override
	{
		return bDoCapture;
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
		float DepthFadeForWorld,
		const FIntRect& ViewRect,
		const FVector4f& DepthTextureScaleOffset,
		const FVector4f& ViewTextureScaleOffset
	)override
	{
		if (CapturedFrameResource == nullptr || CapturedFrameResource->GetRenderTargetTexture() == nullptr)return;
		if (!bDoCapture)return;
		bDoCapture = false;
		SCOPE_CYCLE_COUNTER(STAT_FrameGrabber);
		auto& RHICmdList = GraphBuilder.RHICmdList;

		TRefCountPtr<IPooledRenderTarget> ScreenResolvedTexture;
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
			Renderer->AddResolvePass(GraphBuilder, FRDGTextureMSAA(ResolveSrc, ResolveDst), false, 0, FIntRect(0, 0, Size.X, Size.Y), NumSamples, GlobalShaderMap);
		}

		auto CapturedFrameTexture = (FTextureRHIRef)CapturedFrameResource->GetRenderTargetTexture();
		if (bCaptureFullScreen)
		{
			Renderer->CopyRenderTarget(GraphBuilder, GlobalShaderMap
				, NumSamples > 1 ? ScreenResolvedTexture->GetRHI() : ScreenTargetTexture.GetReference()
				, CapturedFrameTexture, true);
		}
		else
		{
			//copy rect area from screen image to a render target
			auto modelViewProjectionMatrix = objectToWorldMatrix * ViewProjectionMatrix;
			Renderer->CopyRenderTargetOnMeshRegion(GraphBuilder
				, RegisterExternalTexture(GraphBuilder, CapturedFrameTexture, TEXT("LGUI_FrameCaptureTargetTexture"))
				, NumSamples > 1 ? ScreenResolvedTexture->GetRHI() : ScreenTargetTexture.GetReference()
				, GlobalShaderMap
				, renderScreenToMeshRegionVertexArray
				, modelViewProjectionMatrix
				, FIntRect(0, 0, CapturedFrameTexture->GetSizeXYZ().X, CapturedFrameTexture->GetSizeXYZ().Y)
				, ViewTextureScaleOffset
				, true
			);
		}
		*OwnerIsFrameReady = true;
		
		if (ScreenResolvedTexture.IsValid())
		{
			ScreenResolvedTexture.SafeRelease();
		}
	}
};

TSharedPtr<FUIPostProcessRenderProxy> UUIFrameCapture::GetRenderProxy()
{
	if (!RenderProxy.IsValid())
	{
		auto Proxy = TSharedPtr<FUIFrameCaptureRenderProxy>(new FUIFrameCaptureRenderProxy());
		Proxy->OwnerIsFrameReady = &this->bIsFrameReady;
		RenderProxy = Proxy;
		if (this->RenderCanvas.IsValid())
		{
			SendRegionVertexDataToRenderProxy();
		}
	}
	return RenderProxy;
}

void UUIFrameCapture::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	UpdateRenderTarget();
	SendOthersDataToRenderProxy();
}

void UUIFrameCapture::SendOthersDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = (FUIFrameCaptureRenderProxy*)(RenderProxy.Get());
		ENQUEUE_RENDER_COMMAND(FUIBackgroundPixelate_UpdateData)
			([TempRenderProxy, bCaptureFullScreen = this->bCaptureFullScreen, RenderTargetResource = CapturedFrame->GameThread_GetRenderTargetResource(), bPendingCapture = bPendingCapture](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->bCaptureFullScreen = bCaptureFullScreen;
					TempRenderProxy->CapturedFrameResource = RenderTargetResource;
					TempRenderProxy->bDoCapture = bPendingCapture;
				});
		bPendingCapture = false;
	}
}
void UUIFrameCapture::UpdateRenderTarget()
{
	FIntPoint DesiredRenderTargetSize(this->GetWidth(), this->GetHeight());
	if (this->bCaptureFullScreen)
	{
		if (auto pc = this->GetWorld()->GetFirstPlayerController())
		{
			pc->GetViewportSize(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y);
		}
	}
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X <= 0 || DesiredRenderTargetSize.Y <= 0)
	{
		return;
	}
	DesiredRenderTargetSize.X = FMath::Min(DesiredRenderTargetSize.X, MaxAllowedDrawSize);
	DesiredRenderTargetSize.Y = FMath::Min(DesiredRenderTargetSize.Y, MaxAllowedDrawSize);

	if (CapturedFrame == nullptr)
	{
		CapturedFrame = NewObject<UTextureRenderTarget2D>(this, NAME_None, EObjectFlags::RF_Transient);
		CapturedFrame->AddressX = TextureAddress::TA_Clamp;
		CapturedFrame->AddressY = TextureAddress::TA_Clamp;
		CapturedFrame->ClearColor = FLinearColor::Transparent;
		CapturedFrame->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
	}
	else
	{
		if (CapturedFrame->SizeX != DesiredRenderTargetSize.X || CapturedFrame->SizeY != DesiredRenderTargetSize.Y)
		{
			CapturedFrame->ClearColor = FLinearColor::Transparent;
			CapturedFrame->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
			CapturedFrame->UpdateResourceImmediate();
#if WITH_EDITOR
			CapturedFrame->Modify();
#endif
		}
	}
}

void UUIFrameCapture::DoCapture(const FUIFrameCapture_OnFrameReady_DynamicDelegate& InDelegate)
{
	MarkOneFrameCapture();
	OnFrameReady.AddLambda([InDelegate](UTextureRenderTarget2D* InCapturedFrame) {
		InDelegate.ExecuteIfBound(InCapturedFrame);
		});
}
void UUIFrameCapture::DoCapture(const FUIFrameCapture_OnFrameReady_Delegate& InDelegate)
{
	MarkOneFrameCapture();
	OnFrameReady.Add(InDelegate);
}
void UUIFrameCapture::DoCapture(const TFunction<void(UTextureRenderTarget2D*)>& InFunction)
{
	MarkOneFrameCapture();
	OnFrameReady.AddLambda(InFunction);
}

void UUIFrameCapture::MarkOneFrameCapture()
{
	bPendingCapture = true;
	SendOthersDataToRenderProxy();
}
