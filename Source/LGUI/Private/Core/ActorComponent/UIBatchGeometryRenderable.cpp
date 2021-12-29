// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/Actor/LGUIManagerActor.h"

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable ApplyModifier"), STAT_ApplyModifier, STATGROUP_LGUI);

UUIBatchGeometryRenderable::UUIBatchGeometryRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::UIBatchGeometryRenderable;
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

void UUIBatchGeometryRenderable::ApplyUIActiveState()
{
	Super::ApplyUIActiveState();
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
			if (NeedTextureToCreateGeometry())
			{
				if (!IsValid(GetTextureToCreateGeometry()))//need texture, but texture is not valid
				{
					UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
				}
				else
				{
					geometry->texture = GetTextureToCreateGeometry();
					drawcall->textureChanged = true;
				}
			}
		}
	}
	MarkCanvasUpdate(true, false, false);
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
	}
	MarkCanvasUpdate(true, false, false);
}

void UUIBatchGeometryRenderable::AddGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	int count = GeometryModifierComponentArray.Num();
	if (count > 0)
	{
		int32 index;
		if (!GeometryModifierComponentArray.Find(InModifier, index))
		{
			GeometryModifierComponentArray.Add(InModifier);
			GeometryModifierComponentArray.Sort([](const UUIGeometryModifierBase& A, const UUIGeometryModifierBase& B) {
				return A.GetExecuteOrder() < B.GetExecuteOrder();
			});
		}
	}
	else
	{
		GeometryModifierComponentArray.Add(InModifier);
	}
}
void UUIBatchGeometryRenderable::RemoveGeometryModifier(class UUIGeometryModifierBase* InModifier)
{
	GeometryModifierComponentArray.Remove(InModifier);
}

void UUIBatchGeometryRenderable::MarkAllDirtyRecursive()
{
	bLocalVertexPositionChanged = true;
	bUVChanged = true;
	bTriangleChanged = true;
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
bool UUIBatchGeometryRenderable::HaveGeometryModifier()
{
#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::EditorPreview)
	{
		if (auto modifierComp = GetOwner()->FindComponentByClass<UUIGeometryModifierBase>())
		{
			return true;
		}
	}
	else
#endif
	{
		return GeometryModifierComponentArray.Num() > 0;
	}
	return false;
}
bool UUIBatchGeometryRenderable::ApplyGeometryModifier(bool uvChanged, bool colorChanged, bool vertexPositionChanged, bool layoutChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_ApplyModifier);
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
		int32 originVerticesCount = geometry->originVerticesCount;
		int32 originTriangleIndicesCount = geometry->originTriangleCount;
		bool modifierAffectTriangleCount = false;
		for (int i = 0; i < count; i++)
		{
			auto modifierComp = GeometryModifierComponentArray[i];
			bool thisModifierAffectTriangleCount = false;
			modifierComp->ModifyUIGeometry(geometry, originVerticesCount, originTriangleIndicesCount, thisModifierAffectTriangleCount,
				uvChanged, colorChanged, vertexPositionChanged, layoutChanged);
			if (thisModifierAffectTriangleCount)modifierAffectTriangleCount = true;
		}
		return modifierAffectTriangleCount;
	}
	return false;
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
		CreateGeometry();
		goto COMPLETE;
	}
	else//if geometry is created, update data
	{
		if (bTriangleChanged)//triangle change, need to recreate geometry
		{
			if (CreateGeometry())
			{
				drawcall->needToRebuildMesh = true;
			}
			goto COMPLETE;
		}
		else//update geometry
		{
			OnUpdateGeometry(bLocalVertexPositionChanged, bUVChanged, bColorChanged);
			
			if (ApplyGeometryModifier(bUVChanged, bColorChanged, bLocalVertexPositionChanged, bTransformChanged))//vertex data change, need to update geometry's vertex
			{
				drawcall->needToRebuildMesh = true;
				drawcall->needToUpdateVertex = true;
			}
			else
			{
				drawcall->needToUpdateVertex = true;
			}
			if (bLocalVertexPositionChanged)
			{
				CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
			}
			if (bLocalVertexPositionChanged || bTransformChanged)
			{
				UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
			}
		}
	}
COMPLETE:
	bTriangleChanged = false;
	bLocalVertexPositionChanged = false;
	bUVChanged = false;
	bColorChanged = false;
	bTransformChanged = false;
	;
}

bool UUIBatchGeometryRenderable::CreateGeometry()
{
	geometry->Clear();
	if (HaveDataToCreateGeometry())
	{
		if (NeedTextureToCreateGeometry() && !IsValid(GetTextureToCreateGeometry()))
		{
			UE_LOG(LGUI, Error, TEXT("[UUIGeometryRenderable::CreateGeometry]Need texture to create geometry, but texture is no valid!"));
			return false;
		}
		geometry->texture = GetTextureToCreateGeometry();
		geometry->material = CustomUIMaterial;
		OnCreateGeometry();
		ApplyGeometryModifier(true, true, true, true);
		CalculateLocalBounds();//CalculateLocalBounds must stay before TransformVertices, because TransformVertices will also cache bounds for Canvas to check 2d overlap.
		UIGeometry::TransformVertices(RenderCanvas.Get(), this, geometry);
		return true;
	}
	else
	{
		this->LocalMinPoint = this->LocalMaxPoint = FVector2D(0, 0);
		return false;
	}
}

bool UUIBatchGeometryRenderable::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	return UUIBaseRenderable::LineTraceUIGeometry(geometry, OutHit, Start, End);
}

void UUIBatchGeometryRenderable::CalculateLocalBounds()
{
	auto& vertices = geometry->originPositions;
	float horizontalMin = MAX_flt, horizontalMax = -MAX_flt;
	float verticalMin = MAX_flt, verticalMax = -MAX_flt;
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
	}
	this->LocalMinPoint = FVector2D(horizontalMin, verticalMin);
	this->LocalMaxPoint = FVector2D(horizontalMax, verticalMax);
}

void UUIBatchGeometryRenderable::GetGeometryBoundsInLocalSpace(FVector2D& OutMinPoint, FVector2D& OutMaxPoint)const
{
	OutMinPoint = this->LocalMinPoint;
	OutMaxPoint = this->LocalMaxPoint;
}
