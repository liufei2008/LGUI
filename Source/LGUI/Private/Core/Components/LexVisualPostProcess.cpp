// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexVisualPostProcess.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexUIManager.h"
#include "Core/LexVisualPostProcessRenderProxy.h"
#include "Core/Components/LexWidget.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"


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

void ULexVisualPostProcess::OnRegister()
{
	Super::OnRegister();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->GetOnPostUpdateDrawCall().AddUObject(this, &ULexVisualPostProcess::PostUpdateDrawCall);
	}
}

void ULexVisualPostProcess::OnUnregister()
{
	Super::OnUnregister();
	OnRenderTargetChanged.Broadcast(nullptr);
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->GetOnPostUpdateDrawCall().RemoveAll(this);
	}
}

#if WITH_EDITOR
void ULexVisualPostProcess::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (RenderType != ELexVisualPostProcessRenderType::RenderTarget)
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
}
void ULexVisualPostProcess::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
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
		bWidgetOrGeometryDirty = true;
	}
	if (RenderScreenToMeshRegionVertexArray.Num() == 0)
	{
		//full-screen vertex position
		RenderScreenToMeshRegionVertexArray =
		{
			FLexUIPostProcessCopyMeshRegionVertex(FVector2f(-1, -1), FVector2f(0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector2f(1, -1), FVector2f(0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector2f(-1, 1), FVector2f(0.0f, 0.0f)),
			FLexUIPostProcessCopyMeshRegionVertex(FVector2f(1, 1), FVector2f(0.0f, 0.0f))
		};
	}
	if (RenderMeshRegionToScreenVertexArray.Num() == 0)
	{
		RenderMeshRegionToScreenVertexArray.SetNumZeroed(4);
	}

	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
}

void ULexVisualPostProcess::PostUpdateDrawCall()
{
	auto Widget = GetWidget();
	auto RenderCanvas = Widget->GetRenderCanvas();

	FIntRect ViewRect(FIntPoint::ZeroValue, FIntPoint::ZeroValue);
	FMatrix ViewProjectionMatrix = FMatrix::Identity;
	
	auto RootCanvas = RenderCanvas->GetRootCanvas();
	if (RenderCanvas->IsRenderToWorldSpace())
	{
#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())
		{
			if (auto EditorViewportClient = ULexUIManagerWorldSubsystem::GetInstance(GetWorld())->GetEditorViewportClient())
			{
				FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(EditorViewportClient->Viewport, EditorViewportClient->GetScene(), EditorViewportClient->EngineShowFlags));
				auto SceneView = EditorViewportClient->CalcSceneView(&ViewFamily);
				ViewRect = SceneView->UnscaledViewRect;
				ViewProjectionMatrix = SceneView->ViewMatrices.GetViewProjectionMatrix();
			}
		}
		else
#endif
		{
			auto Player = UGameplayStatics::GetPlayerController(this, 0);
			ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
			if (LP && LP->ViewportClient)
			{
				// get the projection data
				FSceneViewProjectionData ProjectionData;
				if (LP->GetProjectionData(LP->ViewportClient->Viewport, /*out*/ ProjectionData))
				{
					ViewRect = ProjectionData.GetConstrainedViewRect();
					ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
				}
			}
		}
	}
	else
	{
		ViewRect = FIntRect(FIntPoint::ZeroValue, RootCanvas->GetViewportSize());
		ViewProjectionMatrix = RootCanvas->GetViewProjectionMatrix();
	}
	if (bWidgetOrGeometryDirty
		|| CacheViewRect != ViewRect
		|| CacheViewProjectionMatrix != ViewProjectionMatrix
		&& (ViewRect.Max != ViewRect.Min && ViewProjectionMatrix != FMatrix::Identity)//make sure view is valid
		)
	{
		bWidgetOrGeometryDirty = false;
		CacheViewRect = ViewRect;
		CacheViewProjectionMatrix = ViewProjectionMatrix;
	}
	else
	{
		return;//nothing change, no need to calculate
	}

	//calculate screen space AABB rect
	FVector2f Min = FVector2f(UE_MAX_FLT, UE_MAX_FLT);
	FVector2f Max = -Min;
	if (RenderCanvas->IsRenderToWorldSpace())
	{
		auto ProjectWorldToScreen = [](const FVector& WorldPosition, const FIntRect& ViewRect, const FMatrix& ViewProjectionMatrix, FVector2D& out_ScreenPos)
		{
			FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
			bool bIsInsideView = Result.W > 0.0f;
			double W = Result.W;
	
			// the result of this will be x and y coords in -1..1 projection space
			const float RHW = 1.0f / W;
			FPlane PosInScreenSpace = FPlane(Result.X * RHW, Result.Y * RHW, Result.Z * RHW, W);

			// Move from projection space to normalized 0..1 UI space
			const float NormalizedX = ( PosInScreenSpace.X / 2.f ) + 0.5f;
			const float NormalizedY = 1.f - ( PosInScreenSpace.Y / 2.f ) - 0.5f;

			FVector2D RayStartViewRectSpace(
				( NormalizedX * (float)ViewRect.Width() ),
				( NormalizedY * (float)ViewRect.Height() )
				);

			out_ScreenPos = RayStartViewRectSpace + FVector2D(static_cast<float>(ViewRect.Min.X), static_cast<float>(ViewRect.Min.Y));

			return bIsInsideView;
		};
		
		auto ModelMatrix = Widget->GetWorldTransform().ToMatrixWithScale();
		const bool bPlayerViewportRelative = true;
		auto& Vertices = Geometry->OriginVertices;
		for (int i = 0; i < Vertices.Num(); i++)
		{
			auto& Vert = Vertices[i];
			auto WorldPosition = ModelMatrix.TransformPosition(FVector(Vert.Position));
			FVector2D ScreenPosition = FVector2D::Zero();
			bool bResult = ProjectWorldToScreen(WorldPosition, CacheViewRect, CacheViewProjectionMatrix, ScreenPosition);
			if (bResult)
			{
				if (bPlayerViewportRelative)
				{
					ScreenPosition -= FVector2D(CacheViewRect.Min);
				}
				Min.X = FMath::Min(Min.X, ScreenPosition.X);
				Min.Y = FMath::Min(Min.Y, ScreenPosition.Y);
				Max.X = FMath::Max(Max.X, ScreenPosition.X);
				Max.Y = FMath::Max(Max.Y, ScreenPosition.Y);
			}
		}
	}
	else
	{
		auto ModelMatrix = Widget->GetWorldTransform().ToMatrixWithScale();
		auto& Vertices = Geometry->OriginVertices;
		for (int i = 0; i < Vertices.Num(); i++)
		{
			auto& Vert = Vertices[i];
			auto WorldPosition = ModelMatrix.TransformPosition(FVector(Vert.Position));
			FVector2D ScreenPosition = FVector2D::Zero();
			if (RootCanvas->Project3DToScreen(WorldPosition, ScreenPosition))
			{
				Min.X = FMath::Min(Min.X, ScreenPosition.X);
				Min.Y = FMath::Min(Min.Y, ScreenPosition.Y);
				Max.X = FMath::Max(Max.X, ScreenPosition.X);
				Max.Y = FMath::Max(Max.Y, ScreenPosition.Y);
			}
		}
	}
	//clamp to viewport rect
	Min.X = FMath::Max(Min.X, 0);
	Min.Y = FMath::Max(Min.Y, 0);
	Max.X = FMath::Min(Max.X, ViewRect.Width());
	Max.Y = FMath::Min(Max.Y, ViewRect.Height());
	
	MeshRectInScreen = FBox2f(Min, Max);
	float Inv_ViewWidth = 1.0f / ViewRect.Width();
	float Inv_ViewHeight = 1.0f / ViewRect.Height();
	RectInScreen01 = FVector4f(Min.X * Inv_ViewWidth, Min.Y * Inv_ViewHeight, (Max.X - Min.X) * Inv_ViewWidth, (Max.Y - Min.Y) * Inv_ViewHeight);
	
	UpdateRenderTarget();
	UpdateRegionVertex(ViewRect.Size());
	
	for (auto& MID : MaterialsUsingThisBackBuffer)
	{
		if (!MID.IsValid())continue;
		MID->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
	}
}

