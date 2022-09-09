// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/UIGeometry.h"
#include "Core/UIPostProcessRenderProxy.h"

UUIPostProcessRenderable::UUIPostProcessRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	UIRenderableType = EUIRenderableType::UIPostProcessRenderable;
	geometry_Simple = TSharedPtr<UIGeometry>(new UIGeometry);
	geometry_Sliced = TSharedPtr<UIGeometry>(new UIGeometry);

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

	CheckSpriteData();
	SendMaskTextureToRenderProxy();
}
bool UUIPostProcessRenderable::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUIPostProcessRenderable, MaskTextureType))
		{
			return IsValid(maskTexture);
		}
		else if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUIPostProcessRenderable, MaskTextureSpriteInfo))
		{
			return IsValid(maskTexture) && MaskTextureType == EUIPostProcessMaskTextureType::Sliced;
		}
	}
	return Super::CanEditChange(InProperty);
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
		geometry_Simple->Clear();
		geometry_Sliced->Clear();
		OnUpdateGeometry(true, true, true, true);
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry_Simple.Get());
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry_Sliced.Get());

		UpdateRegionVertex();
	}
	else//if geometry is created, update data
	{
		if (bLocalVertexPositionChanged || bUVChanged || bColorChanged)
		{
			geometry_Simple->Clear();
			geometry_Sliced->Clear();
			OnUpdateGeometry(false, bLocalVertexPositionChanged, bUVChanged, bColorChanged);
		}
		if (bLocalVertexPositionChanged || bTransformChanged)
		{
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry_Simple.Get());
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry_Sliced.Get());
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
void UUIPostProcessRenderable::OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	//simple rect geometry for render from screen image to mesh region and inverse
	{
		auto& vertices = geometry_Simple->vertices;
		auto& originVertices = geometry_Simple->originVertices;
		UIGeometry::LGUIGeometrySetArrayNum(vertices, 4);
		UIGeometry::LGUIGeometrySetArrayNum(originVertices, 4);
		if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
		{
			if (InVertexPositionChanged)
			{
				//offset and size
				float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
				UIGeometry::CalculateOffsetAndSize(this->GetWidth(), this->GetHeight(), this->GetPivot(), FLGUISpriteInfo(), pivotOffsetX, pivotOffsetY, halfW, halfH);
				//positions
				float minX = -halfW + pivotOffsetX;
				float minY = -halfH + pivotOffsetY;
				float maxX = halfW + pivotOffsetX;
				float maxY = halfH + pivotOffsetY;
				originVertices[0].Position = FVector(0, minX, minY);
				originVertices[1].Position = FVector(0, maxX, minY);
				originVertices[2].Position = FVector(0, minX, maxY);
				originVertices[3].Position = FVector(0, maxX, maxY);
				//snap pixel
				if (RenderCanvas->GetActualPixelPerfect())
				{
					UIGeometry::AdjustPixelPerfectPos(originVertices, 0, 4, RenderCanvas.Get(), this);
				}
			}

			if (InVertexUVChanged)
			{
				vertices[0].TextureCoordinate[0] = MaskTextureSpriteInfo.GetUV0();
				vertices[1].TextureCoordinate[0] = MaskTextureSpriteInfo.GetUV1();
				vertices[2].TextureCoordinate[0] = MaskTextureSpriteInfo.GetUV2();
				vertices[3].TextureCoordinate[0] = MaskTextureSpriteInfo.GetUV3();

				vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2D(1, 1);
				vertices[2].TextureCoordinate[1] = FVector2D(0, 0);
				vertices[3].TextureCoordinate[1] = FVector2D(1, 0);
			}

			if (InVertexColorChanged)
			{
				UIGeometry::UpdateUIColor(geometry_Simple.Get(), GetFinalColor());
			}
		}
	}

	auto TempMaskTextureType = EUIPostProcessMaskTextureType::Simple;
	if (IsValid(maskTexture) && MaskTextureType == EUIPostProcessMaskTextureType::Sliced)
	{
		TempMaskTextureType = EUIPostProcessMaskTextureType::Sliced;
	}
	//sliced geometry
	if (TempMaskTextureType == EUIPostProcessMaskTextureType::Sliced)
	{
		auto& vertices = geometry_Sliced->vertices;
		auto& originVertices = geometry_Sliced->originVertices;
		auto verticesCount = 16;
		UIGeometry::LGUIGeometrySetArrayNum(vertices, verticesCount);
		UIGeometry::LGUIGeometrySetArrayNum(originVertices, verticesCount);
		if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
		{
			if (InVertexPositionChanged)
			{
				//pivot offset
				float pivotOffsetX = 0, pivotOffsetY = 0, halfW = 0, halfH = 0;
				UIGeometry::CalculateOffsetAndSize(this->GetWidth(), this->GetHeight(), this->GetPivot(), MaskTextureSpriteInfo, pivotOffsetX, pivotOffsetY, halfW, halfH);
				float geoWidth = halfW * 2;
				float geoHeight = halfH * 2;
				//vertices
				float x0, x1, x2, x3, y0, y1, y2, y3;
				int widthBorder = MaskTextureSpriteInfo.borderLeft + MaskTextureSpriteInfo.borderRight;
				int heightBorder = MaskTextureSpriteInfo.borderTop + MaskTextureSpriteInfo.borderBottom;
				float widthScale = geoWidth < widthBorder ? geoWidth / widthBorder : 1.0f;
				float heightScale = geoHeight < heightBorder ? geoHeight / heightBorder : 1.0f;
				x0 = (-halfW + pivotOffsetX);
				x1 = (x0 + MaskTextureSpriteInfo.borderLeft * widthScale);
				x3 = (halfW + pivotOffsetX);
				x2 = (x3 - MaskTextureSpriteInfo.borderRight * widthScale);
				y0 = (-halfH + pivotOffsetY);
				y1 = (y0 + MaskTextureSpriteInfo.borderBottom * heightScale);
				y3 = (halfH + pivotOffsetY);
				y2 = (y3 - MaskTextureSpriteInfo.borderTop * heightScale);

				originVertices[0].Position = FVector(0, x0, y0);
				originVertices[1].Position = FVector(0, x1, y0);
				originVertices[2].Position = FVector(0, x2, y0);
				originVertices[3].Position = FVector(0, x3, y0);

				originVertices[4].Position = FVector(0, x0, y1);
				originVertices[5].Position = FVector(0, x1, y1);
				originVertices[6].Position = FVector(0, x2, y1);
				originVertices[7].Position = FVector(0, x3, y1);

				originVertices[8].Position = FVector(0, x0, y2);
				originVertices[9].Position = FVector(0, x1, y2);
				originVertices[10].Position = FVector(0, x2, y2);
				originVertices[11].Position = FVector(0, x3, y2);

				originVertices[12].Position = FVector(0, x0, y3);
				originVertices[13].Position = FVector(0, x1, y3);
				originVertices[14].Position = FVector(0, x2, y3);
				originVertices[15].Position = FVector(0, x3, y3);

				//snap pixel
				if (RenderCanvas->GetActualPixelPerfect())
				{
					UIGeometry::AdjustPixelPerfectPos(originVertices, 0, verticesCount, RenderCanvas.Get(), this);
				}
			}

			if (InVertexUVChanged)
			{
				//uv0
				vertices[0].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv0X, MaskTextureSpriteInfo.uv0Y);
				vertices[1].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv0X, MaskTextureSpriteInfo.uv0Y);
				vertices[2].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv3X, MaskTextureSpriteInfo.uv0Y);
				vertices[3].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv3X, MaskTextureSpriteInfo.uv0Y);

				vertices[4].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv0X, MaskTextureSpriteInfo.buv0Y);
				vertices[5].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv0X, MaskTextureSpriteInfo.buv0Y);
				vertices[6].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv3X, MaskTextureSpriteInfo.buv0Y);
				vertices[7].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv3X, MaskTextureSpriteInfo.buv0Y);

				vertices[8].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv0X, MaskTextureSpriteInfo.buv3Y);
				vertices[9].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv0X, MaskTextureSpriteInfo.buv3Y);
				vertices[10].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv3X, MaskTextureSpriteInfo.buv3Y);
				vertices[11].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv3X, MaskTextureSpriteInfo.buv3Y);

				vertices[12].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv0X, MaskTextureSpriteInfo.uv3Y);
				vertices[13].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv0X, MaskTextureSpriteInfo.uv3Y);
				vertices[14].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.buv3X, MaskTextureSpriteInfo.uv3Y);
				vertices[15].TextureCoordinate[0] = FVector2D(MaskTextureSpriteInfo.uv3X, MaskTextureSpriteInfo.uv3Y);

				//uv1
				float widthReciprocal = 1.0f / this->GetWidth();
				float heightReciprocal = 1.0f / this->GetHeight();
				float buv0X = MaskTextureSpriteInfo.borderLeft * widthReciprocal;
				float buv3X = 1.0f - MaskTextureSpriteInfo.borderRight * widthReciprocal;
				float buv0Y = 1.0f - MaskTextureSpriteInfo.borderBottom * heightReciprocal;
				float buv3Y = MaskTextureSpriteInfo.borderTop * heightReciprocal;

				vertices[0].TextureCoordinate[1] = FVector2D(0, 1);
				vertices[1].TextureCoordinate[1] = FVector2D(buv0X, 1);
				vertices[2].TextureCoordinate[1] = FVector2D(buv3X, 1);
				vertices[3].TextureCoordinate[1] = FVector2D(1, 1);

				vertices[4].TextureCoordinate[1] = FVector2D(0, buv0Y);
				vertices[5].TextureCoordinate[1] = FVector2D(buv0X, buv0Y);
				vertices[6].TextureCoordinate[1] = FVector2D(buv3X, buv0Y);
				vertices[7].TextureCoordinate[1] = FVector2D(1, buv0Y);

				vertices[8].TextureCoordinate[1] = FVector2D(0, buv3Y);
				vertices[9].TextureCoordinate[1] = FVector2D(buv0X, buv3Y);
				vertices[10].TextureCoordinate[1] = FVector2D(buv3X, buv3Y);
				vertices[11].TextureCoordinate[1] = FVector2D(1, buv3Y);

				vertices[12].TextureCoordinate[1] = FVector2D(0, 0);
				vertices[13].TextureCoordinate[1] = FVector2D(buv0X, 0);
				vertices[14].TextureCoordinate[1] = FVector2D(buv3X, 0);
				vertices[15].TextureCoordinate[1] = FVector2D(1, 0);
			}

			if (InVertexColorChanged)
			{
				UIGeometry::UpdateUIColor(geometry_Sliced.Get(), this->GetFinalColor());
			}
		}
	}
}
void UUIPostProcessRenderable::UpdateRegionVertex()
{
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

	auto& vertices_Simple = geometry_Simple->vertices;
	for (int i = 0; i < 4; i++)
	{
		auto& copyVert = renderScreenToMeshRegionVertexArray[i];
		copyVert.LocalPosition = vertices_Simple[i].Position;
	}

	auto TempMaskTextureType = EUIPostProcessMaskTextureType::Simple;
	if (IsValid(maskTexture) && MaskTextureType == EUIPostProcessMaskTextureType::Sliced)
	{
		TempMaskTextureType = EUIPostProcessMaskTextureType::Sliced;
	}
	switch (TempMaskTextureType)
	{
	default:
	case EUIPostProcessMaskTextureType::Simple:
	{
		const int VertexBufferSize = 4;
		if (renderMeshRegionToScreenVertexArray.Num() != VertexBufferSize)
		{
			renderMeshRegionToScreenVertexArray.SetNumZeroed(VertexBufferSize);
		}

		for (int i = 0; i < VertexBufferSize; i++)
		{
			auto& copyVert = renderMeshRegionToScreenVertexArray[i];
			copyVert.Position = vertices_Simple[i].Position;
			copyVert.TextureCoordinate0 = vertices_Simple[i].TextureCoordinate[1];
			copyVert.TextureCoordinate1 = vertices_Simple[i].TextureCoordinate[0];
		}
	}
	break;
	case EUIPostProcessMaskTextureType::Sliced:
	{
		auto& vertices_Sliced = geometry_Sliced->vertices;
		const int VertexBufferSize = 16;
		if (renderMeshRegionToScreenVertexArray.Num() != VertexBufferSize)
		{
			renderMeshRegionToScreenVertexArray.SetNumZeroed(VertexBufferSize);
		}

		for (int i = 0; i < VertexBufferSize; i++)
		{
			auto& copyVert = renderMeshRegionToScreenVertexArray[i];
			copyVert.Position = vertices_Sliced[i].Position;
			copyVert.TextureCoordinate0 = vertices_Sliced[i].TextureCoordinate[1];
			copyVert.TextureCoordinate1 = vertices_Sliced[i].TextureCoordinate[0];
		}
	}
	break;
	}

	SendRegionVertexDataToRenderProxy();
}

