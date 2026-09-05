// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexVisualPostProcess.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "Core/Components/LexWidget.h"
#include "Engine/TextureRenderTarget2D.h"



ULexVisualPostProcess::ULexVisualPostProcess(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	VisualType = ELexVisualType::PostProcess;
	Geometry = TSharedPtr<FLexUIGeometry>(new FLexUIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void ULexVisualPostProcess::BeginPlay()
{
	Super::BeginPlay();

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void ULexVisualPostProcess::BeginDestroy()
{
	ENQUEUE_RENDER_COMMAND(FLexPostProcess_ReleaseRenderProxy)
			([RenderProxyPtr = RenderProxy](FRHICommandListImmediate& RHICmdList)
				{
					delete RenderProxyPtr;
				});
	Super::BeginDestroy();
}

void ULexVisualPostProcess::OnUnregister()
{
	Super::OnUnregister();
	OnRenderTargetChanged.Broadcast(nullptr);
}

#if WITH_EDITOR
void ULexVisualPostProcess::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (RenderType == ELexBackgroundBlurRenderType::RenderTarget)
	{
		UpdateRenderTarget();
	}
	else
	{
		OnRenderTargetChanged.Broadcast(nullptr);
	}
	
	SendMaskTextureToRenderProxy();
	SendRenderTargetToRenderProxy();
}
bool ULexVisualPostProcess::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();
	}
	return Super::CanEditChange(InProperty);
}
#endif


void ULexVisualPostProcess::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
    Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
    if (InPivotChange || InWidthChange || InHeightChange)
    {
	    MarkVertexPositionDirty();
    }
	if (InWidthChange || InHeightChange)
	{
		UpdateRenderTarget();
	}
}
void ULexVisualPostProcess::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
	UpdateRenderTarget();
}

void ULexVisualPostProcess::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	GetWidget()->MarkCanvasUpdate(true);
}
void ULexVisualPostProcess::MarkUVDirty()
{
	bUVChanged = true;
	GetWidget()->MarkCanvasUpdate(false);
}

void ULexVisualPostProcess::MarkAllDirty()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	Super::MarkAllDirty();
	SendRenderTargetToRenderProxy();
}

DECLARE_CYCLE_STAT(TEXT("UIPostProcessRenderable UpdateGeometry"), STAT_UIPostProcessRenderableUpdate, STATGROUP_LGUI);
void ULexVisualPostProcess::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UIPostProcessRenderableUpdate);
	auto Widget = GetWidget();
	auto RenderCanvas = Widget->GetRenderCanvas();
	check(RenderCanvas);

	Super::UpdateGeometry();
	
	if (bLocalVertexPositionChanged || bUVChanged || bColorChanged)
	{
		Geometry->Clear();
		OnUpdateGeometry(false, bLocalVertexPositionChanged, bUVChanged, bColorChanged);
	}
	if (bClipDataPositionChanged)
	{
		UpdateGeometryClipData(*Geometry.Get(), ClipDataStartPosition);
	}
	if (bLocalVertexPositionChanged || bTransformChanged)
	{
		FLexUIGeometry::TransformVertices(RenderCanvas, this, Geometry.Get());
	}
	if (bLocalVertexPositionChanged || bUVChanged || bColorChanged || bTransformChanged || bClipDataPositionChanged)
	{
		UpdateRegionVertex();
	}

	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
}
void ULexVisualPostProcess::OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	//simple rect geometry for render from screen image to mesh region and inverse
	{
		auto& Vertices = Geometry->Vertices;
		auto& OriginVertices = Geometry->OriginVertices;
		FLexUIGeometry::LexUIGeometrySetArrayNum(Vertices, 4);
		FLexUIGeometry::LexUIGeometrySetArrayNum(OriginVertices, 4);
		if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
		{
			if (InVertexPositionChanged)
			{
				auto Widget = bUseFullSize ? GetWidget()->GetRenderCanvas()->GetRootCanvas()->GetWidget() : this->GetWidget();
				//offset and size
				float pivotOffsetX = 0, pivotOffsetY = 0;
				FLexUIGeometry::CalculatePivotOffset(Widget->GetWidth(), Widget->GetHeight(), FVector2f(Widget->GetPivot()), pivotOffsetX, pivotOffsetY);
				float halfW = Widget->GetWidth() * 0.5f, halfH = Widget->GetHeight() * 0.5f;
				//positions
				float minX = -halfW + pivotOffsetX;
				float minY = -halfH + pivotOffsetY;
				float maxX = halfW + pivotOffsetX;
				float maxY = halfH + pivotOffsetY;
				OriginVertices[0].Position = FVector3f(0, minX, minY);
				OriginVertices[1].Position = FVector3f(0, maxX, minY);
				OriginVertices[2].Position = FVector3f(0, minX, maxY);
				OriginVertices[3].Position = FVector3f(0, maxX, maxY);
				//snap pixel
				if (Widget->GetPixelSnappingInHierarchy())
				{
					FLexUIGeometry::AdjustPixelPerfectPos(OriginVertices, 0, 4, Widget->GetRenderCanvas(), this);
				}
			}

			if (InVertexUVChanged)
			{
				Vertices[0].TextureCoordinate[0] = FVector2f(0, 1);
				Vertices[1].TextureCoordinate[0] = FVector2f(1, 1);
				Vertices[2].TextureCoordinate[0] = FVector2f(0, 0);
				Vertices[3].TextureCoordinate[0] = FVector2f(1, 0);
			}

			if (InVertexColorChanged)
			{
				FLexUIGeometry::UpdateUIColor(Geometry.Get(), GetFinalColor());
			}
		}
	}
}

