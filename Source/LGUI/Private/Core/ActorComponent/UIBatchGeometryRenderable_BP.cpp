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
void ULGUICreateGeometryHelper::AddTriangle(int index0, int index1, int index2)
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
	auto& triangles = UIGeo->triangles;
	if (triangles.Num() < InIndices.Num())
	{
		triangles.SetNumUninitialized(InIndices.Num());
	}
	for (int i = 0; i < InIndices.Num(); i++)
	{
		triangles[i] = InIndices[i];
	}

	auto& vertices = UIGeo->vertices;
	auto& originPositions = UIGeo->originPositions;
	auto& originNormals = UIGeo->originNormals;
	auto& originTangents = UIGeo->originTangents;
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



UUIBatchGeometryRenderable_BP::UUIBatchGeometryRenderable_BP(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIBatchGeometryRenderable_BP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
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
	}
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}

DECLARE_CYCLE_STAT(TEXT("UUIBatchGeometryRenderable_BP.OnUpdateGeometry"), STAT_BatchGeometryRenderable_OnUpdateGeometry, STATGROUP_LGUI);
void UUIBatchGeometryRenderable_BP::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		createGeometryHelper->UIGeo = &InGeo;
		SCOPE_CYCLE_COUNTER(STAT_BatchGeometryRenderable_OnUpdateGeometry);
		ReceiveOnUpdateGeometry(createGeometryHelper, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
}
