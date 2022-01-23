// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBatchGeometryRenderable_BP.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"


void ULGUICreateGeometryHelper::AddVertexSimple(FVector position, FColor color, FVector2D uv0)
{
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
	uiGeometry->originTangents.Add(vertex.tagent);
}
void ULGUICreateGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
#if WITH_EDITOR
	int vertCount = uiGeometry->originVerticesCount;
	if (index0 >= vertCount || index1 >= vertCount || index2 >= vertCount)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::AddTriangle]Triangle index reference out of range vertex."));
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

void ULGUIUpdateGeometryHelper::BeginUpdateVertices()
{
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
		vert.tagent = originTangents[i];
		vert.uv0 = vertices[i].TextureCoordinate[0];
	}
}
void ULGUIUpdateGeometryHelper::EndUpdateVertices()
{
	auto& vertices = uiGeometry->vertices;
	int count = vertices.Num();
	if (count != cacheVertices.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUIUpdateGeometryHelper::EndUpdateVertices]Don't change vertices size here! if you really need that, call MarkRebuildGeometry(), then OnCreateGeometry() will be called automatically."));
		return;
	}

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
		originTangents[i] = vert.tagent;
		vertices[i].TextureCoordinate[0] = vert.uv0;
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

bool UUIBatchGeometryRenderable_BP::NeedTextureToCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		return ReceiveNeedTextureToCreateGeometry();
	}
	return false;
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
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		ReceiveOnBeforeCreateOrUpdateGeometry();
	}
}
void UUIBatchGeometryRenderable_BP::OnCreateGeometry()
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(createGeometryHelper))
		{
			createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
			createGeometryHelper->uiGeometry = geometry;
		}
		ReceiveOnCreateGeometry(createGeometryHelper);
	}
}
void UUIBatchGeometryRenderable_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (GetClass()->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !GetClass()->HasAnyClassFlags(CLASS_Native))
	{
		if (!IsValid(updateGeometryHelper))
		{
			updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
			updateGeometryHelper->uiGeometry = geometry;
		}
		updateGeometryHelper->BeginUpdateVertices();
		ReceiveOnUpdateGeometry(updateGeometryHelper, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
		updateGeometryHelper->EndUpdateVertices();
	}
}
void UUIBatchGeometryRenderable_BP::MarkVertexChanged()
{
	MarkVertexPositionDirty();
	MarkColorDirty();
	MarkUVDirty();
}
void UUIBatchGeometryRenderable_BP::MarkRebuildGeometry()
{
	MarkTriangleDirty();
}