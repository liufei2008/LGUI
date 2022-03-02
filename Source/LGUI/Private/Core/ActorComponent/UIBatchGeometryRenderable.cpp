// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/Actor/LGUIManagerActor.h"

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable GeometryModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIBatchGeometryRenderable::UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	UIRenderableType = EUIRenderableType::UIBatchGeometryRenderable;
	geometry = TSharedPtr<UIGeometry>(new UIGeometry);

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::BeginPlay()
{
	Super::BeginPlay();

	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
}

void UUIBatchGeometryRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

#if WITH_EDITOR
void UUIBatchGeometryRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (auto Property = PropertyChangedEvent.Property)
	{

	}
}
#endif
void UUIBatchGeometryRenderable::OnRegister()
{
	Super::OnRegister();
}
void UUIBatchGeometryRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIBatchGeometryRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	Super::OnRenderCanvasChanged(OldCanvas, NewCanvas);
}

void UUIBatchGeometryRenderable::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
    Super::OnAnchorChange(InPivotChange, InSizeChange, InDiscardCache);
	if (InPivotChange || InSizeChange)
    {
        MarkVertexPositionDirty();
    }
}

void UUIBatchGeometryRenderable::MarkVertexChanged()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUIBatchGeometryRenderable::MarkRebuildGeometry()
{
	MarkTriangleDirty();
}

void UUIBatchGeometryRenderable::MarkVertexPositionDirty()
{
	bLocalVertexPositionChanged = true;
	MarkCanvasUpdate(false, true, false);
}
void UUIBatchGeometryRenderable::MarkUVDirty()
{
	bUVChanged = true;
	MarkCanvasUpdate(false, false, false);
}
void UUIBatchGeometryRenderable::MarkTriangleDirty()
{
	bTriangleChanged = true;
	MarkCanvasUpdate(false, false, false, true);
}
void UUIBatchGeometryRenderable::MarkTextureDirty()
{
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->texture = GetTextureToCreateGeometry();
			drawcall->textureChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
}
void UUIBatchGeometryRenderable::MarkMaterialDirty()
{
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->material = CustomUIMaterial;
			drawcall->materialChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
}

void UUIBatchGeometryRenderable::AddGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	auto Index = GeometryModifierComponentArray.AddUnique(InModifier);
	if (Index > 0)
	{
		SortGeometryModifier();
	}
	MarkTriangleDirty();
}
void UUIBatchGeometryRenderable::RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	GeometryModifierComponentArray.Remove(InModifier);
	MarkTriangleDirty();
}
void UUIBatchGeometryRenderable::SortGeometryModifier()
{
	GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
		return A.GetExecuteOrder() < B.GetExecuteOrder();
		});
	MarkTriangleDirty();
}

void UUIBatchGeometryRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
	if (RenderCanvas.IsValid())
	{
		if (drawcall.IsValid())
		{
			geometry->texture = GetTextureToCreateGeometry();
			drawcall->textureChanged = true;

			geometry->material = CustomUIMaterial;
			drawcall->materialChanged = true;
		}
		MarkCanvasUpdate(true, false, false);
	}
	Super::MarkAllDirtyRecursive();
}
void UUIBatchGeometryRenderable::SetCustomUIMaterial(UMaterialInterface* inMat)
{
	if (CustomUIMaterial != inMat)
	{
		CustomUIMaterial = inMat;
		MarkMaterialDirty();
	}
}

UMaterialInstanceDynamic* UUIBatchGeometryRenderable::GetMaterialInstanceDynamic()const
{
	if (IsValid(CustomUIMaterial))
	{
		if (CustomUIMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
		{
			return (UMaterialInstanceDynamic*)CustomUIMaterial;//if CustomUIMaterial is a MaterialInstanceDynamic then just return it directly
		}
	}
	if (drawcall.IsValid())
	{
		return drawcall->materialInstanceDynamic.Get();
	}
	return nullptr;
}
bool UUIBatchGeometryRenderable::HaveGeometryModifier(bool includeDisabled)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(GeometryModifierComponentArray, false);
		GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
			return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
	}
#endif
	if (includeDisabled)
	{
		return GeometryModifierComponentArray.Num() > 0;
	}
	else
	{
		int32 enabledCount = 0;
		if (GeometryModifierComponentArray.Num() > 0)
		{
			for (auto& Item : GeometryModifierComponentArray)
			{
				if (Item->GetEnable())
				{
					enabledCount++;
				}
			}
		}
		return enabledCount > 0;
	}
	return false;
}

void UUIBatchGeometryRenderable::GeometryModifierWillChangeVertexData(bool& OutTriangleIndices, bool& OutVertexPosition, bool& OutUV, bool& OutColor)
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(GeometryModifierComponentArray, false);
		GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
			return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
	}
#endif

	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			if (modifierComp->GetEnable())
			{
				bool TempTriangleIndices = false, TempVertexPosition = false, TempUV = false, TempColor = false;
				modifierComp->ModifierWillChangeVertexData(TempTriangleIndices, TempVertexPosition, TempUV, TempColor);
				if (TempTriangleIndices)OutTriangleIndices = true;
				if (TempVertexPosition)OutVertexPosition = true;
				if (TempUV)OutUV = true;
				if (TempColor)OutColor = true;
			}
		}
	}
}

