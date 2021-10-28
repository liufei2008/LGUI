﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIGeometry.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "Core/LGUISpriteData_BaseObject.h"

UUIPostProcessRenderable::UUIPostProcessRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIPostProcessRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void UUIPostProcessRenderable::BeginPlay()
{
	Super::BeginPlay();
	MarkCanvasUpdate();

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
	if (RenderCanvas.IsValid() && drawcall.IsValid())
	{
		RenderCanvas->RemoveUIRenderable(this);
	}
}

void UUIPostProcessRenderable::ApplyUIActiveState()
{
	bUVChanged = true;
	Super::ApplyUIActiveState();
}

void UUIPostProcessRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	Super::OnRenderCanvasChanged(OldCanvas, NewCanvas);
}
void UUIPostProcessRenderable::WidthChanged()
{
	MarkVertexPositionDirty();
}
void UUIPostProcessRenderable::HeightChanged()
{
	MarkVertexPositionDirty();
}
void UUIPostProcessRenderable::PivotChanged()
{
	MarkVertexPositionDirty();
}

void UUIPostProcessRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate();
}
void UUIPostProcessRenderable::MarkUVDirty()
{
	bUVChanged = true;
	MarkCanvasUpdate();
}

void UUIPostProcessRenderable::UpdateCachedData()
{
	cacheForThisUpdate_LocalVertexPositionChanged = bLocalVertexPositionChanged;
	cacheForThisUpdate_UVChanged = bUVChanged;
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	Super::UpdateCachedData();
}
void UUIPostProcessRenderable::UpdateCachedDataBeforeGeometry()
{
	if (bLocalVertexPositionChanged)cacheForThisUpdate_LocalVertexPositionChanged = true;
	if (bUVChanged)cacheForThisUpdate_UVChanged = true;
	Super::UpdateCachedDataBeforeGeometry();
}

void UUIPostProcessRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	Super::MarkAllDirtyRecursive();
}

void UUIPostProcessRenderable::CreateGeometry()
{
	geometry->Clear();
	OnCreateGeometry();
	UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
	UpdateRegionVertex();
}

DECLARE_CYCLE_STAT(TEXT("UIPostProcessRenderable UpdateRenderable"), STAT_UIPostProcessRenderableUpdate, STATGROUP_LGUI);
void UUIPostProcessRenderable::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UIPostProcessRenderableUpdate);
	if (GetIsUIActiveInHierarchy() == false)return;
	if (!CheckRenderCanvas())return;

	Super::UpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		CreateGeometry();
		RenderCanvas->AddUIRenderable(this);
		goto COMPLETE;
	}
	else//if already renderred, update data
	{
		//update geometry
		{
			OnUpdateGeometry(cacheForThisUpdate_LocalVertexPositionChanged, cacheForThisUpdate_UVChanged, cacheForThisUpdate_ColorChanged);

			if (cacheForThisUpdate_LocalVertexPositionChanged || cacheForThisUpdate_LayoutChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
			}
			UpdateRegionVertex();
		}
	}
COMPLETE:
	;
}
void UUIPostProcessRenderable::OnCreateGeometry()
{
	UIGeometry::FromUIRectSimple(widget.width, widget.height, widget.pivot, GetFinalColor(), geometry, FLGUISpriteInfo(), RenderCanvas.Get(), this);
}
void UUIPostProcessRenderable::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (InVertexPositionChanged)
	{
		UIGeometry::UpdateUIRectSimpleVertex(geometry, widget.width, widget.height, widget.pivot, FLGUISpriteInfo(), RenderCanvas.Get(), this);
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
void UUIPostProcessRenderable::UpdateRegionVertex()
{
	auto& vertices = geometry->vertices;
	if (vertices.Num() <= 0)return;
	if (renderScreenToMeshRegionVertexArray.Num() == 0)
	{
		//full screen vertex position
		renderScreenToMeshRegionVertexArray =
		{
			FLGUIPostProcessCopyMeshRegionVertex(FVector(-1, -1, 0), FVector(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector(1, -1, 0), FVector(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector(-1, 1, 0), FVector(0.0f, 0.0f, 0.0f)),
			FLGUIPostProcessCopyMeshRegionVertex(FVector(1, 1, 0), FVector(0.0f, 0.0f, 0.0f))
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
			FUIWidget widget;
			FMatrix objectToWorldMatrix;
		};
		auto updateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		updateData->renderMeshRegionToScreenVertexArray = this->renderMeshRegionToScreenVertexArray;
		updateData->renderScreenToMeshRegionVertexArray = this->renderScreenToMeshRegionVertexArray;
		updateData->widget = this->widget;
		updateData->objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateData)
			([TempRenderProxy, updateData](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->renderScreenToMeshRegionVertexArray = updateData->renderScreenToMeshRegionVertexArray;
					TempRenderProxy->renderMeshRegionToScreenVertexArray = updateData->renderMeshRegionToScreenVertexArray;
					TempRenderProxy->widget = updateData->widget;
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
		if (IsValid(this->maskTexture) && this->maskTexture->Resource != nullptr)
		{
			maskTextureResource = (FTexture2DResource*)this->maskTexture->Resource;
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
					TempRenderProxy->rectClipOffsetAndSize = rectClipOffsetAndSize;
					TempRenderProxy->rectClipFeather = rectClipFeather;
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
		if (IsValid(ClipTex) && ClipTex->Resource != nullptr)
		{
			clipTextureResource = (FTexture2DResource*)ClipTex->Resource;
		}
		ENQUEUE_RENDER_COMMAND(FUIPostProcess_UpdateClipData)
			([TempRenderProxy, textureClipOffsetAndSize, clipTextureResource](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->clipTexture = clipTextureResource;
					TempRenderProxy->textureClipOffsetAndSize = textureClipOffsetAndSize;
				});
	}
}
