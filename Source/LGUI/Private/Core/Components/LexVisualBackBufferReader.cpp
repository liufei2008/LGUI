// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexVisualBackBufferReader.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUIGeometry.h"
#include "Core/LexUIManager.h"
#include "Core/LexVisualBackBufferRenderProxy.h"
#include "Core/Components/LexWidget.h"
#include "Kismet/GameplayStatics.h"


ULexVisualBackBufferReader::ULexVisualBackBufferReader(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	VisualType = ELexVisualType::BackBufferReader;
	Geometry = TSharedPtr<FLexUIGeometry>(new FLexUIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void ULexVisualBackBufferReader::BeginPlay()
{
	Super::BeginPlay();

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
}

void ULexVisualBackBufferReader::BeginDestroy()
{
	ENQUEUE_RENDER_COMMAND(ULexVisualBackBufferReader_ReleaseRenderProxy)
			([RenderProxyPtr = RenderProxy](FRHICommandListImmediate& RHICmdList)
				{
					delete RenderProxyPtr;
				});
	Super::BeginDestroy();
}

void ULexVisualBackBufferReader::OnRegister()
{
	Super::OnRegister();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->GetOnPostUpdateDrawCall().AddUObject(this, &ULexVisualBackBufferReader::PostUpdateDrawCall);
	}
}

void ULexVisualBackBufferReader::OnUnregister()
{
	Super::OnUnregister();
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->GetOnPostUpdateDrawCall().RemoveAll(this);
	}
}

#if WITH_EDITOR
void ULexVisualBackBufferReader::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bUVChanged = true;
	bLocalVertexPositionChanged = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
bool ULexVisualBackBufferReader::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();
	}
	return Super::CanEditChange(InProperty);
}
#endif


void ULexVisualBackBufferReader::OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange)
{
    Super::OnDimensionChanged(InPivotChange, InWidthChange, InHeightChange);
    if (InPivotChange || InWidthChange || InHeightChange)
    {
	    MarkVertexPositionDirty();
    }
}
void ULexVisualBackBufferReader::OnTransformChanged(bool InPositionChanged, bool InScaleChanged)
{
	Super::OnTransformChanged(InPositionChanged, InScaleChanged);
}

void ULexVisualBackBufferReader::SetBackBufferReaderType(ELexVisualBackBufferReaderMode InBackBufferReaderType)
{
	if (BackBufferReaderType != InBackBufferReaderType)
	{
		BackBufferReaderType = InBackBufferReaderType;
	    MarkVertexPositionDirty();
	}
}

void ULexVisualBackBufferReader::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	GetWidget()->MarkCanvasUpdate(true);
}
void ULexVisualBackBufferReader::MarkUVDirty()
{
	bUVChanged = true;
	GetWidget()->MarkCanvasUpdate(false);
}

void ULexVisualBackBufferReader::MarkAllDirty()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	Super::MarkAllDirty();
}

DECLARE_CYCLE_STAT(TEXT("UIPostProcessRenderable UpdateGeometry"), STAT_UIPostProcessRenderableUpdate, STATGROUP_LGUI);
void ULexVisualBackBufferReader::UpdateGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_UIPostProcessRenderableUpdate);
	auto Widget = GetWidget();
	auto Canvas = Widget->GetRenderCanvas();
	check(Canvas);

	Super::UpdateGeometry();
	
	if (bLocalVertexPositionChanged || bUVChanged)
	{
		Geometry->Clear();
		OnUpdateGeometry(false, bLocalVertexPositionChanged, bUVChanged);
	}
	if (bWidgetPropertyDataStartPositionChanged)
	{
		bWidgetPropertyDataStartPositionChanged = false;
		UpdateGeometryWidgetPropertyData(Geometry->Vertices, Geometry->Vertices.Num(), this->WidgetPropertyDataStartPosition);
	}
	if (bWidgetPropertyDataFontMarkDirty)
	{
		bWidgetPropertyDataFontMarkDirty = false;
		FillWidgetPropertyDataForMaterial_InitialMark(Canvas->GetWidgetPropertyDataAsTexture(), 0);
	}
	if (bClipDataPositionChanged)
	{
		UpdateGeometryClipData(*Geometry.Get(), ClipDataStartPosition);
		/** Only update the clip data position coordinate. */
		FillWidgetPropertyDataForMaterial_ClipDataCoordinate(Canvas->GetWidgetPropertyDataAsTexture());
	}
	if (bLocalVertexPositionChanged || bTransformChanged)
	{
		FLexUIGeometry::TransformVertices(Canvas, this, Geometry.Get());
	}
	if (bLocalVertexPositionChanged || bUVChanged || bTransformChanged || bClipDataPositionChanged)
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

	bClipDataPositionChanged = false;
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bTransformChanged = false;
}

void ULexVisualBackBufferReader::PostUpdateDrawCall()
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
					if (BackBufferReaderType == ELexVisualBackBufferReaderMode::Rect)
					{
						ViewProjectionMatrix = ProjectionData.ComputeViewProjectionMatrix();
					}
				}
			}
		}
	}
	else
	{
		ViewRect = FIntRect(FIntPoint::ZeroValue, RootCanvas->GetViewportSize());
		if (BackBufferReaderType == ELexVisualBackBufferReaderMode::Rect)
		{
			ViewProjectionMatrix = RootCanvas->GetViewProjectionMatrix();
		}
	}
	if (BackBufferReaderType == ELexVisualBackBufferReaderMode::Rect)
	{
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
	}
	else
	{
		if (bWidgetOrGeometryDirty
			|| CacheViewRect != ViewRect
			&& (ViewRect.Max != ViewRect.Min)//make sure view is valid
			)
		{
			bWidgetOrGeometryDirty = false;
			CacheViewRect = ViewRect;
		}
		else
		{
			return;//nothing change, no need to calculate
		}
		
		MeshRectInScreen = FBox2f(FVector2f::Zero(), ViewRect.Size());
		RectInScreen01 = FVector4f(0, 0, 1, 1);
	}
	
	UpdateRegionVertex(ViewRect.Size());
}