void UUIPostProcessRenderable::CheckSpriteData()
{
	if (IsValid(maskTexture))
	{
		MaskTextureSpriteInfo.width = maskTexture->GetSurfaceWidth();
		MaskTextureSpriteInfo.height = maskTexture->GetSurfaceHeight();
		MaskTextureSpriteInfo.ApplyUV(0, 0, MaskTextureSpriteInfo.width, MaskTextureSpriteInfo.height, 1.0f / MaskTextureSpriteInfo.width, 1.0f / MaskTextureSpriteInfo.height, MaskTextureUVRect);
		MaskTextureSpriteInfo.ApplyBorderUV(1.0f / MaskTextureSpriteInfo.width, 1.0f / MaskTextureSpriteInfo.height);
	}
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
			FVector2D RectSize;
			FMatrix objectToWorldMatrix;
		};
		auto updateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		updateData->renderMeshRegionToScreenVertexArray = this->renderMeshRegionToScreenVertexArray;
		updateData->renderScreenToMeshRegionVertexArray = this->renderScreenToMeshRegionVertexArray;
		updateData->RectSize = FVector2D(this->GetWidth(), this->GetHeight());
		updateData->objectToWorldMatrix = this->RenderCanvas->GetUIItem()->GetComponentTransform().ToMatrixWithScale();
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

		bLocalVertexPositionChanged = true;
		bUVChanged = true;
		bColorChanged = true;
		MarkCanvasUpdate(false, true, false);
	}
}
void UUIPostProcessRenderable::SetMaskTextureType(EUIPostProcessMaskTextureType value)
{
	if (MaskTextureType != value)
	{
		MaskTextureType = value;
		SendMaskTextureToRenderProxy();

		bLocalVertexPositionChanged = true;
		bUVChanged = true;
		bColorChanged = true;
		MarkCanvasUpdate(false, true, false);
	}
}
void UUIPostProcessRenderable::SetMaskTextureSpriteInfo(const FLGUISpriteInfo& value)
{
	if (MaskTextureSpriteInfo != value)
	{
		MaskTextureSpriteInfo = value;

		bLocalVertexPositionChanged = true;
		bUVChanged = true;
		bColorChanged = true;
		MarkCanvasUpdate(false, true, false);
	}
}
void UUIPostProcessRenderable::SetMaskTextureUVRect(const FVector4& value)
{
	if (MaskTextureUVRect != value)
	{
		MaskTextureUVRect = value;

		bUVChanged = true;
		CheckSpriteData();
		MarkCanvasUpdate(false, false, false);
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
			([TempRenderProxy, maskTextureResource, tempMaskTextureType = MaskTextureType](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->maskTexture = maskTextureResource;
					TempRenderProxy->MaskTextureType = tempMaskTextureType;
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

bool UUIPostProcessRenderable::HaveValidData()const
{
	return geometry_Simple->vertices.Num() > 0 || geometry_Sliced->vertices.Num() > 0;
}

bool UUIPostProcessRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (RaycastType == EUIRenderableRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == EUIRenderableRaycastType::Geometry)
	{
		return LineTraceUIGeometry(geometry_Simple.Get(), OutHit, Start, End);
	}
	else
	{
		return LineTraceUICustom(OutHit, Start, End);
	}
}