void ULexVisualPostProcess::OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	//simple rect geometry for render from screen image to mesh region and inverse
	auto& Vertices = Geometry->Vertices;
	auto& OriginVertices = Geometry->OriginVertices;
	FLexUIGeometry::LexUIGeometrySetArrayNum(Vertices, 4);
	FLexUIGeometry::LexUIGeometrySetArrayNum(OriginVertices, 4);
	if (InVertexUVChanged || InVertexPositionChanged || InVertexColorChanged)
	{
		if (InVertexPositionChanged)
		{
			auto Widget = this->GetWidget();
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
				FLexUIGeometry::AdjustPixelSnappingPosition(OriginVertices, 0, 4, Widget->GetRenderCanvas(), this);
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

void ULexVisualPostProcess::UpdateRegionVertex(FIntPoint InViewportSize)
{
	FVector2f Inv_ViewportSize(1.0f / InViewportSize.X, 1.0f / InViewportSize.Y);
	RenderScreenToMeshRegionVertexArray[0].TextureCoordinate = FVector2f(MeshRectInScreen.Min.X, MeshRectInScreen.Max.Y) * Inv_ViewportSize;
	RenderScreenToMeshRegionVertexArray[1].TextureCoordinate = FVector2f(MeshRectInScreen.Max.X, MeshRectInScreen.Max.Y) * Inv_ViewportSize;
	RenderScreenToMeshRegionVertexArray[2].TextureCoordinate = FVector2f(MeshRectInScreen.Min.X, MeshRectInScreen.Min.Y) * Inv_ViewportSize;
	RenderScreenToMeshRegionVertexArray[3].TextureCoordinate = FVector2f(MeshRectInScreen.Max.X, MeshRectInScreen.Min.Y) * Inv_ViewportSize;

	auto& Vertices = Geometry->Vertices;
	constexpr int VertexBufferSize = 4;
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
	auto Widget = this->GetWidget();
	if (!Widget)return;

	auto RenderCanvas = Widget->GetRenderCanvas();
	if (RenderProxy && RenderCanvas)
	{
		auto TempRenderProxy = RenderProxy;
		struct FUIPostProcess_SendRegionVertexDataToRenderProxy
		{
			TArray<FLexUIPostProcessCopyMeshRegionVertex, TFixedAllocator<4>> RenderScreenToMeshRegionVertexArray;
			TArray<FLexUIPostProcessVertex, TFixedAllocator<4>> RenderMeshRegionToScreenVertexArray;
			FBox2f MeshRectInScreen;
			FVector4f RectInScreen01;
			FMatrix44f ObjectToWorldMatrix;
			FTexture2DDynamicResource* ClipDataTexture = nullptr;
		};
		auto UpdateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		UpdateData->RenderMeshRegionToScreenVertexArray = this->RenderMeshRegionToScreenVertexArray;
		UpdateData->RenderScreenToMeshRegionVertexArray = this->RenderScreenToMeshRegionVertexArray;
		UpdateData->MeshRectInScreen = this->MeshRectInScreen;
		UpdateData->RectInScreen01 = this->RectInScreen01;
		UpdateData->ObjectToWorldMatrix = FMatrix44f(RenderCanvas->GetWidget()->GetWorldTransform().ToMatrixWithScale());
		auto ClipDataTex = this->GetClipDataTexture();
		if (IsValid(ClipDataTex) && ClipDataTex->GetResource() != nullptr)
		{
			UpdateData->ClipDataTexture = (FTexture2DDynamicResource*)ClipDataTex->GetResource();
		}
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateData)
			([TempRenderProxy, UpdateData](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->RenderScreenToMeshRegionVertexArray = MoveTemp(UpdateData->RenderScreenToMeshRegionVertexArray);
					TempRenderProxy->RenderMeshRegionToScreenVertexArray = MoveTemp(UpdateData->RenderMeshRegionToScreenVertexArray);
					TempRenderProxy->MeshRectInScreen = MoveTemp(UpdateData->MeshRectInScreen);
					TempRenderProxy->RectInScreen01 = MoveTemp(UpdateData->RectInScreen01);
					TempRenderProxy->ObjectToWorldMatrix = MoveTemp(UpdateData->ObjectToWorldMatrix);
					TempRenderProxy->ClipDataTexture = UpdateData->ClipDataTexture;
					delete UpdateData;
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

void ULexVisualPostProcess::SetRenderType(ELexVisualPostProcessRenderType Value)
{
	if (RenderType != Value)
	{
		RenderType = Value;
		GetWidget()->MarkCanvasUpdate(false);
		SendRenderTargetToRenderProxy();
	}
}

void ULexVisualPostProcess::ClearMaterialsUsingThisBackBuffer()
{
	MaterialsUsingThisBackBuffer.Reset();
}

void ULexVisualPostProcess::RegisterMaterialsUsingThisBackBuffer(UMaterialInstanceDynamic* InMaterialInstanceDynamic)
{
	MaterialsUsingThisBackBuffer.Add(InMaterialInstanceDynamic);
	InMaterialInstanceDynamic->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, OutputRenderTarget);
	InMaterialInstanceDynamic->SetVectorParameterValue(ULexCanvas::LexUI_BackBufferRect_MaterialParameterName, RectInScreen01);
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
		if (RenderType == ELexVisualPostProcessRenderType::RenderTarget && IsValid(OutputRenderTarget))
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

void ULexVisualPostProcess:: UpdateRenderTarget()
{
	if (RenderType != ELexVisualPostProcessRenderType::RenderTarget)return;
	auto DesiredRenderTargetSize = MeshRectInScreen.GetSize();
	static const int32 MaxAllowedDrawSize = GetMax2DTextureDimension();
	if (DesiredRenderTargetSize.X < 1 || DesiredRenderTargetSize.Y < 1)
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
		//update material's texture, because OutputRenderTarget could be null when register
		for (auto& MID : MaterialsUsingThisBackBuffer)
		{
			if (!MID.IsValid())continue;
			MID->SetTextureParameterValue(ULexCanvas::LexUI_BackBufferTexture_MaterialParameterName, OutputRenderTarget);
		}
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


