// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexVisualPostProcess.h"
#include "Core/LexVisualBackBufferRenderProxy.h"
#include "Core/Components/LexWidget.h"


ULexVisualPostProcess::ULexVisualPostProcess(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

#if WITH_EDITOR
void ULexVisualPostProcess::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	SendMaskTextureToRenderProxy();
}
bool ULexVisualPostProcess::CanEditChange(const FProperty* InProperty) const
{
	return Super::CanEditChange(InProperty);
}
#endif


void ULexVisualPostProcess::SetMaskTexture(UTexture2D* Value)
{
	if (MaskTexture != Value)
	{
		MaskTexture = Value;
		SendMaskTextureToRenderProxy();
		MarkAllDirty();
	}
}
void ULexVisualPostProcess::SetMaskTextureUVRect(const FVector4& Value)
{
	if (MaskTextureUVRect != Value)
	{
		MaskTextureUVRect = Value;
		MarkUVDirty();
	}
}

void ULexVisualPostProcess::SendMaskTextureToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = (FLexVisualPostProcessRenderProxy*)RenderProxy;
		FTexture2DResource* MaskTextureResource = nullptr;
		if (IsValid(this->MaskTexture) && this->MaskTexture->GetResource() != nullptr)
		{
			MaskTextureResource = (FTexture2DResource*)this->MaskTexture->GetResource();
		}
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateMaskTexture)
			([TempRenderProxy, MaskTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->MaskTexture = MaskTextureResource;
				});
	}
}

void ULexVisualPostProcess::SendRegionVertexDataToRenderProxy()
{
	Super::SendRegionVertexDataToRenderProxy();
	auto Widget = this->GetWidget();
	if (!Widget)return;

	auto RenderCanvas = Widget->GetRenderCanvas();
	if (RenderProxy && RenderCanvas)
	{
		auto TempRenderProxy = (FLexVisualPostProcessRenderProxy*)RenderProxy;
		auto ClipDataTex = this->GetClipDataTexture();
		if (IsValid(ClipDataTex) && ClipDataTex->GetResource() != nullptr)
		{
			auto ClipDataTexture = (FTexture2DDynamicResource*)ClipDataTex->GetResource();
			ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateData)
				([TempRenderProxy, ClipDataTexture](FRHICommandListImmediate& RHICmdList)
					{
						TempRenderProxy->ClipDataTexture = ClipDataTexture;
					});
		}
	}
}

