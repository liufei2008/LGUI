// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIGeometry.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "Core/LGUISpriteData_BaseObject.h"

UUIPostProcessRenderable::UUIPostProcessRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	UIRenderableType = EUIRenderableType::UIPostProcessRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void UUIPostProcessRenderable::BeginPlay()
{
	Super::BeginPlay();

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void UUIPostProcessRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}


#if WITH_EDITOR
void UUIPostProcessRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIPostProcessRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIPostProcessRenderable::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
    if (InPivotChange || InSizeChange)
    {
	    MarkVertexPositionDirty();
    }
}

void UUIPostProcessRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate(false, true, false);
}
void UUIPostProcessRenderable::MarkUVDirty()
{
	bUVChanged = true;
	MarkCanvasUpdate(false, false, false);
}

void UUIPostProcessRenderable::MarkAllDirty()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	Super::MarkAllDirty();
}

DECLARE_CYCLE_STAT(TEXT("UIPostProcessRenderable UpdateRenderable"), STAT_UIPostProcessRenderableUpdate, STATGROUP_LGUI);
void UUIPostProcessRenderable::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UIPostProcessRenderableUpdate);
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!RenderCanvas.IsValid())return;

	Super::UpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		geometry->Clear();
		OnUpdateGeometry(geometry.Get(), true, true, true, true);
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
		UpdateRegionVertex();
	}
	else//if geometry is created, update data
	{
		if (bLocalVertexPositionChanged || bUVChanged || bColorChanged)
		{
			geometry->Clear();
			OnUpdateGeometry(geometry.Get(), false, bLocalVertexPositionChanged, bUVChanged, bColorChanged);
		}
		if (bLocalVertexPositionChanged || bTransformChanged)
		{
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
		}
		if (bLocalVertexPositionChanged || bUVChanged || bColorChanged || bTransformChanged)
		{
			UpdateRegionVertex();
		}
	}

	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
}
void UUIPostProcessRenderable::OnUpdateGeometry(UIGeometry* InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	UIGeometry::UpdateUIRectSimpleVertex(InGeo, this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), FLGUISpriteInfo(), RenderCanvas.Get(), this, this->GetFinalColor(), 
		InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
}
void UUIPostProcessRenderable::UpdateRegionVertex()
{
	auto& vertices = geometry->vertices;
	if (vertices.Num() <= 0)return;
	if (renderScreenToMeshRegionVertexArray.Num() == 0)
	{
		//full screen vertex position
		renderScreenToMeshRegionVertexArray =
		{
			FLGUIPostProcessCopyMeshRegionVertex(FVector3f(-1, -1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector3f(1, -1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector3f(-1, 1, 0), FVector3f(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector3f(1, 1, 0), FVector3f(0.0f, 0.0f, 0.0f))
		};
	}
	if (renderMeshRegionToScreenVertexArray.Num() == 0)
	{
		renderMeshRegionToScreenVertexArray.AddUninitialized(4);
	}

	{
		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = renderScreenToMeshRegionVertexArray[i];
			copyVert.LocalPosition = vertices[i].Position;
		}

		for (int i = 0; i < 4; i++)
		{
			auto& copyVert = renderMeshRegionToScreenVertexArray[i];
			copyVert.Position = vertices[i].Position;
			copyVert.TextureCoordinate0 = vertices[i].TextureCoordinate[0];
		}
	}
	SendRegionVertexDataToRenderProxy();
}

void UUIPostProcessRenderable::SendRegionVertexDataToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = RenderProxy.Get();
		struct FUIPostProcess_SendRegionVertexDataToRenderProxy
		{
			TArray<FLGUIPostProcessCopyMeshRegionVertex> renderScreenToMeshRegionVertexArray;
			TArray<FLGUIPostProcessVertex> renderMeshRegionToScreenVertexArray;
			FVector2f RectSize;
			FMatrix44f objectToWorldMatrix;
		};
		auto updateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		updateData->renderMeshRegionToScreenVertexArray = this->renderMeshRegionToScreenVertexArray;
		updateData->renderScreenToMeshRegionVertexArray = this->renderScreenToMeshRegionVertexArray;
		updateData->RectSize = FVector2f(this->GetWidth(), this->GetHeight());
		updateData->objectToWorldMatrix = FMatrix44f(this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale());
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateData)
			([TempRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->renderScreenToMeshRegionVertexArray = updateData->renderScreenToMeshRegionVertexArray;
					TempRenderProxy->renderMeshRegionToScreenVertexArray = updateData->renderMeshRegionToScreenVertexArray;
					TempRenderProxy->RectSize = updateData->RectSize;
					TempRenderProxy->objectToWorldMatrix = updateData->objectToWorldMatrix;
					delete updateData;
				});
	}
}

void UUIPostProcessRenderable::SetMaskTexture(UTexture2D* newValue)
{
	if (maskTexture != newValue)
	{
		maskTexture = newValue;
		SendMaskTextureToRenderProxy();
	}
}
void UUIPostProcessRenderable::SendMaskTextureToRenderProxy()
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = RenderProxy.Get();
		FTexture2DResource* maskTextureResource = nullptr;
		if (IsValid(this->maskTexture) && this->maskTexture->GetResource() != nullptr)
		{
			maskTextureResource = (FTexture2DResource*)this->maskTexture->GetResource();
		}
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateMaskTexture)
			([TempRenderProxy, maskTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->maskTexture = maskTextureResource;
				});
	}
}

bool UUIPostProcessRenderable::IsRenderProxyValid()const
{
	return RenderProxy.IsValid();
}
void UUIPostProcessRenderable::SetClipType(ELGUICanvasClipType clipType)
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = RenderProxy.Get();
		auto tempClipType = clipType;
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateClipData)
			([TempRenderProxy, tempClipType](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->clipType = tempClipType;
				});
	}
}
void UUIPostProcessRenderable::SetRectClipParameter(const FVector4& OffsetAndSize, const FVector4& Feather)
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = RenderProxy.Get();
		auto rectClipOffsetAndSize = OffsetAndSize;
		auto rectClipFeather = Feather;
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateClipData)
			([TempRenderProxy, rectClipOffsetAndSize, rectClipFeather](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->rectClipOffsetAndSize = FVector4f(rectClipOffsetAndSize);
					TempRenderProxy->rectClipFeather = FVector4f(rectClipFeather);
				});
	}
}
void UUIPostProcessRenderable::SetTextureClipParameter(UTexture* ClipTex, const FVector4& OffsetAndSize)
{
	if (RenderProxy.IsValid())
	{
		auto TempRenderProxy = RenderProxy.Get();
		FTexture2DResource* clipTextureResource = nullptr;
		auto textureClipOffsetAndSize = OffsetAndSize;
		if (IsValid(ClipTex) && ClipTex->GetResource() != nullptr)
		{
			clipTextureResource = (FTexture2DResource*)ClipTex->GetResource();
		}
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateClipData)
			([TempRenderProxy, textureClipOffsetAndSize, clipTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->clipTexture = clipTextureResource;
					TempRenderProxy->textureClipOffsetAndSize = FVector4f(textureClipOffsetAndSize);
				});
	}
}

bool UUIPostProcessRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (RaycastType == EUIRenderableRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == EUIRenderableRaycastType::Geometry)
	{
		return LineTraceUIGeometry(geometry.Get(), OutHit, Start, End);
	}
	else
	{
		return LineTraceUICustom(OutHit, Start, End);
	}
}