void ULexVisualPostProcess::UpdateRegionVertex()
{
	if (RenderScreenToMeshRegionVertexArray.Num() == 0)
	{
		//full screen vertex position
		RenderScreenToMeshRegionVertexArray =
		{
			FLexUIPostProcessCopyMeshRegionVertex(FVector3f(-1, -1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector3f(1, -1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector3f(-1, 1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector3f(1, 1, 0), FVector3f(0.0f, 0.0f, 0.0f))
		};
	}

	auto& Vertices = Geometry->Vertices;
	for (int i = 0; i < 4; i++)
	{
		auto& copyVert = RenderScreenToMeshRegionVertexArray[i];
		copyVert.LocalPosition = Vertices[i].Position;
	}
	
	constexpr int VertexBufferSize = 4;
	if (RenderMeshRegionToScreenVertexArray.Num() != VertexBufferSize)
	{
		RenderMeshRegionToScreenVertexArray.SetNumZeroed(VertexBufferSize);
	}

	for (int i = 0; i < VertexBufferSize; i++)
	{
		auto& copyVert = RenderMeshRegionToScreenVertexArray[i];
		copyVert.Position = Vertices[i].Position;
		copyVert.TextureCoordinate0 = Vertices[i].TextureCoordinate[0];
		copyVert.TextureCoordinate1 = Vertices[i].TextureCoordinate[1];
	}

	SendRegionVertexDataToRenderProxy();
}

void ULexVisualPostProcess::UpdateGeometryClipData(FLexUIGeometry& InMesh, int InDataStartPosition)
{
	auto& vertices = InMesh.Vertices;
	for (int i = 0; i < vertices.Num(); i++)
	{
		vertices[i].TextureCoordinate[1].X = InDataStartPosition;
	}
}

void ULexVisualPostProcess::SendRegionVertexDataToRenderProxy()
{
	auto ThisWidget = this->GetWidget();
	if (!ThisWidget)return;
	ULexWidget* Widget = nullptr;
	if (bUseFullSize)
	{
		auto ThisRenderCanvas = ThisWidget->GetRenderCanvas();
		if (!ThisRenderCanvas)return;
		Widget = ThisRenderCanvas->GetRootCanvas()->GetWidget();
	}
	else
	{
		Widget = ThisWidget;
	}
	auto RenderCanvas = Widget->GetRenderCanvas();
	if (RenderProxy && RenderCanvas)
	{
		auto TempRenderProxy = RenderProxy;
		struct FUIPostProcess_SendRegionVertexDataToRenderProxy
		{
			TArray<FLexUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
			TArray<FLexUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
			FVector2f RectSize;
			FMatrix44f objectToWorldMatrix;
			FTexture2DDynamicResource* ClipDataTexture = nullptr;
			bool bUseFullSize;
			FBox BoundingBox;
		};
		auto updateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		updateData->renderMeshRegionToScreenVertexArray = this->RenderMeshRegionToScreenVertexArray;
		updateData->renderScreenToMeshRegionVertexArray = this->RenderScreenToMeshRegionVertexArray;
		updateData->RectSize = FVector2f(Widget->GetWidth(), Widget->GetHeight());
		updateData->objectToWorldMatrix = FMatrix44f(RenderCanvas->GetWidget()->GetWorldTransform().ToMatrixWithScale());
		updateData->bUseFullSize = bUseFullSize;
		{
			updateData->BoundingBox = FBox(EForceInit::ForceInit);
			FVector2D Min, Max;
			this->GetGeometryBoundsInLocalSpace(Min, Max);
			auto WorldMin = this->GetWidget()->GetWorldTransform().TransformPosition(FVector(0, Min.X, Min.Y));
			auto WorldMax = this->GetWidget()->GetWorldTransform().TransformPosition(FVector(0, Max.X, Max.Y));
			updateData->BoundingBox += WorldMin;
			updateData->BoundingBox += WorldMax;
		}
		auto ClipDataTex = this->GetClipDataTexture();
		if (IsValid(ClipDataTex) && ClipDataTex->GetResource() != nullptr)
		{
			updateData->ClipDataTexture = (FTexture2DDynamicResource*)ClipDataTex->GetResource();
		}
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateData)
			([TempRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->RenderScreenToMeshRegionVertexArray = updateData->renderScreenToMeshRegionVertexArray;
					TempRenderProxy->RenderMeshRegionToScreenVertexArray = updateData->renderMeshRegionToScreenVertexArray;
					TempRenderProxy->RectSize = updateData->RectSize;
					TempRenderProxy->ObjectToWorldMatrix = updateData->objectToWorldMatrix;
					TempRenderProxy->ClipDataTexture = updateData->ClipDataTexture;
					TempRenderProxy->bUseFullSize = updateData->bUseFullSize;
					TempRenderProxy->BoundingBox = updateData->BoundingBox;
					delete updateData;
				});
	}
}

void ULexVisualPostProcess::SetMaskTexture(UTexture2D* Value)
{
	if (MaskTexture != Value)
	{
		MaskTexture = Value;
		SendMaskTextureToRenderProxy();

		bLocalVertexPositionChanged = true;
		bUVChanged = true;
		bColorChanged = true;
		GetWidget()->MarkCanvasUpdate(true);
	}
}
void ULexVisualPostProcess::SetMaskTextureUVRect(const FVector4& Value)
{
	if (MaskTextureUVRect != Value)
	{
		MaskTextureUVRect = Value;

		bUVChanged = true;
		GetWidget()->MarkCanvasUpdate(false);
	}
}

void ULexVisualPostProcess::SetRenderType(ELexBackgroundBlurRenderType Value)
{
	if (RenderType != Value)
	{
		RenderType = Value;
		GetWidget()->MarkCanvasUpdate(false);
		SendRenderTargetToRenderProxy();
	}
}

void ULexVisualPostProcess::SetUseFullSize(bool Value)
{
	if (bUseFullSize != Value)
	{
		bUseFullSize = Value;
		MarkVertexPositionDirty();
	}
}

void ULexVisualPostProcess::SendMaskTextureToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = RenderProxy;
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

void ULexVisualPostProcess::SendRenderTargetToRenderProxy()
{
	if (RenderProxy)
	{
		auto TempRenderProxy = RenderProxy;
		FTextureRenderTargetResource* RenderTargetResource = nullptr;
		if (!bUseFullSize && RenderType == ELexBackgroundBlurRenderType::RenderTarget && IsValid(OutputRenderTarget))
		{
			RenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
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

bool ULexVisualPostProcess::HaveValidData()const
{
	return Geometry->Vertices.Num() > 0;
}

bool ULexVisualPostProcess::LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
{
	if (RaycastType == ELexVisualRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == ELexVisualRaycastType::Mesh)
	{
		return LineTraceUIGeometry(Geometry.Get(), OutHit, Start, End);
	}
	else
	{
		return LineTraceUICustom(OutHit, Start, End);
	}
}

void ULexVisualPostProcess::UpdateRenderTarget()
{
	if (RenderType != ELexBackgroundBlurRenderType::RenderTarget)return;
	auto Widget = GetWidget();
	FIntPoint DesiredRenderTargetSize(Widget->GetWidth(), Widget->GetHeight());
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X <= 0 || DesiredRenderTargetSize.Y <= 0)
	{
		return;
	}
	DesiredRenderTargetSize.X = FMath::Min(DesiredRenderTargetSize.X, MaxAllowedDrawSize);
	DesiredRenderTargetSize.Y = FMath::Min(DesiredRenderTargetSize.Y, MaxAllowedDrawSize);

	if (OutputRenderTarget == nullptr)
	{
		OutputRenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, EObjectFlags::RF_Transient);
		OutputRenderTarget->AddressX = TextureAddress::TA_Clamp;
		OutputRenderTarget->AddressY = TextureAddress::TA_Clamp;
		OutputRenderTarget->ClearColor = FLinearColor::Transparent;
		OutputRenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
		SendRenderTargetToRenderProxy();
		OnRenderTargetChanged.Broadcast(OutputRenderTarget);
	}
	else
	{
		if (OutputRenderTarget->SizeX != DesiredRenderTargetSize.X || OutputRenderTarget->SizeY != DesiredRenderTargetSize.Y)
		{
			OutputRenderTarget->ClearColor = FLinearColor::Transparent;
			OutputRenderTarget->InitCustomFormat(DesiredRenderTargetSize.X, DesiredRenderTargetSize.Y, EPixelFormat::PF_B8G8R8A8, false);
			OutputRenderTarget->UpdateResourceImmediate();
#if WITH_EDITOR
			OutputRenderTarget->Modify();
#endif
			SendRenderTargetToRenderProxy();
		}
	}

#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		if (!OutputRenderTarget->GameThread_GetRenderTargetResource())
		{
			OutputRenderTarget->InitCustomFormat(OutputRenderTarget->SizeX, OutputRenderTarget->SizeY, EPixelFormat::PF_B8G8R8A8, false);
			SendRenderTargetToRenderProxy();
		}
	}
#endif
}


