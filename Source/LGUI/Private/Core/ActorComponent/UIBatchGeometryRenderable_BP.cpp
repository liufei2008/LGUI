// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable_BP.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/LGUISpriteData.h"


void ULGUICreateGeometryHelper::AddVertexSimple(FVector position, FColor color, FVector2D uv0)
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
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	uiGeometry->originVerticesCount += 1;
	auto& originPositions = uiGeometry->originPositions;
	originPositions.Add(position);
	auto& vertices = uiGeometry->vertices;
	FDynamicMeshVertex vert;
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vertices.Add(vert);
	uiGeometry->originNormals.Add(FVector(0, 0, -1));
	uiGeometry->originTangents.Add(FVector(1, 0, 0));
}
void ULGUICreateGeometryHelper::AddVertexFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent)
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
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	uiGeometry->originVerticesCount += 1;
	auto& originPositions = uiGeometry->originPositions;
	originPositions.Add(position);
	auto& vertices = uiGeometry->vertices;
	FDynamicMeshVertex vert;
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vert.TextureCoordinate[1] = uv1;
	vert.TextureCoordinate[2] = uv2;
	vert.TextureCoordinate[3] = uv3;
	uiGeometry->originNormals.Add(normal);
	uiGeometry->originTangents.Add(tangent);
	vertices.Add(vert);
}
void ULGUICreateGeometryHelper::AddVertexStruct(FLGUIGeometryVertex vertex)
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
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	uiGeometry->originVerticesCount += 1;
	auto& originPositions = uiGeometry->originPositions;
	originPositions.Add(vertex.position);
	auto& vertices = uiGeometry->vertices;
	FDynamicMeshVertex vert;
	vert.Color = vertex.color;
	vert.TextureCoordinate[0] = vertex.uv0;
	vert.TextureCoordinate[1] = vertex.uv1;
	vert.TextureCoordinate[2] = vertex.uv2;
	vert.TextureCoordinate[3] = vertex.uv3;
	vertices.Add(vert);
	uiGeometry->originNormals.Add(vertex.normal);
	uiGeometry->originTangents.Add(vertex.tangent);
}
void ULGUICreateGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
#if !UE_BUILD_SHIPPING
	int vertCount = uiGeometry->originVerticesCount;
	if (index0 >= vertCount || index1 >= vertCount || index2 >= vertCount)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddTriangle]Triangle index reference out of vertex range."));
		return;
	}
#endif
	uiGeometry->originTriangleCount += 3;
	auto& triangles = uiGeometry->triangles;
	triangles.Reserve(triangles.Num() + 3);
	triangles.Add(index0);
	triangles.Add(index1);
	triangles.Add(index2);
}
void ULGUICreateGeometryHelper::SetGeometry(const TArray<FLGUIGeometryVertex>& InVertices, const TArray<int>& InIndices)
{
	int vertCount = InVertices.Num();
#if !UE_BUILD_SHIPPING
	for(auto& i : InIndices)
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
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::SetGeometry]Vertex position contains NaN!."));
			return;
		}
	}
#endif
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	auto& triangles = uiGeometry->triangles;
	if (triangles.Num() < InIndices.Num())
	{
		triangles.SetNumUninitialized(InIndices.Num());
	}
	for (int i = 0; i < InIndices.Num(); i++)
	{
		triangles[i] = InIndices[i];
	}
	uiGeometry->originTriangleCount = InIndices.Num();

	auto& vertices = uiGeometry->vertices;
	auto& originPositions = uiGeometry->originPositions;
	auto& originNormals = uiGeometry->originNormals;
	auto& originTangents = uiGeometry->originTangents;
	if (vertices.Num() < vertCount)
	{
		vertices.SetNumUninitialized(vertCount);
	}
	if (originPositions.Num() < vertCount)
	{
		originPositions.SetNumUninitialized(vertCount);
	}
	if (originNormals.Num() < vertCount)
	{
		originNormals.SetNumUninitialized(vertCount);
	}
	if (originTangents.Num() < vertCount)
	{
		originTangents.SetNumUninitialized(vertCount);
	}
	uiGeometry->originVerticesCount = vertCount;

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