void ULexVisualBackBufferReader::OnUpdateGeometry(bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged)
{
	//simple rect geometry for render from screen image to mesh region and inverse
	auto& Vertices = Geometry->Vertices;
	auto& OriginVertices = Geometry->OriginVertices;
	FLexUIGeometry::LexUIGeometrySetArrayNum(Vertices, 4);
	FLexUIGeometry::LexUIGeometrySetArrayNum(OriginVertices, 4);
	if (InVertexUVChanged || InVertexPositionChanged)
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
	}
}

void ULexVisualBackBufferReader::UpdateRegionVertex(FIntPoint InViewportSize)
{
	if (BackBufferReaderType == ELexVisualBackBufferReaderMode::Rect)
	{
		FVector2f Inv_ViewportSize(1.0f / InViewportSize.X, 1.0f / InViewportSize.Y);
		RenderScreenToMeshRegionVertexArray[0].TextureCoordinate = FVector2f(MeshRectInScreen.Min.X, MeshRectInScreen.Max.Y) * Inv_ViewportSize;
		RenderScreenToMeshRegionVertexArray[1].TextureCoordinate = FVector2f(MeshRectInScreen.Max.X, MeshRectInScreen.Max.Y) * Inv_ViewportSize;
		RenderScreenToMeshRegionVertexArray[2].TextureCoordinate = FVector2f(MeshRectInScreen.Min.X, MeshRectInScreen.Min.Y) * Inv_ViewportSize;
		RenderScreenToMeshRegionVertexArray[3].TextureCoordinate = FVector2f(MeshRectInScreen.Max.X, MeshRectInScreen.Min.Y) * Inv_ViewportSize;
		
		//RenderMeshRegionToScreenVertexArray only needed in rect mode
		auto& Vertices = Geometry->Vertices;
		constexpr int VertexBufferSize = 4;
		for (int i = 0; i < VertexBufferSize; i++)
		{
			auto& copyVert = RenderMeshRegionToScreenVertexArray[i];
			copyVert.Position = Vertices[i].Position;
			copyVert.TextureCoordinate0 = Vertices[i].TextureCoordinate[0];
			copyVert.TextureCoordinate1 = Vertices[i].TextureCoordinate[1];
		}
	}
	else
	{
		RenderScreenToMeshRegionVertexArray[0].TextureCoordinate = FVector2f(0, 1);
		RenderScreenToMeshRegionVertexArray[1].TextureCoordinate = FVector2f(1, 1);
		RenderScreenToMeshRegionVertexArray[2].TextureCoordinate = FVector2f(0, 0);
		RenderScreenToMeshRegionVertexArray[3].TextureCoordinate = FVector2f(1, 0);
	}

	SendRegionVertexDataToRenderProxy();
}

void ULexVisualBackBufferReader::UpdateGeometryClipData(FLexUIGeometry& InMesh, int InDataStartPosition)
{
	auto& vertices = InMesh.Vertices;
	for (int i = 0; i < vertices.Num(); i++)
	{
		vertices[i].TextureCoordinate[1].X = InDataStartPosition;
	}
}

void ULexVisualBackBufferReader::SendRegionVertexDataToRenderProxy()
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
			bool bFullViewport;
			FMatrix44f ObjectToWorldMatrix;
		};
		auto UpdateData = new FUIPostProcess_SendRegionVertexDataToRenderProxy();
		UpdateData->RenderMeshRegionToScreenVertexArray = this->RenderMeshRegionToScreenVertexArray;
		UpdateData->RenderScreenToMeshRegionVertexArray = this->RenderScreenToMeshRegionVertexArray;
		UpdateData->MeshRectInScreen = this->MeshRectInScreen;
		UpdateData->RectInScreen01 = this->RectInScreen01;
		UpdateData->bFullViewport = this->BackBufferReaderType == ELexVisualBackBufferReaderMode::Viewport;
		UpdateData->ObjectToWorldMatrix = FMatrix44f(RenderCanvas->GetWidget()->GetWorldTransform().ToMatrixWithScale());
		ENQUEUE_RENDER_COMMAND(FLexPostProcess_UpdateData)
			([TempRenderProxy, UpdateData](FRHICommandListImmediate& RHICmdList)
				{
					TempRenderProxy->RenderScreenToMeshRegionVertexArray = MoveTemp(UpdateData->RenderScreenToMeshRegionVertexArray);
					TempRenderProxy->RenderMeshRegionToScreenVertexArray = MoveTemp(UpdateData->RenderMeshRegionToScreenVertexArray);
					TempRenderProxy->MeshRectInScreen = MoveTemp(UpdateData->MeshRectInScreen);
					TempRenderProxy->RectInScreen01 = MoveTemp(UpdateData->RectInScreen01);
					TempRenderProxy->bFullViewport = UpdateData->bFullViewport;
					TempRenderProxy->ObjectToWorldMatrix = MoveTemp(UpdateData->ObjectToWorldMatrix);
					delete UpdateData;
				});
	}
}

bool ULexVisualBackBufferReader::HaveValidData()const
{
	return Geometry->Vertices.Num() > 0;
}

bool ULexVisualBackBufferReader::LineTraceUI(FLexUIHitResult& OutHit, const FVector& Start, const FVector& End)const
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