void UUIBatchGeometryRenderable::ApplyGeometryModifier(bool triangleChanged, bool uvChanged, bool colorChanged, bool vertexPositionChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyModifier);

	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			if (modifierComp->GetEnable())
			{
				modifierComp->ModifyUIGeometry(*(geometry.Get()), triangleChanged, uvChanged, colorChanged, vertexPositionChanged);
			}
		}
	}
}

void UUIBatchGeometryRenderable::MarkFlattenHierarchyIndexDirty()
{
	Super::MarkFlattenHierarchyIndexDirty();
	if (drawcall.IsValid())
	{
		drawcall->shouldSortRenderObjectList = true;//hierarchy order change, should sort objects in drawcall
	}
}

void UUIBatchGeometryRenderable::UpdateGeometry()
{
	Super::UpdateGeometry();

	OnBeforeCreateOrUpdateGeometry();
	if (!drawcall.IsValid()//not add to render yet
		)
	{
		geometry->Clear();
		geometry->texture = GetTextureToCreateGeometry();
		geometry->material = CustomUIMaterial;
		OnUpdateGeometry(*(geometry.Get()), true, true, true, true);
		ApplyGeometryModifier(true, true, true, true);
		CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
	}
	else//if geometry is created, update data
	{
		if (bTriangleChanged || bLocalVertexPositionChanged || bColorChanged || bUVChanged)
		{
			auto PrevVerticesCount = geometry->vertices.Num();
			auto PrevTriangleCount = geometry->triangles.Num();
			geometry->Clear();
			//check if GeometryModifier will affect vertex data, if so we need to update these data in OnUpdateGeometry
			{
				bool TempTriangleIndices = false, TempVertexPosition = false, TempUV = false, TempColor = false;
				GeometryModifierWillChangeVertexData(TempTriangleIndices, TempVertexPosition, TempUV, TempColor);
				if (TempTriangleIndices)bTriangleChanged = true;
				if (TempVertexPosition)bLocalVertexPositionChanged = true;
				if (TempUV)bUVChanged = true;
				if (TempColor)bColorChanged = true;
			}
			OnUpdateGeometry(*(geometry.Get()), bTriangleChanged, bLocalVertexPositionChanged, bUVChanged, bColorChanged);
			ApplyGeometryModifier(bTriangleChanged, bUVChanged, bColorChanged, bLocalVertexPositionChanged);
			drawcall->needToUpdateVertex = true;
			if (bLocalVertexPositionChanged)
			{
				CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
			}
		}
		if (bLocalVertexPositionChanged || bTransformChanged)
		{
			UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry.Get());
			drawcall->needToUpdateVertex = true;
		}
	}

	bTriangleChanged = false;
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
}

bool UUIBatchGeometryRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (RaycastType == EUIRenderableRaycastType::Rect)
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
	else if (RaycastType == EUIRenderableRaycastType::Geometry)
	{
		return LineTraceUIGeometry(geometry, OutHit, Start, End);
	}
	else
	{
		return LineTraceUICustom(OutHit, Start, End);
	}
}

void UUIBatchGeometryRenderable::CalculateLocalBounds()
{
	auto& vertices = geometry->originPositions;
	float horizontalMin = MAX_flt, horizontalMax = -MAX_flt;
	float verticalMin = MAX_flt, verticalMax = -MAX_flt;
#if WITH_EDITOR
	float forwardMin = MAX_flt, forwardMax = -MAX_flt;
#endif
	if (vertices.Num() == 0)
	{
		horizontalMin = horizontalMax = verticalMin = verticalMax = 0;
#if WITH_EDITOR
		forwardMin = forwardMax = 0;
#endif
	}
	else
	{
		for (auto& Vert : vertices)
		{
			if (Vert.Y < horizontalMin)
			{
				horizontalMin = Vert.Y;
			}
			if (Vert.Y > horizontalMax)
			{
				horizontalMax = Vert.Y;
			}
			if (Vert.Z < verticalMin)
			{
				verticalMin = Vert.Z;
			}
			if (Vert.Z > verticalMax)
			{
				verticalMax = Vert.Z;
			}
#if WITH_EDITOR
			if (Vert.X < forwardMin)
			{
				forwardMin = Vert.X;
			}
			if (Vert.X > forwardMax)
			{
				forwardMax = Vert.X;
			}
#endif
		}
	}
	this->LocalMinPoint = FVector2D(horizontalMin, verticalMin);
	this->LocalMaxPoint = FVector2D(horizontalMax, verticalMax);
#if WITH_EDITOR
	this->LocalMinPoint3D = FVector(forwardMin, horizontalMin, verticalMin);
	this->LocalMaxPoint3D = FVector(forwardMax, horizontalMax, verticalMax);
#endif
}

void UUIBatchGeometryRenderable::GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const
{
	OutMinPoint = this->LocalMinPoint;
	OutMaxPoint = this->LocalMaxPoint;
}

#if WITH_EDITOR
void UUIBatchGeometryRenderable::GetGeometryBounds3DInLocalSpace(FVector& OutMinPoint, FVector& OutMaxPoint)const
{
	OutMinPoint = this->LocalMinPoint3D;
	OutMaxPoint = this->LocalMaxPoint3D;
}
#endif