void ULGUIUpdateGeometryHelper::BeginUpdateVertices()
{
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	const auto& vertices = uiGeometry->vertices;
	const auto& originPositions = uiGeometry->originPositions;
	const auto& originNormals = uiGeometry->originNormals;
	const auto& originTangents = uiGeometry->originTangents;

	int count = vertices.Num();
	cacheVertices.SetNumUninitialized(count);
	for (int i = 0; i < count; i++)
	{
		auto& vert = cacheVertices[i];
		const auto& originVert = vertices[i];
		vert.position = originPositions[i];
		vert.color = originVert.Color;
		vert.uv0 = originVert.TextureCoordinate[0];
		vert.uv1 = originVert.TextureCoordinate[1];
		vert.uv2 = originVert.TextureCoordinate[2];
		vert.uv3 = originVert.TextureCoordinate[3];
		vert.normal = originNormals[i];
		vert.tangent = originTangents[i];
	}
}
void ULGUIUpdateGeometryHelper::EndUpdateVertices()
{
	auto uiGeometry = UIBatchGeometryRenderable->GetGeometry();
	auto& vertices = uiGeometry->vertices;
	int count = vertices.Num();
	if (count != cacheVertices.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndUpdateVertices]Don't change vertices size here! if you really need that, call MarkRebuildGeometry(), then OnCreateGeometry() will be called automatically."));
		return;
	}
#if !UE_BUILD_SHIPPING
	for (auto& vertex : cacheVertices)
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
			UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndUpdateVertices]Vertex position contains NaN!."));
			return;
		}
	}
#endif

	auto& originPositions = uiGeometry->originPositions;
	auto& originNormals = uiGeometry->originNormals;
	auto& originTangents = uiGeometry->originTangents;

	for (int i = 0; i < count; i++)
	{
		const auto& vert = cacheVertices[i];
		auto& originVert = vertices[i];
		originPositions[i] = vert.position;
		originVert.Color = vert.color;
		originVert.TextureCoordinate[0] = vert.uv0;
		originVert.TextureCoordinate[1] = vert.uv1;
		originVert.TextureCoordinate[2] = vert.uv2;
		originVert.TextureCoordinate[3] = vert.uv3;
		originNormals[i] = vert.normal;
		originTangents[i] = vert.tangent;
	}
}




UUIBatchGeometryRenderable_BP::UUIBatchGeometryRenderable_BP(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBatchGeometryRenderable_BP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(createGeometryHelper))
	{
		createGeometryHelper->ConditionalBeginDestroy();
	}
	if (IsValid(updateGeometryHelper))
	{
		updateGeometryHelper->ConditionalBeginDestroy();
	}

	Super::EndPlay(EndPlayReason);
}

UTexture* UUIBatchGeometryRenderable_BP::GetTextureToCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveGetTextureToCreateGeometry();
	}
	return nullptr;
}

void UUIBatchGeometryRenderable_BP::OnBeforeCreateOrUpdateGeometry()
{
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		createGeometryHelper->UIBatchGeometryRenderable = this;
	}
	if (!IsValid(updateGeometryHelper))
	{
		updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
		updateGeometryHelper->UIBatchGeometryRenderable = this;
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}
void UUIBatchGeometryRenderable_BP::OnCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnCreateGeometry(createGeometryHelper);
	}
}
DECLARE_CYCLE_STAT(TEXT("UUIBatchGeometryRenderable_BP.OnUpdateGeometry"), STAT_BatchGeometryRenderable_OnUpdateGeometry, STATGROUP_LGUI);
void UUIBatchGeometryRenderable_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		updateGeometryHelper->BeginUpdateVertices();
		{
			SCOPE_CYCLE_COUNTER(STAT_BatchGeometryRenderable_OnUpdateGeometry);
			ReceiveOnUpdateGeometry(updateGeometryHelper, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		}
		updateGeometryHelper->EndUpdateVertices();
	}
}
