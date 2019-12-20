// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIRenderable_BP.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"


void ULGUICreateGeometryHelper::AddVertSimple(FVector position, FColor color, FVector2D uv0)
{
	uiGeometry->originVerticesCount += 1;
	auto& originPositions = uiGeometry->originPositions;
	originPositions.Add(position);
	auto& vertices = uiGeometry->vertices;
	FDynamicMeshVertex vert;
	vert.Color = color;
	vert.TextureCoordinate[0] = uv0;
	vertices.Add(vert);
}
void ULGUICreateGeometryHelper::AddVertFull(FVector position, FColor color, FVector2D uv0, FVector2D uv1, FVector2D uv2, FVector2D uv3, FVector normal, FVector tangent)
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
void ULGUICreateGeometryHelper::AddTriangle(int index0, int index1, int index2)
{
	uiGeometry->originTriangleCount += 3;
	auto& triangles = uiGeometry->triangles;
	triangles.Reserve(triangles.Num() + 3);
	triangles.Add(index0);
	triangles.Add(index1);
	triangles.Add(index2);
}

void ULGUIUpdateGeometryHelper::BeginUpdateVertices(TArray<FLGUIGeometryVertex>& vertices)
{

}
void ULGUIUpdateGeometryHelper::EndUpdateVertices(UPARAM(ref) TArray<FLGUIGeometryVertex>& vertices)
{

}




UUIRenderable_BP::UUIRenderable_BP()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIRenderable_BP::BeginPlay()
{
	Super::BeginPlay();
}

void UUIRenderable_BP::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

void UUIRenderable_BP::EndPlay(const EEndPlayReason::Type EndPlayReason)
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

void UUIRenderable_BP::OnBeforeCreateOrUpdateGeometry()
{
	OnBeforeCreateOrUpdateGeometry_BP();
}
void UUIRenderable_BP::OnCreateGeometry()
{
	if (!IsValid(createGeometryHelper))
	{
		createGeometryHelper = NewObject<ULGUICreateGeometryHelper>(this);
		createGeometryHelper->uiGeometry = geometry;
	}
	OnCreateGeometry_BP(createGeometryHelper);

}
void UUIRenderable_BP::OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (!IsValid(updateGeometryHelper))
	{
		updateGeometryHelper = NewObject<ULGUIUpdateGeometryHelper>(this);
		updateGeometryHelper->uiGeometry = geometry;
	}
	OnUpdateGeometry_BP(updateGeometryHelper, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
}
void UUIRenderable_BP::MarkVertexChanged_BP()
{
	
}