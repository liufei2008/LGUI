// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIMesh.h"
#include "LGUI.h"


UUIMesh::UUIMesh()
{
	PrimaryComponentTick.bCanEverTick = false;
	bSelectable = false;
	bCastStaticShadow = false;
	bAffectDynamicIndirectLighting = false;
	bReceivesDecals = false;
	bApplyImpulseOnDamage = false;
}

void UUIMesh::BeginPlay()
{
	Super::BeginPlay();
}

void UUIMesh::GenerateOrUpdateMesh(bool vertexPositionChanged)
{
	int vertexCount = MeshSection.vertices.Num();
	int indexCount = MeshSection.triangles.Num();

	if (prevIndexCount == indexCount && prevVertexCount == vertexCount)//if vertex count and triangle count not change, just update
	{
		UpdateMeshSection(vertexPositionChanged);
	}
	else
	{
		CreateMeshSection();
	}

	prevVertexCount = vertexCount;
	prevIndexCount = indexCount;
}

#if WITH_EDITOR
void UUIMesh::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif