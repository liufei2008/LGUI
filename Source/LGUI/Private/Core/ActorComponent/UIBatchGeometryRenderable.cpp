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

void UUIBatchGeometryRenderable::MarkVertexDirty()
{
	MarkTriangleDirty();
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
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

UTexture* UUIBatchGeometryRenderable::GetTextureToCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveGetTextureToCreateGeometry();
	}
	return nullptr;
}

void UUIBatchGeometryRenderable::OnBeforeCreateOrUpdateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}

DECLARE_CYCLE_STAT(TEXT("UIBatchGeometryRenderable_Blueprint.OnUpdateGeometry"), STAT_BatchGeometryRenderable_OnUpdateGeometry, STATGROUP_LGUI);
void UUIBatchGeometryRenderable::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(GeometryHelper))
		{
			GeometryHelper = NewObject<ULGUIGeometryHelper>(this);
		}
		GeometryHelper->UIGeo = &InGeo;
		SCOPE_CYCLE_COUNTER(STAT_BatchGeometryRenderable_OnUpdateGeometry);
		ReceiveOnUpdateGeometry(GeometryHelper, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}



void ULGUIGeometryHelper::AddVertexSimple(FVector position, FColor color, FVector2D uv0)
{
#if !UE_BUILD_SHIPPING
	if (position.ContainsNaN()
		|| uv0.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexFull]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originPositions = UIGeo->originPositions;
	originPositions.Add(position);
	auto& vertices = UIGeo->vertices;
	FDynamicMeshVertex vert;
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vertices.Add(vert);
	UIGeo->originNormals.Add(FVector(0, 0, -1));
	UIGeo->originTangents.Add(FVector(1, 0, 0));
}
void ULGUIGeometryHelper::AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent)
{
#if !UE_BUILD_SHIPPING
	if (position.ContainsNaN()
		|| normal.ContainsNaN()
		|| tangent.ContainsNaN()
		|| uv0.ContainsNaN()
		|| uv1.ContainsNaN()
		|| uv2.ContainsNaN()
		|| uv3.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexFull]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originPositions = UIGeo->originPositions;
	originPositions.Add(position);
	auto& vertices = UIGeo->vertices;
	FDynamicMeshVertex vert;
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vert.TextureCoordinate[1] = uv1;
	vert.TextureCoordinate[2] = uv2;
	vert.TextureCoordinate[3] = uv3;
	UIGeo->originNormals.Add(normal);
	UIGeo->originTangents.Add(tangent);
	vertices.Add(vert);
}
void ULGUIGeometryHelper::AddVertexStruct(FLGUIGeometryVertex vertex)
{
#if !UE_BUILD_SHIPPING
	if (vertex.position.ContainsNaN()
		|| vertex.normal.ContainsNaN()
		|| vertex.tangent.ContainsNaN()
		|| vertex.uv0.ContainsNaN()
		|| vertex.uv1.ContainsNaN()
		|| vertex.uv2.ContainsNaN()
		|| vertex.uv3.ContainsNaN()
		)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexStruct]Vertex data contains NaN!."));
		return;
	}
#endif
	auto& originPositions = UIGeo->originPositions;
	originPositions.Add(vertex.position);
	auto& vertices = UIGeo->vertices;
	FDynamicMeshVertex vert;
	vert.Color = vertex.color;
	vert.TextureCoordinate[0] = vertex.uv0;
	vert.TextureCoordinate[1] = vertex.uv1;
	vert.TextureCoordinate[2] = vertex.uv2;
	vert.TextureCoordinate[3] = vertex.uv3;
	vertices.Add(vert);
	UIGeo->originNormals.Add(vertex.normal);
	UIGeo->originTangents.Add(vertex.tangent);
}
void ULGUIGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
#if !UE_BUILD_SHIPPING
	int vertCount = UIGeo->vertices.Num();
	if (index0 >= vertCount || index1 >= vertCount || index2 >= vertCount)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddTriangle]Triangle index reference out of vertex range."));
		return;
	}
#endif
	auto& triangles = UIGeo->triangles;
	triangles.Reserve(triangles.Num() + 3);
	triangles.Add(index0);
	triangles.Add(index1);
	triangles.Add(index2);
}
void ULGUIGeometryHelper::SetGeometry(const TArray<FLGUIGeometryVertex>& InVertices, const TArray<int>& InIndices)
{
	int vertCount = InVertices.Num();
#if !UE_BUILD_SHIPPING
	for (auto& i : InIndices)
	{
		if (i >= vertCount)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Triangle index reference out of vertex range."));
			return;
		}
	}
	if ((InIndices.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Indices count must be multiple of 3."));
		return;
	}
	for (auto& vertex : InVertices)
	{
		if (vertex.position.ContainsNaN()
			|| vertex.normal.ContainsNaN()
			|| vertex.tangent.ContainsNaN()
			|| vertex.uv0.ContainsNaN()
			|| vertex.uv1.ContainsNaN()
			|| vertex.uv2.ContainsNaN()
			|| vertex.uv3.ContainsNaN()
			)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Vertex data contains NaN!."));
			break;
		}
	}
#endif
	auto& triangles = UIGeo->triangles;
	triangles.SetNumUninitialized(InIndices.Num());
	for (int i = 0; i < InIndices.Num(); i++)
	{
		triangles[i] = InIndices[i];
	}

	auto& vertices = UIGeo->vertices;
	auto& originPositions = UIGeo->originPositions;
	auto& originNormals = UIGeo->originNormals;
	auto& originTangents = UIGeo->originTangents;
	vertices.SetNumUninitialized(vertCount);
	originPositions.SetNumUninitialized(vertCount);
	originNormals.SetNumUninitialized(vertCount);
	originTangents.SetNumUninitialized(vertCount);

	for (int i = 0; i < vertCount; i++)
	{
		auto& originVert = InVertices[i];
		originPositions[i] = originVert.position;
		originNormals[i] = originVert.normal;
		originTangents[i] = originVert.tangent;
		auto& vert = vertices[i];
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = originVert.uv0;
		vert.TextureCoordinate[1] = originVert.uv1;
		vert.TextureCoordinate[2] = originVert.uv2;
		vert.TextureCoordinate[3] = originVert.uv3;
	}
}

void ULGUIGeometryHelper::AddVertexTriangleStream(const TArray<FLGUIGeometryVertex>& InVertexTriangleStream)
{
#if !UE_BUILD_SHIPPING
	if ((InVertexTriangleStream.Num() % 3) != 0)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexTriangleStream]Indices count must be multiple of 3."));
		return;
	}
	for (auto& vertex : InVertexTriangleStream)
	{
		if (vertex.position.ContainsNaN()
			|| vertex.normal.ContainsNaN()
			|| vertex.tangent.ContainsNaN()
			|| vertex.uv0.ContainsNaN()
			|| vertex.uv1.ContainsNaN()
			|| vertex.uv2.ContainsNaN()
			|| vertex.uv3.ContainsNaN()
			)
		{
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddVertexTriangleStream]Vertex data contains NaN!."));
			return;
		}
	}
#endif
	auto& triangles = UIGeo->triangles;
	auto& vertices = UIGeo->vertices;
	auto& originPositions = UIGeo->originPositions;
	auto& originNormals = UIGeo->originNormals;
	auto& originTangents = UIGeo->originTangents;
	auto vertCount = vertices.Num();
	triangles.Reserve(InVertexTriangleStream.Num());
	for (int i = 0; i < InVertexTriangleStream.Num(); i++)
	{
		triangles.Add(vertCount + i);
	}

	vertices.Reserve(InVertexTriangleStream.Num());
	originPositions.Reserve(InVertexTriangleStream.Num());
	originNormals.Reserve(InVertexTriangleStream.Num());
	originTangents.Reserve(InVertexTriangleStream.Num());

	for (int i = 0; i < InVertexTriangleStream.Num(); i++)
	{
		auto& originVert = InVertexTriangleStream[i];
		originPositions.Add(originVert.position);
		originNormals.Add(originVert.normal);
		originTangents.Add(originVert.tangent);
		FDynamicMeshVertex vert(FVector::ZeroVector);
		vert.Color = originVert.color;
		vert.TextureCoordinate[0] = originVert.uv0;
		vert.TextureCoordinate[1] = originVert.uv1;
		vert.TextureCoordinate[2] = originVert.uv2;
		vert.TextureCoordinate[3] = originVert.uv3;
		vertices.Add(vert);
	}
}

void ULGUIGeometryHelper::ClearVerticesAndIndices()
{
	UIGeo->Clear();
}

void ULGUIGeometryHelper::GetVertexTriangleStream(TArray<FLGUIGeometryVertex>& OutVertexTriangleStream)
{
	auto& triangles = UIGeo->triangles;
	auto& vertices = UIGeo->vertices;
	auto& originPositions = UIGeo->originPositions;
	auto& originNormals = UIGeo->originNormals;
	auto& originTangents = UIGeo->originTangents;
	auto vertCount = vertices.Num();
	bool hasNormal = originNormals.Num() >= vertCount;
	bool hasTangent = originTangents.Num() >= vertCount;
	OutVertexTriangleStream.Reserve(triangles.Num());
	for (int i = 0; i < triangles.Num(); i++)
	{
		FLGUIGeometryVertex vertex;
		auto vertIndex = triangles[i];
		auto& originVert = vertices[vertIndex];
		vertex.uv0 = originVert.TextureCoordinate[0];
		vertex.uv1 = originVert.TextureCoordinate[1];
		vertex.uv2 = originVert.TextureCoordinate[2];
		vertex.uv3 = originVert.TextureCoordinate[3];
		vertex.color = originVert.Color;
		vertex.position = originPositions[vertIndex];
		if (hasNormal) vertex.normal = originNormals[vertIndex];
		if (hasTangent) vertex.tangent = originTangents[vertIndex];
		OutVertexTriangleStream.Add(vertex);
	}
}
